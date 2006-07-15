/* $Id: prefframe.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file prefframe.cpp
@brief Preferences Window Implimentation
*/
//#include "app.h"
#include "prefframe.h"
#include "mainframe.h"
#include "frameutils.h"
#include "prefs.h"
#include "hostcache.h"
#include "typeconv.h"
#include "utils.h"

#include "../pixmaps/prefframe.xpm"
#include "../pixmaps/prefframe_user.xpm"
#include "../pixmaps/prefframe_connection.xpm"
#include "../pixmaps/prefframe_filetransfer.xpm"
#include "../pixmaps/prefframe_buddylist.xpm"

#define CONFIG_PREF_WINDOW wxT("prefframe")
#define ICON_DIMENS 32

enum	{	
		ID_NOTEBOOK=8030,
		ID_BTN_CLOSE,
		ID_BTN_CANCEL,
		ID_SHARING_BTN_CHOOSE
	};

BEGIN_EVENT_TABLE(PrefNotebook, wxNotebook)
    EVT_NOTEBOOK_PAGE_CHANGED(ID_NOTEBOOK, PrefNotebook::OnNotebook)
    EVT_NOTEBOOK_PAGE_CHANGING(ID_NOTEBOOK, PrefNotebook::OnNotebook)
END_EVENT_TABLE()

void
PrefNotebook::OnNotebook(wxNotebookEvent& event)
{
    wxEventType eventType = event.GetEventType();

    if (eventType == wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED)
    {
        //
    }
    else if (eventType == wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING)
    {
        if (!Save(event.GetOldSelection()))
		event.Veto();
    }
    event.Skip();
}

PrefNotebook::PrefNotebook(wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style)
    : wxNotebook(parent, id, pos, size, style)
{;}

wxPanel* PrefNotebook::CreatePageConnection()
{
    	wxPanel* panel = new wxPanel(this);

	portTxt_ = new wxTextCtrl(panel, -1, Prefs::Instance()->Get(Prefs::PORT));
	portTxt_->SetToolTip("The port the local server will run on allowing other nodes to contact you.  If you are behind a firewall you must forward this port to your local machine.");

	wxBoxSizer* sizerPanel = new wxBoxSizer(wxVERTICAL);

	sizerPanel->Add(new wxStaticText(panel, -1, _T("Server Port")), 0, wxALL, 5);
	sizerPanel->Add(portTxt_, 0, wxALL, 5);

	generateCheck_ = new wxCheckBox(panel, -1, "Generate New RSA Keys on Every Signon");
	generateCheck_->SetToolTip("Generating new RSA keys takes longer to signon, but is more secure incase your current local keys have been comprimised.");
	sizerPanel->Add(generateCheck_,0,wxALL,5);

	if(Prefs::Instance()->Get(Prefs::REGENRSA) == "1")
    		generateCheck_->SetValue(TRUE);
  	else
    		generateCheck_->SetValue(FALSE);

	useFirewallCheck_ = new wxCheckBox(panel, -1, "I am Behind a Firewall");
	useFirewallCheck_->SetToolTip("Check this box if you are behind a firewall.  Remember, you will need to forward the port you chose in the port box on your firewall for the network to work.");
	sizerPanel->Add(useFirewallCheck_,0,wxALL,5);

	if(Prefs::Instance()->Get(Prefs::USEFIREWALL) == "1")
    		useFirewallCheck_->SetValue(TRUE);
  	else
    		useFirewallCheck_->SetValue(FALSE);


	wrkgrpKeyTxt_ = new wxTextCtrl(panel, -1, Prefs::Instance()->Get(Prefs::WRKGRPKEY),wxDefaultPosition,wxSize(400,25));
	wrkgrpKeyTxt_->SetToolTip("If not blank, only nodes with the same key will be able to join the same network.");
	sizerPanel->Add(new wxStaticText(panel, -1, _T("Workgroup Key (minimum 20 characters, leave blank if not in use)")), 0, wxALL, 5);
	sizerPanel->Add(wrkgrpKeyTxt_, 0, wxALL, 5);

	sizerPanel->Add(new wxStaticText(panel, -1, _T("Current Hostcache")), 0, wxALL, 5);

	wxListBox* hostcacheList = new wxListBox( panel, -1,
                                     wxDefaultPosition, /*wxDefaultSize*/wxSize(120,120),
                                     Prefs::Instance()->GetHostCache()->Size(),
				     Prefs::Instance()->GetHostCache()->GetArray(), wxLB_SORT );

	// disable the box, we arn't doing anything with it yet
     	hostcacheList->Disable();

	sizerPanel->Add(hostcacheList, 0, wxALL, 5);

	panel->SetSizer(sizerPanel);

    	return panel;
}

