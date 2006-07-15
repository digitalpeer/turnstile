/* $Id: fileframe.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file fileframe.cpp
@brief File Transfer Window Implimentation
*/
#include "fileframe.h"
#include "frameutils.h"
#include "network.h"
#include "mainframe.h"
#include "aboutdialog.h"
#include "connectionthread.h"
#include "filetransferthread.h"
#include "utils.h"
#include "crypto.h"
#include "prefs.h"
#include "files.h"
#include "lookupresponse.h"

#include <wx/tokenzr.h>

#define CONFIG_FILE_WINDOW "filewindow"

#define TXT_MENU_CLOSE	"Close\tAlt-C"
#define TXT_MENU_ABOUT	"&About"
#define TXT_MENU_HELP	"&Help"
#define TXT_MENU_FILE	"&File"

#include "../pixmaps/fileframe.xpm"
#include "../pixmaps/fileframe_search.xpm"
#include "../pixmaps/fileframe_upload.xpm"
#include "../pixmaps/fileframe_download.xpm"

// ids
enum	{
		ID_NOTEBOOK = 8500,
		IDwxTIMER,
		ID_FILEFRAME,
    		ID_LIST_CTRL,
		ID_MENU_CLOSE,
		ID_MENU_ABOUT,
		ID_UPLOAD_BTN_CANCEL,
		ID_UPLOAD_BTN_CLEARFINISHED,
		ID_DOWNLOAD_BTN_PAUSE,
		ID_DOWNLOAD_BTN_RESUME,
		ID_DOWNLOAD_BTN_CANCEL,
		ID_DOWNLOAD_BTN_CLEARFINISHED,
		ID_SEARCH_BTN_CLEAR,
		ID_SEARCH_BTN_DOWNLOAD,
		ID_SEARCH_BTN_FIND,
		ID_SEARCH_TXT
	};

FileNotebook::FileNotebook(wxWindow *parent, wxWindowID id,
    		const wxPoint& pos, const wxSize& size, long style)
    			: wxNotebook(parent, id, pos, size, style),uploadList_(0)
{;}

wxPanel* FileNotebook::CreatePageUploads()
{
    	wxPanel* panel = new wxPanel(this,-1);

	wxButton* cancelBtn = new wxButton(panel, ID_UPLOAD_BTN_CANCEL,
					wxT("Cancel All"));

	wxButton* clearFinishedBtn = new wxButton(panel, ID_UPLOAD_BTN_CLEARFINISHED,
					wxT("Clear Finished/Cancelled"));

	uploadList_ = new FileListCtrl(panel, ID_LIST_CTRL,
					wxDefaultPosition, wxDefaultSize,
					wxLC_VRULES|wxLC_REPORT | wxSUNKEN_BORDER);

	wxListItem itemCol;
	itemCol.m_mask = wxLIST_MASK_TEXT/* | wxLIST_MASK_IMAGE*/;
	itemCol.m_text = "File";
	//itemCol.m_image = 0;
	uploadList_->InsertColumn(0, itemCol);
	itemCol.m_text = "Size";
	//itemCol.m_image = 1;
	uploadList_->InsertColumn(1, itemCol);
	itemCol.m_text = "Status";
	//itemCol.m_image = 2;
	uploadList_->InsertColumn(2, itemCol);
	itemCol.m_text = "Sent";
	//itemCol.m_image = 3;
	uploadList_->InsertColumn(3, itemCol);
	itemCol.m_text = "Speed";
	//itemCol.m_image = 3;
	uploadList_->InsertColumn(4, itemCol);
	itemCol.m_text = "MD5";
	//itemCol.m_image = 4;
	uploadList_->InsertColumn(5, itemCol);

	uploadList_->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
	uploadList_->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
	uploadList_->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
	uploadList_->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
	uploadList_->SetColumnWidth(4, wxLIST_AUTOSIZE_USEHEADER);
	uploadList_->SetColumnWidth(5, wxLIST_AUTOSIZE_USEHEADER);

    	wxBoxSizer* sizerPanel = new wxBoxSizer(wxVERTICAL);

	sizerPanel->Add(uploadList_, 1, wxGROW|wxALL, 2);

	wxBoxSizer* bottomsizer = new wxBoxSizer (wxHORIZONTAL);
	bottomsizer->Add(cancelBtn, 0, wxALIGN_RIGHT|wxALL, 2);
	bottomsizer->Add(clearFinishedBtn, 0, wxALIGN_RIGHT|wxALL, 2);

 	sizerPanel->Add(bottomsizer,0,wxGROW|wxALL,2);

	panel->SetSizer(sizerPanel);

    	return panel;
}

