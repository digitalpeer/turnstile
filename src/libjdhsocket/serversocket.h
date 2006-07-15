/*
   $Id: serversocket.h,v 1.3 2004/08/14 17:54:02 dp Exp $

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
	@file serversocket.h
	@brief Socket Class for Servers Declaration
*/
#ifndef JDH_SERVER_SOCKET_H
#define JDH_SERVER_SOCKET_H

#include "socketbase.h"

namespace JdhSocket
{

class ClientSocket;

class ServerSocket : public SocketBase
{
public:

	ServerSocket();

	/**
		@brief Bind a server socket to a port.
	*/
	bool Bind(const int port);

	/**
		@brief Make server socket listen.
	*/
	bool Listen(int maxConnections = 10) const;

	/**
		@brief Accept a new connection.
	*/
	ClientSocket* Accept() const;

	virtual ~ServerSocket();
};

}

#endif
