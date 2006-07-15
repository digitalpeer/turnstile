/* $Id: findbuddyframe.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file findbuddyframe.cpp
@brief Find Buddy Window Implimentation
*/
#include "findbuddyframe.h"
#include "frameutils.h"
#include "mainframe.h"
#include "aboutdialog.h"
#include "connectionthread.h"
#include "utils.h"
#include "app.h"
#include "crypto.h"
#include "prefs.h"
#include "buddy.h"
#include "lookupresponse.h"

#include <wx/tokenzr.h>

#define CONFIG_FINDBUDDY_WINDOW "findbuddywindow"

#define TXT_MENU_CLOSE	"Close\tAlt-C"
#define TXT_MENU_ABOUT	"&About"
#define TXT_MENU_HELP	"&Help"
#define TXT_MENU_FILE	"&File"

#include "../pixmaps/findbuddyframe.xpm"

// ids
enum	{
		ID_BUDDYFRAME,
    		ID_LIST_CTRL,
		ID_MENU_CLOSE,
		ID_MENU_ABOUT,
		ID_SEARCH_BTN_CLEAR,
		ID_SEARCH_BTN_ADDBUDDY,
		ID_SEARCH_BTN_FIND,
		ID_SEARCH_TXT
	};


BEGIN_EVENT_TABLE(FindBuddyFrame, wxFrame)
	EVT_CLOSE(FindBuddyFrame::OnClose)

    	EVT_MENU(ID_MENU_CLOSE, FindBuddyFrame::OnMenuClose)
    	EVT_MENU(ID_MENU_ABOUT, FindBuddyFrame::OnMenuAbout)

	EVT_BUTTON(ID_SEARCH_BTN_CLEAR,FindBuddyFrame::OnButtons)
	EVT_BUTTON(ID_SEARCH_BTN_ADDBUDDY,FindBuddyFrame::OnButtons)
	EVT_BUTTON(ID_SEARCH_BTN_FIND,FindBuddyFrame::OnButtons)
	EVT_TEXT_ENTER(ID_SEARCH_TXT,FindBuddyFrame::OnButtons)
END_EVENT_TABLE()


void FindBuddyFrame::OnButtons(wxCommandEvent& event)
{
    	switch (event.GetId())
    	{
	case ID_SEARCH_BTN_CLEAR:
	{
		searchList_->DeleteAllItems();
		break;
	}
	case ID_SEARCH_BTN_ADDBUDDY:
	{
		long item = -1;
		for (;;)
		{
			item = searchList_->GetNextItem(item,
							wxLIST_NEXT_ALL,
							wxLIST_STATE_SELECTED);
			if (item == -1)
				break;

			Buddy* buddy = new Buddy(*((Buddy*)searchList_->GetItemData(item)));

			wxGetApp().GetMainWindow()->AddBuddy(buddy);
		}
		break;
	}
	case ID_SEARCH_TXT:
	case ID_SEARCH_BTN_FIND:
	{
		if (searchTxt_->GetValue() != "")
		{
			searchBtn_->Disable();
			searchBtn_->SetLabel("Searching...");
			searchList_->DeleteAllItems();

			wxString hash = crypto::Sha1Hash(searchTxt_->GetValue().MakeLower());

			ConnectionThread* thread = new ConnectionThread(this,ID_BUDDYFRAME,Connection::TYPE_QUERY,"",hash);

			if (thread->Create() != wxTHREAD_NO_ERROR)
				LogDebug("Error: Failed to create connection thread!");

			// run the thread
			thread->Run();
		}
		break;
	}
    	}
}