wxPanel* FileNotebook::CreatePageDownloads()
{
    	wxPanel* panel = new wxPanel(this,-1);

	wxButton* pauseBtn = new wxButton(panel, ID_DOWNLOAD_BTN_PAUSE,
					wxT("Pause Selected"));

	resumeBtn_ = new wxButton(panel, ID_DOWNLOAD_BTN_RESUME,
					wxT("Resume Selected"));

	wxButton* cancelBtn = new wxButton(panel, ID_DOWNLOAD_BTN_CANCEL,
					wxT("Cancel Selected"));

	wxButton* clearFinishedBtn = new wxButton(panel, ID_DOWNLOAD_BTN_CLEARFINISHED,
					wxT("Clear Finished/Cancelled"));


	downloadList_ = new FileListCtrl(panel, ID_LIST_CTRL,
					wxDefaultPosition, wxDefaultSize,
					wxLC_VRULES|wxLC_REPORT | wxSUNKEN_BORDER );

	wxListItem itemCol;
	itemCol.m_mask = wxLIST_MASK_TEXT/* | wxLIST_MASK_IMAGE*/;
	itemCol.m_text = wxT("File");
	//itemCol.m_image = -1;
	downloadList_->InsertColumn(0, itemCol);
	itemCol.m_text = wxT("Size");
	downloadList_->InsertColumn(1, itemCol);
itemCol.m_text = wxT("Status");
	downloadList_->InsertColumn(2, itemCol);
itemCol.m_text = wxT("Progress");
	downloadList_->InsertColumn(3, itemCol);
itemCol.m_text = wxT("Speed");
	downloadList_->InsertColumn(4, itemCol);
itemCol.m_text = wxT("Source");
	downloadList_->InsertColumn(5, itemCol);
itemCol.m_text = wxT("MD5");
	downloadList_->InsertColumn(6, itemCol);

	downloadList_->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
	downloadList_->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
	downloadList_->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
	downloadList_->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
	downloadList_->SetColumnWidth(4, wxLIST_AUTOSIZE_USEHEADER);
	downloadList_->SetColumnWidth(5, wxLIST_AUTOSIZE_USEHEADER);
	downloadList_->SetColumnWidth(6, wxLIST_AUTOSIZE_USEHEADER);

    	wxBoxSizer* sizerPanel = new wxBoxSizer(wxVERTICAL);

	sizerPanel->Add(downloadList_, 1, wxGROW|wxALL, 2);

	wxBoxSizer* bottomsizer = new wxBoxSizer (wxHORIZONTAL);
	bottomsizer->Add(cancelBtn, 0, wxALIGN_RIGHT|wxALL, 2);
 	bottomsizer->Add(resumeBtn_, 0, wxALIGN_RIGHT|wxALL, 2);
	bottomsizer->Add(pauseBtn, 0, wxALIGN_RIGHT|wxALL, 2);
	bottomsizer->Add(clearFinishedBtn, 0, wxALIGN_RIGHT|wxALL, 2);

 	sizerPanel->Add(bottomsizer,0,wxGROW|wxALL,2);

	panel->SetSizer(sizerPanel);

    	return panel;
}

