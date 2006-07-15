/* $Id: aboutdialog.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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
#include "aboutdialog.h"
#include "eraser.h"

#include "wx/dialog.h"
#include "wx/html/htmlwin.h"

#include "../pixmaps/aboutframe.xpm"
#include "../pixmaps/about_logo.xpm"

#ifdef __WXMSW__
#define DLOG_HEIGHT 440
#else
#define DLOG_HEIGHT 410
#endif

enum	{
		ID_BTN_CLOSE=10000
	};

BEGIN_EVENT_TABLE(AboutDialog, wxDialog)
	EVT_BUTTON(ID_BTN_CLOSE,AboutDialog::OnButtons)
END_EVENT_TABLE()

AboutDialog::AboutDialog(wxWindow* parent)
:  wxDialog(parent, -1, wxT("Turnstile - About"),
         wxDefaultPosition, wxSize(400, DLOG_HEIGHT))
{
	/*
	VC++ can't have a string over 2048 bytes, so notice the string cats.
	*/
   const char* ABOUT_TEXT = wxT("<html>\
				<body bgcolor=#EEEEEE>\
				<font size=1>\
				<center>\
				<h3>Turnstile 0.0.1 Beta</h3>\
				Second Generation P2P IM and File Sharing Client\
				</center>\
				<p>\
				Turnstile is a peer-to-peer client using a custom protocol named \
				YAWP (Yet Another Wonderful Protocol).  YAWP implements a network \
				providing distribution of data, secure real-time communication with \
				trusted and untrusted users, and scalability.   YAWP aims to conquer \
				problems experienced with existing peer-to-peer protocols and their \
				susceptibility to attack and limited widespread usage.  The YAWP protocol \
				is largely based on the usage of a distributed hash table to effectively \
				locate data quickly in a dynamic network of computers.\
				</p><p>\
				Copyright (C) 2004, J.D. Henderson <www.digitalpeer.com>\
				<br><br>\
				This program is free software; you can redistribute it and/or modify\
				it under the terms of the GNU General Public License as published by\
				the Free Software Foundation; either version 2 of the License, or\
				(at your option) any later version.\
				<br><br>\
				This program is distributed in the hope that it will be useful,\
				but WITHOUT ANY WARRANTY; without even the implied warranty of\
				MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\
				GNU General Public License for more details.\
				<br><br>"
			"You should have received a copy of the GNU General Public License\
				along with this program; if not, write to the Free Software\
				Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\
				<br><br>\
				Turnstile includes a copy of the Crypto++ library:\
				<br><br>\
				Copyright (c) 1995-2003 by Wei Dai.  All rights reserved.<br>\
				The following files are copyrighted by their respective original authors,\
				and their use is subject to additional licenses included in these files.\
				<br><br>\
				mars.cpp - Copyright 1998 Brian Gladman.\
				<br><br>\
				Joan Daemen - 3way.cpp<br>\
				Leonard Janke - cast.cpp, seal.cpp<br>\
				Steve Reid - cast.cpp<br>\
				Phil Karn - des.cpp<br>\
				Michael Paul Johnson - diamond.cpp<br>\
				Andrew M. Kuchling - md2.cpp, md4.cpp<br>\
				Colin Plumb - md5.cpp, md5mac.cpp<br>\
				Seal Woods - rc6.cpp<br>"
			"Chris Morgan - rijndael.cpp<br>\
				Paulo Baretto - rijndael.cpp, skipjack.cpp, square.cpp<br>\
				Richard De Moliner - safer.cpp<br>\
				Matthew Skala - twofish.cpp<br>\
				<br>\
				All other files in the Crypto++ library are placed in the public domain by\
				Wei Dai and other contributors.\
				</p><br><br>\
				<center><b>Turnstile Credits</b></center>\
				<br>\
				<table border=0>\
				<tr>\
				<td>J.D. Henderson</td>\
				<td>Lead Developer</td>\
				</tr>\
				</table>\
				<br><br>\
				<center><b>Thanks To</b></center>\
				<br>\
				<table border=0>\
				<tr>\
				<td>The-Fixer</td>\
				<td>freenode.net, #wxWidgets</td>\
				</tr>\
				</table>\
				<br>\
				</font>\
				</body>\
				</html>");

	SetIcon(wxIcon(aboutframe));

	Eraser* panel = new Eraser(this, -1,
				wxPoint(0, 0),
				wxSize(400, DLOG_HEIGHT));

	wxHtmlWindow* html = new wxHtmlWindow(panel, -1,
						wxPoint(10, 210),
						wxSize(380, 150));
	html->SetPage(wxString(ABOUT_TEXT));

	wxButton* ok = new wxButton(panel, ID_BTN_CLOSE,
				    wxT("Close"),
				wxPoint(150, 375),
				wxSize(100, 28));

	Centre(wxBOTH|wxCENTRE_ON_SCREEN);

#ifndef TARGET_CARBON
   	ok->SetDefault();
   	ok->SetFocus();
#endif

	logo_ = new wxBitmap(about_logo);

// yay for size differenced
#ifdef WIN32
   	icon_ = new wxStaticBitmap(panel, -1, *logo_, wxPoint(0, 0),
                          		wxSize(215, 190));
#else
	icon_ = new wxStaticBitmap(panel, -1, *logo_, wxPoint(93, 10),
                          		wxSize(215, 190));
#endif

}

void 
AboutDialog::OnButtons(wxCommandEvent& event)
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

AboutDialog::~AboutDialog()
{
   	delete icon_;
   	delete logo_;
}
