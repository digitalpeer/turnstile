/* $Id: network.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file network.h
@brief Generic Network Definition for the YAWP Protocol
*/
#ifndef TURNSTILE_NETWORK_H
#define TURNSTILE_NETWORK_H

#include "dhtutils.h"
#include "finger.h"
#include "buddytreectrl.h"

#include <wx/event.h>
#include <wx/datetime.h>

#include <vector>

class Connection;
class MainFrame;
class HostCache;
class Packet;
class wxCriticalSection;
class wxTimer;
class wxTimerEvent;
class wxTreeItemId;
class ShareFile;
class HashTable;

#define NEXT_TABLE_SIZE 5

/*
*signon*
find initial node
connect to initial node and get nid and parent
connect to parent
send new child
receive hashes
*/

/*
seed node
assign self a nid
node's next and previous pointers point to itself
add local hashes to hash table
start server

enter
start server
bootstrap and get nid and next
connect to parent and tell it it has a new child
parent will transfer all hashes less than new node's nid to new node
parent will tell current child it has a new parent pointer
child will say hey to new node
node will distribute all hashes it does not own (anything not between its nid and its child nid)
node will distribute buddy hash
node will alert all buddies on buddy list it is online

leave
node will transfer all hashes to parent
node will tell child it has a new parent
child will say hey to parent

HELLO_PARENT

NEW_PARENT
nid
address
*/

/** @class Network
@brief Main Interface to the YAWP Protocol
This object is responsible for maintaining the core connection and handling all other
connections.
*/
class Network : public wxEvtHandler
{
	friend class Connection;
	friend class ConnectionThread;

public:
	/** Network statuses.
	Every time there is a change to the Network status, an updated status
	event is generated.
	*/
	enum 	{
			STATUS_NONE = 		0x00000000,
			STATUS_ALIVE = 		0x00000001,
			STATUS_SEEDING = 	0x00000010,
			STATUS_BOOTSTRAPPING = 	0x00000100,
			STATUS_ENCRYPTION = 	0x00001000,
			STATUS_REFRESHSHARE = 	0x00010000,
			STATUS_ERROR = 		0x00100000
		};

	enum	{
			EVENT_STATUSUPDATE
		};

	/** Some control IDs */
	enum	{
			ID_NETWORK = 7020,
			ID_TIMER
		};

	/** This is a singleton class and this is the only way you are going to get
	access to it.
	*/
	static Network* Instance();

	static wxCriticalSection* joinCrit;
	bool allowJoin;

	bool Join()
	{
		wxCriticalSectionLocker locker(*joinCrit);
		return allowJoin;
	}

	void Join(bool set)
	{
		wxCriticalSectionLocker locker(*joinCrit);
		allowJoin = set;
	}

	/** Start the network.
	*/
	void Start(wxEvtHandler* starter, int starterId, bool seed = FALSE);

	/** Called by a timer to constantly stabalize the next pointer.
	*/
	void OnStabalize(wxTimerEvent& event);

	std::vector<ShareFile*> uploads;
	std::vector<ShareFile*> downloads;

	void AddUpload(ShareFile* file);
	void RemoveUpload(ShareFile* file);
	void RemoveDownload(ShareFile* file);
	void AddDownload(ShareFile* file);

	unsigned int UploadSize()
	{ return uploads.size(); }

	unsigned int DownloadSize()
	{ return downloads.size(); }

	/** Is the network running. */
	//inline int GetStatus()
	//{ return status_; }

	void Stop();

	/** Used by the threads to see if they should kill themselves.
	*/
	bool KillAll();

	/*
	Connect to node.
	Get nid and parent.
	Connect to parent and say hello.
	Parent will give all hashes less than new node's id to new node.
	Parent will tell child of new parent and tell new node of child.
	New node will distribute its current hashes.
	*/

	/** Return my public addr/port.
	*/
	wxString PublicAddress();
	bool IsMe(const wxString& addr);
#ifdef DEBUG
	void PrintNext();
#endif
	bool GetNext(unsigned int index, Node& next);
	void AddNext(const Node& next);
	void ClearNext();
	void EraseNext(unsigned int x);
	unsigned int NextSize();

	void Prev(const Node& node);

	Node Prev();

	wxString ErrorMsg();
	wxString StatusMsg();

   void Insert(wxString value, wxString info = wxT(""), bool getEvent = FALSE);

	wxString ComputeNid(const wxString& address);



	bool seed_;



	bool IsSuccessor(wxString nid, wxString prev,/*wxString next,*/wxString hash);

	bool GetStatus(int status);
	void SetStatusOn(int status,wxString msg = wxT(""));
	void SetStatusOff(int status,wxString msg = wxT(""));

	wxString rsaPub;
	wxString rsaPriv;

	/** Kill the network.
	This closes all connections down and stops the server.
	*/
	void KillNetwork(wxString msg = wxT(""));

	HashTable* hashTable;

	void SetGateway(const wxString& addr);

	~Network();

private:

	FingerTable finger;

	void InitNetwork();

	/** Singleton class. */
	Network();

	static Network* instance_;

	/** Stabalize by checking to see if we have a new parent.
	*/
	void Stabalize();

	/** Distribute all of our local hashes (including our buddy hash)
	*/
	void Distribute();

	/** Handle a connection event.
	*/
	void OnConnectionEvent(wxCommandEvent& event);

	/** Start the server and wait for connections.
	*/
	bool StartServer(int port);

	/** Start the bootstrap sequence.  If it fails down the line
	it will eventually recall itself until it has no more hosts to try.
	*/
	void Bootstrap();

	/** Connect to event handler for connection events.
	*/
	void ConnectEvent();

	/** Disconnect from event handler for connection events.
	*/
	void DisconnectEvent();

	/** Say hello to all of the buddies our our buddy list.
	*/
	void SayHelloToBuddies(const wxTreeItemId idParent, wxTreeItemIdValue cookie);
	void AllBuddiesOffline(const wxTreeItemId idParent, wxTreeItemIdValue cookie);


	/** Time network was started.  used to get uptime. */
	//wxDateTime startTime_;

	wxEvtHandler* starter_;

	wxString gateway_;

	int starterId_;

	/** Current network status. */
	int status_;

	/** This timer calls the Stabalize() function while the network is up. */
	wxTimer* stabalizeTimer_;

	/** Previous pointer. */
	Node prev_;


	/*
	How the next table is used.

	1.  Join network
		- get parent and add to clean nexttable
		- rebuild next table

	2.  Stabalize
		- start at first node in next table and stabalize
		- if a node cannot be contacted, get next node and reset next pointer as necessary
		- if next table is < 2 then rebuild it

	2.1.  New parent found on stabalize response
		- insert new parent at beginning of existing next table
		- rebuild next table
	*/

	std::vector<Node> nextTable_;


	/** Mutex for dealing with the connection list. */
	static wxCriticalSection* conCrit_;
	static wxCriticalSection* nextCrit_;
	static wxCriticalSection* prevCrit_;
	static wxCriticalSection* transferCrit_;
	static wxCriticalSection* killCrit_;

	bool killAll_;

	//int insertCount_;

	wxString statMsg_;
	wxString errMsg_;
	//bool distributeOnce;



	void PostNetworkEvent(int id, void* data = 0);

	DECLARE_EVENT_TABLE()
};

#endif