wxPanel* FileNotebook::CreatePageSearch()
{
    	wxPanel* panel = new wxPanel(this,-1);

	searchBtn_ = new wxButton(panel, ID_SEARCH_BTN_FIND,
					wxT("Find"));

	dlbutton_ = new wxButton(panel, ID_SEARCH_BTN_DOWNLOAD,
					wxT("Download Selected"));

	wxButton* clearbutton = new wxButton(panel, ID_SEARCH_BTN_CLEAR,
					wxT("Clear"));

	searchTxt_ = new wxTextCtrl(panel, ID_SEARCH_TXT, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxSUNKEN_BORDER|wxTE_PROCESS_ENTER);

	searchList_ = new FileListCtrl(panel, ID_LIST_CTRL,
					wxDefaultPosition, wxDefaultSize,
					wxLC_VRULES|wxLC_REPORT | wxSUNKEN_BORDER );

	wxListItem itemCol;
	itemCol.m_mask = wxLIST_MASK_TEXT/* | wxLIST_MASK_IMAGE*/;
	itemCol.m_text = "File";
	//itemCol.m_image = -1;
	searchList_->InsertColumn(0, itemCol);
	itemCol.m_text = "Size";
	searchList_->InsertColumn(1, itemCol);
	itemCol.m_text = "Source";
	searchList_->InsertColumn(2, itemCol);
	itemCol.m_text = "MD5";
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
 	bottomsizer->Add(dlbutton_, 0, wxALIGN_RIGHT|wxALL, 2);
 	sizerPanel->Add(bottomsizer,0,wxGROW|wxALL,2);
	
	panel->SetSizer(sizerPanel);
	
    	return panel;
}

void FileNotebook::CreatePages()
{
	wxPanel *panel;

	panel = CreatePageSearch();
	AddPage(panel, "Search", FALSE, 0);
	
	panel = CreatePageDownloads();
	AddPage(panel, "Downloads", FALSE, 1);
	
	panel = CreatePageUploads();
	AddPage(panel, "Uploads", FALSE, 2);

	SetSelection(0);
}

FileNotebook::~FileNotebook()
{;}


BEGIN_EVENT_TABLE(FileFrame, wxFrame)
	EVT_TIMER(IDwxTIMER, FileFrame::OnRefreshList)

	EVT_CLOSE(FileFrame::OnClose)

    	EVT_MENU(ID_MENU_CLOSE, FileFrame::OnMenuClose)
    	EVT_MENU(ID_MENU_ABOUT, FileFrame::OnMenuAbout)
	EVT_NOTEBOOK_PAGE_CHANGED(ID_NOTEBOOK, FileFrame::OnNotebook)
    	EVT_NOTEBOOK_PAGE_CHANGING(ID_NOTEBOOK, FileFrame::OnNotebook)

	EVT_BUTTON(ID_UPLOAD_BTN_CANCEL, FileFrame::OnButtons)
	EVT_BUTTON(ID_UPLOAD_BTN_CLEARFINISHED, FileFrame::OnButtons)
	EVT_BUTTON(ID_DOWNLOAD_BTN_PAUSE, FileFrame::OnButtons)
	EVT_BUTTON(ID_DOWNLOAD_BTN_RESUME, FileFrame::OnButtons)
	EVT_BUTTON(ID_DOWNLOAD_BTN_CANCEL, FileFrame::OnButtons)
	EVT_BUTTON(ID_DOWNLOAD_BTN_CLEARFINISHED, FileFrame::OnButtons)
	EVT_BUTTON(ID_SEARCH_BTN_CLEAR, FileFrame::OnButtons)
	EVT_BUTTON(ID_SEARCH_BTN_DOWNLOAD, FileFrame::OnButtons)
	EVT_BUTTON(ID_SEARCH_BTN_FIND, FileFrame::OnButtons)
	EVT_TEXT_ENTER(ID_SEARCH_TXT, FileFrame::OnButtons)
END_EVENT_TABLE()

