/*
   $Id: socketbase.cpp,v 1.3 2004/08/14 17:54:03 dp Exp $

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

#include "socketbase.h"
#include "config.h"

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/signal.h>
#endif

#include <string.h>

#if !HAVE_GETHOSTBYNAME
#error libjdhsocket requires gethostbyname().
#endif

#if MAKE_SOCKET_WXTHREAD_SAFE
#include <wx/thread.h>
#endif

/*
#ifdef LINUX
#define jdh_socklen_t socklen_t
#else
#define jdh_socklen_t int
#else
*/

namespace JdhSocket
{

using namespace std;

#if MAKE_SOCKET_WXTHREAD_SAFE
static wxMutex* SocketBase::errnoCrit_ = new wxMutex;
#endif

bool SocketBase::initFinished_ = false;

/** Simple function to grab the error code. */
inline int
GetErrorCode()
{
#ifndef WIN32
	return errno;
#else
	return ::WSAGetLastError();
#endif
}

SocketBase::SocketBase() : sock_(-1)
{;}

hostent*
SocketBase::GetHostByName(const char* host)
{
#if MAKE_SOCKET_WXTHREAD_SAFE
	while (errnoCrit_.Lock() != wxMUTEX_NO_ERROR);
#endif
        struct hostent* hp = new hostent;
	memset(hp, 0, sizeof(*hp));

        hp = ::gethostbyname(host);

	if (hp == 0)
	{
#if MAKE_SOCKET_WXTHREAD_SAFE
		errnoCrit_.Unlock();
#endif
		DEBUG_SOCKET("failed to getbyhostname() for host " << host)
		//delete hp;
		return 0;
	}

	//memcpy(hp,tmp,sizeof(*tmp));

#if MAKE_SOCKET_WXTHREAD_SAFE
	errnoCrit_.Unlock();
#endif

        return hp;
}

char*
SocketBase::InetNtoA(struct in_addr& in)
{
#if MAKE_SOCKET_WXTHREAD_SAFE
	static wxMutex crit;
	while (crit.Lock() != wxMUTEX_NO_ERROR);
#endif

	char* ip = ::inet_ntoa(in);
	char* answer = new char[strlen(ip)+1];
	strcpy(answer,ip);

#if MAKE_SOCKET_WXTHREAD_SAFE
	crit.Unlock();
#endif

	return answer;
}

unsigned int
SocketBase::InetAddr(const char *cp)
{

#if MAKE_SOCKET_WXTHREAD_SAFE
	static wxMutex crit;
	while (crit.Lock() != wxMUTEX_NO_ERROR);
#endif
	unsigned int answer = ::inet_addr(cp);
#if MAKE_SOCKET_WXTHREAD_SAFE
	crit.Unlock();
#endif
	return answer;
}

bool
SocketBase::Create()
{
	if (!initFinished_)
	{
		if (!InitSock())
			return false;
	}

	int err = -1;
	sock_ = Socket(AF_INET,SOCK_STREAM,0,err);

	if (!Valid())
	{
		DEBUG_SOCKET("could not create socket")
		return false;
	}

	return true;
}

bool
SocketBase::Connect(int sockfd, const struct sockaddr *serv_addr, int
			addrlen, unsigned int timeout, int& err)
{
	if (timeout == 0)
	{
#if MAKE_SOCKET_WXTHREAD_SAFE
		while (errnoCrit_.Lock() != wxMUTEX_NO_ERROR);
#endif
		int status = ::connect(sockfd,serv_addr,addrlen);
		err = GetErrorCode();

#if MAKE_SOCKET_WXTHREAD_SAFE
		errnoCrit_.Unlock();
#endif
		return status == 0 ? true : false;

	}
	else
	{
		fd_set fds;
		struct timeval tv;

#if MAKE_SOCKET_WXTHREAD_SAFE
		while (errnoCrit_.Lock() != wxMUTEX_NO_ERROR);
#endif

		int status = ::connect(sockfd,serv_addr,(socklen_t)addrlen);
		err = GetErrorCode();

#if MAKE_SOCKET_WXTHREAD_SAFE
		errnoCrit_.Unlock();
#endif

		if(status == 0)
			return true;

		FD_ZERO(&fds);
		FD_SET(sockfd,&fds);

		tv.tv_sec = timeout/1000;
		unsigned int remainder = timeout % 1000;
		tv.tv_usec = remainder*1000;

		status = ::select(sockfd+1,0,&fds,0,&tv);

		if(status==0)
		{
			return false;
		}

		unsigned int len = 4;
		int val = 0;
		status = getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&val,&len);

		if(status < 0 || val != 0)
			return false;

		return true;
	}

	return false;
}

