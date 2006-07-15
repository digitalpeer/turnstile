/* $Id: app.cpp,v 1.3 2004/08/29 23:16:25 dp Exp $
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

/** @file app.cpp
@brief Application Implimentation
*/
#include "config.h"
#include "app.h"
#include "mainframe.h"
#include "network.h"
#include "utils.h"
#include "prefs.h"

#include "libjdhsocket/socketbase.h"

#include <wx/log.h>
#include <wx/snglinst.h>

#include <iostream>
using namespace std;

#include "../pixmaps/mainframe.xpm"

IMPLEMENT_APP(Turnstile)

Turnstile::Turnstile()
{;}

#ifndef DEBUG
void
Turnstile::OnFatalException()
{
   LogError(wxT("Turnstile has encountered a fatal error.  Aborting."));

	wxApp::OnFatalException();
}
#endif

bool
Turnstile::OnInit()
{
	/*if (!SocketBase::InitSock())
	{
		wxMessageDialog diag(0,"Failed to initialize sockets.","Error",wxOK);
		diag.ShowModal();
		return FALSE;
	}*/

	// set app name
   SetAppName(wxT(PACKAGE_NAME));

#ifndef WIN32
	//wxLog* logger = new wxLogStream(&std::cout);
  	//wxLog::SetActiveTarget(logger);
#else
	// boo, windows doesn't support stdout in an app
	wxLog::SetActiveTarget(NULL);
#endif

#ifndef DEBUG
	// check and see if this app is already running
	const wxString name = wxString::Format(wxT("TURNSTILE-%s"), wxGetUserId().c_str());
	wxSingleInstanceChecker checker_(name);
	if (checker_.IsAnotherRunning())
	{
	   wxMessageDialog* diag = new wxMessageDialog(frame_,wxT("Another instance of Turnstile is already running."),wxT("Error"),wxOK);
		diag->ShowModal();
		return FALSE;
	}
#endif
	/*
	See if we are to be a seed node.
	*/
	if (argc == 2)
	{
	   if (wxString(argv[1]) == wxT("--seed"))
		{
			Network::Instance()->seed_ = TRUE;
			LogDebug(wxT("Started as seed node."));
		}
		else
		{
		   LogError(wxT("Unknown command line option: '") + wxString(argv[1]) + wxT("'"));
			Network::Instance()->seed_ = FALSE;
		}
	}
	else
	{
		Network::Instance()->seed_ = FALSE;
	}

	//wxMessageDialog* diag = new wxMessageDialog(0,"This is a test version of Turnstile that is not to be redistributed or copied.","Warning",wxOK);
	//diag->ShowModal();

	// create the main frame window
	frame_ = new MainFrame();
	// set an icon
	frame_->SetIcon(wxIcon(mainframe));
	SetTopWindow(frame_);
	frame_->Show(TRUE);

	// load prefs
	if (!Prefs::Instance()->Load())
	{
	   wxMessageDialog* diag = new wxMessageDialog(frame_,wxT("A preferences file could not be found or opened.  A default file will be created for you.  Please modify preferences to suit you."),wxT("Warning"),wxOK);
		diag->ShowModal();
		frame_->ShowPrefFrame();
	}

#ifdef DEBUG
	if (argc == 5)
	{
		Prefs::Instance()->Set(Prefs::PUBLICKEY,wxString(argv[2]));
		Prefs::Instance()->Set(Prefs::ALIAS,wxString(argv[3]));
		Prefs::Instance()->Set(Prefs::PORT,wxString(argv[4]));
		LogDebug(wxString().Format(wxT("Autorunning ... %s %s %s"),argv[2],argv[3],argv[4]));
		frame_->Show(FALSE);
		frame_->AutoStart();

	}
#endif

	// app started successfully!
   	return TRUE;
}

MainFrame*
Turnstile::GetMainWindow()
{ return frame_; }

int
Turnstile::OnExit()
{

	JdhSocket::SocketBase::DeInitSock();

	return wxApp::OnExit();
}