void
FileFrame::RefreshList()
{
	int id = notebook_->GetSelection();
	switch(id)
	{
	case 0:	// search
	{
		if (!Network::Instance()->GetStatus(Network::STATUS_ALIVE))
		{
			notebook_->searchBtn_->Disable();
			notebook_->dlbutton_->Disable();
		}
		else
		{
			if (!notebook_->searchBtn_->IsEnabled())
			{
				notebook_->searchBtn_->Enable();
				notebook_->dlbutton_->Enable();
			}
		}

		break;
	}
	case 1:	// downloads
	{
		if (!Network::Instance()->GetStatus(Network::STATUS_ALIVE))
		{
			notebook_->resumeBtn_->Disable();
		}
		else
		{
			if (!notebook_->resumeBtn_->IsEnabled())
			{
				notebook_->resumeBtn_->Enable();
			}
		}

		for (unsigned int x = 0; x < notebook_->downloadList_->GetItemCount();x++)
		{
			ShareFile* file = (ShareFile*)notebook_->downloadList_->GetItemData(x);

			notebook_->downloadList_->SetItem(x, 0, file->Name());
			notebook_->downloadList_->SetItem(x, 1, HumanFileSize(file->Size()));
			notebook_->downloadList_->SetItem(x, 2, ShareFile::STATUSES[file->Status()]);
			notebook_->downloadList_->SetItem(x, 3, wxString().Format("%d percent",(int)(((double)file->CompletedSize()/(double)file->Size())*100.0)));
			notebook_->downloadList_->SetItem(x, 4, wxString().Format("%.2f kb/s",file->Kbs()));
			notebook_->downloadList_->SetItem(x, 5, file->Node());
			notebook_->downloadList_->SetItem(x, 6, file->MD5());
		}

		break;
	}
	case 2:	// uploads
	{
		for (unsigned int x = 0; x < notebook_->uploadList_->GetItemCount();x++)
		{
			ShareFile* file = (ShareFile*)notebook_->uploadList_->GetItemData(x);

			notebook_->uploadList_->SetItem(x, 0,file->Name());
			notebook_->uploadList_->SetItem(x, 1, HumanFileSize(file->Size()));
			notebook_->uploadList_->SetItem(x, 2, ShareFile::STATUSES[file->Status()]);
			notebook_->uploadList_->SetItem(x, 3, HumanFileSize(file->CompletedSize()));
			notebook_->uploadList_->SetItem(x, 4, wxString().Format("%.2f kb/s",file->Kbs()));
			notebook_->uploadList_->SetItem(x, 5, file->MD5());
		}

		break;
	}
	}
}

void
FileFrame::OnRefreshList(wxTimerEvent& event)
{
	RefreshList();
}

void
FileFrame::AddDownload(ShareFile* file)
{
	// file, size, status, progress, sources, hash
	int item = notebook_->downloadList_->InsertItem(0, file->Name());
	notebook_->downloadList_->SetItemData(item, (int)file);

	//RefreshList();
}

void
FileFrame::AddUpload(ShareFile* file)
{
	// file, size, status, progress, sources, hash
	int item = notebook_->uploadList_->InsertItem(0, file->Name());
	notebook_->uploadList_->SetItemData(item, (int)file);

	//RefreshList();

}