int
SocketBase::ReadSocket(int s, char* buf, int len, int flags, int& err)
{
#if MAKE_SOCKET_WXTHREAD_SAFE
	while (errnoCrit_.Lock() != wxMUTEX_NO_ERROR);
#endif

	int status = ::recv(s,buf,(size_t)len,flags);
	err = GetErrorCode();

#if MAKE_SOCKET_WXTHREAD_SAFE
	errnoCrit_.Unlock();
#endif
	return status;
}

int
SocketBase::WriteSocket(int s, const char* msg, int len, int flags,int& err)
{
#if MAKE_SOCKET_WXTHREAD_SAFE
	while (errnoCrit_.Lock() != wxMUTEX_NO_ERROR);
#endif

	int status = ::send(s,msg,(size_t)len,flags);
	err = GetErrorCode();

#if MAKE_SOCKET_WXTHREAD_SAFE
	errnoCrit_.Unlock();
#endif

	return status;
}

int
SocketBase::Socket(int domain, int type, int protocol,int& err)
{
#if MAKE_SOCKET_WXTHREAD_SAFE
	while (errnoCrit_.Lock() != wxMUTEX_NO_ERROR);
#endif

	int status = ::socket(domain,type,protocol);
	err = GetErrorCode();

#if MAKE_SOCKET_WXTHREAD_SAFE
	errnoCrit_.Unlock();
#endif
	return status;
}


int
SocketBase::GetHostName(char *name, int len, int& err)
{
#if MAKE_SOCKET_WXTHREAD_SAFE
	while (errnoCrit_.Lock() != wxMUTEX_NO_ERROR);
#endif

	int status = ::gethostname(name, (size_t)len);
	err = GetErrorCode();

#if MAKE_SOCKET_WXTHREAD_SAFE
	errnoCrit_.Unlock();
#endif
	return status;
}

int
SocketBase::Bind(int sockfd, struct sockaddr *my_addr, int addrlen, int& err)
{
#if MAKE_SOCKET_WXTHREAD_SAFE
	while (errnoCrit_.Lock() != wxMUTEX_NO_ERROR);
#endif

	int status = ::bind(sockfd,my_addr,addrlen);
	err = GetErrorCode();

#if MAKE_SOCKET_WXTHREAD_SAFE
	errnoCrit_.Unlock();
#endif
	return status;
}

int
SocketBase::Shutdown(int s, int how, int& err)
{
#if MAKE_SOCKET_WXTHREAD_SAFE
	while (errnoCrit_.Lock() != wxMUTEX_NO_ERROR);
#endif
	int status = ::shutdown(s,how);
	err = GetErrorCode();
#if MAKE_SOCKET_WXTHREAD_SAFE
	errnoCrit_.Unlock();
#endif
	return status;
}

string
SocketBase::LocalIP()
{
	const unsigned int MAX_HOSTNAME_SIZE = 1024;

	string answer;
	struct in_addr** in;

	int err = 0;
	char* local = new char[MAX_HOSTNAME_SIZE+1];
	if (GetHostName(local,MAX_HOSTNAME_SIZE,err) != 0)
	{
		/** @todo check if a retry is quickly needed */
		DEBUG_SOCKET("could not get local hostname")
		return answer;
	}

	struct hostent* hp = GetHostByName(local);

	if (hp == 0)
	{
		/** @todo check if a retry is quickly needed */
		DEBUG_SOCKET("could not get local ip by name")
		return answer;
	}

	delete [] local;

#if 0
	/*
	just a quick check to make sure we are ipv4
	*/
	switch (hp->h_addrtype)
	{
	case AF_INET:
	{
		break;
	}
	default:
	{
		break;
	}
	}
#endif

	in = (struct in_addr **)hp->h_addr_list;

	// HACK!  Just pull the last address
	while(*in != 0)
	{
		// double dereference!
		char* tmp = InetNtoA(**(in++));
		answer = tmp;
		delete [] tmp;
		//LogDebug(wxString().Format("Looking address at:%s",answer.c_str()));
	}
	//LogDebug(wxString().Format("My local address is:%s",answer.c_str()));

	delete hp;

	return answer;
}

int
SocketBase::Fd() const
{
	return sock_;
}

bool
SocketBase::SetNonBlocking()
{
	if (!Valid())
	{
		DEBUG_SOCKET("invalid socket")
		return false;
	}

#ifdef WIN32
	/*
	Non zero means enable non blocking.
	*/
	int mode = 1;
	if (ioctlsocket(sock_,FIONBIO,(u_long FAR*)&mode) != 0)
#else
	if (fcntl(sock_, F_SETFL, O_NDELAY) != 0)
#endif
	{
		DEBUG_SOCKET("failed to set socket non blocking")
		return false;
    	}
	return true;

}

