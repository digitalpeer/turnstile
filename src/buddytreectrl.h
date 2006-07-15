/* $Id: buddytreectrl.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file buddytreectrl.h
@brief Buddylist Definition
@todo use wxTipWindow to create buddy tooltips!
*/
#ifndef TURNSTILE_BUDDYLIST_TREE_H
#define TURNSTILE_BUDDYLIST_TREE_H

#include <wx/wx.h>
#include <wx/colordlg.h>
#include <wx/image.h>
#include <wx/imaglist.h>
#include <wx/treectrl.h>

#include <vector>

class Buddy;

#ifdef WIN32
    // this is not supported by native control
    #define NO_VARIABLE_HEIGHT
#endif

/** Tree ids.
*/
enum	{
		ID_TREE_CTRL=1000,
		BUDDY_POPUP_GETINFO,
		BUDDY_POPUP_IM,
		BUDDY_POPUP_CONNECT,
		BUDDY_POPUP_VIEWLOG,
		BUDDY_POPUP_REMOVE,
		BUDDY_POPUP_BAN,
		GROUP_POPUP_REMOVE
	};

/** @class BuddyTreeCtrl
*/
class BuddyTreeCtrl : public wxTreeCtrl
{
public:

    	BuddyTreeCtrl()
    	{;}

    	BuddyTreeCtrl(wxWindow *parent, const wxWindowID id,
               const wxPoint& pos, const wxSize& size,
               long style);

	virtual ~BuddyTreeCtrl();

	/** @defgroup tree_events Tree callbacks.
	@{*/
    	void OnBeginDrag(wxTreeEvent& event);
    	void OnEndDrag(wxTreeEvent& event);
	//void OnDeleteItem(wxTreeEvent& event);
    	void OnItemActivated(wxTreeEvent& event);
	void OnItemRightClick(wxTreeEvent& event);
	void OnLMouseDClick(wxMouseEvent& event);
	/**@}*/


	/** @defgroup popupmenu_events Buddy/Group popup menu callbacks.
	@{*/
	void OnGetBuddyInfo(wxCommandEvent& event);
	void OnOpenBuddyWindow(wxCommandEvent& event);
    	void OnViewBuddyLog(wxCommandEvent& event);
    	void OnRemoveBuddy(wxCommandEvent& event);
	void OnBanBuddy(wxCommandEvent& event);
	void OnRemoveGroup(wxCommandEvent& event);
	void OnConnect(wxCommandEvent& event);
	/**@}*/


	//void OnIm(wxString uid, wxString data);
	
	//void SetBuddyStatus(wxString uid, int status);

	void SetBuddyData(const wxString& uid, const wxString& alias, const wxString& profile, const wxString& addr);

	/** Create Image List for buddies and groups.
	*/
    	void CreateImageList();

	/** Simply add a group or buddy to the list.  If save is true (default) it
	will re-dump the buddy list to the file.
	*/
	wxTreeItemId AddGroup(wxString& groupName, bool save = true);
	void AddBuddy(Buddy* buddy, wxString group, bool save);

	void ShowBuddyMenu(wxTreeItemId id, const wxPoint& pt);
	void ShowGroupMenu(wxTreeItemId id, const wxPoint& pt);

	void GetGroupArray(std::vector<wxString>& groups);

	Buddy* GetBuddy(wxString publicKey);

	void RefreshBuddies();

protected:

    	virtual int OnCompareItems(const wxTreeItemId& i1, const wxTreeItemId& i2);

private:

	wxTreeItemId FindBuddy(wxString publicKey);
	wxTreeItemId FindGroup(wxString alias);

	void DeAllocateList(const wxTreeItemId& idParent, wxTreeItemIdValue cookie);

	wxTreeItemId lastItem_;                // for OnEnsureVisible()
	wxTreeItemId draggedItem_;             // item being dragged right now

	/**
	@note must use DECLARE_DYNAMIC_CLASS() to overload OnCompareItems() under windows
	*/
	DECLARE_DYNAMIC_CLASS(BuddyTreeCtrl)
	DECLARE_EVENT_TABLE()
};


#endif
