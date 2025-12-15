/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Timer Plugin
 * Author:   Mike Rossiter
 *
 ***************************************************************************
 *   Copyright (C) 2013 by Mike Rossiter                                   *
 *   $EMAIL$                                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#include "e_timergui_impl.h"
#include "e_timer_pi.h"
#include "e_timergui.h"
#include "icons.h"

#include "wx/dir.h"
#include <cmath>
#include <list>
#include <wx/datetime.h>
#include <wx/progdlg.h>
#include <wx/timer.h>
#include <wx/wx.h>
#include "qtstylesheet.h"
#include <iostream>
#include <iomanip>
#include <sstream>

#ifdef __ANDROID__
wxWindow* g_Window;
#endif

#define FAIL(X)  \
  do {           \
    error = X;   \
    goto failed; \
  } while (0)

Dlg::Dlg(wxWindow* parent, e_timer_pi* ppi) : m_Dialog(parent) {
  this->Fit();
  dbg = false;  // for debug output set to true

  pPlugIn = ppi;
  pParent = parent;

#ifdef __ANDROID__

  m_binResize = false;

  g_Window = this;
  GetHandle()->setStyleSheet(qtStyleSheet);
  Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(Dlg::OnMouseEvent));
  Connect(wxEVT_LEFT_UP, wxMouseEventHandler(Dlg::OnMouseEvent));

  Connect(wxEVT_MOTION, wxMouseEventHandler(Dlg::OnMouseEvent));
#endif

  wxString s = "/";
  const char* pName = "e_timer_pi";
  wxString sound_dir = GetPluginDataDir(pName) + s + "data" + s;

  // Set reasonable defaults
  sound_dir.Append("sounds");
  sound_dir.Append(wxFileName::GetPathSeparator());

  // create sound file
  m_soundFile = sound_dir + "short-alarm.wav";
  m_empty_soundFile = sound_dir + "empty.wav";
  g_tick = 0;
  // Hide choices not selected
  m_staticTextCD->Hide();
  m_staticTextCD2->Hide();
  m_choiceCD->Hide();

  m_staticTextDuration->Hide();
  m_duration->Hide();
  m_staticTextDuration2->Hide();

  m_staticTextRepeat->Hide();
  m_choiceRepeat->Hide();
  m_staticTextRepeat2->Hide();

  m_checkBoxDuration->SetValue(false);
  m_checkBoxCountdown->SetValue(false);
  m_checkBoxRepeat->SetValue(false);

  m_timer2.Start(1000, wxTIMER_CONTINUOUS);
  m_textTime->SetValue("   00:00");
  stop_notify = false;
}

Dlg::~Dlg() {}

#ifdef __ANDROID__
wxPoint g_startPos;
wxPoint g_startMouse;
wxPoint g_mouse_pos_screen;

void Dlg::OnPopupClick(wxCommandEvent& evt) {
  switch (evt.GetId()) {
    case ID_SOMETHING:
      m_binResize = true;
      break;
      // case ID_SOMETHING_ELSE:
      //   break;
  }
}

void Dlg::OnDLeftClick(wxMouseEvent& event) {
  wxMenu mnu;
  mnu.Append(ID_SOMETHING, "Resize...");
  // mnu.Append(ID_SOMETHING_ELSE, "Do something else");
  mnu.Connect(wxEVT_COMMAND_MENU_SELECTED,
              wxCommandEventHandler(Dlg::OnPopupClick), NULL, this);
  PopupMenu(&mnu);
}

void Dlg::OnMouseEvent(wxMouseEvent& event) {
  if (m_binResize) {
    wxSize currentSize = g_Window->GetSize();
    wxSize par_size = GetOCPNCanvasWindow()->GetClientSize();
    wxPoint par_pos = g_Window->GetPosition();
    if (event.LeftDown()) {
      m_resizeStartPoint = event.GetPosition();
      m_resizeStartSize = currentSize;
      m_binResize2 = true;
    }

    if (m_binResize2) {
      if (event.Dragging()) {
        wxPoint p = event.GetPosition();

        wxSize dragSize = m_resizeStartSize;

        dragSize.y = p.y;  //  - m_resizeStartPoint.y;
        dragSize.x = p.x;  //  - m_resizeStartPoint.x;
        ;
        /*
        if ((par_pos.y + dragSize.y) > par_size.y)
            dragSize.y = par_size.y - par_pos.y;

        if ((par_pos.x + dragSize.x) > par_size.x)
            dragSize.x = par_size.x - par_pos.x;
*/
        // not too small
        dragSize.x = wxMax(dragSize.x, 150);
        dragSize.y = wxMax(dragSize.y, 150);

        int x = wxMax(0, m_resizeStartPoint.x);
        int y = wxMax(0, m_resizeStartPoint.y);
        int xmax = ::wxGetDisplaySize().x - GetSize().x;
        x = wxMin(x, xmax);
        int ymax =
            ::wxGetDisplaySize().y - (GetSize().y);  // Some fluff at the bottom
        y = wxMin(y, ymax);

        g_Window->Move(x, y);
      }
      if (event.LeftUp()) {
        wxPoint p = event.GetPosition();

        wxSize dragSize = m_resizeStartSize;

        dragSize.y = p.y;
        dragSize.x = p.x;

        // not too small
        dragSize.x = wxMax(dragSize.x, 150);
        dragSize.y = wxMax(dragSize.y, 150);

        g_Window->SetSize(dragSize);

        m_binResize = false;
        m_binResize2 = false;
      }
    }
  } else {
    if (event.Dragging()) {
      m_resizeStartPoint = event.GetPosition();
      int x = wxMax(0, m_resizeStartPoint.x);
      int y = wxMax(0, m_resizeStartPoint.y);
      int xmax = ::wxGetDisplaySize().x - GetSize().x;
      x = wxMin(x, xmax);
      int ymax =
          ::wxGetDisplaySize().y - (GetSize().y);  // Some fluff at the bottom
      y = wxMin(y, ymax);

      g_Window->Move(x, y);
    }
  }
}

