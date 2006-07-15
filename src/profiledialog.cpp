/* $Id: profiledialog.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

#include "config.h"
#include "eraser.h"
#include "profiledialog.h"
#include "buddy.h"

#include "wx/dialog.h"
#include "wx/html/htmlwin.h"

#include "../pixmaps/profiledialog.xpm"

#ifdef __WXMSW__
#define DLOG_HEIGHT 	460
#else
#define DLOG_HEIGHT 	410
#endif
#define DLOG_WIDTH 	400

enum	{
		ID_BTN_CLOSE=10001
	};

//#define PROFILE_HTML	"<html><body bgcolor=#EEEEEE><p><font size=1>Alias: <b>%s</b><br>Public Key: <b>%s</b><br>Online Since: <b>%s</b><br>Status: <b>%s</b><hr noshade size=1>%s</font></p></body></html>"
#define PROFILE_HTML	"<html><body bgcolor=#EEEEEE><p><font size=1>Alias: <b>%s</b><br>Public Key: <b>%s</b><br>Status: <b>%s</b><hr noshade size=1>%s</font></p></body></html>"

BEGIN_EVENT_TABLE(ProfileDialog, wxDialog)
	EVT_BUTTON(ID_BTN_CLOSE,ProfileDialog::OnButtons)
END_EVENT_TABLE()


ProfileDialog::ProfileDialog(wxWindow* parent, Buddy* buddy)
:  wxDialog(parent, -1, "Turnstile - View Profile",
         wxDefaultPosition, wxSize(DLOG_WIDTH, DLOG_HEIGHT)),buddy_(buddy)
{
	//wxBusyCursor wait;

	SetIcon(wxIcon(profiledialog));

	Eraser* panel = new Eraser(this, -1,
				wxPoint(0, 0),
				wxSize(DLOG_WIDTH, DLOG_HEIGHT));

	wxStaticText* title = new wxStaticText(panel, -1, "Buddy Profile", wxPoint(10, 10), wxSize(380, 50)/*,wxST_NO_AUTORESIZE*/);
	wxFont font(20, wxDEFAULT, wxNORMAL, wxBOLD);
	title->SetFont(font);

	/*
	Get the text we want to display for the status.
	*/
	wxString statusText;
	if (buddy_->GetStatus(Buddy::STATUS_ONLINE))
		statusText = "Online";
	else
		statusText = "Offline";

	if (buddy_->GetStatus(Buddy::STATUS_CONNECTED))
		statusText += " (connected)";
	else
		statusText += " (not connected)";

	wxString htmlString;
	htmlString = wxString().Format(wxT(PROFILE_HTML),buddy_->Get(Buddy::DATA_ALIAS).c_str(),buddy_->Get(Buddy::DATA_PUBLICKEY).c_str(),/*"time",*/statusText.c_str(),buddy_->Get(Buddy::DATA_PROFILE).c_str());

	wxHtmlWindow* html = new wxHtmlWindow(panel, -1,
						wxPoint(10, 60),
						wxSize(380, DLOG_HEIGHT-110));
						//wxPoint(10, 210),
						//wxSize(380, 150));
	html->SetPage(htmlString);

	wxButton* ok = new wxButton(panel, ID_BTN_CLOSE,
				"Close",
				wxPoint(10, DLOG_HEIGHT-40),
				wxSize(380, 28));
				//wxPoint(150, 375),
				//wxSize(100, 28));

	Centre(wxBOTH|wxCENTRE_ON_SCREEN);

#ifndef TARGET_CARBON
   	ok->SetDefault();
   	ok->SetFocus();
#endif


}

void 
ProfileDialog::OnButtons(wxCommandEvent& event)
{
    	switch (event.GetId())
    	{
	case ID_BTN_CLOSE:
	{
		EndModal(wxID_YES);
		break;
	}
    	}
}

ProfileDialog::~ProfileDialog()
{;}
