/*
   $Id: clientsocket.h,v 1.4 2004/08/14 17:54:02 dp Exp $

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
	@file socket.h
	@brief Socket Class Declaration
*/
#ifndef JDH_SOCKET_H
#define JDH_SOCKET_H

#include "socketbase.h"

namespace JdhSocket
{

/**
	@class ClientSocket
	@brief Blocking Thread-Safe Client Socket Interface

	@todo setsockopt() is not thread safe in here.
*/
class ClientSocket : public SocketBase
{
public:

	ClientSocket();

	/**
		@brief Connect to a host.
	*/
	bool Connect(const std::string& host, short port, unsigned int timeout = 0);

	/**
		@brief Get the ip address of the connected host.
	*/
	std::string GetPeerHost() const;

	inline std::string ErrorMessage() const
	{ return eMessage_; }

	/**
		@brief Send the size specified (will not return until it has finished)
		@return True on success.
	*/
	bool ForceSend(const char* buffer, int count);
	bool ForceSend(const std::string& buffer);

	/**
		@brief Received the size specified or wait for the timeout to end.
		@return True on success.
	*/
	bool ForceRecv(char* buffer, int count, unsigned int timeout = 0);
	bool ForceRecv(std::string& buffer, unsigned int timeout = 0);

	/**
		@brief See if there is any data waiting to be received.
	*/
	bool ReadReady(int seconds = 0);

	/**
		@brief See if it is ok to write.
	*/
	bool WriteReady(int seconds = 0);

	virtual ~ClientSocket();

private:


};

}

#endif