wxPanel* PrefNotebook::CreatePageUser()
{
    	wxPanel* panel = new wxPanel(this);

	aliasTxt_ = 	new wxTextCtrl(panel, -1, Prefs::Instance()->Get(Prefs::ALIAS),wxDefaultPosition,wxSize(100,25),wxWS_EX_BLOCK_EVENTS);
	publicKeyTxt_ = new wxTextCtrl(panel, -1, Prefs::Instance()->Get(Prefs::PUBLICKEY),wxDefaultPosition,wxSize(400,25));

	wxBoxSizer* sizerPanel = new wxBoxSizer(wxVERTICAL);

	aliasTxt_->SetToolTip("This does not have to be unique, however this is what buddies will see on their buddy list.");
	publicKeyTxt_->SetToolTip("This is what people use to uniqely identify you.  If you change it, your buddies will no longer be able to uniqley identify you.  If somebody else gets it, they can steal your identity.");


	sizerPanel->Add(new wxStaticText(panel, -1, _T("User Alias")), 0, wxALL, 5);
	sizerPanel->Add(aliasTxt_, 0, wxALL, 5);

	sizerPanel->Add(new wxStaticText(panel, -1, "Private Identification Key  (minimum 10 characters)"), 0, wxALL, 5);
	sizerPanel->Add(publicKeyTxt_, 0, wxALL, 5);

	panel->SetSizer(sizerPanel);

    	return panel;
}

wxPanel* PrefNotebook::CreatePageShare()
{
    	wxPanel* panel = new wxPanel(this);

	shareFolderTxt_ = new wxTextCtrl(panel, -1, Prefs::Instance()->Get(Prefs::SHAREDIR),wxDefaultPosition,wxSize(400,25),wxTE_READONLY);
	wxButton* shareFolderBtn = new wxButton(panel,ID_SHARING_BTN_CHOOSE,"Choose Share Directory" );

	shareFolderTxt_->SetToolTip("This is the directory that other users will be able to download files\nfrom as well as the directory where the files you download will be saved.");

	wxBoxSizer *sizerPanel = new wxBoxSizer(wxVERTICAL);

	sizerPanel->Add(new wxStaticText(panel, -1, _T("Share Directory")), 0, wxALL, 5);

    	sizerPanel->Add(shareFolderTxt_, 0, wxALL, 5);
	sizerPanel->Add(shareFolderBtn, 0, wxALL, 5);

  	maxUploadsSpinCtrl_ = new wxSpinCtrl(panel, -1, _T(""), wxDefaultPosition, wxSize(80, -1));
    	maxUploadsSpinCtrl_->SetRange(0,1000);
    	maxUploadsSpinCtrl_->SetValue(convert<int>(Prefs::Instance()->Get(Prefs::MAXUPLOADS)));

	sizerPanel->Add(new wxStaticText(panel, -1, _T("Max Concurrent Uploads")), 0, wxALL, 5);
	sizerPanel->Add(maxUploadsSpinCtrl_, 0, wxALL, 5);

	maxDownloadsSpinCtrl_ = new wxSpinCtrl(panel, -1, _T(""), wxDefaultPosition, wxSize(80, -1));
    	maxDownloadsSpinCtrl_->SetRange(1,1000);
    	maxDownloadsSpinCtrl_->SetValue(convert<int>(Prefs::Instance()->Get(Prefs::MAXDOWNLOADS)));

	sizerPanel->Add(new wxStaticText(panel, -1, _T("Max Concurrent Downloads")), 0, wxALL, 5);
	sizerPanel->Add(maxDownloadsSpinCtrl_, 0, wxALL, 5);

	panel->SetSizer(sizerPanel);

    	return panel;
}

wxPanel* PrefNotebook::CreatePageBuddylist()
{
    	wxPanel* panel = new wxPanel(this);

	wxBoxSizer *sizerPanel = new wxBoxSizer(wxVERTICAL);

	showOfflineCheck_ = new wxCheckBox(panel, -1, "Show Offline Buddies");
	showOfflineCheck_->SetToolTip("Show buddies in the buddy list when they are offline.");
	sizerPanel->Add(showOfflineCheck_,0,wxALL,5);

	if(Prefs::Instance()->Get(Prefs::SHOWOFFLINE) == "1")
    		showOfflineCheck_->SetValue(TRUE);
  	else
    		showOfflineCheck_->SetValue(FALSE);

	panel->SetSizer(sizerPanel);

	return panel;
}

