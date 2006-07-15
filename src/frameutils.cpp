/* $Id: frameutils.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

#include "frameutils.h"

#include <wx/wx.h>
#include <wx/config.h>

#define CONFIG_KEY "turnstilewincfg"

wxRect 
DetermineFrameSize (const wxString& directive)
{
    	const int minFrameWidth = 100;
    	const int minFrameHight = 100;

    	// load stored size or defaults
    	wxRect rect;
    	wxConfig* cfg = new wxConfig(CONFIG_KEY);
        
	if (cfg->Exists(directive))
	{
	   rect.x = cfg->Read (wxT("/") + directive + wxT("/locx"), rect.x);
	   rect.y = cfg->Read (wxT("/") + directive + wxT("/locy"), rect.y);
	   rect.width = cfg->Read (wxT("/") + directive + wxT("/locw"), rect.width);
	   rect.height = cfg->Read (wxT("/") + directive + wxT("/loch"), rect.height);
	}
    
	delete cfg;

	// check for reasonable values (within screen)
	wxSize scr = wxGetDisplaySize();
	rect.x = wxMin (abs (rect.x), (scr.x - minFrameWidth));
	rect.y = wxMin (abs (rect.y), (scr.y - minFrameHight));
	rect.width = wxMax (abs (rect.width), (minFrameWidth));
	rect.width = wxMin (abs (rect.width), (scr.x - rect.x));
	rect.height = wxMax (abs (rect.height), (minFrameHight));
	rect.height = wxMin (abs (rect.height), (scr.y - rect.y));
    
	return rect;
}

void 
StoreFrameSize(const wxRect& rect, const wxString& directive)
{
	wxConfig* cfg = new wxConfig(CONFIG_KEY);
    	cfg->Write (wxT("/") + directive +wxT("/locx"), rect.x);
	cfg->Write (wxT("/") + directive +wxT("/locy"), rect.y);
    	cfg->Write (wxT("/") + directive +wxT("/locw"), rect.width);
	cfg->Write (wxT("/") + directive +wxT("/loch"), rect.height);
	
#ifdef LINUX
	cfg->SetUmask(0077);
#endif
		
	delete cfg;
}

