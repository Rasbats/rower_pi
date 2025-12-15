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

#ifndef _CALCULATORGUI_IMPL_H_
#define _CALCULATORGUI_IMPL_H_

#ifdef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "e_timer_pi.h"
#include "e_timergui.h"
#include "wx/window.h"
#include "ocpn_plugin.h"
#include <wx/menu.h>

#include <list>
#include <vector>

#define ID_SOMETHING 2001
#define ID_SOMETHING_ELSE 2002

using namespace std;

class e_timer_pi;

class Dlg : public m_Dialog {
public:
  Dlg(wxWindow* parent, e_timer_pi* ppi);
  ~Dlg();

  wxWindow* pParent;
  e_timer_pi* pPlugIn;

#ifdef __ANDROID__
  void OnMouseEvent(wxMouseEvent& event);
  wxPoint m_resizeStartPoint;
  wxSize m_resizeStartSize;
  bool m_binResize;
  bool m_binResize2;

  void OnPopupClick(wxCommandEvent& evt);
  void OnDLeftClick(wxMouseEvent& event);

#endif

private:
  void OnStartTimer(wxCommandEvent& event);
  void OnStopTimer(wxCommandEvent& event);
  void OnTimer(wxTimerEvent& event);
  void OnTimer3(wxTimerEvent& event);
  void OnTimer4(wxTimerEvent& event);
  void OnDuration(wxCommandEvent& event);
  void OnCountdown(wxCommandEvent& event);
  void OnRepeat(wxCommandEvent& event);
  void OnClose(wxCloseEvent& event);
  void Notify();
  void Notify2();
  void Notify3();
  void Notify4();
  void OnClock(wxTimerEvent& event);
  void UpdateClock();
  bool dbg;
  int g_tick;
  wxString g_anchorwatch_sound_file;
  wxString m_soundFile;
  wxString m_empty_soundFile;
  bool play_sound;
  bool stop_notify;
  wxDateTime m_dtNow;
  void FillCountdown();
};

#endif
