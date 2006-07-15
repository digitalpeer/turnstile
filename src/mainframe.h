/* $Id: mainframe.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

#ifndef TURNSTILE_MAIN_FRAME_H
#define TURNSTILE_MAIN_FRAME_H

#include <wx/wx.h>

class Network;
class BuddyTreeCtrl;
class DebugFrame;
class FileFrame;
class PrefFrame;
class FindBuddyFrame;
class Buddy;
class wxProgressDialog;
class wxTimer;
class wxTimerEvent;
class wxStaticBitmap;

enum
{
	ID_MAINFRAME=7500
};

/** @class MainFrame
*/
class MainFrame: public wxFrame
{
public:
   	MainFrame();
    
    	/** @defgroup mainwindow_menu_callbacks Main menu callbacks.
    	@{*/
	void OnFindBuddy(wxCommandEvent& event);
    	void OnAddBuddy(wxCommandEvent& event);
   void AddBuddy(Buddy* buddy, wxString group = wxT(""));
    	void OnAddGroup(wxCommandEvent& event);
    	void OnSignOnOff(wxCommandEvent& event);
    	void OnExit(wxCommandEvent& event);
    	void OnShowFileFrame(wxCommandEvent& event);
    	void OnShowDebugFrame(wxCommandEvent& event);
    	void OnShowPrefFrame(wxCommandEvent& event);
	void OnShowFindBuddyFrame(wxCommandEvent& event);
    	//void OnHelp(wxCommandEvent& event);
    	void OnAbout(wxCommandEvent& event);
	void OnCloseDebugFrame();
	void OnCloseFindBuddyFrame();
	void OnCloseFileFrame();
	void OnClosePrefFrame();
	//void OnMenuExit(wxMenuEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnButtons(wxCommandEvent& event);
	void OnSignonTimer(wxTimerEvent& event);


	void OnNetworkEvent(wxCommandEvent& event);

    	/**@}*/
    
    	/** @defgroup mainwindow_frame_callbacks Main Frame Callbacks.
    	@{*/
    	//void OnSize(wxSizeEvent& event);
	/**@}*/

	BuddyTreeCtrl* GetTreeCtrl()
	{
		return treeCtrl_;
	}


	void ShowPrefFrame();

	virtual ~MainFrame();

	void AutoStart();

private:

	void KillSignonDialog();
	void DoSignon();

	/** Pointer to the debug window (when it is open)
	*/
	DebugFrame* debugFrame_;

	//void TellBuddiesBye();

	/** Create the tree with a selected style.
	*/
	//void CreateTree();
	void CreateMenu();

	/** Resize the tree to make it fit the frame.
	*/
	//void ResizeTree();

	/** Buddy list tree. */
	BuddyTreeCtrl* treeCtrl_;

	/** Pointer to the file frame (it always exists but might not
	always be showing) */
	FileFrame* fileFrame_;

	FindBuddyFrame* findBuddyFrame_;

	/** Pointer to the network.
	*/
    	//Network* network_;
	
	//wxStatusBar* statusBar;
	//wxStaticText* statusTxt_;
	
	PrefFrame* prefFrame_;
    
    
    	wxMenu* network_menu;
    	wxMenu* options_menu;
	wxMenu* help_menu;
	wxMenuBar* menu_bar;
	
	//void UpdateStatus();

	wxProgressDialog* signonDialog_;
	wxTimer* signonTimer_;

	wxStaticBitmap* statusImage_;
        
    	DECLARE_EVENT_TABLE()
};

#endif
