/* $Id: imframe.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file imframe.h
@brief Instant Message Window Definition
*/
#ifndef TURNSTILE_IM_FRAME_H
#define TURNSTILE_IM_FRAME_H

#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/sizer.h>

class Buddy;
class wxSplitterWindow;

/** @class ImFrame
@brief File Transfer Window
*/
class ImFrame: public wxFrame
{
	friend class Buddy;

public:
	/*enum	{
			IM_TYPE_BUDDY,
			IM_TYPE_SYSTEM,
			IM_TYPE_YES_TYPING,
			IM_TYPE_NO_TYPING
		};*/


   	ImFrame(Buddy* buddy);

	/** @defgroup filewindow_menu_callbacks File menu callbacks.
	@{*/
	void OnSave(wxCommandEvent& event);
	void OnHide(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnMenuClose(wxCommandEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnSendIm(wxCommandEvent& event);
	void OnButtons(wxCommandEvent& event);
	/**@}*/

	void ReceiveIm(int type, wxString text);

	void Sent();

	void ConnectionFailed();
	void ConnectionLost();
	void ConnectionMade();

	virtual ~ImFrame();

private:
	/** ImFrame Control Ids */
	enum	{
			MENU_SAVE = 10000,
			MENU_LOAD,
			MENU_HIDE,
			MENU_ABOUT,
			MENU_HELP,
			MENU_CLOSE,
			TEXT_IN,
			TEXT_OUT,
			ID_SEND_BTN,
			ID_HIDE_BTN,
			ID_CLOSE_BTN
		};


	void SendIm();

	Buddy* buddy_;

	wxTextCtrl* textOutgoing_;
	wxTextCtrl* textIncomming_;
	wxScrolledWindow* top_;
	wxScrolledWindow* bottom_;
    	wxSplitterWindow* splitter_;

     	DECLARE_EVENT_TABLE()
};

/** @class SplitterWindow
*/
class SplitterWindow : public wxSplitterWindow
{
public:
	SplitterWindow(wxFrame *parent);
	void OnDClick(wxSplitterEvent& event);
	
private:
	wxFrame* parent_;
	
	DECLARE_EVENT_TABLE()
};

#endif
