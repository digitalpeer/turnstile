/* $Id: buddytreectrl.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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
 * but WITHOUT FANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/** @file buddytreectrl.cpp
@brief Buddylist Implimentation
*/
#include "buddytreectrl.h"
#include "app.h"
#include "prefs.h"
#include "utils.h"
#include "buddy.h"
#include "network.h"
#include "profiledialog.h"

using namespace std;

#include "../pixmaps/buddy_online.xpm"
#include "../pixmaps/buddy_offline.xpm"
#include "../pixmaps/buddy_connected.xpm"
#include "../pixmaps/buddy_group.xpm"
#include "../pixmaps/menu_buddy_info.xpm"
#include "../pixmaps/menu_buddy_im.xpm"
#include "../pixmaps/menu_buddy_remove.xpm"
#include "../pixmaps/menu_buddy_log.xpm"
#include "../pixmaps/menu_buddy_ban.xpm"
#include "../pixmaps/menu_buddy_connect.xpm"

#define ICON_DIMENS 16

enum	{
		ICON_BUDDY_GROUP,
		ICON_BUDDY_ONLINE,
		ICON_BUDDY_CONNECTED,
		ICON_BUDDY_OFFLINE,
		//ICON_BUDDY_GROUP,
		//
		ICONS_SIZE
	};

BEGIN_EVENT_TABLE(BuddyTreeCtrl, wxTreeCtrl)
    	EVT_TREE_BEGIN_DRAG(ID_TREE_CTRL, BuddyTreeCtrl::OnBeginDrag)
    	EVT_TREE_END_DRAG(ID_TREE_CTRL, BuddyTreeCtrl::OnEndDrag)
    	EVT_TREE_ITEM_ACTIVATED(ID_TREE_CTRL, BuddyTreeCtrl::OnItemActivated)
    	EVT_TREE_ITEM_RIGHT_CLICK(ID_TREE_CTRL, BuddyTreeCtrl::OnItemRightClick)
    	EVT_LEFT_DCLICK(BuddyTreeCtrl::OnLMouseDClick)
    	EVT_MENU(BUDDY_POPUP_GETINFO,BuddyTreeCtrl::OnGetBuddyInfo)
	EVT_MENU(BUDDY_POPUP_IM,BuddyTreeCtrl::OnOpenBuddyWindow)
    	EVT_MENU(BUDDY_POPUP_VIEWLOG,BuddyTreeCtrl::OnViewBuddyLog)
    	EVT_MENU(BUDDY_POPUP_REMOVE,BuddyTreeCtrl::OnRemoveBuddy)
	EVT_MENU(BUDDY_POPUP_BAN,BuddyTreeCtrl::OnBanBuddy)
	EVT_MENU(BUDDY_POPUP_CONNECT,BuddyTreeCtrl::OnConnect)
	EVT_MENU(GROUP_POPUP_REMOVE,BuddyTreeCtrl::OnRemoveGroup)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(BuddyTreeCtrl, wxTreeCtrl)

BuddyTreeCtrl::BuddyTreeCtrl(wxWindow *parent, const wxWindowID id,const wxPoint& pos,
				const wxSize& size,long style)
          : wxTreeCtrl(parent, id, pos, size, style)
{
    	CreateImageList();
}

void BuddyTreeCtrl::CreateImageList()
{
	// small icon image list
	wxImageList *images = new wxImageList(ICON_DIMENS, ICON_DIMENS, TRUE);

	images->Add(wxIcon(buddy_group));
	images->Add(wxIcon(buddy_online));
	images->Add(wxIcon(buddy_connected));
	images->Add(wxIcon(buddy_offline));

	AssignImageList(images);
}

int
BuddyTreeCtrl::OnCompareItems(const wxTreeItemId& item1,
                               const wxTreeItemId& item2)
{
	return wxTreeCtrl::OnCompareItems(item1, item2);
}

void
BuddyTreeCtrl::AddBuddy(Buddy* buddy, wxString group, bool save)
{
	wxTreeItemId groupId = FindGroup(group);

	if (!groupId.IsOk())
	{
	   LogError(wxT("Invalid group id when adding buddy."));
		return;
	}

	wxTreeItemId id = AppendItem(groupId, buddy->Get(Buddy::DATA_ALIAS), ICON_BUDDY_OFFLINE,
				-1, buddy);

	Expand(groupId);

	SetItemTextColour(id, wxColour(180,180,180));

	if (save)
	{
		Prefs::Instance()->Save();
	}
}

