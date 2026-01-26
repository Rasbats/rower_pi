import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
import threading
import socket
import json
import subprocess
import os
import sys
from datetime import datetime
import paho.mqtt.client as mqtt

class MQTTUDPBridge:
    def __init__(self, root):
        self.root = root
        self.root.title("MQTT to UDP Bridge")
        self.root.geometry("800x750")
        
        # State variables
        self.broker_process = None
        self.embedded_broker = None
        self.mqtt_client = None
        self.udp_socket = None
        self.subscriber_running = False
        self.broker_type = "mosquitto"  # or "embedded" or "external"
        
        self.create_widgets()
        self.check_mosquitto()
        
    def create_widgets(self):
        # Broker Section
        broker_frame = ttk.LabelFrame(self.root, text="MQTT Broker", padding=10)
        broker_frame.pack(fill="x", padx=10, pady=5)
        
        # Broker type selection
        ttk.Label(broker_frame, text="Broker Mode:").grid(row=0, column=0, sticky="w")
        self.broker_mode = ttk.Combobox(broker_frame, width=15, state="readonly")
        self.broker_mode.grid(row=0, column=1, sticky="w", padx=5)
        self.broker_mode['values'] = ('Mosquitto', 'Embedded Python', 'External')
        self.broker_mode.current(0)
        self.broker_mode.bind('<<ComboboxSelected>>', self.on_broker_mode_change)
        
        ttk.Label(broker_frame, text="Port:").grid(row=0, column=2, sticky="w", padx=(10,0))
        self.broker_port = ttk.Entry(broker_frame, width=10)
        self.broker_port.insert(0, "1883")
        self.broker_port.grid(row=0, column=3, sticky="w", padx=5)
        
        self.allow_anonymous = tk.BooleanVar(value=True)
        ttk.Checkbutton(broker_frame, text="Allow Anonymous", 
                       variable=self.allow_anonymous).grid(row=0, column=4, padx=10)
        
        self.broker_start_btn = ttk.Button(broker_frame, text="Start Broker", 
                                           command=self.start_broker)
        self.broker_start_btn.grid(row=1, column=0, padx=5, pady=5, sticky="w")
        
        self.broker_stop_btn = ttk.Button(broker_frame, text="Stop Broker", 
                                          command=self.stop_broker, state="disabled")
        self.broker_stop_btn.grid(row=1, column=1, padx=5, pady=5, sticky="w")
        
        self.broker_status = ttk.Label(broker_frame, text="Status: Stopped", 
                                       foreground="red")
        self.broker_status.grid(row=1, column=2, columnspan=2, padx=20, pady=5, sticky="w")
        
        self.broker_info = ttk.Label(broker_frame, text="", foreground="blue", wraplength=700)
        self.broker_info.grid(row=2, column=0, columnspan=4, sticky="w", pady=5)
        
        # MQTT Subscriber Section
        sub_frame = ttk.LabelFrame(self.root, text="MQTT Subscriber", padding=10)
        sub_frame.pack(fill="x", padx=10, pady=5)
        
        ttk.Label(sub_frame, text="Broker Address:").grid(row=0, column=0, sticky="w")
        self.mqtt_host = ttk.Entry(sub_frame, width=20)
        self.mqtt_host.insert(0, "localhost")
        self.mqtt_host.grid(row=0, column=1, sticky="w", padx=5)
        
        ttk.Label(sub_frame, text="Port:").grid(row=0, column=2, sticky="w", padx=(10,0))
        self.mqtt_port = ttk.Entry(sub_frame, width=10)
        self.mqtt_port.insert(0, "1883")
        self.mqtt_port.grid(row=0, column=3, sticky="w", padx=5)
        
        ttk.Label(sub_frame, text="Topic:").grid(row=1, column=0, sticky="w", pady=5)
        self.mqtt_topic = ttk.Entry(sub_frame, width=30)
        self.mqtt_topic.insert(0, "test/topic")
        self.mqtt_topic.grid(row=1, column=1, columnspan=3, sticky="ew", padx=5, pady=5)
        
        # UDP Section
        udp_frame = ttk.LabelFrame(self.root, text="UDP Output", padding=10)
        udp_frame.pack(fill="x", padx=10, pady=5)
        
        ttk.Label(udp_frame, text="Destination IP:").grid(row=0, column=0, sticky="w")
        self.udp_host = ttk.Entry(udp_frame, width=20)
        self.udp_host.insert(0, "127.0.0.1")
        self.udp_host.grid(row=0, column=1, sticky="w", padx=5)
        
        ttk.Label(udp_frame, text="Port:").grid(row=0, column=2, sticky="w", padx=(10,0))
        self.udp_port = ttk.Entry(udp_frame, width=10)
        self.udp_port.insert(0, "5000")
        self.udp_port.grid(row=0, column=3, sticky="w", padx=5)
        
        ttk.Label(udp_frame, text="StrokeRate Field:").grid(row=1, column=0, sticky="w", pady=5)
        self.payload_field = ttk.Entry(udp_frame, width=20)
        self.payload_field.insert(0, "strokeRate")
        self.payload_field.grid(row=1, column=1, sticky="w", padx=5, pady=5)
        
        ttk.Label(udp_frame, text="(JSON field containing strokeRate value)").grid(
            row=1, column=2, columnspan=2, sticky="w", padx=5)
        
        ttk.Label(udp_frame, text="Output Format:").grid(row=2, column=0, sticky="w", pady=5)
        ttk.Label(udp_frame, text="NMEA 0183 VTG (speed = strokeRate * 60/1852)", 
                 foreground="blue").grid(row=2, column=1, columnspan=3, sticky="w", padx=5)
        
        # Control Buttons
        control_frame = ttk.Frame(self.root)
        control_frame.pack(fill="x", padx=10, pady=10)
        
        self.sub_start_btn = ttk.Button(control_frame, text="Start Subscriber", 
                                        command=self.start_subscriber)
        self.sub_start_btn.pack(side="left", padx=5)
        
        self.sub_stop_btn = ttk.Button(control_frame, text="Stop Subscriber", 
                                       command=self.stop_subscriber, state="disabled")
        self.sub_stop_btn.pack(side="left", padx=5)
        
        self.clear_btn = ttk.Button(control_frame, text="Clear Log", 
                                    command=self.clear_log)
        self.clear_btn.pack(side="left", padx=5)
        
        self.sub_status = ttk.Label(control_frame, text="Subscriber: Stopped", 
                                    foreground="red")
        self.sub_status.pack(side="left", padx=20)
        
        # Log Section
        log_frame = ttk.LabelFrame(self.root, text="Log", padding=10)
        log_frame.pack(fill="both", expand=True, padx=10, pady=5)
        
        self.log_text = scrolledtext.ScrolledText(log_frame, height=15, wrap=tk.WORD)
        self.log_text.pack(fill="both", expand=True)
        
    def check_mosquitto(self):
        """Check if mosquitto is available"""
        try:
            if sys.platform == "win32":
                result = subprocess.run(["where", "mosquitto"], 
                                      capture_output=True, text=True, timeout=2)
            else:
                result = subprocess.run(["which", "mosquitto"], 
                                      capture_output=True, text=True, timeout=2)
            
            if result.returncode == 0:
                self.log("✓ Mosquitto found in PATH")
                self.broker_info.config(text="")
            else:
                self.handle_mosquitto_not_found()
        except:
            self.handle_mosquitto_not_found()
            
    def handle_mosquitto_not_found(self):
        """Handle case when mosquitto is not found"""
        self.log("⚠ Mosquitto not found - switching to Embedded Python broker")
        self.broker_mode.current(1)  # Switch to embedded
        self.broker_info.config(
            text="💡 Tip: For better performance, install Mosquitto:\n" +
                 "  • Ubuntu/Debian: sudo apt-get install mosquitto\n" +
                 "  • Mac: brew install mosquitto\n" +
                 "  • Windows: Download from mosquitto.org"
        )
        
    def on_broker_mode_change(self, event=None):
        """Handle broker mode change"""
        mode = self.broker_mode.get()
        if mode == 'External':
            self.broker_start_btn.config(state="disabled")
            self.broker_stop_btn.config(state="disabled")
            self.broker_status.config(text="Using External Broker", foreground="blue")
            self.broker_info.config(text="Connect to your existing MQTT broker below")
        else:
            self.broker_start_btn.config(state="normal")
            self.broker_stop_btn.config(state="disabled")
            self.broker_status.config(text="Status: Stopped", foreground="red")
            if mode == 'Embedded Python':
                self.broker_info.config(text="Using built-in Python MQTT broker (requires: pip install gmqtt)")
            else:
                self.broker_info.config(text="")
        
    def log(self, message):
        timestamp = datetime.now().strftime("%H:%M:%S")
        self.log_text.insert(tk.END, f"[{timestamp}] {message}\n")
        self.log_text.see(tk.END)
        
    def start_broker(self):
        mode = self.broker_mode.get()
        port = self.broker_port.get()
        
        if mode == 'Mosquitto':
            self.start_mosquitto_broker(port)
        elif mode == 'Embedded Python':
            self.start_embedded_broker(port)
            
    def start_mosquitto_broker(self, port):
        try:
            if sys.platform == "win32":
                mosquitto_cmd = "mosquitto"
            else:
                mosquitto_cmd = "mosquitto"
            
            # Create temporary config file for anonymous access
            config_content = f"""
listener {port}
allow_anonymous true
"""
            config_file = "temp_mosquitto.conf"
            with open(config_file, 'w') as f:
                f.write(config_content)
            
            cmd = [mosquitto_cmd, "-c", config_file, "-v"]
            
            self.broker_process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            
            anon_status = "with anonymous access" if self.allow_anonymous.get() else ""
            self.log(f"✓ Mosquitto broker started on port {port} {anon_status}")
            self.broker_status.config(text="Status: Running", foreground="green")
            self.broker_start_btn.config(state="disabled")
            self.broker_stop_btn.config(state="normal")
            
        except FileNotFoundError:
            messagebox.showerror("Mosquitto Not Found", 
                "Mosquitto is not installed or not in PATH.\n\n"
                "Options:\n"
                "1. Install Mosquitto (recommended)\n"
                "2. Switch to 'Embedded Python' mode\n"
                "3. Use 'External' mode with your own broker")
            self.log("ERROR: Mosquitto not found")
            self.handle_mosquitto_not_found()
        except Exception as e:
            messagebox.showerror("Error", f"Failed to start broker: {str(e)}")
            self.log(f"ERROR: Failed to start broker - {str(e)}")
            
    def start_embedded_broker(self, port):
        """Start embedded Python MQTT broker"""
        try:
            # Try to import gmqtt
            try:
                import gmqtt
                from gmqtt import Server
            except ImportError:
                messagebox.showwarning("Package Required",
                    "Embedded broker requires 'gmqtt' package.\n\n"
                    "Install it with:\n"
                    "  pip install gmqtt\n\n"
                    "Or switch to Mosquitto/External mode.")
                self.log("ERROR: gmqtt not installed. Run: pip install gmqtt")
                return
            
            # Start broker in thread
            def run_broker():
                import asyncio
                
                async def start_server():
                    # Configure broker to allow anonymous connections
                    broker = Server(host='0.0.0.0', port=int(port))
                    await broker.start()
                    anon_status = "with anonymous access" if self.allow_anonymous.get() else ""
                    self.log(f"✓ Embedded broker started on port {port} {anon_status}")
                    # Keep running
                    while self.embedded_broker:
                        await asyncio.sleep(1)
                    await broker.shutdown()
                
                asyncio.run(start_server())
            
            self.embedded_broker = threading.Thread(target=run_broker, daemon=True)
            self.embedded_broker.start()
            
            self.log(f"✓ Embedded Python MQTT broker started on port {port}")
            self.broker_status.config(text="Status: Running", foreground="green")
            self.broker_start_btn.config(state="disabled")
            self.broker_stop_btn.config(state="normal")
            
        except Exception as e:
            messagebox.showerror("Error", f"Failed to start embedded broker: {str(e)}")
            self.log(f"ERROR: Failed to start embedded broker - {str(e)}")
            
    def stop_broker(self):
        mode = self.broker_mode.get()
        
        if mode == 'Mosquitto' and self.broker_process:
            self.broker_process.terminate()
            self.broker_process.wait()
            self.broker_process = None
            # Clean up temp config file
            try:
                if os.path.exists("temp_mosquitto.conf"):
                    os.remove("temp_mosquitto.conf")
            except:
                pass
            self.log("Mosquitto broker stopped")
            
        elif mode == 'Embedded Python' and self.embedded_broker:
            self.embedded_broker = None
            self.log("Embedded broker stopped")
            
        self.broker_status.config(text="Status: Stopped", foreground="red")
        self.broker_start_btn.config(state="normal")
        self.broker_stop_btn.config(state="disabled")
        
    def calculate_nmea_checksum(self, sentence):
        """Calculate NMEA 0183 checksum (XOR of all characters between $ and *)"""
        checksum = 0
        for char in sentence:
            checksum ^= ord(char)
        return f"{checksum:02X}"
    
    def create_vtg_sentence(self, stroke_rate):
        """Create NMEA 0183 VTG sentence from stroke rate"""
        # Calculate speed in knots: strokeRate * 60/1852
        speed_knots = stroke_rate * 60.0 / 1852.0
        # Convert to km/h: knots * 1.852
        speed_kph = speed_knots * 1.852
        
        # VTG sentence format: $GPVTG,course,T,course,M,speed_knots,N,speed_kph,K,mode*checksum
        # Heading is 0 as specified
        heading = 0.0
        mode = "A"  # Autonomous mode
        
        # Build sentence without checksum
        sentence_body = f"GPVTG,{heading:.1f},T,{heading:.1f},M,{speed_knots:.2f},N,{speed_kph:.2f},K,{mode}"
        
        # Calculate checksum
        checksum = self.calculate_nmea_checksum(sentence_body)
        
        # Complete sentence
        nmea_sentence = f"${sentence_body}*{checksum}"
        
        return nmea_sentence
    
    def on_mqtt_message(self, client, userdata, msg):
        try:
            payload = msg.payload.decode('utf-8')
            self.log(f" MQTT received on '{msg.topic}': {payload}")
            
            # Extract strokeRate field
            field_name = self.payload_field.get().strip()
            stroke_rate = None
            
            if field_name:
                try:
                    data = json.loads(payload)
                    if field_name in data:
                        stroke_rate = float(data[field_name])
                        self.log(f"   StrokeRate extracted: {stroke_rate}")
                    else:
                        self.log(f"⚠ WARNING: Field '{field_name}' not found in payload")
                        return
                except json.JSONDecodeError:
                    self.log("⚠ WARNING: Payload is not valid JSON")
                    return
                except ValueError:
                    self.log(f"⚠ WARNING: Could not convert '{field_name}' to number")
                    return
            else:
                self.log("⚠ WARNING: No strokeRate field specified")
                return
            
            # Create NMEA VTG sentence
            nmea_sentence = self.create_vtg_sentence(stroke_rate)
            
            # Send via UDP
            if self.udp_socket:
                udp_host = self.udp_host.get()
                udp_port = int(self.udp_port.get())
                # Add CRLF as per NMEA standard
                udp_data = nmea_sentence + "\r\n"
                self.udp_socket.sendto(udp_data.encode('ascii'), (udp_host, udp_port))
                self.log(f" UDP sent to {udp_host}:{udp_port}: {nmea_sentence}")
                
        except Exception as e:
            self.log(f"❌ ERROR processing message: {str(e)}")
            
    def start_subscriber(self):
        try:
            # Create UDP socket
            self.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            
            # Create MQTT client
            self.mqtt_client = mqtt.Client()
            self.mqtt_client.on_message = self.on_mqtt_message
            
            # Connect to broker
            host = self.mqtt_host.get()
            port = int(self.mqtt_port.get())
            topic = self.mqtt_topic.get()
            
            self.mqtt_client.connect(host, port, 60)
            self.mqtt_client.subscribe(topic)
            
            # Start loop in separate thread
            self.subscriber_running = True
            threading.Thread(target=self.mqtt_client.loop_forever, daemon=True).start()
            
            self.log(f"✓ Subscriber started - listening on '{topic}'")
            self.sub_status.config(text="Subscriber: Running", foreground="green")
            self.sub_start_btn.config(state="disabled")
            self.sub_stop_btn.config(state="normal")
            
        except Exception as e:
            messagebox.showerror("Error", f"Failed to start subscriber: {str(e)}")
            self.log(f"❌ ERROR: Failed to start subscriber - {str(e)}")
            if self.udp_socket:
                self.udp_socket.close()
                self.udp_socket = None
                
    def stop_subscriber(self):
        self.subscriber_running = False
        
        if self.mqtt_client:
            self.mqtt_client.loop_stop()
            self.mqtt_client.disconnect()
            self.mqtt_client = None
            
        if self.udp_socket:
            self.udp_socket.close()
            self.udp_socket = None
            
        self.log("Subscriber stopped")
        self.sub_status.config(text="Subscriber: Stopped", foreground="red")
        self.sub_start_btn.config(state="normal")
        self.sub_stop_btn.config(state="disabled")
        
    def clear_log(self):
        self.log_text.delete(1.0, tk.END)
        
    def on_closing(self):
        self.stop_subscriber()
        self.stop_broker()
        # Clean up temp files
        try:
            if os.path.exists("temp_mosquitto.conf"):
                os.remove("temp_mosquitto.conf")
        except:
            pass
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = MQTTUDPBridge(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()