void FileFrame::OnButtons(wxCommandEvent& event)
{
    	switch (event.GetId())
    	{
	case ID_UPLOAD_BTN_CANCEL:
	{
		long item = -1;
		for (;;)
		{
			item = notebook_->uploadList_->GetNextItem(item,
							wxLIST_NEXT_ALL,
							wxLIST_STATE_SELECTED);
			if (item == -1)
				break;

			ShareFile* file = (ShareFile*)notebook_->uploadList_->GetItemData(item);

			file->Status(ShareFile::STATUS_CANCELLED);
		}

		break;
	}
	case ID_UPLOAD_BTN_CLEARFINISHED:
	{
		long item = -1;
		for (;;)
		{
			item = notebook_->uploadList_->GetNextItem(item,
							wxLIST_NEXT_ALL/*,
							wxLIST_STATE_SELECTED*/);
			if (item == -1)
				break;

			ShareFile* file = (ShareFile*)notebook_->uploadList_->GetItemData(item);

			if (file->Status() == ShareFile::STATUS_FINISHED ||
				file->Status() == ShareFile::STATUS_CANCELLED)
			{
				notebook_->uploadList_->DeleteItem(item);
				Network::Instance()->RemoveUpload(file);
				item = -1;
			}
		}
	}
	case ID_DOWNLOAD_BTN_PAUSE:
	{
		long item = -1;
		for (;;)
		{
			item = notebook_->downloadList_->GetNextItem(item,
							wxLIST_NEXT_ALL,
							wxLIST_STATE_SELECTED);
			if (item == -1)
				break;

			ShareFile* file = (ShareFile*)notebook_->downloadList_->GetItemData(item);

			file->Status(ShareFile::STATUS_PAUSED);
		}

		break;
	}
	case ID_DOWNLOAD_BTN_RESUME:
	{
		long item = -1;
		for (;;)
		{
			item = notebook_->downloadList_->GetNextItem(item,
							wxLIST_NEXT_ALL,
							wxLIST_STATE_SELECTED);
			if (item == -1)
				break;

			ShareFile* file = (ShareFile*)notebook_->downloadList_->GetItemData(item);

			file->Status(ShareFile::STATUS_PAUSED);

			FileTransferThread* thread = new FileTransferThread(file);

			if (thread->Create() != wxTHREAD_NO_ERROR)
				LogDebug("Can't create fiel transfer thread!");

			// run the thread
			thread->Run();
		}
		break;
	}
	case ID_DOWNLOAD_BTN_CANCEL:
	{
		long item = -1;
		for (;;)
		{
			item = notebook_->downloadList_->GetNextItem(item,
							wxLIST_NEXT_ALL,
							wxLIST_STATE_SELECTED);
			if (item == -1)
				break;

			ShareFile* file = (ShareFile*)notebook_->downloadList_->GetItemData(item);

			file->Status(ShareFile::STATUS_CANCELLED);
		}

		break;
	}
	case ID_DOWNLOAD_BTN_CLEARFINISHED:
	{
		long item = -1;
		for (;;)
		{
			item = notebook_->downloadList_->GetNextItem(item,
							wxLIST_NEXT_ALL/*,
							wxLIST_STATE_SELECTED*/);
			if (item == -1)
				break;

			ShareFile* file = (ShareFile*)notebook_->downloadList_->GetItemData(item);

			if (file->Status() == ShareFile::STATUS_FINISHED ||
				file->Status() == ShareFile::STATUS_CANCELLED)
			{
				notebook_->downloadList_->DeleteItem(item);
				Network::Instance()->RemoveDownload(file);
				item = -1;
			}
		}
		break;
	}
	case ID_SEARCH_BTN_CLEAR:
	{
		notebook_->searchList_->DeleteAllItems();
		break;
	}
	case ID_SEARCH_BTN_DOWNLOAD:
	{
		long item = -1;
		for (;;)
		{
			item = notebook_->searchList_->GetNextItem(item,
							wxLIST_NEXT_ALL,
							wxLIST_STATE_SELECTED);
			if (item == -1)
				break;

			ShareFile* file = new ShareFile(*((ShareFile*)notebook_->searchList_->GetItemData(item)));
			notebook_->searchList_->DeleteItem(item);
			AddDownload(file);
			Network::Instance()->AddDownload(file);

			// start a thread for the file
			FileTransferThread* thread = new FileTransferThread(file);

			if (thread->Create() != wxTHREAD_NO_ERROR)
				LogDebug("Can't create fiel transfer thread!");

			// run the thread
			thread->Run();
		}
		break;
	}
	case ID_SEARCH_TXT:
	case ID_SEARCH_BTN_FIND:
	{
		if (notebook_->searchTxt_->GetValue() != "")
		{
			notebook_->searchBtn_->Disable();
			notebook_->searchBtn_->SetLabel("Searching...");
			notebook_->searchList_->DeleteAllItems();

			wxString hash = crypto::Sha1Hash(notebook_->searchTxt_->GetValue().MakeLower());

			ConnectionThread* thread = new ConnectionThread(this,ID_FILEFRAME,Connection::TYPE_QUERY,"",hash);

			if (thread->Create() != wxTHREAD_NO_ERROR)
				LogDebug("Error: Failed to create connection thread!");

			// run the thread
			thread->Run();
		}
		break;
	}
    	}
}

