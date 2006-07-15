/* $Id: mainframe.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file mainframe.cpp Buddylist Window Implimentation
*/
#include "mainframe.h"
#include "buddytreectrl.h"
#include "app.h"
#include "frameutils.h"
#include "fileframe.h"
#include "debugframe.h"
#include "prefframe.h"
#include "aboutdialog.h"
#include "network.h"
#include "utils.h"
#include "buddy.h"
#include "findbuddyframe.h"

#include <wx/tooltip.h>
#include <wx/progdlg.h>
#include <wx/statline.h>

#include <vector>
using namespace std;

#include "../pixmaps/menu_find.xpm"
#include "../pixmaps/menu_sign.xpm"
#include "../pixmaps/menu_exit.xpm"
#include "../pixmaps/menu_about.xpm"
#include "../pixmaps/menu_debug.xpm"
#include "../pixmaps/menu_findbuddy.xpm"
#include "../pixmaps/menu_addbuddy.xpm"
#include "../pixmaps/menu_addgroup.xpm"
#include "../pixmaps/menu_prefs.xpm"
#include "../pixmaps/mainframe_findfile.xpm"
#include "../pixmaps/mainframe_findbuddy.xpm"
#include "../pixmaps/online.xpm"
#include "../pixmaps/offline.xpm"

#define CONFIG_MAIN_WINDOW 		"mainwindow"

#define TXT_MENU_ADDBUDDY 		"Add &Buddy"
#define TXT_MENU_FINDBUDDY 		"Find Buddies"
#define TXT_MENU_ADDGROUP		"Add &Group"
#define TXT_MENU_SIGNONOFF_ON		"Sign On"
#define TXT_MENU_SIGNONOFF_OFF		"Sign Off"
#define TXT_MENU_EXIT			"E&xit\tAlt-X"
#define TXT_MENU_FINDFILES		"&Find Files"
#define TXT_MENU_OPENDEBUG		"&Debug Window"
#define TXT_MENU_PREFERENCES		"&Preferences"
#define TXT_MENU_ABOUT			"&About"
#define TXT_WINDOW	"Turnstile"

/** Main frame ids.
*/
enum
{
	MENU_ADDBUDDY=7501,
	MENU_FINDBUDDY,
	MENU_ADDGROUP,
	MENU_SIGNONOFF,
	MENU_EXIT,
	MENU_FINDFILES,
	MENU_OPENDEBUG,
	MENU_PREFERENCES,
	MENU_ABOUT,
	ID_BUDDY_BTN,
	ID_FILE_BTN,
	ID_SIGNON_TIMER,
	STATUS_IMAGE
};

#define SIGNON_START		0
#define SIGNON_REFRESHSHARE	10
#define SIGNON_ENCRYPTION	20
#define SIGNON_BOOTSTRAPPING	30
#define SIGNON_WAITING		40	// skip this if not seed node
#define SIGNON_END		50

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
   	EVT_TIMER(ID_SIGNON_TIMER, MainFrame::OnSignonTimer)
	EVT_MENU(MENU_FINDBUDDY, MainFrame::OnFindBuddy)
   	EVT_MENU(MENU_ADDBUDDY, MainFrame::OnAddBuddy)
   	EVT_MENU(MENU_ADDGROUP, MainFrame::OnAddGroup)
   	EVT_MENU(MENU_SIGNONOFF, MainFrame::OnSignOnOff)
	EVT_BUTTON(STATUS_IMAGE, MainFrame::OnSignOnOff)
   	EVT_MENU(MENU_EXIT, MainFrame::OnExit)
   	EVT_MENU(MENU_FINDFILES, MainFrame::OnShowFileFrame)
   	EVT_MENU(MENU_OPENDEBUG, MainFrame::OnShowDebugFrame)
   	EVT_MENU(MENU_PREFERENCES, MainFrame::OnShowPrefFrame)
   	EVT_MENU(MENU_ABOUT, MainFrame::OnAbout)
	EVT_CLOSE(MainFrame::OnClose)
	EVT_BUTTON(ID_BUDDY_BTN,MainFrame::OnButtons)
	EVT_BUTTON(ID_FILE_BTN,MainFrame::OnButtons)
