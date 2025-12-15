///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.0.0-0-g0efcecf)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/gdicmn.h>
#include <wx/textctrl.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/timer.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

#define ID_TIMER_4 1000
#define ID_TIMER_2 1001
#define ID_TIMER_3 1002
#define ID_TIMER_1 1003

///////////////////////////////////////////////////////////////////////////////
/// Class m_Dialog
///////////////////////////////////////////////////////////////////////////////
class m_Dialog : public wxDialog
{
	private:

	protected:
		wxStaticLine* m_staticline2;
		wxStaticText* m_staticText71;
		wxCheckBox* m_checkBoxDuration;
		wxStaticText* m_staticText8;
		wxStaticText* m_staticText9;
		wxCheckBox* m_checkBoxCountdown;
		wxStaticText* m_staticText10;
		wxStaticText* m_staticText11;
		wxCheckBox* m_checkBoxRepeat;
		wxStaticLine* m_staticline6;
		wxStaticText* m_staticTextDuration;
		wxStaticText* m_staticTextDuration2;
		wxStaticText* m_staticTextCD2;
		wxStaticText* m_staticTextRepeat;
		wxStaticText* m_staticTextRepeat2;
		wxStaticLine* m_staticline61;
		wxStaticLine* m_staticline4;
		wxButton* m_button3111;
		wxButton* m_button31111;
		wxStaticLine* m_staticline62;
		wxTimer m_timer4;
		wxTimer m_timer2;
		wxTimer m_timer3;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnDLeftClick( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnDuration( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCountdown( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRepeat( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnStartTimer( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnStopTimer( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTimer4( wxTimerEvent& event ) { event.Skip(); }
		virtual void OnClock( wxTimerEvent& event ) { event.Skip(); }
		virtual void OnTimer3( wxTimerEvent& event ) { event.Skip(); }
		virtual void OnTimer( wxTimerEvent& event ) { event.Skip(); }


	public:
		wxBoxSizer* bSizerMain;
		wxTextCtrl* m_ClockTime;
		wxChoice* m_duration;
		wxBoxSizer* bSizerCountdown;
		wxStaticText* m_staticTextCD;
		wxChoice* m_choiceCD;
		wxChoice* m_choiceRepeat;
		wxTextCtrl* m_textTime;
		wxBoxSizer* bSizer71;
		wxTimer m_timer1;

		m_Dialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxRESIZE_BORDER );

		~m_Dialog();

};