FileFrame::FileFrame(MainFrame* parent)
       : wxFrame(0, -1, "Turnstile File Window",wxDefaultPosition, wxDefaultSize),
	  	parent_(parent), notebook_(0)
{
	refreshTimer_ = new wxTimer(this, IDwxTIMER);

    	SetIcon(wxIcon(fileframe));

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


    	// create image list for notebook tabs
	tabImageList_ = new wxImageList(32,32,TRUE);
	tabImageList_->Add(wxIcon(fileframe_search));
	tabImageList_->Add(wxIcon(fileframe_download));
	tabImageList_->Add(wxIcon(fileframe_upload));

	// create notebook
   	notebook_ = new FileNotebook(panel, ID_NOTEBOOK,
                                wxDefaultPosition, wxDefaultSize,
                                wxNB_TOP);

	// use images on notebook tabs
	notebook_->SetImageList(tabImageList_);

	// create pages
    	notebook_->CreatePages();

	//notebook_->searchBtn_->Disable();

	wxEvtHandler::Connect(ID_FILEFRAME, wxEVT_COMMAND_MENU_SELECTED,
		(wxObjectEventFunction)
                (wxEventFunction)
                (wxCommandEventFunction)&FileFrame::OnConnectionEvent,
		(wxObject*) NULL);

	wxBoxSizer* sizerPanel = new wxBoxSizer(wxHORIZONTAL);
	sizerPanel->Add(notebook_,1,wxGROW|wxALL);
	panel->SetSizer(sizerPanel);

	// set frame size/position
	SetSize(DetermineFrameSize(CONFIG_FILE_WINDOW));

	// add all of our current downloads to the list
	for (unsigned int x = 0; x < Network::Instance()->downloads.size();x++)
	{
		AddDownload(Network::Instance()->downloads[x]);
	}

	refreshTimer_->Start(1000);
}


void
FileFrame::OnNotebook(wxNotebookEvent& event)
{
//#if 0
    wxEventType eventType = event.GetEventType();

   // if (notebook_ != 0)
   // {
	if (eventType == wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED)
	{
		/*
		clear all items and re-add them (incase there are some new ones)
		*/
		if (notebook_->uploadList_)
		{
			notebook_->uploadList_->DeleteAllItems();

			for (unsigned int x = 0; x < Network::Instance()->UploadSize();x++)
			{
				ShareFile* file = Network::Instance()->uploads[x];
				AddUpload(file);
			}
		}
	}
	else if (eventType == wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING)
	{;}
   // }
//#endif
    //event.Skip();
}

void
FileFrame::OnMenuClose(wxCommandEvent& WXUNUSED(event))
{
    	Close();
}

void
FileFrame::OnMenuAbout(wxCommandEvent& WXUNUSED(event))
{
#ifdef DEBUG
	Connection* queryCon = new Connection(0,-1,-1,"");
	queryCon->AnalyzeNetwork();
	delete queryCon;

	//crypto::runtest();
#else
	AboutDialog it(this);
   	it.ShowModal();
#endif
}

void
FileFrame::OnClose(wxCloseEvent& WXUNUSED(event))
{
	parent_->OnCloseFileFrame();
	Destroy();
	
	StoreFrameSize(GetRect(),CONFIG_FILE_WINDOW);
}