bool
SocketBase::SetBlocking()
{
	if (!Valid())
	{
		DEBUG_SOCKET("invalid socket")
		return false;
	}

#ifdef WIN32
	/*
	Non zero means enable non blocking.
	*/
	int mode = 0;
	if (ioctlsocket(sock_,FIONBIO,(u_long FAR*)&mode) != 0)
#else
	if (fcntl(sock_, F_SETFL, 0) != 0)
#endif
	{
		DEBUG_SOCKET("failed to set socket blocking")
		return false;
    	}
	return true;

}

void
SocketBase::Close()
{
	if (Valid())
	{
		int err = 0;
#ifdef WIN32
		Shutdown(sock_, SD_BOTH, err);
		closesocket(sock_);
#else
		Shutdown(sock_, SHUT_RDWR, err);
		close(sock_);
#endif
		sock_ = -1;
	}
}


bool
SocketBase::SetRecvTimeout(int value)
{
	if (!Valid())
	{
		DEBUG_SOCKET("invalid socket")
		return false;
	}

	if(setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO
			,(const char *)&value ,sizeof(value)) != 0)
	{
		DEBUG_SOCKET("failed to set recv timeout")
		return false;
	}
	return true;
}

bool
SocketBase::SetSendTimeout(int value)
{
	if (!Valid())
	{
		DEBUG_SOCKET("invalid socket")
		return false;
	}

	if(setsockopt(sock_, SOL_SOCKET, SO_SNDTIMEO
			,(const char *)&value, sizeof(value)) != 0)
	{
		DEBUG_SOCKET("failed to set send timeout")
		return false;
	}
	return true;
}