void PrefNotebook::CreatePages()
{
	wxPanel* panel = 0;

	panel = CreatePageUser();
	AddPage(panel, "User Prefs", FALSE, 0);

	panel = CreatePageConnection();
	AddPage(panel, "Network", FALSE, 1);

	panel = CreatePageShare();
	AddPage(panel, "File Sharing", FALSE, 2);

	panel = CreatePageBuddylist();
	AddPage(panel, "Buddy List & IM", FALSE, 3);

	SetSelection(0);
}

bool
PrefNotebook::Save(int id)
{
	switch (id)
	{
	case 0:
	{
		//wxString tmp;
		/*
		Save user preferences.
		*/
		if (aliasTxt_->GetValue().size() == 0)
		{
			wxMessageDialog(m_parent,"User alias may not be blank.","Error",wxOK).ShowModal();
			return FALSE;
		}

		if (publicKeyTxt_->GetValue().size() < 10)
		{
			wxMessageDialog* diag = new wxMessageDialog(m_parent,"Public key must be atleast 10 characters.","Error",wxOK);
			diag->ShowModal();
			return FALSE;
		}

		Prefs::Instance()->Set(Prefs::ALIAS,aliasTxt_->GetValue());

		if (Prefs::Instance()->Set(Prefs::PUBLICKEY,publicKeyTxt_->GetValue()))
		{
			wxMessageDialog* diag = new wxMessageDialog(m_parent,"You must reconnect to the network for changes to take effect.","Warning",wxOK);
			diag->ShowModal();
		}

		break;
	}
	case 1:
	{
		wxString tmp = wrkgrpKeyTxt_->GetValue();
		if (tmp.size() < 20 &&
			tmp != wxEmptyString)
		{
			wxMessageDialog* diag = new wxMessageDialog(m_parent,"Workgroup key must be atleast 20 characters.","Error",wxOK);
			diag->ShowModal();
			//LogError("need wrkgrp key");
			return FALSE;
		}

		Prefs::Instance()->Set(Prefs::WRKGRPKEY,wrkgrpKeyTxt_->GetValue());

		if (Prefs::Instance()->Set(Prefs::ALIAS,aliasTxt_->GetValue()))
		{
			wxMessageDialog* diag = new wxMessageDialog(m_parent,"You must reconnect to the network for changes to take effect.","Warning",wxOK);
			diag->ShowModal();
		}


		Prefs::Instance()->Set(Prefs::PORT,portTxt_->GetValue());

		if (generateCheck_->IsChecked())
			Prefs::Instance()->Set(Prefs::REGENRSA,"1");
		else
			Prefs::Instance()->Set(Prefs::REGENRSA,"0");

		if (useFirewallCheck_->IsChecked())
			Prefs::Instance()->Set(Prefs::USEFIREWALL,"1");
		else
			Prefs::Instance()->Set(Prefs::USEFIREWALL,"0");

		break;
	}
	case 2:
	{
		wxString tmp;

		tmp = shareFolderTxt_->GetValue();
		if (tmp == wxEmptyString)
		{
			wxMessageDialog* diag = new wxMessageDialog(m_parent,"The share directory must not be blank.","Error",wxOK);
			diag->ShowModal();
			return FALSE;
		}
		else if (!wxDirExists(shareFolderTxt_->GetValue()))
		{
			wxMessageDialog* diag = new wxMessageDialog(m_parent,"Specified directory does not exist.","Error",wxOK);
			diag->ShowModal();
			return FALSE;
		}

		if (Prefs::Instance()->Set(Prefs::SHAREDIR,shareFolderTxt_->GetValue()))
		{
			wxMessageDialog* diag = new wxMessageDialog(m_parent,"The new share directory will now be indexed.","Warning",wxOK);
			diag->ShowModal();

			Prefs::Instance()->ReloadShareDir();
		}

		Prefs::Instance()->Set(Prefs::MAXUPLOADS,toString(maxUploadsSpinCtrl_->GetValue()));
		Prefs::Instance()->Set(Prefs::MAXDOWNLOADS, toString(maxDownloadsSpinCtrl_->GetValue()));

		break;
	}
	case 3:
	{

		if (showOfflineCheck_->IsChecked())
			Prefs::Instance()->Set(Prefs::SHOWOFFLINE,"1");
		else
			Prefs::Instance()->Set(Prefs::SHOWOFFLINE,"0");

		break;
	}
	default:
	{
	 	wxFAIL;
		return FALSE;
	}
	}

	Prefs::Instance()->Save();

	return TRUE;
}

