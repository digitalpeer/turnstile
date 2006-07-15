/*
   $Id: socketbase.h,v 1.4 2004/08/29 23:16:30 dp Exp $

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
	@file socketbase.h
	@brief Base Socket Class (with thread safety)

	These functions are meant to replace the low level c function calls. They are
	slightly modified in some cases, but are 100% thread safe.
*/

/** @brief Make the sockets functions thread safe using wxWidgets threads. */
#define MAKE_SOCKET_WXTHREAD_SAFE 0

#ifndef JDH_SOCKET_BASE_H
#define JDH_SOCKET_BASE_H

#include <string>

struct hostent;
struct in_addr;
struct sockaddr;
#if MAKE_SOCKET_WXTHREAD_SAFE
class wxMutex;
#endif

#ifdef DEBUG
#include <iostream>

/** @brief Socket Debug Macro (this is a easy, but not really the best way, to do this) */
#define DEBUG_SOCKET(text) \
	std::cout << "Socket Error: " << __FILE__ << ":" << __LINE__; \
	std::cout << " " << text << std::endl;
#else
#define DEBUG_SOCKET(text)
#endif

namespace JdhSocket
{

/**
	@class SocketBase The base class for socket operations.

	This class is not meant to be used directly.  It is the base class
	for the server and client sockets.  This class wraps the standard C socket
	API making it thread safe and making it a little easier to use.
*/
class SocketBase
{
	friend class ClientSocket;
	friend class ServerSocket;
public:

	SocketBase();

	static std::string LocalIP();
	static int GetHostName(char *name, int len, int& err);
	static hostent* GetHostByName(const char* host);
	static char* InetNtoA(struct in_addr& in);
	static unsigned int InetAddr(const char *cp);

	/** @brief Get the current error message. */
	std::string GetErrorMessage(int id) const;

	/**
		@brief Initialize the socket infastructure.
	*/
	static bool InitSock();

	/**
		@brief De-initialize the socket infastructure.
	*/
	static bool DeInitSock();

	/**
		@brief Determine whether the socket is in a valid state for I/O.
	*/
	inline bool Valid() const
	{ return sock_ >= 0; }

	/**
		@brief Close the socket properly (shutdown and reset)
	*/
	void Close();

	/**
		@brief Get the socket file descriptor.
	*/
	int Fd() const;

	/**
		@brief Set socket non blocking.
	*/
	bool SetNonBlocking();

	/**
		@brief Set socket blocking.
	*/
	bool SetBlocking();

	/**
		@brief Set the recv timeout for socket read operations.
	*/
	bool SetRecvTimeout(int value);

	/**
		@brief Set the send timeout for socket read operations.
	*/
	bool SetSendTimeout(int value);

	virtual ~SocketBase();

private:
 	/**
		Create a new socket.
	*/
	bool Create();

	/**
		@brief Allocate a new socket.
	*/
	int Socket(int domain, int type, int protocol,int& err);

	/**
		@brief Connect to server.
	*/
	bool Connect(int sockfd, const struct sockaddr *serv_addr, int
			addrlen, unsigned int timeout, int& err);

	int Bind(int sockfd, struct sockaddr *my_addr, int addrlen, int& err);

	/**
		@brief Read from socket.
	*/
	int ReadSocket(int s, char* buf, int len, int flags,int& err);

	/**
		@brief Write to socket.
	*/
	int WriteSocket(int s, const char* msg, int len, int flags,int& err);

	/**
		@brief Properly shutdown socket.
	*/
	int Shutdown(int s, int how, int& err);

#if MAKE_SOCKET_WXTHREAD_SAFE
	static wxMutex* errnoCrit_;
#endif

	/** Socket FD */
	int sock_;

	/** Init Trigger */
	static bool initFinished_;

	/** Error message for the last called function that generates an error. */
	std::string eMessage_;

};

}

#endif
