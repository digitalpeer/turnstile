/*
   $Id: clientsocket.cpp,v 1.4 2004/08/14 17:54:02 dp Exp $

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
	@file socket.cpp
	@brief ClientSocket Class Implementation
*/
#include "clientsocket.h"

#ifdef WIN32
#include <winsock2.h>
#else
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#if MAKE_SOCKET_WXTHREAD_SAFE
#include <wx/thread.h>
#endif

namespace JdhSocket
{

using namespace std;

ClientSocket::ClientSocket()
{;}

bool
ClientSocket::Connect(const string& host, short port, unsigned int timeout)
{
	if (!SocketBase::Valid())
	{
		if (!SocketBase::Create())
		{
			DEBUG_SOCKET("could not create socket")
			return false;
		}
	}

	sockaddr_in* addr = new sockaddr_in;
	memset(addr, 0, sizeof(*addr));

	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);

	hostent* hp = SocketBase::GetHostByName(host.c_str());
	if (hp == 0)
	{
		DEBUG_SOCKET("failed to get host by name in connect")
		return false;
	}

	char* ip = InetNtoA(*(struct in_addr*)*hp->h_addr_list);
	addr->sin_addr.s_addr = InetAddr(ip);

	int err = 0;
	bool success = SocketBase::Connect(sock_,reinterpret_cast<struct sockaddr*>(addr),
					sizeof(*addr),timeout,err);

	if (!success)
	{
		DEBUG_SOCKET("failed to connect to " << host << ":" << port)
		eMessage_ = SocketBase::GetErrorMessage(err);
	}

	return success;
}

bool
ClientSocket::ForceSend(const char* buffer, int count)
{
	if (!Valid())
	{
		DEBUG_SOCKET("invalid socket " << sock_)
		return false;
	}

	int response;
	int err;

	do
    	{
		do
		{
			err = 0;
			response = SocketBase::WriteSocket(sock_, buffer, count, MSG_NOSIGNAL, err);
		}
		while ((response < 0 && err == EAGAIN) || (err == EINTR));

		if (response < 0)
		{
			switch (err)
			{
			case EBADF:  eMessage_ = "An invalid descriptor was specified."; break;

			case ENOTSOCK:
				eMessage_ = "The argument s is not a socket."; break;

			case EFAULT: eMessage_ = "An invalid user space address was specified for a parameter."; break;

			case EMSGSIZE:
				eMessage_ = "The  socket  requires  that  message be sent atomically, and the\
				size of the message to be sent made this impossible."; break;

			case EAGAIN:
	//		case EWOULDBLOCK:
				eMessage_ = "The socket is marked non-blocking and  the  requested  operation\
				would block."; break;

			case ENOBUFS:
				eMessage_ = "The  output queue for a network interface was full.  This gener-\
				ally indicates that the interface has stopped sending,  but  may\
				be  caused  by  transient  congestion.  (Normally, this does not\
				occur in Linux. Packets are just silently dropped when a  device\
				queue overflows.)"; break;

			case EINTR:  eMessage_ = "A signal occurred."; break;

			case ENOMEM: eMessage_ = "No memory available."; break;

			case EINVAL: eMessage_ = "Invalid argument passed."; break;

			case EPIPE:  eMessage_ = "The  local  end  has  been  shut  down  on a connection oriented\
				socket.  In this case the process will also  receive  a  SIGPIPE\
				unless MSG_NOSIGNAL is set."; break;

			}

			//eMessage_ = GetErrorMessage(err);
			DEBUG_SOCKET("socket send failed: " << eMessage_)
			return false;
		}

		count -= response;
		buffer += response;
    	}
    	while (count);

    	return true;
}

bool
ClientSocket::ForceSend(const string& buffer)
{
	uint size = buffer.size();
	return ForceSend((char*)&size,sizeof(size)) &&
			ForceSend(buffer.c_str(),buffer.size());
}

bool
ClientSocket::ReadReady(int seconds)
{
	struct timeval tv;
	tv.tv_sec = seconds;
	// if seconds is zero might as well wait a little
	tv.tv_usec = 100;

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sock_,&fds);
	int max = sock_ + 1;

	int ready = select(max, &fds,(fd_set*)0,(fd_set*)0,&tv);

	if (ready < 0)
		DEBUG_SOCKET("select failed")

	return ready == 1/*FD_ISSET(sock_,&fds)*/;
}

bool
ClientSocket::WriteReady(int seconds)
{
	struct timeval tv;
	tv.tv_sec = seconds;
	// if seconds is zero might as well wait a little
	tv.tv_usec = 100;

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sock_,&fds);
	int max = sock_ + 1;

	int ready = select(max, (fd_set*)0,&fds,(fd_set*)0,&tv);

	if (ready < 0)
		DEBUG_SOCKET("select failed")

	return ready == 1 /*FD_ISSET(sock_,&fds)*/;
}

bool
ClientSocket::ForceRecv(char* buffer, int count, unsigned int timeout)
{
	if (!Valid())
	{
		DEBUG_SOCKET("invalid socket " << sock_)
		return false;
	}

	int response;
	int err;

	/** @todo Add in timout! */
	do
	{
		do
		{
			err = 0;
			response = SocketBase::ReadSocket(sock_, buffer, count, MSG_NOSIGNAL, err);
		}
		while ((response < 0 && err == EAGAIN) || (err == EINTR));

		if (response <= 0)
		{
			eMessage_ = GetErrorMessage(err);
			DEBUG_SOCKET("socket recv failed: " << eMessage_)
			return false;
		}

		count -= response;
		buffer += response;
	}
	while (count);

	return true;
}

bool
ClientSocket::ForceRecv(string& buffer, unsigned int timeout)
{
	uint size = 0;
	if (!ForceRecv((char*)&size,sizeof(size),timeout))
		return false;

	if (size)
	{
		char* buf = new char[size];
		if (!ForceRecv(buf,size,timeout))
			return false;

		buf[size] = 0;
		buffer = buf;

		delete buf;
	}
	else
		buffer.clear();

	return true;
}

string
ClientSocket::GetPeerHost() const
{
	string answer;

	sockaddr_in* addr = new sockaddr_in;
	memset(addr, 0, sizeof(*addr));

// cheap fix
#ifdef WIN32
	int size = sizeof(*addr);
	if (getpeername(sock_,(struct sockaddr*)addr,&size) != 0)
#else
	unsigned int size = sizeof(*addr);
	if (getpeername(sock_,(struct sockaddr*)addr,(socklen_t*)&size) != 0)
#endif
	{
		DEBUG_SOCKET("failed to get peer host")
		return answer;
	}

	answer = SocketBase::InetNtoA(addr->sin_addr);

	delete addr;

	return answer;
}

ClientSocket::~ClientSocket()
{
	//Close();
}

}