void
FileFrame::OnConnectionEvent(wxCommandEvent& event)
{
	int n = event.GetInt();
	
	switch(n)
	{
	case Connection::EVENT_QUERYFOUND:
	{
		LogDebug("Got query found.");
		LookupResponse* response = (LookupResponse*)event.GetClientData();
		// add all the files in the response to the search list
		AddFiles(response);
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
	notebook_->searchBtn_->Enable();
	notebook_->searchBtn_->SetLabel("Find");
}

void
FileFrame::AddFiles(LookupResponse* response)
{
	LogDebug(wxString().Format("%i responses in the response packet",response->responses_.size()));
	for (unsigned int x = 0; x < response->responses_.size();x++)
	{
		LogDebug(wxString().Format("Parsing info '%s'",response->responses_[x].info.c_str()));

		wxStringTokenizer tokens(response->responses_[x].info, ":");

		if (tokens.GetNextToken() == "FILE")
		{
			wxString filename = tokens.GetNextToken();
			wxString size = tokens.GetNextToken();
			wxString md5 = tokens.GetNextToken();

			int item = notebook_->searchList_->InsertItem(0, filename);
			notebook_->searchList_->SetItem(item, 1, HumanFileSize(atoi(size.c_str())));
			notebook_->searchList_->SetItem(item, 2, response->responses_[x].node);
			notebook_->searchList_->SetItem(item, 3, md5);

			/*
			Create the file object.
			*/
			ShareFile* file = new ShareFile();
			file->Path(Prefs::Instance()->Get(Prefs::SHAREDIR) + "/" + filename);
			file->Size(atoi(size.c_str()));
			file->Node(response->responses_[x].node);
			file->MD5(md5);
			notebook_->searchList_->SetItemData(item, (int)file);
		}
	}

	//notebook_->searchList_->SetColumnWidth(0, wxLIST_AUTOSIZE);
	//notebook_->searchList_->SetColumnWidth(1, wxLIST_AUTOSIZE);
	//notebook_->searchList_->SetColumnWidth(2, wxLIST_AUTOSIZE);
	//notebook_->searchList_->SetColumnWidth(3, wxLIST_AUTOSIZE);
}

FileFrame::~FileFrame()
{
	wxEvtHandler::Disconnect(ID_FILEFRAME, wxEVT_COMMAND_MENU_SELECTED);

	delete refreshTimer_;
}


BEGIN_EVENT_TABLE(FileListCtrl, wxListCtrl)
	EVT_LIST_DELETE_ITEM(ID_LIST_CTRL, FileListCtrl::OnDeleteItem)
	EVT_LIST_DELETE_ALL_ITEMS(ID_LIST_CTRL, FileListCtrl::OnDeleteAllItems)
	//EVT_LIST_GET_INFO(ID_LIST_CTRL, FileListCtrl::OnGetInfo)
	EVT_LIST_ITEM_SELECTED(ID_LIST_CTRL, FileListCtrl::OnSelected)
	EVT_LIST_KEY_DOWN(ID_LIST_CTRL, FileListCtrl::OnListKeyDown)
	EVT_LIST_ITEM_ACTIVATED(ID_LIST_CTRL, FileListCtrl::OnActivated)
	EVT_LIST_COL_CLICK(ID_LIST_CTRL,FileListCtrl::OnColClick)
	EVT_CHAR(FileListCtrl::OnChar)
END_EVENT_TABLE()

FileListCtrl::FileListCtrl(wxWindow* parent,const wxWindowID id,const wxPoint& pos,
				const wxSize& size,long style)
				: wxListCtrl(parent, id, pos, size, style)
{;}

void
FileListCtrl::SetColumnImage(int col, int image)
{
	wxListItem item;
	item.SetMask(wxLIST_MASK_IMAGE);
	item.SetImage(image);
	SetColumn(col, item);
}

int wxCALLBACK MyCompareFunction(long item1, long item2, long sortData)
{
    // inverse the order
    if (item1 < item2)
    	return -1;
    if (item1 > item2)
    	return 1;

    return 0;
}

void
FileListCtrl::OnColClick(wxListEvent& event)
{
    int col = event.GetColumn();
    SortItems(MyCompareFunction, col);
}


void
FileListCtrl::OnDeleteItem(wxListEvent& WXUNUSED(event))
{;}

void
FileListCtrl::OnDeleteAllItems(wxListEvent& WXUNUSED(event))
{;}


void FileListCtrl::OnSelected(wxListEvent& event)
{
//    LogEvent(event, wxT("OnSelected"));

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
FileListCtrl::OnActivated(wxListEvent& WXUNUSED(event))
{;}

void
FileListCtrl::OnListKeyDown(wxListEvent& event)
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

                wxLogMessage(wxT("Focusing item %ld"), item);

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

                    wxLogMessage(wxT("Item %ld deleted"), item);

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
//            LogEvent(event, wxT("OnListKeyDown"));

            event.Skip();
    }
}

void FileListCtrl::OnChar(wxKeyEvent& event)
{
    wxLogMessage(wxT("Got char event."));

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