FindBuddyFrame::FindBuddyFrame(MainFrame* parent)
       : wxFrame(0, -1, "Turnstile Find Buddy",wxDefaultPosition, wxDefaultSize),
	  	parent_(parent)
{
    	SetIcon(wxIcon(findbuddyframe));

	// create main window menus
	wxMenu* file_menu = new wxMenu;
	file_menu->Append(ID_MENU_CLOSE, TXT_MENU_CLOSE);

	wxMenu* help_menu = new wxMenu;
	help_menu->Append(ID_MENU_ABOUT, TXT_MENU_ABOUT);

    	wxMenuBar* menu_bar = new wxMenuBar;
    	menu_bar->Append(file_menu, TXT_MENU_FILE);
    	menu_bar->Append(help_menu, TXT_MENU_HELP);
    	SetMenuBar(menu_bar);

    	// create the panel
	wxPanel* panel = new wxPanel(this, -1, wxDefaultPosition, wxDefaultSize,
        			wxTAB_TRAVERSAL | wxCLIP_CHILDREN | wxNO_BORDER);

	wxEvtHandler::Connect(ID_BUDDYFRAME, wxEVT_COMMAND_MENU_SELECTED,
		(wxObjectEventFunction)
                (wxEventFunction)
                (wxCommandEventFunction)&FindBuddyFrame::OnConnectionEvent,
		(wxObject*) NULL);

	searchBtn_ = new wxButton(panel, ID_SEARCH_BTN_FIND,
					_T("Find"));

	wxButton* dlbutton = new wxButton(panel, ID_SEARCH_BTN_ADDBUDDY,
					_T("Add Selected Buddies"));

	wxButton* clearbutton = new wxButton(panel, ID_SEARCH_BTN_CLEAR,
					_T("Clear"));

	searchTxt_ = new wxTextCtrl(panel, ID_SEARCH_TXT, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxSUNKEN_BORDER);

	searchList_ = new BuddyListCtrl(panel, ID_LIST_CTRL,
					wxDefaultPosition, wxDefaultSize,
					wxLC_VRULES|wxLC_REPORT | wxSUNKEN_BORDER);

	wxListItem itemCol;
	itemCol.m_mask = wxLIST_MASK_TEXT/* | wxLIST_MASK_IMAGE*/;
	itemCol.m_text = "Alias";
	searchList_->InsertColumn(0, itemCol);
	itemCol.m_text = "Public Key";
	searchList_->InsertColumn(1, itemCol);
	itemCol.m_text = "IP Address";
	searchList_->InsertColumn(2, itemCol);
	itemCol.m_text = "Profile";
	searchList_->InsertColumn(3, itemCol);

	searchList_->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
	searchList_->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
	searchList_->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
	searchList_->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);

    	wxBoxSizer* sizerPanel = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* topsizer = new wxBoxSizer (wxHORIZONTAL);
 	topsizer->Add(searchTxt_, 1, wxALL, 2);
 	topsizer->Add(searchBtn_, 0, wxALL, 2);
	sizerPanel->Add(topsizer,0,wxGROW|wxALL,2);

	sizerPanel->Add(searchList_, 1, wxGROW|wxALL, 2);

	wxBoxSizer* bottomsizer = new wxBoxSizer (wxHORIZONTAL);
	bottomsizer->Add(clearbutton, 0, wxALIGN_RIGHT|wxALL, 2);
 	bottomsizer->Add(dlbutton, 0, wxALIGN_RIGHT|wxALL, 2);
 	sizerPanel->Add(bottomsizer,0,wxGROW|wxALL,2);

	panel->SetSizer(sizerPanel);

	// set frame size/position
	SetSize(DetermineFrameSize(CONFIG_FINDBUDDY_WINDOW));
}

void
FindBuddyFrame::OnMenuClose(wxCommandEvent& WXUNUSED(event))
{
    	Close();
}

void
FindBuddyFrame::OnMenuAbout(wxCommandEvent& WXUNUSED(event))
{
	AboutDialog it(this);
   	it.ShowModal();
}

void
FindBuddyFrame::OnClose(wxCloseEvent& WXUNUSED(event))
{
	parent_->OnCloseFindBuddyFrame();
	Destroy();

	StoreFrameSize(GetRect(),CONFIG_FINDBUDDY_WINDOW);
}

void
FindBuddyFrame::OnConnectionEvent(wxCommandEvent& event)
{
	int n = event.GetInt();
	
	switch(n)
	{
	case Connection::EVENT_QUERYFOUND:
	{
		LogDebug("Got query found.");
		LookupResponse* response = (LookupResponse*)event.GetClientData();
		// add all the files in the response to the search list
		AddBuddies(response);
		// make sure and delete the event data
		delete response;
		break;
	}
	case Connection::EVENT_QUERYNOTFOUND:
	{
		LogDebug("Got query not found.");
		break;
	}
	default:
		LogDebug(wxString().Format("Unhandled connection event %d to search.",n));
	}

	// re-enable the search button
	searchBtn_->Enable();
	searchBtn_->SetLabel("Find");
}

void
FindBuddyFrame::AddBuddies(LookupResponse* response)
{
	LogDebug(wxString().Format("%i responses in the response packet",response->responses_.size()));
	for (unsigned int x = 0; x < response->responses_.size();x++)
	{
		LogDebug(wxString().Format("Parsing info '%s'",response->responses_[x].info.c_str()));

		wxStringTokenizer tokens(response->responses_[x].info, ":");

		if (tokens.GetNextToken() == "BUDDY")
		{
			wxString alias = tokens.GetNextToken();
			wxString publicKey = tokens.GetNextToken();
			wxString profile = tokens.GetNextToken();

			int item = searchList_->InsertItem(0, alias);
			searchList_->SetItem(item, 1, publicKey);
			searchList_->SetItem(item, 2, response->responses_[x].node);
			searchList_->SetItem(item, 3, profile);

			Buddy* buddy = new Buddy();
			buddy->Set(Buddy::DATA_ALIAS,alias);
			buddy->Set(Buddy::DATA_PUBLICKEY,publicKey);
			buddy->Set(Buddy::DATA_PROFILE,profile);
			searchList_->SetItemData(item, (int)buddy);
		}
	}

	searchList_->SetColumnWidth(0, wxLIST_AUTOSIZE);
	searchList_->SetColumnWidth(1, wxLIST_AUTOSIZE);
	searchList_->SetColumnWidth(2, wxLIST_AUTOSIZE);
	searchList_->SetColumnWidth(3, wxLIST_AUTOSIZE);
}

