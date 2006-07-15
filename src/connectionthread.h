/* $Id: connectionthread.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file connectionthread.h
@brief Connection Thread Definition
*/
#ifndef TURNSTILE_CONNECTIONTHREAD_H
#define TURNSTILE_CONNECTIONTHREAD_H

#include "connection.h"

#include <wx/thread.h>
#include <wx/string.h>

class wxEvtHandler;
namespace JdhSocket { class ClientSocket; }

/** @class Connection
This defines a connection and brings everything together for a connection
into one place.
*/
class ConnectionThread : public wxThread,Connection
{
public:
	void* Entry();
	void OnExit();

	/**  Create a new connection off a socket from the server.
	*/
	ConnectionThread(wxEvtHandler* parent,int parentId, JdhSocket::ClientSocket* socket);

	/**  Create a connection that's expected to come up with its own socket.
	The value of a and addr depends on what type is.
	*/
	ConnectionThread(wxEvtHandler* parent,int parentId, int type,
			 wxString addr = wxT(""),wxString a = wxT(""),wxString b = wxT(""));

private:

	wxString b_;
};

#endif