END_EVENT_TABLE()

void
MainFrame::OnButtons(wxCommandEvent& event)
{
	switch (event.GetId())
    	{
	case ID_FILE_BTN:
	{
		if (fileFrame_ == 0)
		{
			fileFrame_ = new FileFrame(this);
			fileFrame_->Show(TRUE);
		}
		else
		{
			fileFrame_->Raise();
		}

		break;
	}
	case ID_BUDDY_BTN:
	{
		if (findBuddyFrame_ == 0)
		{
			findBuddyFrame_ = new FindBuddyFrame(this);
			findBuddyFrame_->Show(TRUE);
		}
		else
		{
			findBuddyFrame_->Raise();
		}

		break;
	}
	}
}

MainFrame::MainFrame()
       : wxFrame(0, -1, TXT_WINDOW, wxDefaultPosition, wxDefaultSize/*,wxDEFAULT_FRAME_STYLE|wxFULL_REPAINT_ON_RESIZE|wxWS_EX_PROCESS_UI_UPDATES*/),
       		treeCtrl_(0),debugFrame_(0),prefFrame_(0),fileFrame_(0),findBuddyFrame_(0)
{
#ifdef WIN32
	SetBackgroundColour(wxColour(180,180,180));
#endif

	// enable tooltips
	wxToolTip::Enable(TRUE);

	// create the menu
   	CreateMenu();


	// main sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	/*
	Create tree control.
	*/
	const int style = wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT | wxTR_FULL_ROW_HIGHLIGHT | wxTR_NO_LINES;
			//wxTR_SINGLE | wxTR_HIDE_ROOT | wxTR_FULL_ROW_HIGHLIGHT |
			//wxTR_NO_LINES | wxTR_HAS_BUTTONS | wxTR_TWIST_BUTTONS;

	treeCtrl_ = new BuddyTreeCtrl(this, ID_TREE_CTRL,wxDefaultPosition, wxDefaultSize,style);
	sizer->Add(treeCtrl_, 1, wxGROW, 0);

	/*
	Create bottom buttons.
	*/

	// sizer for bottom buttons
	wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);

   	statusImage_ = new wxStaticBitmap(this, STATUS_IMAGE, wxBitmap(offline));
	statusImage_->SetToolTip("Offline");
	btnSizer->Add(statusImage_, 0, wxALL, 10);

	btnSizer->Add(new wxStaticLine(this, -1, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxGROW|wxALL, 5);

	wxBitmapButton* findFileBtn = new wxBitmapButton(this, ID_FILE_BTN, wxBitmap(mainframe_findfile));
	findFileBtn->SetToolTip("Find files on the network.");
 	btnSizer->Add(findFileBtn, 0, /*wxALIGN_CENTRE|wxALIGN_CENTER_HORIZONTAL|*/wxALL, 5);

	wxBitmapButton* findBuddyBtn = new wxBitmapButton(this, ID_BUDDY_BTN, wxBitmap(mainframe_findbuddy));
	findBuddyBtn->SetToolTip("Find buddies on the network.");
 	btnSizer->Add(findBuddyBtn, 0, /*wxALIGN_CENTRE|wxALIGN_CENTER_HORIZONTAL|*/wxALL, 5);

	sizer->Add(btnSizer,0);
	SetSizer(sizer);


	/*
	Handle network events.
	*/
	wxEvtHandler::Connect(ID_MAINFRAME, wxEVT_COMMAND_MENU_SELECTED,
				(wxObjectEventFunction)
                (wxEventFunction)
                (wxCommandEventFunction)&MainFrame::OnNetworkEvent,
				(wxObject*) NULL);

	/*
	Set the frame size from last saved config.
	*/
	SetSize(DetermineFrameSize(CONFIG_MAIN_WINDOW));
}

void
MainFrame::AutoStart()
{
	DoSignon();
}

void
MainFrame::KillSignonDialog()
{
	delete signonTimer_;
	//delete signonDialog_;
	signonDialog_->Destroy();
}

