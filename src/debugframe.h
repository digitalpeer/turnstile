/* $Id: debugframe.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file debugframe.h Debug Window Definition
*/
#ifndef TURNSTILE_DEBUG_FRAME_H
#define TURNSTILE_DEBUG_FRAME_H

#include <wx/wx.h>

class MainFrame;

/** @class DebugFrame
Debug Window
*/
class DebugFrame: public wxFrame
{
public:

    DebugFrame(MainFrame* parent);
    	
	/** @defgroup debugwindow_menu_callbacks Debug menu callbacks.
	*/
	void OnSize(wxSizeEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnClose(wxCloseEvent& event);
	/**@}*/

	void Log(wxString text);

	~DebugFrame();

private:

	void Resize();

	/** Create my own parent pointer because if you use the internal
    one then on windows the child windows will always be on top of the parent
    and that's just not necessary */
    MainFrame* parent_;
	
	/** Text control to display the debug messages. */
    wxTextCtrl* text_;
	
	wxLog* m_logTargetOld;

    DECLARE_EVENT_TABLE()
};

#endif
