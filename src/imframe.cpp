/* $Id: imframe.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file imframe.cpp
@brief Instant Message Window Implimentation
*/
#include "app.h"
#include "imframe.h"
#include "buddy.h"
#include "frameutils.h"
#include "prefs.h"
#include "aboutdialog.h"
#include "packet.h"
#include "mainframe.h"

#include <wx/datetime.h>

#include "../pixmaps/imframe.xpm"
#include "../pixmaps/imframe_send.xpm"
#include "../pixmaps/imframe_hide.xpm"
#include "../pixmaps/imframe_close.xpm"

#define IM_WINDOW "imwindow"

#define TXT_MENU_SAVE	"Save Conversation"
#define TXT_MENU_LOAD	"Load Conversation"
#define TXT_MENU_HIDE	"Hide\tAlt-X"
#define TXT_MENU_CLOSE	"Close\tAlt-C"
#define TXT_MENU_ABOUT	"&About"

BEGIN_EVENT_TABLE(ImFrame, wxFrame)
   	EVT_MENU(MENU_SAVE, ImFrame::OnSave)
	EVT_MENU(MENU_SAVE, ImFrame::OnMenuClose)
    	EVT_MENU(MENU_HIDE, ImFrame::OnHide)
    	EVT_MENU(MENU_ABOUT, ImFrame::OnAbout)
	EVT_TEXT_ENTER(TEXT_OUT, ImFrame::OnSendIm)
	EVT_BUTTON(ID_SEND_BTN,ImFrame::OnButtons)
	EVT_BUTTON(ID_HIDE_BTN,ImFrame::OnButtons)
	EVT_BUTTON(ID_CLOSE_BTN,ImFrame::OnButtons)
	EVT_CLOSE(ImFrame::OnClose)
END_EVENT_TABLE()

ImFrame::ImFrame(Buddy* buddy)
       : wxFrame(0, -1, buddy->Get(Buddy::DATA_ALIAS) + " - Turnstile IM Window",wxDefaultPosition, wxDefaultSize),
	  	buddy_(buddy)
{
#ifdef WIN32
	SetBackgroundColour(wxColour(180,180,180));
#endif

	// set the frame size from last saved config
	SetSize (DetermineFrameSize(IM_WINDOW));

    	// set an icon
    	SetIcon(wxIcon(imframe));

     	// create main window menus
	wxMenu* buddy_menu = new wxMenu;
	wxMenu* help_menu = new wxMenu;

	buddy_menu->Append(MENU_SAVE, TXT_MENU_SAVE);
	buddy_menu->Append(MENU_LOAD, TXT_MENU_LOAD);
	buddy_menu->AppendSeparator();
    	buddy_menu->Append(MENU_HIDE, TXT_MENU_HIDE);
	buddy_menu->Append(MENU_CLOSE, TXT_MENU_CLOSE);

	help_menu->Append(MENU_ABOUT, TXT_MENU_ABOUT);

    	wxMenuBar* menu_bar = new wxMenuBar;
    	menu_bar->Append(buddy_menu,"&Buddy");
    	menu_bar->Append(help_menu, "&Help");
    	SetMenuBar(menu_bar);

	// main sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	splitter_ = new SplitterWindow(this);
	sizer->Add(splitter_, 1, wxGROW, 0 );

    	// create text boxes
  	textIncomming_  = new wxTextCtrl(splitter_, TEXT_IN,
                           "",
                           wxDefaultPosition, wxDefaultSize,
                           wxTE_MULTILINE|wxTE_WORDWRAP|wxTE_LEFT|wxTE_READONLY|wxTE_AUTO_URL);

	textOutgoing_  = new wxTextCtrl(splitter_, TEXT_OUT,
                           "",
                           	wxDefaultPosition, wxDefaultSize,
                		wxTE_MULTILINE|wxTE_WORDWRAP|wxTE_LEFT|
#ifndef WIN32
				wxTE_PROCESS_ENTER|
#endif
				wxTE_AUTO_URL);
			   
	wxSize size = GetClientSize();
	splitter_->SplitHorizontally(textIncomming_, textOutgoing_, 2*size.y/3);

	// sizer for bottom buttons
	wxBoxSizer* btnSizer = new wxBoxSizer (wxHORIZONTAL);

    	wxBitmapButton* sendButton = new wxBitmapButton(this, ID_SEND_BTN, wxBitmap(imframe_send));
	sendButton->SetToolTip("Send instant message.");
 	btnSizer->Add(sendButton, 0, wxALL, 5);
	wxBitmapButton* hideButton = new wxBitmapButton(this, ID_HIDE_BTN, wxBitmap(imframe_hide));
	hideButton->SetToolTip("Hide this window and save conversation.");
 	btnSizer->Add(hideButton, 0, wxALL, 5);
	wxBitmapButton* closeButton = new wxBitmapButton(this, ID_CLOSE_BTN, wxBitmap(imframe_close));
	closeButton->SetToolTip("Close window.");
 	btnSizer->Add(closeButton, 0, wxALL, 5);
	
	sizer->Add(btnSizer,0);

	SetSizer(sizer);

	// create status bar
	CreateStatusBar(1);

	SetStatusText("Idle", 0);

	//but1->SetFocus();
  	//but1->SetDefault();
}