void
MainFrame::OnSignonTimer(wxTimerEvent& event)
{
	LogDebug("*");

	if (Network::Instance()->GetStatus(Network::STATUS_ALIVE))
	{
		if (!signonDialog_->Update(SIGNON_END, "Online!"))
		{
			KillSignonDialog();
			Network::Instance()->KillNetwork();
			network_menu->FindItem(MENU_SIGNONOFF)->SetText(TXT_MENU_SIGNONOFF_ON);
		}
		else
		{
			KillSignonDialog();
		}
	}
	else if (Network::Instance()->GetStatus(Network::STATUS_BOOTSTRAPPING))
	{
		if (!signonDialog_->Update(SIGNON_BOOTSTRAPPING, "Bootstrapping to network ..."))
		{
			KillSignonDialog();
			Network::Instance()->KillNetwork();
			network_menu->FindItem(MENU_SIGNONOFF)->SetText(TXT_MENU_SIGNONOFF_ON);
		}
	}
	else if (Network::Instance()->GetStatus(Network::STATUS_SEEDING))
	{
		if (!signonDialog_->Update(SIGNON_WAITING, "Seeding network (waiting for connection) ..."))
		{
			KillSignonDialog();
			Network::Instance()->KillNetwork();
			network_menu->FindItem(MENU_SIGNONOFF)->SetText(TXT_MENU_SIGNONOFF_ON);
		}
	}
	else
	{
		KillSignonDialog();
		Network::Instance()->KillNetwork();
		network_menu->FindItem(MENU_SIGNONOFF)->SetText(TXT_MENU_SIGNONOFF_ON);
	}
}

void
MainFrame::OnNetworkEvent(wxCommandEvent& WXUNUSED(event))
{
	if (Network::Instance()->GetStatus(Network::STATUS_ALIVE))
	{
		statusImage_->SetBitmap(wxBitmap(online));
		statusImage_->SetToolTip("Online");
	}
	else
	{
		statusImage_->SetBitmap(wxBitmap(offline));
		statusImage_->SetToolTip("Offline");
		network_menu->FindItem(MENU_SIGNONOFF)->SetText(TXT_MENU_SIGNONOFF_ON);

		if (Network::Instance()->GetStatus(Network::STATUS_ERROR))
		{
			wxString msg = Network::Instance()->ErrorMsg();
			if (msg != "")
			{
				wxMessageDialog diag(0,msg,"Turnstile Network Error",wxOK|wxICON_ERROR);
				diag.ShowModal();
			}
		}
	}
}

void
MainFrame::OnAddBuddy(wxCommandEvent& WXUNUSED(event))
{
	vector<wxString> groups;
	treeCtrl_->GetGroupArray(groups);
	if (groups.size() == 0)
	{
		wxMessageDialog(this, "You must first add a group!", "Add Buddy",wxOK).ShowModal();
		return;
	}

	wxSingleChoiceDialog dialog((wxWindow*)0,"Select Group",
					"Add Buddy to Group",
					groups.size(), &groups[0],0,wxCANCEL);

	if (dialog.ShowModal() == wxID_OK)
	{
		wxString group = dialog.GetStringSelection();
		wxString buddyalias = wxGetTextFromUser("Buddy Alias","Add Buddy","");

		if (buddyalias != "")
		{
			Buddy* buddy = new Buddy();
			buddy->Set(Buddy::DATA_ALIAS,buddyalias);
			treeCtrl_->AddBuddy(buddy,group,true);
		}
	}
}

void
MainFrame::AddBuddy(Buddy* buddy, wxString group)
{
	//buddy->Set(Buddy::DATA_ALIAS,buddyalias);
	treeCtrl_->AddBuddy(buddy,group,true);
}

void
MainFrame::OnAddGroup(wxCommandEvent& WXUNUSED(event))
{
	wxString group = wxGetTextFromUser(
		"Enter the name of a new group:",
		"Add Group",
		"");

	if (group != "")
		treeCtrl_->AddGroup(group);
}

