/* $Id: findbuddyframe.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file findbuddyframe.h
@brief Find Buddy Frame Definition
*/
#ifndef TURNSTILE_FIND_BUDDY_FRAME_H
#define TURNSTILE_FIND_BUDDY_FRAME_H

#include <wx/wx.h>
//#include <wx/imaglist.h>
#include <wx/listctrl.h>
//#include <wx/artprov.h>

class MainFrame;
class ShareFile;
class LookupResponse;

class BuddyListCtrl: public wxListCtrl
{
public:
	BuddyListCtrl(wxWindow *parent,const wxWindowID id,const wxPoint& pos,
				const wxSize& size,long style);

	void OnDeleteItem(wxListEvent& event);
	void OnDeleteAllItems(wxListEvent& event);
	//void OnGetInfo(wxListEvent& event);
	void OnSelected(wxListEvent& event);
	void OnListKeyDown(wxListEvent& event);
	void OnActivated(wxListEvent& event);
	void OnChar(wxKeyEvent& event);
	void OnColClick(wxListEvent& event);

private:

	void SetColumnImage(int col, int image);

    	DECLARE_EVENT_TABLE()
};

/** @class FindBuddyFrame
@brief Find Buddy Frame
*/
class FindBuddyFrame: public wxFrame
{
public:

    	FindBuddyFrame(MainFrame* parent);
    	virtual ~FindBuddyFrame();

	/** @defgroup filewindow_menu_callbacks File menu callbacks.
	@{*/
	void OnSize(wxSizeEvent& event);
	void OnMenuClose(wxCommandEvent& event);
	void OnMenuAbout(wxCommandEvent& event);
	void OnClose(wxCloseEvent& event);
 //   	void OnNotebook(wxNotebookEvent& event);
	void OnButtons(wxCommandEvent& event);
	/**@}*/

	/** Handle the search event. */
	void OnConnectionEvent(wxCommandEvent& event);

	/** Timer event to refresh a the upload or download list
	(whichever is in view).
	*/
	//void OnRefreshList(wxTimerEvent& event);

private:

	/** Add file from a query to the search list.
	*/
	void AddBuddies(LookupResponse* response);

	/** Add a download to the list.
	*/
	//void AddDownload(ShareFile* file);

	/** Add an upload to the list.
	*/
	//void AddUpload(ShareFile* file);


	/** Resize everything to fit the window right.
	*/
	//void Resize();

	//void RefreshList();

	/** Create my own parent pointer because if you use the internal
    	one then child windows will always be on top of the parent
    	and that's just not necessary */
	MainFrame* parent_;

	BuddyListCtrl* searchList_;
	wxTextCtrl* searchTxt_;
	wxButton* searchBtn_;

	//FileNotebook* notebook_;
	//wxImageList* tabImageList_;
	//wxTimer* refreshTimer_;

   	DECLARE_EVENT_TABLE()
};

#endif
