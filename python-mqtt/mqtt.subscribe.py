import tkinter as tk
from tkinter import ttk, scrolledtext
import paho.mqtt.client as mqtt
import socket
import threading
import json
from datetime import datetime

class MQTTtoUDPApp:
    def __init__(self, root):
        self.root = root
        self.root.title("MQTT to UDP Bridge")
        self.root.geometry("600x550")
        
        self.mqtt_client = None
        self.udp_socket = None
        self.is_running = False
        
        self.create_widgets()
        
    def create_widgets(self):
        # MQTT Settings Frame
        mqtt_frame = ttk.LabelFrame(self.root, text="MQTT Broker Settings", padding=10)
        mqtt_frame.grid(row=0, column=0, padx=10, pady=5, sticky="ew")
        
        ttk.Label(mqtt_frame, text="Broker:").grid(row=0, column=0, sticky="w", pady=2)
        self.mqtt_broker = ttk.Entry(mqtt_frame, width=30)
        self.mqtt_broker.insert(0, "localhost")
        self.mqtt_broker.grid(row=0, column=1, padx=5, pady=2)
        
        ttk.Label(mqtt_frame, text="Port:").grid(row=1, column=0, sticky="w", pady=2)
        self.mqtt_port = ttk.Entry(mqtt_frame, width=30)
        self.mqtt_port.insert(0, "1883")
        self.mqtt_port.grid(row=1, column=1, padx=5, pady=2)
        
        ttk.Label(mqtt_frame, text="Topic:").grid(row=2, column=0, sticky="w", pady=2)
        self.mqtt_topic = ttk.Entry(mqtt_frame, width=30)
        self.mqtt_topic.insert(0, "#")
        self.mqtt_topic.grid(row=2, column=1, padx=5, pady=2)
        
        # UDP Settings Frame
        udp_frame = ttk.LabelFrame(self.root, text="UDP Destination Settings", padding=10)
        udp_frame.grid(row=1, column=0, padx=10, pady=5, sticky="ew")
        
        ttk.Label(udp_frame, text="Host:").grid(row=0, column=0, sticky="w", pady=2)
        self.udp_host = ttk.Entry(udp_frame, width=30)
        self.udp_host.insert(0, "127.0.0.1")
        self.udp_host.grid(row=0, column=1, padx=5, pady=2)
        
        ttk.Label(udp_frame, text="Port:").grid(row=1, column=0, sticky="w", pady=2)
        self.udp_port = ttk.Entry(udp_frame, width=30)
        self.udp_port.insert(0, "5000")
        self.udp_port.grid(row=1, column=1, padx=5, pady=2)
        
        # Control Buttons
        btn_frame = ttk.Frame(self.root)
        btn_frame.grid(row=2, column=0, padx=10, pady=10)
        
        self.start_btn = ttk.Button(btn_frame, text="Start Bridge", command=self.start_bridge)
        self.start_btn.grid(row=0, column=0, padx=5)
        
        self.stop_btn = ttk.Button(btn_frame, text="Stop Bridge", command=self.stop_bridge, state="disabled")
        self.stop_btn.grid(row=0, column=1, padx=5)
        
        # Status
        self.status_label = ttk.Label(self.root, text="Status: Stopped", foreground="red")
        self.status_label.grid(row=3, column=0, padx=10, pady=5)
        
        # Log Frame
        log_frame = ttk.LabelFrame(self.root, text="Log", padding=10)
        log_frame.grid(row=4, column=0, padx=10, pady=5, sticky="nsew")
        
        self.log_text = scrolledtext.ScrolledText(log_frame, height=12, width=70)
        self.log_text.grid(row=0, column=0, sticky="nsew")
        
        self.root.grid_rowconfigure(4, weight=1)
        self.root.grid_columnconfigure(0, weight=1)
        
    def log(self, message):
        timestamp = datetime.now().strftime("%H:%M:%S")
        self.log_text.insert(tk.END, f"[{timestamp}] {message}\n")
        self.log_text.see(tk.END)
        
    def on_mqtt_connect(self, client, userdata, flags, rc):
        if rc == 0:
            self.log("Connected to MQTT broker")
            topic = self.mqtt_topic.get()
            client.subscribe(topic)
            self.log(f"Subscribed to topic: {topic}")
        else:
            self.log(f"Failed to connect to MQTT broker. Code: {rc}")
            
    def on_mqtt_message(self, client, userdata, msg):
        try:
            # Parse payload as JSON
            payload = msg.payload.decode('utf-8')
            
            try:
                data = json.loads(payload)
                stroke_rate = float(data.get('strokeRate', 0.0))
            except (ValueError, json.JSONDecodeError) as e:
                self.log(f"Warning: Could not parse JSON or find strokeRate, using 0.0")
                stroke_rate = 0.0
            
            # Calculate speed: strokeRate * 60 / 1852
            speed_knots = (stroke_rate * 60) / 1852
            speed_kmh = speed_knots * 1.852  # Convert knots to km/h
            
            # Create NMEA 0183 VTG sentence
            # $GPVTG,track_true,T,track_mag,M,speed_knots,N,speed_kmh,K,mode*checksum
            heading = 0.0
            sentence_body = f"GPVTG,{heading:.1f},T,,M,{speed_knots:.1f},N,{speed_kmh:.1f},K,A"
            
            # Calculate checksum (XOR of all characters between $ and *)
            checksum = 0
            for char in sentence_body:
                checksum ^= ord(char)
            
            nmea_sentence = f"${sentence_body}*{checksum:02X}\r\n"
            
            # Send via UDP
            self.udp_socket.sendto(nmea_sentence.encode('utf-8'), (self.udp_host.get(), int(self.udp_port.get())))
            
            self.log(f"Sent: {nmea_sentence.strip()} (strokeRate: {stroke_rate})")
        except Exception as e:
            self.log(f"Error forwarding message: {e}")
            
    def start_bridge(self):
        try:
            # Create UDP socket
            self.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.log("UDP socket created")
            
            # Create MQTT client
            self.mqtt_client = mqtt.Client()
            self.mqtt_client.on_connect = self.on_mqtt_connect
            self.mqtt_client.on_message = self.on_mqtt_message
            
            # Connect to MQTT broker
            broker = self.mqtt_broker.get()
            port = int(self.mqtt_port.get())
            
            self.mqtt_client.connect(broker, port, 60)
            self.mqtt_client.loop_start()
            
            self.is_running = True
            self.status_label.config(text="Status: Running", foreground="green")
            self.start_btn.config(state="disabled")
            self.stop_btn.config(state="normal")
            self.log("Bridge started successfully")
            
        except Exception as e:
            self.log(f"Error starting bridge: {e}")
            self.stop_bridge()
            
    def stop_bridge(self):
        try:
            if self.mqtt_client:
                self.mqtt_client.loop_stop()
                self.mqtt_client.disconnect()
                self.log("Disconnected from MQTT broker")
                
            if self.udp_socket:
                self.udp_socket.close()
                self.log("UDP socket closed")
                
            self.is_running = False
            self.status_label.config(text="Status: Stopped", foreground="red")
            self.start_btn.config(state="normal")
            self.stop_btn.config(state="disabled")
            self.log("Bridge stopped")
            
        except Exception as e:
            self.log(f"Error stopping bridge: {e}")

if __name__ == "__main__":
    root = tk.Tk()
    app = MQTTtoUDPApp(root)
    root.mainloop()