BEGIN_EVENT_TABLE(PrefFrame, wxFrame)
    EVT_BUTTON(ID_BTN_CLOSE,PrefFrame::OnButtons)
    EVT_BUTTON(ID_BTN_CANCEL,PrefFrame::OnButtons)
    EVT_BUTTON(ID_SHARING_BTN_CHOOSE,PrefFrame::OnButtons)
    EVT_CLOSE(PrefFrame::OnClose)
END_EVENT_TABLE()

PrefFrame::PrefFrame(MainFrame* parent)
	: wxFrame((wxWindow *)0, -1, "Turnstile Preferences",wxDefaultPosition, wxDefaultSize,wxDEFAULT_FRAME_STYLE/*|wxCLIP_CHILDREN*/|wxNO_FULL_REPAINT_ON_RESIZE)
		,parent_(parent)
{
    	SetIcon(wxIcon(prefframe));

	/* Make sure thes things are refreshed for the panels.
	*/
	Prefs::Instance()->GetHostCache()->Load();

    	// create image list for notebook tabs
	tabImageList_ = new wxImageList(ICON_DIMENS, ICON_DIMENS, TRUE);

	tabImageList_->Add(wxIcon(prefframe_user));
	tabImageList_->Add(wxIcon(prefframe_connection));
	tabImageList_->Add(wxIcon(prefframe_filetransfer));
	tabImageList_->Add(wxIcon(prefframe_buddylist));

	// create the panel
	wxPanel* panel_ = new wxPanel(this, -1, wxDefaultPosition, wxDefaultSize,
        				wxTAB_TRAVERSAL /*| wxCLIP_CHILDREN | wxNO_BORDER | wxNO_FULL_REPAINT_ON_RESIZE*/);

	wxBoxSizer* sizerPanel = new wxBoxSizer(wxVERTICAL);

	// create notebook
   	notebook_ = new PrefNotebook(panel_, ID_NOTEBOOK,
                                wxDefaultPosition, wxDefaultSize,
                                wxNB_TOP);

	// use images on notebook tabs
	notebook_->SetImageList(tabImageList_);

	notebook_->CreatePages();

    	sizerPanel->Add(notebook_, 1, wxEXPAND | wxALL, 4);
    	sizerPanel->Layout();

	wxButton* closeBtn = new wxButton(panel_, ID_BTN_CLOSE,
					_T("Close"));
	wxButton* cancelBtn = new wxButton(panel_, ID_BTN_CANCEL,
					_T("Cancel"));

	wxBoxSizer* bottomsizer = new wxBoxSizer (wxHORIZONTAL);
 	bottomsizer->Add(closeBtn, 0, wxALIGN_RIGHT|wxALL, 2);
	bottomsizer->Add(cancelBtn, 0, wxALIGN_RIGHT|wxALL, 2);
 	sizerPanel->Add(bottomsizer,0,wxGROW|wxALL,1);

	panel_->SetSizer(sizerPanel);

	panel_->SetAutoLayout(TRUE);

	SetSize(DetermineFrameSize(CONFIG_PREF_WINDOW));

	Centre(wxBOTH);
}

void PrefFrame::OnButtons(wxCommandEvent& event)
{
    	switch (event.GetId())
    	{
	case ID_BTN_CLOSE:
	{
		if (notebook_->Save(notebook_->GetSelection()))
			Close();
		break;
	}
	case ID_BTN_CANCEL:
	{
		Close();
		break;
	}
	case ID_SHARING_BTN_CHOOSE:
	{
		wxDirDialog dialog(this, "Choose a share directory",
			 wxGetHomeDir(), wxDD_NEW_DIR_BUTTON);

		if (dialog.ShowModal() == wxID_OK)
			notebook_->shareFolderTxt_->SetValue(dialog.GetPath());

		break;
	}
    	}
}

void
PrefFrame::OnClose(wxCloseEvent& WXUNUSED(event))
{
	// TODO save current notebook page before closing

	StoreFrameSize(GetRect(),CONFIG_PREF_WINDOW);
	parent_->OnClosePrefFrame();
	Destroy();
}

PrefFrame::~PrefFrame()
{;}