wxTreeItemId
BuddyTreeCtrl::AddGroup(wxString& name, bool save)
{
	wxTreeItemId id = AppendItem(GetRootItem(), name, -1/*ICON_BUDDY_GROUP*/, -1, 0);
	//SetItemImage(id, -1, wxTreeItemIcon_Normal);
	//SetItemImage(id, -1, wxTreeItemIcon_Selected);
	SetItemBold(id);

	if (save)
	{
		Prefs::Instance()->Save();
	}

	return id;
}

void
BuddyTreeCtrl::RefreshBuddies()
{
	wxTreeItemIdValue groupCookie;
	wxTreeItemId group = GetFirstChild(GetRootItem(),groupCookie);

	do
	{
		if (ItemHasChildren(group))
		{
			wxTreeItemIdValue cookie;
			wxTreeItemId id = GetFirstChild(group, cookie);

			do
			{
				Buddy* buddy = (Buddy*)GetItemData(id);
				if (buddy != 0)
				{
					if (!buddy->GetStatus(Buddy::STATUS_ONLINE))
					{
						SetItemTextColour(id, wxColour(192,192,192));
						SetItemImage(id, ICON_BUDDY_OFFLINE);
						SetItemImage(id, ICON_BUDDY_OFFLINE, wxTreeItemIcon_Selected);
					}
					else
					{
						SetItemTextColour(id, wxColour(0,0,0));
						SetItemImage(id, ICON_BUDDY_ONLINE);
						SetItemImage(id, ICON_BUDDY_ONLINE, wxTreeItemIcon_Selected);
					}

					if (buddy->GetStatus(Buddy::STATUS_CONNECTED))
					{
						SetItemTextColour(id, wxColour(0,0,0));
						SetItemImage(id, ICON_BUDDY_CONNECTED);
						SetItemImage(id, ICON_BUDDY_CONNECTED, wxTreeItemIcon_Selected);
					}

				}
			}
			while ((id = GetNextChild(group, cookie)) > 0);
		}
	}
	while ((group = GetNextChild(GetRootItem(),groupCookie)) > 0);

	//Refresh();
}

void
BuddyTreeCtrl::SetBuddyData(const wxString& uid, const wxString& alias, const wxString& profile,const wxString& addr)
{
	wxTreeItemId id = FindBuddy(uid);
	if (id > 0)
	{
		// update alias
		SetItemText(id,alias);

		// save other info
		Buddy* buddy = (Buddy*)GetItemData(id);
		buddy->Set(Buddy::DATA_ALIAS, alias);
		buddy->Set(Buddy::DATA_PROFILE, profile);
		buddy->Set(Buddy::DATA_ADDRESS, addr);

		//LogDebug(wxString().Format("Updated data for buddy %s.",GetItemText(id).c_str()));
	}
	else
	{
	   LogError(wxT("Got a data update for a buddy we do not have."));
	}
}

wxTreeItemId
BuddyTreeCtrl::FindBuddy(wxString publicKey)
{
	wxTreeItemIdValue groupCookie;
	wxTreeItemId group = GetFirstChild(GetRootItem(),groupCookie);

	do
	{
		if (ItemHasChildren(group))
		{
			wxTreeItemIdValue cookie;
			wxTreeItemId id = GetFirstChild(group, cookie);

			do
			{
				Buddy* buddy = (Buddy*)GetItemData(id);
				if (buddy != 0)
				{
					if (buddy->Get(Buddy::DATA_PUBLICKEY) == publicKey)
						return id;
				}
			}
			while ((id = GetNextChild(group, cookie)) > 0);
		}
	}
	while ((group = GetNextChild(GetRootItem(),groupCookie)) > 0);

	return wxTreeItemId();
}

Buddy*
BuddyTreeCtrl::GetBuddy(wxString publicKey)
{
	wxTreeItemId id = FindBuddy(publicKey);
	if (id > 0)
	{
		Buddy* buddy = (Buddy*)GetItemData(id);
		if (buddy != 0)
		{
			if (buddy->Get(Buddy::DATA_PUBLICKEY) == publicKey)
				return buddy;
		}
	}

	return 0;
}

