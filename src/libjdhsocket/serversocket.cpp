/*
   $Id: serversocket.cpp,v 1.3 2004/08/14 17:54:02 dp Exp $

   JdhSocket Library - Portable, Thread Safe Sockets Library

   Copyright (C) 2004, J.D. Henderson <www.digitalpeer.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/**
	@file serversocket.cpp
	@brief Socket Class for Server Implementation
*/
#include "serversocket.h"
#include "clientsocket.h"

#ifndef WIN32
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif

#include <fcntl.h>
#include <string.h>

namespace JdhSocket
{

using namespace std;

ServerSocket::ServerSocket()
{;}

bool
ServerSocket::Bind(const int port)
{
	if (!SocketBase::Create())
	{
		DEBUG_SOCKET("could not create socket")
		return false;
	}

	/*
	Allow the server to be replaced quickly.
	*/
	int on = 1;
	if (setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) == -1 )
	{
		DEBUG_SOCKET("failed to set reuse")
		return false;
	}

	sockaddr_in* addr = new sockaddr_in;
	memset(addr, 0, sizeof(*addr));

	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = htonl(INADDR_ANY);
	addr->sin_port = htons(port);

	int err = 0;
	if (SocketBase::Bind(sock_,(struct sockaddr*)addr,/*(socklen_t)*/sizeof(*addr),err) == -1)
	{
		DEBUG_SOCKET("failed to bind")
		eMessage_ = GetErrorMessage(err);
		return false;
	}

	return true;
}

bool
ServerSocket::Listen(int maxConnections) const
{
	if (!Valid())
	{
		DEBUG_SOCKET("invalid socket")
		return false;
	}

	if (::listen(sock_, maxConnections) == -1)
	{
		DEBUG_SOCKET("failed to listen")
		return false;
	}

	return true;
}

ClientSocket*
ServerSocket::Accept() const
{
	int answer = ::accept(sock_,0,0);

	ClientSocket* newSock = new ClientSocket;
	newSock->sock_ = answer;
	if (answer >= 0)
	{
		return newSock;
	}

	return 0;
}

ServerSocket::~ServerSocket()
{;}

}