string
SocketBase::GetErrorMessage(const int id) const
{
	switch(id)
	{
#ifndef WIN32
	case EPROTONOSUPPORT:
		return "The  protocol  type  or  the specified protocol is not supported within this domain.";
	case EAFNOSUPPORT:
		return "The implementation does not support the specified  address  family.";
	case ENFILE:
		return "Not enough kernel memory to allocate a new socket structure.";
	case EMFILE:
		return "Process file table overflow.";
	case EACCES:
		return "Permission  to create a socket of the specified type and/or protocol is denied.";
	case ENOBUFS:
		return "Insufficient memory is available.  The socket cannot be  created until sufficient resources are freed.";
	case EBADF:
		return "The file descriptor is not a valid index in the descriptor table.";
	case ECONNREFUSED:
		return "A remote host refused to allow the network connection (typically because it is not running the requested service).";
	case ENOTCONN:
		return "The socket is associated with a connection-oriented protocol and has not been connected (see connect(2) and accept(2)).";
	case ENOTSOCK:
		return "The argument s does not refer to a socket.";
	case EAGAIN:
		return "The socket is marked non-blocking and the receive operation would block, or a receive timeout had been set and  the  timeout expired before data was received.";
	case EINTR:
		return "The receive was interrupted by delivery of a signal before any data were available.";
	case EFAULT:
		return "The receive buffer pointer(s) point outside the process's address space.";
	case EINVAL:
		return "Invalid argument passed.";
	case ENOMEM:
		return "Could not allocate memory for recvmsg.";
	case EISCONN:
		return "The socket is already connected.";
	case ETIMEDOUT:
		return "Timeout  while attempting connection. The server may be too busy\
			to accept new connections. Note that for IP sockets the  timeout\
			may be very long when syncookies are enabled on the server.";
	case ENETUNREACH:
		return "Network is unreachable.";
	case EADDRINUSE:
		return "Local address is already in use.";
	case EINPROGRESS:
		return "The  socket  is  non-blocking  and the connection cannot be com-\
			pleted immediately.  It is possible to select(2) or poll(2)  for\
			completion  by  selecting  the  socket for writing. After select\
			indicates writability, use getsockopt(2) to  read  the  SO_ERROR\
			option  at  level  SOL_SOCKET  to determine whether connect com-\
			pleted  successfully  (SO_ERROR  is  zero)   or   unsuccessfully\
			(SO_ERROR  is one of the usual error codes listed here, explain-\
			ing the reason for the failure).";
	case EALREADY:
		return "The socket is non-blocking and a previous connection attempt has\
			not yet been completed.";
	case EPERM:
		return "The user tried to connect to a broadcast address without  having\
			the  socket  broadcast  flag  enabled  or the connection request\
			failed because of a local firewall rule.";
#else
	case WSAEACCES:
		return "Permission denied.";
	case WSAEADDRINUSE:
		return "Address already in use.";
	case WSAEADDRNOTAVAIL:
		return "Destination unreachable.";
	case WSAEAFNOSUPPORT:
		return "Address family not supported by protocol family.";
	case WSAEALREADY:
		return "Operation already in progress.";
	case WSAECONNABORTED:
		return "Software caused connection abort.";
	case WSAECONNREFUSED:
		return "Connection refused.";
	case WSAECONNRESET:
		return "Connection reset by peer.";
	case WSAEDESTADDRREQ:
		return "Destination address required.";
	case WSAEFAULT:
		return "Bad address.";
	case WSAEHOSTDOWN:
		return "Host is down.";
	case WSAEHOSTUNREACH:
		return "No route to host.";
	case WSAEINPROGRESS:
		return "Operation now in progress.";
	case WSAEINTR:
		return "Interrupted function call.";
	case WSAEINVAL:
		return "Invalid argument.";
	case WSAEISCONN:
		return "Socket is already connected.";
	case WSAEMFILE:
		return "Too many open files.";
	case WSAEMSGSIZE:
		return "Message too long.";
	case WSAENETDOWN:
		return "Network is down.";
	case WSAENETRESET:
		return "Network dropped connection on reset.";
	case WSAENETUNREACH:
		return "Network is unreachable.";
	case WSAENOBUFS:
		return "No buffer space available.";
	case WSAENOPROTOOPT:
		return "Bad protocol option.";
	case WSAENOTCONN:
		return "Socket is not connected.";
	case WSAENOTSOCK:
		return "Socket operation on non-socket.";
	case WSAEOPNOTSUPP:
		return "Operation not supported.";
	case WSAEPFNOSUPPORT:
		return "Protocol family not supported.";
	case WSAEPROCLIM:
		return "Too many processes.";
	case WSAEPROTONOSUPPORT:
		return "Protocol not supported.";
	case WSAEPROTOTYPE:
		return "Protocol wrong type for socket.";
	case WSAESHUTDOWN:
		return "Cannot send after socket shutdown.";
	case WSAESOCKTNOSUPPORT:
		return "Socket type not supported.";
	case WSAETIMEDOUT:
		return "Connection timed out.";
	case WSATYPE_NOT_FOUND:
		return "Class type not found.";
	case WSAEWOULDBLOCK:
		return "Resource temporarily unavailable.";
	case WSAHOST_NOT_FOUND:
		return "Host not found.";
	case WSA_INVALID_HANDLE:
		return "Specified event object handle is invalid.";
	case WSA_INVALID_PARAMETER:
		return "One or more parameters are invalid.";
	case WSA_IO_INCOMPLETE:
		return "Overlapped I/O event object not in signaled state.";
	case WSA_IO_PENDING:
		return "Overlapped operations will complete later.";
	case WSA_NOT_ENOUGH_MEMORY:
		return "Insufficient memory available.";
	case WSANOTINITIALISED:
		return "Successful WSAStartup not yet performed.";
	case WSANO_DATA:
		return "Valid name, no data record of requested type.";
	case WSANO_RECOVERY:
		return "This is a non-recoverable error.";
	case WSASYSCALLFAILURE:
		return "System call failure.";
	case WSASYSNOTREADY:
		return "Network subsystem is unavailable.";
	case WSATRY_AGAIN:
		return "Non-authoritative host not found.";
	case WSAVERNOTSUPPORTED:
		return "WINSOCK.DLL version out of range.";
	case WSAEDISCON:
		return "Graceful shutdown in progress.";
	case WSA_OPERATION_ABORTED:
		return "Overlapped operation aborted.";
#endif
	}
	return "Unknown socket error.";
}


bool
SocketBase::InitSock()
{
	if (!initFinished_)
	{
#ifdef WIN32
		const int MAJOR_VERSION = 1;
		const int MINOR_VERSION = 1;

		WORD wVersion;
		WSADATA wsaData;

		wVersion = MAKEWORD(MAJOR_VERSION,MINOR_VERSION);
		int result = WSAStartup(wVersion, &wsaData);

		if(result != 0)
		{
			return false;
		}
		else
		{
			if ((LOBYTE(wsaData.wVersion) != MAJOR_VERSION) ||
				(HIBYTE(wsaData.wVersion) != MINOR_VERSION))
			{
				return false;
			}
		}
#else
		/*
		ignore the broken pipe signal when a socket is dropped without shutdown
		*/
		// should have this covered with MSG_NOSIGNAL
		//signal(SIGPIPE, SIG_IGN);
#endif
		initFinished_ = true;
		return true;
	}

	DEBUG_SOCKET("sockets already initialized")

	return false;
}

bool
SocketBase::DeInitSock()
{
	if (initFinished_)
	{
#ifdef WIN32
		if(WSACleanup() == SOCKET_ERROR)
			return false;
#endif
		initFinished_ = false;
		return true;
	}

	DEBUG_SOCKET("do not need to de-init sockets")

	return false;
}

SocketBase::~SocketBase()
{;}

}