#endif  // End of Android functions for move/resize

void Dlg::OnClock(wxTimerEvent& event) {
  Notify2();
  event.Skip();
}

void Dlg::OnStartTimer(wxCommandEvent& event) {
  play_sound = false;

  g_tick = 0;
  if (m_checkBoxDuration->IsChecked()) {
    m_timer1.Start(1000, wxTIMER_CONTINUOUS);
    Notify();
  } else if (m_checkBoxCountdown->IsChecked()) {
    int c = m_choiceCD->GetSelection();
    wxString s = m_choiceCD->GetString(c);
    int cs = atoi(s);
    g_tick = cs * 60;
    m_timer3.Start(1000, wxTIMER_CONTINUOUS);
    Notify3();
  } else if (m_checkBoxRepeat->IsChecked()) {
    m_timer4.Start(1000, wxTIMER_CONTINUOUS);
    Notify4();
  }
}

void Dlg::OnStopTimer(wxCommandEvent& event) {
  play_sound = false;

  m_timer1.Stop();
  m_timer3.Stop();
  m_timer4.Stop();

  m_textTime->SetValue("   00:00");
  g_tick = 0;

  if (m_checkBoxCountdown->IsChecked()) {
    m_checkBoxCountdown->SetValue(false);
    // Hide choices not selected
    m_staticTextCD->Hide();
    m_staticTextCD2->Hide();
    m_choiceCD->Hide();

  } else if (m_checkBoxDuration->IsChecked()) {
    m_checkBoxDuration->SetValue(false);
    m_staticTextDuration->Hide();
    m_duration->Hide();
    m_staticTextDuration2->Hide();

  } else if (!m_checkBoxRepeat->IsChecked()) {
    m_checkBoxRepeat->SetValue(false);
    m_staticTextRepeat->Hide();
    m_choiceRepeat->Hide();
    m_staticTextRepeat2->Hide();
    m_timer4.Stop();
  } else if (m_checkBoxRepeat->IsChecked()) {
    play_sound = false;
    m_textTime->SetValue("   00:00");

    g_tick = 0;
    wxMilliSleep(1000);

    m_timer4.Start(1000, wxTIMER_CONTINUOUS);
  }
}

void Dlg::OnTimer(wxTimerEvent& event) {
  g_tick++;
  Notify();
}

void Dlg::OnTimer3(wxTimerEvent& event) {
  g_tick--;
  Notify3();
}

void Dlg::OnTimer4(wxTimerEvent& event) {
  g_tick++;
  Notify4();
}

void Dlg::Notify() {
  int totalSeconds = g_tick;
  int minutes = totalSeconds / 60;
  totalSeconds = totalSeconds % 60;

  int seconds = totalSeconds;

  stringstream ss;
  ss << setw(2) << setfill('0') << seconds << endl;

  stringstream mm;
  mm << setw(2) << setfill('0') << minutes << endl;

  m_textTime->SetValue("   " + mm.str() + ":" + ss.str());

  int i_interval = this->m_duration->GetSelection();
  wxString s_interval = this->m_duration->GetString(i_interval);
  int myInterval = wxAtoi(s_interval) * 60;

  if (g_tick >= myInterval) {
    // wxMessageBox("sound");
    play_sound = true;
    g_tick = 0;
    m_textTime->SetValue("   00:00");
  }

  if (play_sound) {
    PlugInPlaySound(m_soundFile);
  }
  //      wxMessageBox(interv);
}

void Dlg::Notify2() { UpdateClock(); }

void Dlg::UpdateClock() {
  wxDateTime dt = wxDateTime::Now();
  wxString s = dt.Format("%H:%M:%S");
  m_ClockTime->SetLabel(s);
}