wxTreeItemId
BuddyTreeCtrl::FindGroup(wxString alias)
{
	wxTreeItemIdValue groupCookie;
	wxTreeItemId group = GetFirstChild(GetRootItem(),groupCookie);

	do
	{
		if (GetItemText(group) == alias)
			return group;
	}
	while ((group = GetNextChild(GetRootItem(),groupCookie)) > 0);

	return wxTreeItemIdValue();
}

void
BuddyTreeCtrl::OnBeginDrag(wxTreeEvent& event)
{
	// no dragging groups or the root
	/** @bug not all groups have children */
	if (event.GetItem() != GetRootItem() && GetItemParent(event.GetItem()) != GetRootItem())
	{
		draggedItem_ = event.GetItem();
		event.Allow();
	}
	else
	{
		draggedItem_ = wxTreeItemId();
		event.Skip();
	}
}

void BuddyTreeCtrl::OnEndDrag(wxTreeEvent& event)
{
	if (draggedItem_ == wxTreeItemId())
		return;

	wxTreeItemId itemSrc = draggedItem_;
	wxTreeItemId itemDst = event.GetItem();
	//draggedItem_ = (wxTreeItemId)0l;

	// where to copy the item?
	if (itemDst.IsOk())
	{
		// copy to the parent then
		itemDst = GetItemParent(itemDst);
	}
	else
	{
		// can't drop here
		return;
	}

	wxString text = GetItemText(itemSrc);
	int image = GetItemImage(itemSrc);

	AppendItem(itemDst,text,image,image,GetItemData(itemSrc));

	Delete(itemSrc);

	Prefs::Instance()->Save();
}

void BuddyTreeCtrl::OnItemActivated(wxTreeEvent& event)
{
	// show some info about this item
	/*wxTreeItemId itemId = event.GetItem();
	Buddy* item = (Buddy*)GetItemData(itemId);

	if (item != (Buddy*)0)
	{
		item->ShowProfile();
	}*/
}

void BuddyTreeCtrl::OnItemRightClick(wxTreeEvent& event)
{
	// select the item we just clicked on
	SelectItem(event.GetItem());

	// show the popup menu
	if (GetItemParent(event.GetItem()) != GetRootItem())
		ShowBuddyMenu(event.GetItem(), event.GetPoint());
	else
		ShowGroupMenu(event.GetItem(), event.GetPoint());
}

void BuddyTreeCtrl::OnLMouseDClick(wxMouseEvent& event)
{
    	wxTreeItemId id = HitTest(event.GetPosition());

	if (id == GetSelection() && id.IsOk())
	{
		Buddy* item = (Buddy*)GetItemData(id);
		if (item != 0)
		{
			//if (item->GetStatus(Buddy::STATUS_ONLINE))
				item->OpenWindow();
		}
	}
}

void BuddyTreeCtrl::ShowBuddyMenu(wxTreeItemId id, const wxPoint& pt)
{
	if (id.IsOk())
	{
		Buddy* buddy = (Buddy*)GetItemData(id);

#ifdef WIN32
		wxMenu menu(wxT("Buddy ") + buddy->Get(Buddy::DATA_ALIAS));
#else
		wxMenu menu;
#endif

		wxMenuItem* item;

		item = new wxMenuItem(&menu, BUDDY_POPUP_GETINFO, wxT("View Profile"));
		item->SetBitmap(menu_buddy_info);
		menu.Append(item);
		if (buddy->GetStatus(Buddy::STATUS_ONLINE))
			menu.Enable(BUDDY_POPUP_GETINFO,TRUE);
		else
			menu.Enable(BUDDY_POPUP_GETINFO,FALSE);

		item = new wxMenuItem(&menu, BUDDY_POPUP_IM, wxT("Instant Message"));
		item->SetBitmap(menu_buddy_im);
		menu.Append(item);
		if (buddy->GetStatus(Buddy::STATUS_ONLINE))
			menu.Enable(BUDDY_POPUP_IM,TRUE);
		else
			menu.Enable(BUDDY_POPUP_IM,FALSE);

		item = new wxMenuItem(&menu, BUDDY_POPUP_CONNECT, wxT("Connect"));
		item->SetBitmap(menu_buddy_connect);
		if (buddy->GetStatus(Buddy::STATUS_CONNECTED))
		   item->SetText(wxT("Disconnect"));
		menu.Append(item);
		if (!Network::Instance()->GetStatus(Network::STATUS_ALIVE) ||
			!buddy->GetStatus(Buddy::STATUS_ONLINE))
			menu.Enable(BUDDY_POPUP_CONNECT,FALSE);


		//item = new wxMenuItem(&menu, BUDDY_POPUP_VIEWLOG, wxT("View Log"));
	      	//item->SetBitmap(menu_buddy_log);
		//menu.Append(item);

		menu.AppendSeparator();

		item = new wxMenuItem(&menu, BUDDY_POPUP_REMOVE, wxT("Remove"));
	      	item->SetBitmap(menu_buddy_remove);
		menu.Append(item);

		//item = new wxMenuItem(&menu, BUDDY_POPUP_BAN, wxT("Ban All Interaction"));
	      	//item->SetBitmap(menu_buddy_ban);
		//menu.Append(item);

		// show it!
		PopupMenu(&menu, pt);
	}
}