void
MainFrame::DoSignon()
{
	signonDialog_ = new wxProgressDialog(
				"Turnstile Signon",
				"Initializing ...",
				SIGNON_END,
				0,
				wxPD_CAN_ABORT |
				wxPD_APP_MODAL |
				wxPD_AUTO_HIDE |
				wxPD_ELAPSED_TIME);

	Network::Instance()->Start(this,ID_MAINFRAME,Network::Instance()->seed_);

	signonTimer_ = new wxTimer(this, ID_SIGNON_TIMER);
	signonTimer_->Start(1000,wxTIMER_CONTINUOUS);

	network_menu->FindItem(MENU_SIGNONOFF)->SetText(TXT_MENU_SIGNONOFF_ON);
}

void
MainFrame::OnSignOnOff(wxCommandEvent& WXUNUSED(event))
{
	// if it's running then stop it
	if (!Network::Instance()->GetStatus(Network::STATUS_ALIVE) &&
		!Network::Instance()->GetStatus(Network::STATUS_SEEDING))
	{
		DoSignon();
	}
	else
	{
		Network::Instance()->KillNetwork();
		network_menu->FindItem(MENU_SIGNONOFF)->SetText(TXT_MENU_SIGNONOFF_OFF);
	}
}

/*void
MainFrame::UpdateStatus()
{
	if (Network::Instance()->GetStatus() == Network::STATUS_ALIVE)
	{
		wxString label = wxString().Format("Online [Downloads:%d  Uploads:%d]",Network::Instance()->DownloadSize(),Network::Instance()->UploadSize());
		statusTxt_->SetLabel(label);
		network_menu->FindItem(MENU_SIGNONOFF)->SetText(TXT_MENU_SIGNONOFF_OFF);
	}
	else if (Network::Instance()->GetStatus() == Network::STATUS_INTRY)
	{
		statusTxt_->SetLabel("Connecting to network ...");
		network_menu->FindItem(MENU_SIGNONOFF)->SetText(TXT_MENU_SIGNONOFF_OFF);
	}
	else if (Network::Instance()->GetStatus() == Network::STATUS_STOPPED)
	{
		statusTxt_->SetLabel("Network Stopped.");
		network_menu->FindItem(MENU_SIGNONOFF)->SetText(TXT_MENU_SIGNONOFF_ON);
	}
	else if (Network::Instance()->GetStatus() == Network::STATUS_DEAD)
	{
		statusTxt_->SetLabel("Network Failed (" +  Network::Instance()->Error() + ")");
		network_menu->FindItem(MENU_SIGNONOFF)->SetText(TXT_MENU_SIGNONOFF_ON);
	}
	else if (Network::Instance()->GetStatus() == Network::STATUS_SEEDING)
	{
		statusTxt_->SetLabel("Waiting for connection...");
		network_menu->FindItem(MENU_SIGNONOFF)->SetText(TXT_MENU_SIGNONOFF_ON);
	}
	else
	{
		statusTxt_->SetLabel("*");
	}
}*/

void
MainFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
	Close(TRUE);
}

void
MainFrame::OnClose(wxCloseEvent& WXUNUSED(event))
{
	// stop the network
	Network::Instance()->Stop();

	// save frame state
	StoreFrameSize(GetRect(),CONFIG_MAIN_WINDOW);

	// destroy this window
	Destroy();
}

void
MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
  	AboutDialog it(this);
   	it.ShowModal();
}

void
MainFrame::OnShowDebugFrame(wxCommandEvent& WXUNUSED(event))
{
	if (debugFrame_ == 0)
	{
		debugFrame_ = new DebugFrame(this);
		debugFrame_->Show(TRUE);
	}
	else
	{
		debugFrame_->Raise();
	}
}

void
MainFrame::OnFindBuddy(wxCommandEvent& WXUNUSED(event))
{
	if (findBuddyFrame_ == 0)
	{
		findBuddyFrame_ = new FindBuddyFrame(this);
		findBuddyFrame_->Show(TRUE);
	}
	else
	{
		findBuddyFrame_->Raise();
	}
}