void Dlg::Notify3() {
  int totalSeconds = g_tick;
  int minutes = totalSeconds / 60;
  totalSeconds = totalSeconds % 60;

  int seconds = totalSeconds;

  stringstream ss;
  ss << setw(2) << setfill('0') << seconds << endl;

  stringstream mm;
  mm << setw(2) << setfill('0') << minutes << endl;

  m_textTime->SetValue("   " + mm.str() + ":" + ss.str());
  /*
  int i_interval = this->m_choiceCD->GetSelection();
  wxString s_interval = this->m_choiceCD->GetString(i_interval);
  int myInterval = wxAtoi(s_interval) * 60;*/
  if (g_tick <= 0) {
    // wxMessageBox("sound");
    play_sound = true;
    g_tick = 0;
    m_textTime->SetValue("   00:00");
  }

  if (play_sound) {
    PlugInPlaySound(m_soundFile);
  }
  //      wxMessageBox(interv);
}

void Dlg::Notify4() {
  if (m_checkBoxRepeat->IsChecked()) {
    int totalSeconds = g_tick;
    int minutes = totalSeconds / 60;
    totalSeconds = totalSeconds % 60;

    int seconds = totalSeconds;

    stringstream ss;
    ss << setw(2) << setfill('0') << seconds << endl;

    stringstream mm;
    mm << setw(2) << setfill('0') << minutes << endl;

    m_textTime->SetValue("   " + mm.str() + ":" + ss.str());

    int i_interval = this->m_choiceRepeat->GetSelection();
    wxString s_interval = this->m_choiceRepeat->GetString(i_interval);
    int myInterval = wxAtoi(s_interval) * 60;
    if (g_tick >= myInterval) {
      play_sound = true;
      g_tick = 0;
      m_textTime->SetValue("   00:00");
    } 

    if (play_sound) {
      PlugInPlaySound(m_soundFile);      
    }
  }
}

void Dlg::OnDuration(wxCommandEvent& event) {
  if (m_checkBoxDuration->IsChecked()) {
    m_staticTextDuration->Show();
    m_duration->Show();
    m_staticTextDuration2->Show();

    m_checkBoxCountdown->SetValue(false);
    m_checkBoxRepeat->SetValue(false);
    m_timer4.Stop();
    m_timer1.Stop();
    m_timer3.Stop();

    // Hide choices not selected
    m_staticTextCD->Hide();
    m_staticTextCD2->Hide();
    m_choiceCD->Hide();

    m_staticTextRepeat->Hide();
    m_choiceRepeat->Hide();
    m_staticTextRepeat2->Hide();

    m_textTime->SetValue("   00:00");

  } else {
    m_staticTextDuration->Hide();
    m_duration->Hide();
    m_staticTextDuration2->Hide();
    m_textTime->SetValue("   00:00");
  }
  this->SetSizer(bSizerMain);
  this->Layout();
  bSizerMain->Fit(this);
}

void Dlg::OnCountdown(wxCommandEvent& event) {
  if (m_checkBoxCountdown->IsChecked()) {
    m_staticTextCD->Show();
    m_choiceCD->Show();
    m_staticTextCD2->Show();
    FillCountdown();

    m_checkBoxDuration->SetValue(false);
    m_checkBoxRepeat->SetValue(false);
    m_timer4.Stop();
    m_timer1.Stop();
    m_timer3.Stop();

    // Hide choices not selected
    m_staticTextDuration->Hide();
    m_duration->Hide();
    m_staticTextDuration2->Hide();

    m_staticTextRepeat->Hide();
    m_choiceRepeat->Hide();
    m_staticTextRepeat2->Hide();

    m_textTime->SetValue("   00:00");

  } else {
    m_staticTextCD->Hide();
    m_choiceCD->Hide();
    m_staticTextCD2->Hide();
    m_textTime->SetValue("   00:00");
  }

  this->SetSizer(bSizerMain);
  this->Layout();
  bSizerMain->Fit(this);

  // wxMessageBox("here");
}

void Dlg::OnRepeat(wxCommandEvent& event) {
  if (m_checkBoxRepeat->IsChecked()) {
    m_staticTextRepeat->Show();
    m_choiceRepeat->Show();
    m_staticTextRepeat2->Show();

    m_checkBoxDuration->SetValue(false);
    m_checkBoxCountdown->SetValue(false);
    m_timer1.Stop();
    m_timer3.Stop();

    // Hide choices not selected
    m_staticTextCD->Hide();
    m_staticTextCD2->Hide();
    m_choiceCD->Hide();

    m_staticTextDuration->Hide();
    m_duration->Hide();
    m_staticTextDuration2->Hide();

    m_textTime->SetValue("   00:00");

  } else {
    m_staticTextRepeat->Hide();
    m_choiceRepeat->Hide();
    m_staticTextRepeat2->Hide();
    m_textTime->SetValue("   00:00");
    m_timer4.Stop();
  }

  this->SetSizer(bSizerMain);
  this->Layout();
  bSizerMain->Fit(this);

  // wxMessageBox("here");
}

void Dlg::FillCountdown() {
  for (int i = 1; i <= 60; i++) {
    wxString myInt = wxString::Format("%i", i);
    m_choiceCD->Append(myInt);
  }
}

void Dlg::OnClose(wxCloseEvent& event) { pPlugIn->OnTimerDialogClose(); }