FindBuddyFrame::~FindBuddyFrame()
{
	wxEvtHandler::Disconnect(ID_BUDDYFRAME, wxEVT_COMMAND_MENU_SELECTED);
}


BEGIN_EVENT_TABLE(BuddyListCtrl, wxListCtrl)
	EVT_LIST_DELETE_ITEM(ID_LIST_CTRL, BuddyListCtrl::OnDeleteItem)
	EVT_LIST_DELETE_ALL_ITEMS(ID_LIST_CTRL, BuddyListCtrl::OnDeleteAllItems)
	//EVT_LIST_GET_INFO(ID_LIST_CTRL, BuddyListCtrl::OnGetInfo)
	EVT_LIST_ITEM_SELECTED(ID_LIST_CTRL, BuddyListCtrl::OnSelected)
	EVT_LIST_KEY_DOWN(ID_LIST_CTRL, BuddyListCtrl::OnListKeyDown)
	EVT_LIST_ITEM_ACTIVATED(ID_LIST_CTRL, BuddyListCtrl::OnActivated)
	EVT_LIST_COL_CLICK(ID_LIST_CTRL,BuddyListCtrl::OnColClick)
	EVT_CHAR(BuddyListCtrl::OnChar)
END_EVENT_TABLE()

BuddyListCtrl::BuddyListCtrl(wxWindow* parent,const wxWindowID id,const wxPoint& pos,
				const wxSize& size,long style)
				: wxListCtrl(parent, id, pos, size, style)
{;}

void
BuddyListCtrl::SetColumnImage(int col, int image)
{
	wxListItem item;
	item.SetMask(wxLIST_MASK_IMAGE);
	item.SetImage(image);
	SetColumn(col, item);
}

/*int wxCALLBACK MyCompareFunction(long item1, long item2, long sortData)
{
    // inverse the order
    if (item1 < item2)
    	return -1;
    if (item1 > item2)
    	return 1;

    return 0;
}*/

void
BuddyListCtrl::OnColClick(wxListEvent& event)
{
    int col = event.GetColumn();
    //SortItems(MyCompareFunction, col);
}


void
BuddyListCtrl::OnDeleteItem(wxListEvent& WXUNUSED(event))
{;}

void
BuddyListCtrl::OnDeleteAllItems(wxListEvent& WXUNUSED(event))
{;}

void BuddyListCtrl::OnSelected(wxListEvent& event)
{
//    LogEvent(event, _T("OnSelected"));

    if ( GetWindowStyle() & wxLC_REPORT )
    {
        wxListItem info;
        info.m_itemId = event.m_itemIndex;
        info.m_col = 1;
        info.m_mask = wxLIST_MASK_TEXT;
        if ( GetItem(info) )
        {
           // wxLogMessage(wxT("Value of the 2nd field of the selected item: %s"),
             //            info.m_text.c_str());
        }
        else
        {
            wxFAIL_MSG(wxT("wxListCtrl::GetItem() failed"));
        }
    }
}

void
BuddyListCtrl::OnActivated(wxListEvent& WXUNUSED(event))
{;}

void
BuddyListCtrl::OnListKeyDown(wxListEvent& event)
{
    switch ( event.GetKeyCode() )
    {
        case 'c': // colorize
        case 'C':
            {
                wxListItem info;
                info.m_itemId = event.GetIndex();
                GetItem(info);

                wxListItemAttr *attr = info.GetAttributes();
                if ( !attr || !attr->HasTextColour() )
                {
                    info.SetTextColour(*wxCYAN);

                    SetItem(info);

                    RefreshItem(info.m_itemId);
                }
            }
            break;

        case 'n': // next
        case 'N':
            {
                long item = GetNextItem(-1,
                                        wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
                if ( item++ == GetItemCount() - 1 )
                {
                    item = 0;
                }

                wxLogMessage(_T("Focusing item %ld"), item);

                SetItemState(item, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
                EnsureVisible(item);
            }
            break;

        case WXK_DELETE:
            {
                long item = GetNextItem(-1,
                                        wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
                while ( item != -1 )
                {
                    DeleteItem(item);

                    wxLogMessage(_T("Item %ld deleted"), item);

                    // -1 because the indices were shifted by DeleteItem()
                    item = GetNextItem(item - 1,
                                       wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
                }
            }
            break;

      /*  case WXK_INSERT:
            if ( GetWindowStyle() & wxLC_REPORT )
            {
                if ( GetWindowStyle() & wxLC_VIRTUAL )
                {
                    SetItemCount(GetItemCount() + 1);
                }
                else // !virtual
                {
                    InsertItemInReportView(event.GetIndex());
                }
            }*/
            //else: fall through

       default:
//            LogEvent(event, _T("OnListKeyDown"));

            event.Skip();
    }
}

void BuddyListCtrl::OnChar(wxKeyEvent& event)
{
    wxLogMessage(_T("Got char event."));

    switch ( event.GetKeyCode() )
    {
        case 'n':
        case 'N':
        case 'c':
        case 'C':
            // these are the keys we process ourselves
            break;

        default:
            event.Skip();
    }
}




