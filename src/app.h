/* $Id: app.h,v 1.2 2004/08/29 23:16:25 dp Exp $
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

/** 
    @file app.cpp
    @brief Application Definition
*/
#ifndef TURNSTILE_APP_H
#define TURNSTILE_APP_H

#include <wx/wx.h>

class MainFrame;

/** 
    @class Turnstile
*/
class Turnstile : public wxApp
{
public:
	
	Turnstile();

    	bool OnInit();
	int OnExit();
	
#ifndef DEBUG
	void OnFatalException();
#endif

	MainFrame* GetMainWindow();

private:

	MainFrame* frame_;
};

DECLARE_APP(Turnstile)

#endif
