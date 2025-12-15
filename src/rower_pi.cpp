/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Timer Plugin
 * Author:   Mike Rossiter
 *
 ***************************************************************************
 *   Copyright (C) 2013 by Mike Rossiter                                *
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

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "e_timer_pi.h"
#include "e_timergui_impl.h"
#include "e_timergui.h"

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin *create_pi(void *ppimgr) {
  return new e_timer_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin *p) { delete p; }

//---------------------------------------------------------------------------------------------------------
//
//    Timer PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

#include "icons.h"

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

e_timer_pi::e_timer_pi(void *ppimgr) : opencpn_plugin_118(ppimgr) {
  // Create the PlugIn icons
  initialize_images();

  wxFileName fn;
  wxString tmp_path;

  tmp_path = GetPluginDataDir("e_timer_pi");
  fn.SetPath(tmp_path);
  fn.AppendDir("data");
  fn.SetFullName("e_timer_pi_panel_icon.png");

  wxString shareLocn = fn.GetFullPath();
  wxImage panelIcon(shareLocn);

  if (panelIcon.IsOk())
    m_panelBitmap = wxBitmap(panelIcon);
  else
    wxLogMessage("e_timer_pi panel icon NOT loaded");
  m_bShowTimer = false;
}

e_timer_pi::~e_timer_pi(void) {
  delete _img_e_timer_pi;
  delete _img_Timer;
}

int e_timer_pi::Init(void) {
  AddLocaleCatalog("opencpn-e_timer_pi");

  // Set some default private member parameters
  m_route_dialog_x = 0;
  m_route_dialog_y = 0;
  ::wxDisplaySize(&m_display_width, &m_display_height);

  //    Get a pointer to the opencpn display canvas, to use as a parent for the
  //    POI Manager dialog
  m_parent_window = GetOCPNCanvasWindow();

  //    Get a pointer to the opencpn configuration object
  m_pconfig = GetOCPNConfigObject();

  //    And load the configuration items
  LoadConfig();

  //    This PlugIn needs a toolbar icon, so request its insertion
  if (m_bTimerShowIcon)

#ifdef ocpnUSE_SVG
    m_leftclick_tool_id = InsertPlugInToolSVG(
        "Timer", _svg_timer, _svg_timer, _svg_timer_toggled, wxITEM_CHECK,
        "Timer", "", NULL, CALCULATOR_TOOL_POSITION, 0, this);
#else
    m_leftclick_tool_id = InsertPlugInTool(
        "", _img_Timer, _img_Timer, wxITEM_CHECK, "Timer"), "", NULL,
        CALCULATOR_TOOL_POSITION, 0, this);
#endif

  m_pDialog = NULL;

  return (WANTS_OVERLAY_CALLBACK | WANTS_OPENGL_OVERLAY_CALLBACK |

          WANTS_CURSOR_LATLON | WANTS_TOOLBAR_CALLBACK | INSTALLS_TOOLBAR_TOOL |
          WANTS_CONFIG | WANTS_PLUGIN_MESSAGING

  );
}

bool e_timer_pi::DeInit(void) {
  //    Record the dialog position
  if (NULL != m_pDialog) {
    // Capture dialog position
    wxPoint p = m_pDialog->GetPosition();
    SetCalculatorDialogX(p.x);
    SetCalculatorDialogY(p.y);
    m_pDialog->Close();
    delete m_pDialog;
    m_pDialog = NULL;

    m_bShowTimer = false;
    SetToolbarItemState(m_leftclick_tool_id, m_bShowTimer);
  }

  SaveConfig();

  RequestRefresh(m_parent_window);  // refresh main window

  return true;
}

int e_timer_pi::GetAPIVersionMajor() { return atoi(API_VERSION); }

int e_timer_pi::GetAPIVersionMinor() {
  std::string v(API_VERSION);
  size_t dotpos = v.find('.');
  return atoi(v.substr(dotpos + 1).c_str());
}

int e_timer_pi::GetPlugInVersionMajor() { return PLUGIN_VERSION_MAJOR; }

int e_timer_pi::GetPlugInVersionMinor() { return PLUGIN_VERSION_MINOR; }

int GetPlugInVersionPatch() { return PLUGIN_VERSION_PATCH; }

int GetPlugInVersionPost() { return PLUGIN_VERSION_TWEAK; }

const char *GetPlugInVersionPre() { return PKG_PRERELEASE; }

const char *GetPlugInVersionBuild() { return PKG_BUILD_INFO; }

wxBitmap *e_timer_pi::GetPlugInBitmap() { return &m_panelBitmap; }

wxString e_timer_pi::GetCommonName() { return PLUGIN_API_NAME; }
wxString e_timer_pi::GetShortDescription() { return PKG_SUMMARY; }
wxString e_timer_pi::GetLongDescription() { return PKG_DESCRIPTION; }

int e_timer_pi::GetToolbarToolCount(void) { return 1; }

void e_timer_pi::SetColorScheme(PI_ColorScheme cs) {
  if (NULL == m_pDialog) return;

  DimeWindow(m_pDialog);
}

void e_timer_pi::OnToolbarToolCallback(int id) {
  if (NULL == m_pDialog) {
    m_pDialog = new Dlg(m_parent_window, this);
    m_pDialog->Move(wxPoint(m_route_dialog_x, m_route_dialog_y));
  }

  m_pDialog->Fit();
  // Toggle
  m_bShowTimer = !m_bShowTimer;

  //    Toggle dialog?
  if (m_bShowTimer) {
    m_pDialog->Show();
  } else
    m_pDialog->Hide();

  // Toggle is handled by the toolbar but we must keep plugin manager b_toggle
  // updated to actual status to ensure correct status upon toolbar rebuild
  SetToolbarItemState(m_leftclick_tool_id, m_bShowTimer);

  RequestRefresh(m_parent_window);  // refresh main window
}

bool e_timer_pi::LoadConfig(void) {
  wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

  if (pConf) {
    pConf->SetPath("/Settings/e_timer_pi");
    pConf->Read("ShowTimerIcon", &m_bTimerShowIcon, 1);

    m_route_dialog_x = pConf->Read("DialogPosX", 20L);
    m_route_dialog_y = pConf->Read("DialogPosY", 20L);

    if ((m_route_dialog_x < 0) || (m_route_dialog_x > m_display_width))
      m_route_dialog_x = 5;
    if ((m_route_dialog_y < 0) || (m_route_dialog_y > m_display_height))
      m_route_dialog_y = 5;
    return true;
  } else
    return false;
}

bool e_timer_pi::SaveConfig(void) {
  wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

  if (pConf) {
    pConf->SetPath("/Settings/e_timer_pi");
    pConf->Write("ShowTimerIcon", m_bTimerShowIcon);

    pConf->Write("DialogPosX", m_route_dialog_x);
    pConf->Write("DialogPosY", m_route_dialog_y);

    return true;
  } else
    return false;
}

void e_timer_pi::OnTimerDialogClose() {
  m_bShowTimer = false;
  SetToolbarItemState(m_leftclick_tool_id, m_bShowTimer);
  m_pDialog->Hide();
  SaveConfig();

  RequestRefresh(m_parent_window);  // refresh main window
}
