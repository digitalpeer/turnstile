/* $Id: debugframe.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
 *
 * Turnstile
 *
 * Copyright (C) 2004, J.D. Henderson <www.digitalpeer.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "debugframe.h"
#include "mainframe.h"
#include "frameutils.h"
#include "aboutdialog.h"

//#include <wx/log.h>

#include "../pixmaps/debugframe.xpm"

#define CONFIG_DEBUG_WINDOW "debugwindow"

enum	{	
		ID_MENU_SAVE,
		ID_MENU_QUIT,
		ID_MENU_ABOUT
	};

BEGIN_EVENT_TABLE(DebugFrame, wxFrame)
    	EVT_SIZE(DebugFrame::OnSize)
	EVT_MENU(ID_MENU_SAVE, DebugFrame::OnSave)
    	EVT_MENU(ID_MENU_QUIT, DebugFrame::OnQuit)
    	EVT_MENU(ID_MENU_ABOUT, DebugFrame::OnAbout)
	EVT_CLOSE(DebugFrame::OnClose)
END_EVENT_TABLE()

DebugFrame::DebugFrame(MainFrame* parent)
	: wxFrame(0, -1, "Turnstile - Debug Window",wxDefaultPosition, wxDefaultSize),
	  	parent_(parent),text_(0)
{
	SetSize(DetermineFrameSize(CONFIG_DEBUG_WINDOW));

	SetIcon(wxIcon(debugframe));

    	SetBackgroundColour(wxColour(255, 255, 255));

	// create main window menus
	wxMenu* file_menu = new wxMenu;
	wxMenu* help_menu = new wxMenu;

	file_menu->Append(ID_MENU_SAVE, "Save");
	file_menu->AppendSeparator();
    	file_menu->Append(ID_MENU_QUIT, "Quit\tAlt-X");

	help_menu->Append(ID_MENU_ABOUT, "&About");

    	wxMenuBar* menu_bar = new wxMenuBar;
    	menu_bar->Append(file_menu, "&File");
    	menu_bar->Append(help_menu, "&Help");
    	SetMenuBar(menu_bar);

	// make a textctrl
  	text_  = new wxTextCtrl(this, -1,
				"** New Debug Session Started **\n",
				wxDefaultPosition, wxDefaultSize,
				wxTE_MULTILINE | wxTE_READONLY);

	// save old target so we can fix it back when we exit
	m_logTargetOld = wxLog::SetActiveTarget(new wxLogTextCtrl(text_));
	//wxStreamToTextRedirector redirect(text_);


}

void
DebugFrame::OnSize(wxSizeEvent& event)
{
	Resize();
	event.Skip();
}

void
DebugFrame::Resize()
{
	if (text_ != 0)
    	{
		wxSize size = GetClientSize();
    		text_->SetSize(0, 0, size.x, size.y);
	}
}

void DebugFrame::OnSave(wxCommandEvent& WXUNUSED(event))
{
	wxString file = wxFileSelector("Save to File", "","", "", "*.*", wxSAVE, this ,-1, -1);

	if (file.Length() != 0)
		text_->SaveFile(file);
}

void DebugFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	parent_->OnCloseDebugFrame();
    	Close(TRUE);
}

void DebugFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    	AboutDialog it(this);
   	it.ShowModal();
}

void
DebugFrame::OnClose(wxCloseEvent& event)
{
	//wxLog::SetActiveTarget(m_logTargetOld);

	StoreFrameSize(GetRect(),CONFIG_DEBUG_WINDOW);
	parent_->OnCloseDebugFrame();
	Destroy();
}

void DebugFrame::Log(wxString text)
{
	text_->AppendText(text);
}

DebugFrame::~DebugFrame()
{
	wxLog::SetActiveTarget(m_logTargetOld);
}