void
MainFrame::OnShowFileFrame(wxCommandEvent& WXUNUSED(event))
{
	if (fileFrame_ == 0)
	{
		fileFrame_ = new FileFrame(this);
		fileFrame_->Show(TRUE);
	}
	else
	{
		fileFrame_->Raise();
	}
}

void
MainFrame::ShowPrefFrame()
{
	if (prefFrame_ == 0)
	{
		prefFrame_ = new PrefFrame(this);
		prefFrame_->Show(TRUE);
	}
	else
	{
		prefFrame_->Raise();
	}
}

void
MainFrame::OnShowPrefFrame(wxCommandEvent& WXUNUSED(event))
{
	ShowPrefFrame();
}

void
MainFrame::OnCloseDebugFrame()
{
	debugFrame_ = 0;
}

void
MainFrame::OnCloseFindBuddyFrame()
{
	findBuddyFrame_ = 0;
}

void
MainFrame::OnClosePrefFrame()
{
	prefFrame_ = 0;
}

void 
MainFrame::OnCloseFileFrame()
{
	fileFrame_ = 0;
}

void MainFrame::CreateMenu()
{
	//wxUpdateUIEvent::SetMode(wxUPDATE_UI_PROCESS_ALL);

	// create main window menus
	network_menu = new wxMenu;
    	options_menu = new wxMenu;
	help_menu = new wxMenu;

	wxMenuItem* item;

	item = new wxMenuItem(network_menu, MENU_FINDFILES, TXT_MENU_FINDFILES);
	item->SetBitmap(menu_find);
	network_menu->Append(item);

	item = new wxMenuItem(options_menu, MENU_FINDBUDDY, TXT_MENU_FINDBUDDY);
	item->SetBitmap(menu_findbuddy);
	network_menu->Append(item);

	network_menu->AppendSeparator();

	item = new wxMenuItem(network_menu, MENU_SIGNONOFF, TXT_MENU_SIGNONOFF_ON);
	item->SetBitmap(menu_sign);
	network_menu->Append(item);

	item = new wxMenuItem(network_menu, MENU_EXIT, TXT_MENU_EXIT);
	item->SetBitmap(menu_exit);
	network_menu->Append(item);

	item = new wxMenuItem(options_menu, MENU_PREFERENCES, TXT_MENU_PREFERENCES);
	item->SetBitmap(menu_prefs);
	options_menu->Append(item);

	options_menu->AppendSeparator();

	item = new wxMenuItem(options_menu, MENU_ADDBUDDY, TXT_MENU_ADDBUDDY);
	item->SetBitmap(menu_addbuddy);
	options_menu->Append(item);

	item = new wxMenuItem(options_menu, MENU_ADDGROUP, TXT_MENU_ADDGROUP);
	item->SetBitmap(menu_addgroup);
	options_menu->Append(item);

	options_menu->AppendSeparator();

	item = new wxMenuItem(options_menu, MENU_OPENDEBUG, TXT_MENU_OPENDEBUG);
	item->SetBitmap(menu_debug);
	options_menu->Append(item);

	item = new wxMenuItem(help_menu, MENU_ABOUT, TXT_MENU_ABOUT);
	item->SetBitmap(menu_about);
	help_menu->Append(item);

    	menu_bar = new wxMenuBar;
    	menu_bar->Append(network_menu, "&Network");
    	menu_bar->Append(options_menu, "&Options");
    	menu_bar->Append(help_menu, "&Help");
    	SetMenuBar(menu_bar);
}

MainFrame::~MainFrame()
{
    	// TODO close all im windows
	
	if (prefFrame_ != 0)
	{
		prefFrame_->Close(TRUE);
	}
	
	if (fileFrame_ != 0)
	{
		fileFrame_->Close(TRUE);
	}
	
	if (debugFrame_ != 0)
	{
		debugFrame_->Close(TRUE);
	}

	if (findBuddyFrame_ != 0)
	{
		findBuddyFrame_->Close(TRUE);
	}
}


//void MainFrame::DoSort(bool reverse)
//{
//    wxTreeItemId item = treeCtrl_->GetSelection();

//    CHECK_ITEM( item );

//    treeCtrl_->DoSortChildren(item, reverse);
//}


