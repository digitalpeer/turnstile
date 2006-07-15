/* $Id: connection.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file connection.h
@brief Connection Definition
*/
#define MAX_CLIENT_CONNECTIONS	10

#ifndef TURNSTILE_CONNECTION_H
#define TURNSTILE_CONNECTION_H

#include "config.h"

#include <wx/string.h>

namespace JdhSocket { class ClientSocket; }
class Packet;
class ShareFile;
class DownloadFile;
class Key;
class LookupResponse;
class Node;
class wxEvtHandler;
class ImEventQueue;
class wxStopWatch;
#ifdef ENABLE_ENCRYPTION
namespace crypto { class TwoFishCipher; }
#endif

#define CONNECTION_WAIT		0

/* 
How a DHT lookup works with connections.

1.  User says create a connection and lookup key x, giving the connection a start address.
2.  On creation, connection connects to host automagically.
3.  Connection handshakes and eventually gets some type of response.
4.  If the key is found, it reports EVENT_SEARCHRESULT events.
5.  If the connection just gets another node to hop to, it will create a new connection
	and start the loop all over again.  If this happens the connection will simply 
	delete itself.
*/

/** @class Connection
This defines a connection and brings everything together for a connection
into one place.
*/
class Connection
{
public:

	/** Connection type ids.
	@note If one of these connection types is not used, it is assumed
	the instance was created for direct usage (Direct*() functions).
	*/
	enum	{	
			TYPE_SERVER,	// [acting as a server]
			//
			TYPE_BOOTSTRAP,	// try and bootstrap and say hello to initial parent
			TYPE_QUERY,	// query network for a hash
			TYPE_DOWNLOAD,	// download a file
			TYPE_INSERT,	// insert a hash into the distributed hash table
			TYPE_HELLOBUDDY,// say hello to a buddy (look him up first)
			TYPE_MANUAL,	// connection will manipulate the connection!
			TYPE_SPREAD,	// say hello to a new parent
			TYPE_IM,
			TYPE_IM_CONNECT,
			//TYPE_BUILD_NEXT_TABLE,	// (re)build next table
			//
			TYPES_SIZE
		};

	/** Connection event ids.
	A lot of stuff happens behind the scenes in a connection.  However, it is
	convenient to know what has happened, so these events are created.  It is
	difficult to explain when an event will happen.  Just use common sense.

	@note An event will only be sent if the parent is specified on connection
	construction.
	*/
	enum 	{
			//
			// events sent no matter what the connection type
			//
			EVENT_MADE,		/// connection made (handshake completed)
			EVENT_FAILED,		/// connection failed
			EVENT_EXIT,		/// connection gracefully exited

			//
			// bootstrap events
			//
			EVENT_NEXT_CONNECTED,	// connected to next
			EVENT_PREV_CONNECTED,	// connected to new previous
			EVENT_NEXT_LOST,	// failed to connect to next/lost connection


			EVENT_RECV,		/// connection got a packet


			//
			// query events
			//
			EVENT_QUERYFOUND,	// query was found
			EVENT_QUERYNOTFOUND,

			//
			// insert events
			//
			EVENT_INSERT_FINISHED,	// an insert is always assumed to be successful

			//
			// server events (events generated from clients connecting to us)
			//
			EVENT_RCV_IM,		// got an im for a buddy
			EVENT_BUDDYCONNECT,	// buddy has connected to us
			//EVENT_ADDRESS,
		};

	/** Connection statuses. */
	enum	{
			STATUS_DEAD,	// it's one or the other
			STATUS_ALIVE
		};

	enum 	{
			/** File chucks in bytes to request at a time.
			*/
			FILE_SEG_BUFFER_SIZE = /*1024*/ /*2048*/ /*5120*/ /*8192*/ 10240
		};

	/**  Create a new connection off a socket from the server.
	*/
	Connection(wxEvtHandler* parent,int parentId, JdhSocket::ClientSocket* socket);
	
	/**  Create a connection that's expected to come up with its own socket.
	The value of a and addr depends on what type is.
	*/
	Connection(wxEvtHandler* parent,int parentId, int type,
		   wxString addr,wxString a = wxT(""));

	/** Get the id of this connection. */
	inline int id()
	{ return id_; }

	/** Get the type of connection this is. */
	inline int Type()
	{ return type_; }

	/** Get the status of this connection. */
	inline int Status()
	{ return status_; }

	virtual ~Connection();

#ifdef DEBUG
	void AnalyzeNetwork();
#endif

private:

	/** This incriments each time a new connection is made and assigns that id
	to the connection to create unique connection ids. */
	static int idCounter_;

protected:

	/** No public default constructor. */
	Connection()
	{;}

	/** If this connection is started as an outgoing thread, it will call this.
	*/
	bool Connect(/*bool create = TRUE*/);

	/** @defgroup direct_function Internal Connection Functions
	These functions are used only by the connections class to create new connections.
	These connections stay in the current connections thread so they don't block the GUI.
	@{*/
	LookupResponse DirectQuery(wxString key);
	bool HelloParent(const Node& node);
	//bool RelocateChild(wxString newParent);
	bool JoinParent();
	/**@}*/

	/** Send a packet on this connection.
	@return FALSE on error/timeout.
	*/
	bool Send(Packet& packet);

	/** Read a packet from the socket.
	Returns true if the read succeeds and the socket is in an ok state.
	Return false if the socket connection is lost or there is a problem reading.

	@note May return a null pointer if a 0 length read happens (will happen
	on a non-blocking socket).
	*/
	bool Receive(Packet*& packet/*, bool block = TRUE*/);

	/** Kill the socket and send EVENT_LOST
	*/
	void KillSocket(int event);

	/** Post an event to the parent (if there is one) based on supplied info.
	*/
	void PostConEvent(int type,void* data = 0);

	wxEvtHandler* parent_;
	int parentId_;
	JdhSocket::ClientSocket* socket_;
	int id_;

	/** @defgroup con_info Information about the Current Connection
	@{*/
	//int currentEncryptionType_;
	int currentNodeRank_;
	wxString currentNid_;
	wxString currentAgent_;

	wxString a_;
	wxString currentAddr_;
	/**@}*/

	/** @defgroup rsakeys Encryption Keys for the Connection
	@{*/
	wxString rsaPub_;
	//wxString rsaPriv_;
	//wxString symKey_;
	/**@}*/


	/** @defgroup packet_count Packet Counters for a Connection
	@{*/
	unsigned int packetsReceived_;
	unsigned int packetsSent_;
	/**@}*/

	/** The type of connection. */
	int type_;

	/** The current status of this connection. */
	int status_;

	/*
	Outgoing
	*/
	bool InitWithHandshake();
	bool InitWithJoin();
   LookupResponse Lookup(wxString key,wxString target=wxT(""));

	/** Put a file */
	void Put(ShareFile* file);

	/*
	Incomming
	*/
	bool ReceiveHandShake();

	void HandleImEvents(ImEventQueue* queue);

	wxStopWatch* sw_;

	void RSAEncrypt(Packet* packet);
	void RSADecrypt(const char* raw,Packet* packet);
	void SymEncrypt(Packet* packet);
	void SymDecrypt(const char* raw,Packet* packet);

#ifdef ENABLE_ENCRYPTION
	crypto::TwoFishCipher* symetric_;
#endif

};

#endif
