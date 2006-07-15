/* $Id: prefframe.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file preferencesframe.h
@brief Preferences Window Definition
*/
#ifndef TURNSTILE_PREFERENCES_FRAME_H
#define TURNSTILE_PREFERENCES_FRAME_H

#include <wx/wx.h>
#include <wx/imaglist.h>
#include <wx/artprov.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>

class MainFrame;
class Prefs;

/** @class PrefNotebook Preferences Notebook Pages
*/
class PrefNotebook : public wxNotebook
{
public:
    	PrefNotebook(wxWindow *parent, wxWindowID id = -1,
			const wxPoint& pos = wxDefaultPosition,
        		const wxSize& size = wxDefaultSize, long style = 0);

   	void CreatePages();
	bool Save(int id);

	wxTextCtrl* aliasTxt_;
	wxTextCtrl* publicKeyTxt_;
	wxTextCtrl* wrkgrpKeyTxt_;
	wxCheckBox* useFirewallCheck_;

	wxTextCtrl* shareFolderTxt_;
	wxSpinCtrl* maxUploadsSpinCtrl_;
	wxSpinCtrl* maxDownloadsSpinCtrl_;

	wxTextCtrl* portTxt_;
	wxCheckBox* generateCheck_;

	wxCheckBox* showOfflineCheck_;

	void OnNotebook(wxNotebookEvent& event);

private:
 	wxPanel* CreatePageUser();
	wxPanel* CreatePageShare();
	wxPanel* CreatePageBuddylist();
	wxPanel* CreatePageConnection();

	DECLARE_EVENT_TABLE()

};

/** @class PrefFrame Preferences Window
*/
class PrefFrame : public wxFrame
{
public:
    	PrefFrame(MainFrame* parent);

	void OnClose(wxCloseEvent& event);

	void OnButtons(wxCommandEvent& event);

	~PrefFrame();

private:



	MainFrame* parent_;
	//wxPanel* panel_;

	//wxPanel* panel_;
	PrefNotebook* notebook_;
	wxImageList* tabImageList_;

	DECLARE_EVENT_TABLE()
};

#endif