void BuddyTreeCtrl::ShowGroupMenu(wxTreeItemId id, const wxPoint& pt)
{
	if (id.IsOk())
	{
		wxMenu menu(wxT(""));

		wxMenuItem* item = new wxMenuItem(&menu, GROUP_POPUP_REMOVE, wxT("Remove"));
	      	item->SetBitmap(menu_buddy_remove);
		menu.Append(item);

		// show it!
		PopupMenu(&menu, pt);
	}
}

void BuddyTreeCtrl::OnGetBuddyInfo(wxCommandEvent& WXUNUSED(event))
{
	wxTreeItemId id = GetSelection();

	Buddy* buddy = (Buddy*)GetItemData(id);
	if (buddy != 0)
	{
		ProfileDialog it(m_parent,buddy);
   		it.ShowModal();
	}
}

void BuddyTreeCtrl::OnOpenBuddyWindow(wxCommandEvent& WXUNUSED(event))
{
	wxTreeItemId id = GetSelection();

	Buddy* item = (Buddy*)GetItemData(id);
	if (item != 0)
	{
		if (item->GetStatus(Buddy::STATUS_ONLINE))
			item->OpenWindow();
	}
}

void BuddyTreeCtrl::OnViewBuddyLog(wxCommandEvent& WXUNUSED(event))
{;}

void BuddyTreeCtrl::OnRemoveBuddy(wxCommandEvent& WXUNUSED(event))
{
	Delete(GetSelection());
	Prefs::Instance()->Save();
}

void BuddyTreeCtrl::OnBanBuddy(wxCommandEvent& WXUNUSED(event))
{;}

void BuddyTreeCtrl::OnConnect(wxCommandEvent& WXUNUSED(event))
{
	wxTreeItemId id = GetSelection();

	Buddy* item = (Buddy*)GetItemData(id);
	if (item != 0)
	{
		if (item->GetStatus(Buddy::STATUS_ONLINE))
		{
			if (item->GetStatus(Buddy::STATUS_CONNECTED))
				item->Disconnect();
			else
				item->Connect();
		}
	}
}

void BuddyTreeCtrl::OnRemoveGroup(wxCommandEvent& WXUNUSED(event))
{
	DeleteChildren(GetSelection());
	Delete(GetSelection());
	Prefs::Instance()->Save();
}

void
BuddyTreeCtrl::GetGroupArray(vector<wxString>& groups)
{
	wxTreeItemIdValue groupCookie;
	wxTreeItemId group = GetFirstChild(GetRootItem(),groupCookie);
	do
	{
		groups.push_back(GetItemText(group));
		//LogDebug(GetItemText(group));
	}
	while ((group = GetNextChild(GetRootItem(),groupCookie)) > 0);
}

void BuddyTreeCtrl::DeAllocateList(const wxTreeItemId& idParent, wxTreeItemIdValue cookie)
{
	wxTreeItemId id;

	if( cookie == wxTreeItemIdValue() )
		id = GetFirstChild(idParent, cookie);
	else
		id = GetNextChild(idParent, cookie);

	if(id <= 0)
		return;

	Buddy* item = (Buddy*)GetItemData(id);
	delete item;

	if (ItemHasChildren(id))
	   DeAllocateList(id,wxTreeItemIdValue());

	DeAllocateList(idParent, cookie);
}

BuddyTreeCtrl::~BuddyTreeCtrl()
{
	// TODO this needs to be done but it randomly? crashes
	//DeAllocateList(GetRootItem(),-1);
}