void
ImFrame::OnButtons(wxCommandEvent& event)
{
	switch (event.GetId())
    	{
	case ID_SEND_BTN:
	{
		SendIm();
		break;
	}
	case ID_HIDE_BTN:
	{
		Show(FALSE);
		break;
	}
	case ID_CLOSE_BTN:
	{
		Close();
		break;
	}
	}
}

void
ImFrame::OnSendIm(wxCommandEvent& event)
{
	SendIm();
}

void
ImFrame::OnSave(wxCommandEvent& WXUNUSED(event))
{
	wxString file = wxFileSelector("Save Conversation to File", "","", "", "*.*", wxSAVE, this ,-1, -1);

	if (file.Length() != 0)
		textIncomming_->SaveFile(file);
}

void
ImFrame::OnMenuClose(wxCommandEvent& WXUNUSED(event))
{
	Close();
}

void
ImFrame::OnHide(wxCommandEvent& WXUNUSED(event))
{
    	Show(FALSE);
}

void
ImFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
     	AboutDialog it(this);
   	it.ShowModal();
}

void
ImFrame::SendIm()
{
	wxString textSend = textOutgoing_->GetValue();

	if (textSend.Length() == 0)
		return;

	buddy_->SendIm(textSend);

	SetStatusText("Sending IM ...", 0);
}

void ImFrame::Sent()
{
	wxString textSend = textOutgoing_->GetValue();

	wxDateTime now = wxDateTime::Now();

	textIncomming_->SetDefaultStyle(wxTextAttr(*wxBLUE));
	textIncomming_->AppendText(wxString().Format("%s %s: %s\n",now.Format("%D %T"/*, wxDateTime::EST*/).c_str(),Prefs::Instance()->Get(Prefs::ALIAS).c_str(),textSend.c_str()));

	textIncomming_->SetScrollPos(wxVERTICAL, textIncomming_->GetScrollRange(wxVERTICAL),TRUE);

	textOutgoing_->Clear();

	SetStatusText("Message successfully sent.", 0);
}

void
ImFrame::ReceiveIm(int type, wxString text)
{
	wxDateTime now = wxDateTime::Now();

	textIncomming_->SetDefaultStyle(wxTextAttr(*wxRED));
	//textIncomming_->AppendText(now.Format("%D %T", wxDateTime::EST) + " " + buddy_->Get(Buddy::DATA_ALIAS) + ": " + text + "\n");
	textIncomming_->AppendText(wxString().Format("%s %s: %s\n",now.Format("%D %T"/*, wxDateTime::EST*/).c_str(),buddy_->Get(Buddy::DATA_ALIAS).c_str(),text.c_str()));
	SetStatusText("Message received.", 0);

	textIncomming_->SetScrollPos(wxVERTICAL, textIncomming_->GetScrollRange(wxVERTICAL),TRUE);

	//::wxBell();
}

void ImFrame::ConnectionFailed()
{
	SetStatusText("Connection to buddy failed.", 0);
}

void ImFrame::ConnectionLost()
{
	SetStatusText("Not connected to buddy.", 0);
}

void ImFrame::ConnectionMade()
{
	SetStatusText("Connected to buddy.", 0);
}

void
ImFrame::OnClose(wxCloseEvent& WXUNUSED(event))
{
	buddy_->OnCloseWindow();
	StoreFrameSize(GetRect(),IM_WINDOW);
	Destroy();
}

ImFrame::~ImFrame()
{;}


BEGIN_EVENT_TABLE(SplitterWindow, wxSplitterWindow)
    	EVT_SPLITTER_DCLICK(-1, SplitterWindow::OnDClick)
END_EVENT_TABLE()

SplitterWindow::SplitterWindow(wxFrame* parent)
                : wxSplitterWindow(parent, -1,
                          wxDefaultPosition, wxDefaultSize,
                          wxSP_3D | wxSP_LIVE_UPDATE | wxCLIP_CHILDREN)
{
    	parent_ = parent;
}

void
SplitterWindow::OnDClick(wxSplitterEvent& event)
{
	/*
	The whole point for me making this class it to stop
	this damn event that allows the user to delete one of the
	splits.
	*/
    	event.Veto();
}

