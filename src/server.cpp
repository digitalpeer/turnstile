/* $Id: server.cpp,v 1.2 2004/07/02 23:24:23 dp Exp $
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

/** @file server.cpp
@brief Server Thread Implimentation
*/
#include "server.h"
#include "connectionthread.h"
#include "libjdhsocket/serversocket.h"
#include "libjdhsocket/clientsocket.h"
#include "utils.h"
#include "network.h"
#include "prefs.h"

using namespace JdhSocket;

void* Server::Entry()
{
	ServerSocket server;

	/*if (!server.Create())
	{
		Network::Instance()->KillNetwork("Failed to create server.");
		return 0;
	}*/
	if (!server.Bind(atoi(Prefs::Instance()->Get(Prefs::PORT).c_str())))
	{
		Network::Instance()->KillNetwork(wxString().Format("Failed to bind to port %s",Prefs::Instance()->Get(Prefs::PORT).c_str()));
		return 0;
	}
	if (!server.Listen())
	{
		Network::Instance()->KillNetwork("Failed to listen with server socket.");
		return 0;
	}
	if (!server.SetNonBlocking())
	{
		Network::Instance()->KillNetwork("Failed to set server socket non blocking.");
		return 0;
	}

	LogDebug("Server thread starting ...");

	ClientSocket* newSocket = 0;

	while (!Network::Instance()->KillAll())
	{
		newSocket=server.Accept();

		if (newSocket != 0)
		{
			LogDebug("Server accepted new connection");

			/** @todo This is not "common" */
			if (!newSocket->SetBlocking())
			{
				LogError("Failed to set socket blocking.");
			}

			ConnectionThread* thread = new ConnectionThread(Network::Instance(),Network::ID_NETWORK,newSocket);

			if (thread->Create() != wxTHREAD_NO_ERROR)
				LogError("Can't create connection thread.");

			thread->Run();
		}

		// prevent from eating cpu with encryption
		wxThread::Sleep(1);
	}

	LogDebug("Server thread got signal to exit.");

	return 0;
}

void Server::OnExit()
{;}