/*void LogKeyEvent(const wxChar *name, const wxKeyEvent& event)
{
    wxString key;
    long keycode = event.KeyCode();
    {
        switch ( keycode )
        {
            case WXK_BACK: key = wxT("BACK"); break;
            case WXK_TAB: key = wxT("TAB"); break;
            case WXK_RETURN: key = wxT("RETURN"); break;
            case WXK_ESCAPE: key = wxT("ESCAPE"); break;
            case WXK_SPACE: key = wxT("SPACE"); break;
            case WXK_DELETE: key = wxT("DELETE"); break;
            case WXK_START: key = wxT("START"); break;
            case WXK_LBUTTON: key = wxT("LBUTTON"); break;
            case WXK_RBUTTON: key = wxT("RBUTTON"); break;
            case WXK_CANCEL: key = wxT("CANCEL"); break;
            case WXK_MBUTTON: key = wxT("MBUTTON"); break;
            case WXK_CLEAR: key = wxT("CLEAR"); break;
            case WXK_SHIFT: key = wxT("SHIFT"); break;
            case WXK_ALT: key = wxT("ALT"); break;
            case WXK_CONTROL: key = wxT("CONTROL"); break;
            case WXK_MENU: key = wxT("MENU"); break;
            case WXK_PAUSE: key = wxT("PAUSE"); break;
            case WXK_CAPITAL: key = wxT("CAPITAL"); break;
            case WXK_PRIOR: key = wxT("PRIOR"); break;
            case WXK_NEXT: key = wxT("NEXT"); break;
            case WXK_END: key = wxT("END"); break;
            case WXK_HOME: key = wxT("HOME"); break;
            case WXK_LEFT: key = wxT("LEFT"); break;
            case WXK_UP: key = wxT("UP"); break;
            case WXK_RIGHT: key = wxT("RIGHT"); break;
            case WXK_DOWN: key = wxT("DOWN"); break;
            case WXK_SELECT: key = wxT("SELECT"); break;
            case WXK_PRINT: key = wxT("PRINT"); break;
            case WXK_EXECUTE: key = wxT("EXECUTE"); break;
            case WXK_SNAPSHOT: key = wxT("SNAPSHOT"); break;
            case WXK_INSERT: key = wxT("INSERT"); break;
            case WXK_HELP: key = wxT("HELP"); break;
            case WXK_NUMPAD0: key = wxT("NUMPAD0"); break;
            case WXK_NUMPAD1: key = wxT("NUMPAD1"); break;
            case WXK_NUMPAD2: key = wxT("NUMPAD2"); break;
            case WXK_NUMPAD3: key = wxT("NUMPAD3"); break;
            case WXK_NUMPAD4: key = wxT("NUMPAD4"); break;
            case WXK_NUMPAD5: key = wxT("NUMPAD5"); break;
            case WXK_NUMPAD6: key = wxT("NUMPAD6"); break;
            case WXK_NUMPAD7: key = wxT("NUMPAD7"); break;
            case WXK_NUMPAD8: key = wxT("NUMPAD8"); break;
            case WXK_NUMPAD9: key = wxT("NUMPAD9"); break;
            case WXK_MULTIPLY: key = wxT("MULTIPLY"); break;
            case WXK_ADD: key = wxT("ADD"); break;
            case WXK_SEPARATOR: key = wxT("SEPARATOR"); break;
            case WXK_SUBTRACT: key = wxT("SUBTRACT"); break;
            case WXK_DECIMAL: key = wxT("DECIMAL"); break;
            case WXK_DIVIDE: key = wxT("DIVIDE"); break;
            case WXK_F1: key = wxT("F1"); break;
            case WXK_F2: key = wxT("F2"); break;
            case WXK_F3: key = wxT("F3"); break;
            case WXK_F4: key = wxT("F4"); break;
            case WXK_F5: key = wxT("F5"); break;
            case WXK_F6: key = wxT("F6"); break;
            case WXK_F7: key = wxT("F7"); break;
            case WXK_F8: key = wxT("F8"); break;
            case WXK_F9: key = wxT("F9"); break;
            case WXK_F10: key = wxT("F10"); break;
            case WXK_F11: key = wxT("F11"); break;
            case WXK_F12: key = wxT("F12"); break;
            case WXK_F13: key = wxT("F13"); break;
            case WXK_F14: key = wxT("F14"); break;
            case WXK_F15: key = wxT("F15"); break;
            case WXK_F16: key = wxT("F16"); break;
            case WXK_F17: key = wxT("F17"); break;
            case WXK_F18: key = wxT("F18"); break;
            case WXK_F19: key = wxT("F19"); break;
            case WXK_F20: key = wxT("F20"); break;
            case WXK_F21: key = wxT("F21"); break;
            case WXK_F22: key = wxT("F22"); break;
            case WXK_F23: key = wxT("F23"); break;
            case WXK_F24: key = wxT("F24"); break;
            case WXK_NUMLOCK: key = wxT("NUMLOCK"); break;
            case WXK_SCROLL: key = wxT("SCROLL"); break;
            case WXK_PAGEUP: key = wxT("PAGEUP"); break;
            case WXK_PAGEDOWN: key = wxT("PAGEDOWN"); break;
            case WXK_NUMPAD_SPACE: key = wxT("NUMPAD_SPACE"); break;
            case WXK_NUMPAD_TAB: key = wxT("NUMPAD_TAB"); break;
            case WXK_NUMPAD_ENTER: key = wxT("NUMPAD_ENTER"); break;
            case WXK_NUMPAD_F1: key = wxT("NUMPAD_F1"); break;
            case WXK_NUMPAD_F2: key = wxT("NUMPAD_F2"); break;
            case WXK_NUMPAD_F3: key = wxT("NUMPAD_F3"); break;
            case WXK_NUMPAD_F4: key = wxT("NUMPAD_F4"); break;
            case WXK_NUMPAD_HOME: key = wxT("NUMPAD_HOME"); break;
            case WXK_NUMPAD_LEFT: key = wxT("NUMPAD_LEFT"); break;
            case WXK_NUMPAD_UP: key = wxT("NUMPAD_UP"); break;
            case WXK_NUMPAD_RIGHT: key = wxT("NUMPAD_RIGHT"); break;
            case WXK_NUMPAD_DOWN: key = wxT("NUMPAD_DOWN"); break;
            case WXK_NUMPAD_PRIOR: key = wxT("NUMPAD_PRIOR"); break;
            case WXK_NUMPAD_PAGEUP: key = wxT("NUMPAD_PAGEUP"); break;
            case WXK_NUMPAD_PAGEDOWN: key = wxT("NUMPAD_PAGEDOWN"); break;
            case WXK_NUMPAD_END: key = wxT("NUMPAD_END"); break;
            case WXK_NUMPAD_BEGIN: key = wxT("NUMPAD_BEGIN"); break;
            case WXK_NUMPAD_INSERT: key = wxT("NUMPAD_INSERT"); break;
            case WXK_NUMPAD_DELETE: key = wxT("NUMPAD_DELETE"); break;
            case WXK_NUMPAD_EQUAL: key = wxT("NUMPAD_EQUAL"); break;
            case WXK_NUMPAD_MULTIPLY: key = wxT("NUMPAD_MULTIPLY"); break;
            case WXK_NUMPAD_ADD: key = wxT("NUMPAD_ADD"); break;
            case WXK_NUMPAD_SEPARATOR: key = wxT("NUMPAD_SEPARATOR"); break;
            case WXK_NUMPAD_SUBTRACT: key = wxT("NUMPAD_SUBTRACT"); break;
            case WXK_NUMPAD_DECIMAL: key = wxT("NUMPAD_DECIMAL"); break;

            default:
            {
               if ( wxIsprint((int)keycode) )
                   key.Printf(wxT("'%c'"), (char)keycode);
               else if ( keycode > 0 && keycode < 27 )
                   key.Printf(_("Ctrl-%c"), wxT('A') + keycode - 1);
               else
                   key.Printf(wxT("unknown (%ld)"), keycode);
            }
        }
    }

  //  LogDebug( wxT("%s event: %s (flags = %c%c%c%c)"),
    //              name,
      //            key.c_str(),
        //          event.ControlDown() ? wxT('C') : wxT('-'),
          //        event.AltDown() ? wxT('A') : wxT('-'),
            //      event.ShiftDown() ? wxT('S') : wxT('-'),
              //    event.MetaDown() ? wxT('M') : wxT('-'));
}

void BuddyTreeCtrl::OnTreeKeyDown(wxTreeEvent& event)
{
    LogKeyEvent(wxT("Tree key down "), event.GetKeyEvent());

    event.Skip();
}
*/



