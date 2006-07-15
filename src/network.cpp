/* $Id: network.cpp,v 1.2 2004/08/29 23:16:25 dp Exp $
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

/** @file network.cpp
@brief Generic Network Definition for the YAWP Protocol Implementation
*/
#include "config.h"
#include "network.h"
#include "app.h"
#include "buddy.h"
#include "mainframe.h"
#include "hostcache.h"
#include "prefs.h"
#include "utils.h"
#include "connectionthread.h"
#include "typeconv.h"
#include "buddytreectrl.h"
#include "crypto.h"
#include "server.h"
#include "hashtable.h"
#include "libjdhsocket/socketbase.h"

#ifdef ENABLE_ENCRYPTION
#include "crypto.h"
#endif

#include <wx/utils.h>

#ifdef DEBUG
#include <iostream>
#endif

using namespace std;
using namespace JdhSocket;

#define STABALIZE_INTERVAL 	9000
#define RSA_LENGTH		4096

BEGIN_EVENT_TABLE(Network, wxEvtHandler)
	EVT_TIMER(ID_TIMER, Network::OnStabalize)
END_EVENT_TABLE()

Network* Network::instance_ = 0;
wxCriticalSection* Network::conCrit_ = new wxCriticalSection;
wxCriticalSection* Network::killCrit_ = new wxCriticalSection;
wxCriticalSection* Network::nextCrit_ = new wxCriticalSection;
wxCriticalSection* Network::prevCrit_ = new wxCriticalSection;
wxCriticalSection* Network::transferCrit_ = new wxCriticalSection;
wxCriticalSection* Network::joinCrit = new wxCriticalSection;

Network* Network::Instance()
{
	if (instance_ == (Network*)0)
	{
		instance_ = new Network();
		return instance_;
	}
	else
	{
		return instance_;
	}
}

Network::Network()
	: status_(STATUS_NONE), killAll_(FALSE)

{
	stabalizeTimer_ = new wxTimer(this, ID_TIMER);
	allowJoin = TRUE;
	//distributeOnce = TRUE;
	hashTable = new HashTable();
}

bool
Network::GetStatus(int status)
{
	return (status_ & status);
}

void
Network::SetStatusOn(int status,wxString msg)
{
	// make sure the bit we are after is on
	status_ = (status_ | status);

	statMsg_ = msg;

	PostNetworkEvent(EVENT_STATUSUPDATE);
}

void
Network::SetStatusOff(int status,wxString msg)
{
	// turn the bit we are after is off
	status_ = (status_ & ~status);

	statMsg_ = msg;

	PostNetworkEvent(EVENT_STATUSUPDATE);
}

bool
Network::KillAll()
{
	wxCriticalSectionLocker locker(*killCrit_);

	return killAll_;
}

void
Network::Start(wxEvtHandler* starter, int starterId, bool seed)
{
	// clear messages
	statMsg_ = "";
	errMsg_ = "";

	// save parent for sending events
	starter_ = starter;
	starterId_ = starterId;

	// clear finger table
	finger.Clear();

	// clear status
	status_ = STATUS_NONE;

	// clear next table
	ClearNext();

	// clear prev
	prev_ = Node();

	//startTime_.SetToCurrent();

	/*
	Refresh shared directory.
	@todo Put in a thread!
	*/
	SetStatusOn(STATUS_REFRESHSHARE);
	Prefs::Instance()->ReloadShareDir();
	SetStatusOff(STATUS_REFRESHSHARE);

	/*
	Generate RSA keys.
	*/
#ifdef ENABLE_ENCRYPTION
	rsaPriv = Prefs::Instance()->Get(Prefs::RSAPRIV);
	rsaPub = Prefs::Instance()->Get(Prefs::RSAPUB);
	if (rsaPriv == "" || rsaPub == "" || Prefs::Instance()->Get(Prefs::REGENRSA) == "1")
	{
		LogMsg("Generating RSA public/private keys.");

		SetStatusOn(STATUS_ENCRYPTION);
		::wxYield();

		string pub;
		string priv;

		if (!crypto::GenerateRSAKeys(RSA_LENGTH,priv,pub))
		{
			KillNetwork("Failed to generate RSA keys.");
			return;
		}

		rsaPriv = priv.c_str();
		rsaPub = pub.c_str();

		Prefs::Instance()->Set(Prefs::RSAPRIV,rsaPriv);
		Prefs::Instance()->Set(Prefs::RSAPUB,rsaPub);
		Prefs::Instance()->Save();

		SetStatusOff(STATUS_ENCRYPTION);
		::wxYield();
	}
#endif
	/*
	Handle connection events
	*/
	ConnectEvent();

	//insertCount_ = 0;

	killCrit_->Enter();
	killAll_ = FALSE;
	killCrit_->Leave();

	// start the server socket
	if (!StartServer(convert<unsigned short>(Prefs::Instance()->Get(Prefs::PORT))))
	{
		KillNetwork("Could not start server.");
		return;
	}

	if (!seed)
	{
		// load a fresh hostcahe
		if (!Prefs::Instance()->GetHostCache()->Load())
		{
			KillNetwork("Failed to load hostcache file.");
			return;
		}


		/*
		call the first Bootstrap() ... after this it will keep itself up
		*/
		Bootstrap();
	}
	else
	{

		// load a fresh hostcahe (don't care if it fails)
		Prefs::Instance()->GetHostCache()->Load();

		/*
		assign self nid and our prev and next pointers are ourself
		*/
		Prefs::Instance()->Set(Prefs::NID,ComputeNid("0123456789"));
		AddNext(Node(Prefs::Instance()->Get(Prefs::NID),PublicAddress()));

		SetStatusOn(STATUS_SEEDING);
	}
}

void
Network::Distribute()
{
	static bool distributeOnce = TRUE;

	if (distributeOnce)
	{

		//wxThread::Sleep(10);

		LogDebug("Distributing ...");

		distributeOnce = FALSE;

//#if 0
		/*
		File Details
		hash = hash(token)
		info = FILE:filename:sizeInBytes:md5
		*/

		/*
		insert hashes we own into distributed hash table
		*/
		for (unsigned int x = 0; x < Prefs::Instance()->sharedFiles.size();x++)
		{
			wxString info = "FILE:" +
					Prefs::Instance()->sharedFiles[x].Name() + ":" +
					wxString().Format("%d",Prefs::Instance()->sharedFiles[x].Size()) + ":" +
					Prefs::Instance()->sharedFiles[x].MD5();

			Insert(Prefs::Instance()->sharedFiles[x].Hash(),info);

		}

		/*
		Buddy Details
		hash = hash(alias)
		info = BUDDY:alias:publickey:profile
		*/
//#endif
		/*
		Insert myself as a buddy.
		*/
		Insert(crypto::Sha1Hash(Prefs::Instance()->Get(Prefs::ALIAS)),
					"BUDDY:" + Prefs::Instance()->Get(Prefs::ALIAS) + ":" +
						Prefs::Instance()->Get(Prefs::PUBLICKEY) + ":" +
							Prefs::Instance()->Get(Prefs::PROFILE),TRUE);
	}
}

void
Network::InitNetwork()
{
	LogMsg("Initializing network ...");

	SetStatusOn(STATUS_ALIVE);
	SetStatusOff(STATUS_SEEDING);

	/*
	Say hello to all of our buddies (if we find them)
	*/
	SayHelloToBuddies(wxGetApp().GetMainWindow()->GetTreeCtrl()->GetRootItem(),wxTreeItemIdValue());

	// start stabalize timer
	stabalizeTimer_->Stop();
	stabalizeTimer_->Start(STABALIZE_INTERVAL,FALSE);
}

bool Network::IsSuccessor(wxString nid, wxString prev,wxString hash)
{
	bool answer = ((MYlte(hash,nid) && MYgt(hash,prev) && prev != "")
			||
			(MYgt(hash,nid) && MYgt(hash,prev) && MYgt(prev,nid) && prev != "")
			||
			(MYlte(hash,nid) && MYgt(prev,nid) && prev != "")
			||
			prev == "");	// if we have no prev we are seed and ALWAYS successor
	if (!answer)
		LogDebug(wxString().Format("I am %s and my prev is %s and i AM NOT the succesor of %s",nid.c_str(),prev.c_str(),hash.c_str()));
	else
		LogDebug(wxString().Format("I am %s and my prev is %s and i AM the succesor of %s",nid.c_str(),prev.c_str(),hash.c_str()));

	return answer;
}

void Network::KillNetwork(wxString msg)
{
	LogMsg(wxString().Format("Shutting network down (%s)",msg.c_str()));

	/*
	Don't handle any more connection events.
	*/
	DisconnectEvent();

	SetStatusOff(STATUS_ALIVE);
	SetStatusOff(STATUS_SEEDING);
	SetStatusOff(STATUS_BOOTSTRAPPING);
	SetStatusOff(STATUS_ENCRYPTION);
	SetStatusOff(STATUS_REFRESHSHARE);

	if (msg != "")
	{
		SetStatusOn(STATUS_ERROR);
		errMsg_ = msg;
	}

	/*
	Tell all the connections to die.
	*/
	killCrit_->Enter();
	killAll_ = TRUE;
	killCrit_->Leave();

	// stop stabalize timer
	stabalizeTimer_->Stop();

	// make sure the hostcache is saved
	Prefs::Instance()->GetHostCache()->Save();

	// set all buddies to offline
	AllBuddiesOffline(wxGetApp().GetMainWindow()->GetTreeCtrl()->GetRootItem(),wxTreeItemIdValue());

}

void
Network::Bootstrap()
{
	// get the next hostcache address
	wxString host = Prefs::Instance()->GetHostCache()->GetNext();

	if (host.Length() == 0)
	{
		KillNetwork("Ran out of bootstrap hosts to try.  Try updating your hostcache file.");
	}
	else
	{
		// don't connect to ourselves, it just can't work
		if (IsMe(host))
		{
			Bootstrap();
			return;
		}

		SetStatusOn(STATUS_BOOTSTRAPPING, host);

		LogMsg("Trying to bootstrap to host " + host);

		ConnectionThread* thread = new ConnectionThread(this,ID_NETWORK,Connection::TYPE_BOOTSTRAP,host);

		if (thread->Create() != wxTHREAD_NO_ERROR)
			LogError("Failed to create bootstrap connection thread!");

		// run the thread
		thread->Run();
	}
}

bool
Network::StartServer(int port)
{
	Server* thread = new Server(port);

	if (thread->Create() != wxTHREAD_NO_ERROR)
	{
		LogDebug("Can't create server thread!");
		return false;
	}

	// run the thread
	thread->Run();

	return TRUE;
}

void
Network::OnConnectionEvent(wxCommandEvent& event)
{
	int n = event.GetInt();
	
	switch(n)
	{
	case Connection::EVENT_NEXT_CONNECTED:
	case Connection::EVENT_PREV_CONNECTED:
	{
		/*
		Distribute all of our shit.
		*/
		Node next;
		Network::Instance()->GetNext(0,next);
		if (prev_.Nid() != "" && next.Nid() != "" && next.Nid() != Prefs::Instance()->Get(Prefs::NID))
			Distribute();
		else
		{
			//
		}

		break;
	}
	case Connection::EVENT_MADE:
	case Connection::EVENT_FAILED:
	case Connection::EVENT_EXIT:
	{
		// we don't give a shit about the minor events of the connection
		// for now
		break;
	}
	case Connection::EVENT_NEXT_LOST:
	{

		stabalizeTimer_->Stop();

		// try to find another next
		Bootstrap();

		break;
	}
	case Connection::EVENT_INSERT_FINISHED:
	{
		/*
		We are online and all of our content has been distributed.
		*/
		InitNetwork();

		break;
	}
	default:
		LogError(wxString().Format("Unhandled (might not be a bad thing) connection event type %d sent to network.",n));
	}
}

void
Network::SayHelloToBuddies(const wxTreeItemId idParent, wxTreeItemIdValue cookie)
{
	BuddyTreeCtrl* tree = wxGetApp().GetMainWindow()->GetTreeCtrl();

	wxTreeItemId id;

	if(cookie == wxTreeItemIdValue())
		id = tree->GetFirstChild(idParent, cookie);
	else
		id = tree->GetNextChild(idParent, cookie);

	if(id <= 0)
		return;

	Buddy* buddy = (Buddy*)tree->GetItemData(id);
	if (buddy != 0)
	{
		//LogDebug(wxString().Format("******************* Saying hello to buddy %s *********************",buddy->Get(Buddy::DATA_ALIAS).c_str()));

		wxString buddyHash = crypto::Sha1Hash(buddy->Get(Buddy::DATA_ALIAS));

		if (buddyHash != "")
		{
			ConnectionThread* thread = new ConnectionThread(0,0,Connection::TYPE_HELLOBUDDY,"",buddyHash);

			if (thread->Create() != wxTHREAD_NO_ERROR)
				LogError("Failed to create connection thread!");

			// run the thread
			thread->Run();
		}
	}

	if (tree->ItemHasChildren(id))
	   SayHelloToBuddies(id,wxTreeItemIdValue());

	SayHelloToBuddies(idParent, cookie);
}

void
Network::AllBuddiesOffline(const wxTreeItemId idParent, wxTreeItemIdValue cookie)
{
	BuddyTreeCtrl* tree = wxGetApp().GetMainWindow()->GetTreeCtrl();

	wxTreeItemId id;

	if(cookie == wxTreeItemIdValue())
		id = tree->GetFirstChild(idParent, cookie);
	else
		id = tree->GetNextChild(idParent, cookie);

	if(id <= 0)
		return;

	Buddy* buddy = (Buddy*)tree->GetItemData(id);
	if (buddy != 0)
	{
		buddy->SetStatusOff(Buddy::STATUS_ONLINE);
	}

	if (tree->ItemHasChildren(id))
	   AllBuddiesOffline(id,wxTreeItemIdValue());

	AllBuddiesOffline(idParent, cookie);
}



void
Network::OnStabalize(wxTimerEvent& event)
{
	Stabalize();
}

void
Network::Stabalize()
{
	if (Join())
	{
		LogDebug("Stabalizing....");

		/*
		See if we need to spread out!  This basically means, see if another node
		slipped in between us and our next
		*/
		ConnectionThread* thread = new ConnectionThread(this,ID_NETWORK,Connection::TYPE_SPREAD);

		if (thread->Create() != wxTHREAD_NO_ERROR)
			LogError("Failed to create connection thread!");

		// run the thread
		thread->Run();
	}
}

void
Network::Insert(wxString hash, wxString info,bool getEvent)
{
	LogDebug(wxString().Format("Inserting %s",hash.c_str()));

	ConnectionThread* thread;
	if (getEvent)
		thread = new ConnectionThread(this,ID_NETWORK,Connection::TYPE_INSERT,"",hash,info);
	else
		thread = new ConnectionThread(0,-1,Connection::TYPE_INSERT,"",hash,info);

	if (thread->Create() != wxTHREAD_NO_ERROR)
		LogError("Failed to create insert connection thread!");

	// run the thread
	thread->Run();

	/*
	Just so we don't pound a particular host.
	*/
	//wxThread::Sleep(500);
}

void
Network::AddUpload(ShareFile* file)
{
	wxCriticalSectionLocker locker(*transferCrit_);

	uploads.push_back(file);
}

void
Network::RemoveUpload(ShareFile* file)
{
	wxCriticalSectionLocker locker(*transferCrit_);

	for (unsigned int x = 0; x < uploads.size();x++)
	{
		if (uploads[x] == file)
		{
			uploads.erase(uploads.begin()+x);
			delete uploads[x];
			break;
		}
	}
}

void
Network::RemoveDownload(ShareFile* file)
{
	wxCriticalSectionLocker locker(*transferCrit_);

	for (unsigned int x = 0; x < downloads.size();x++)
	{
		if (downloads[x] == file)
		{
			downloads.erase(downloads.begin()+x);
			delete downloads[x];
			break;
		}
	}
}

void
Network::AddDownload(ShareFile* file)
{
	wxCriticalSectionLocker locker(*transferCrit_);

	downloads.push_back(file);
}

void
Network::ConnectEvent()
{
	wxEvtHandler::Connect(ID_NETWORK, wxEVT_COMMAND_MENU_SELECTED,
		(wxObjectEventFunction)
                (wxEventFunction)
                (wxCommandEventFunction)&Network::OnConnectionEvent,
		(wxObject*) NULL);
}

void
Network::DisconnectEvent()
{
	wxEvtHandler::Disconnect(ID_NETWORK, wxEVT_COMMAND_MENU_SELECTED);
}

void
Network::PostNetworkEvent(int id, void* data)
{
	wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, starterId_);
	event.SetInt(id);
	event.SetClientData(data);
	wxPostEvent(starter_,event);
}

void
Network::Prev(const Node& node)
{
	wxCriticalSectionLocker locker(*prevCrit_);

	prev_ = node;

	LogDebug(wxString().Format("Current prev is %s",node.Address().c_str()));
}

Node
Network::Prev()
{
	wxCriticalSectionLocker locker(*prevCrit_);

	return prev_;
}

wxString
Network::ErrorMsg()
{
	wxString answer = errMsg_;
	errMsg_ = "";
	return answer;
}

wxString
Network::StatusMsg()
{
	return statMsg_;
}

#ifdef DEBUG
void
Network::PrintNext()
{
	wxCriticalSectionLocker locker(*nextCrit_);

	for (unsigned int x = 0; x < nextTable_.size();++x)
	{
		LogDebug(nextTable_[x].Address());
	}
}
#endif

bool Network::GetNext(unsigned int index, Node& next)
{
	wxCriticalSectionLocker locker(*nextCrit_);

	if (index < nextTable_.size() && nextTable_.size() != 0)
	{
		next = nextTable_[index];
		return TRUE;
	}
	return FALSE;
}

void
Network::AddNext(const Node& next)
{
	wxCriticalSectionLocker locker(*nextCrit_);

	nextTable_.push_back(next);

#ifdef DEBUG
	//LogDebug("Next Table:");
	//PrintNext();
#endif
}

void
Network::ClearNext()
{
	wxCriticalSectionLocker locker(*nextCrit_);

	nextTable_.clear();
}

void
Network::EraseNext(unsigned int x)
{
	wxCriticalSectionLocker locker(*nextCrit_);

	nextTable_.erase(nextTable_.begin()+x);
}

unsigned int
Network::NextSize()
{
	wxCriticalSectionLocker locker(*nextCrit_);

	return nextTable_.size();
}

void
Network::SetGateway(const wxString& addr)
{
	if (addr != "" && addr != wxString(SocketBase::LocalIP().c_str()))
	{
		gateway_ = addr;
	}
}

wxString
Network::PublicAddress()
{
	wxString answer;
	if (gateway_ != "" && Prefs::Instance()->Get(Prefs::USEFIREWALL) == "1")
		answer = gateway_ + "/" + Prefs::Instance()->Get(Prefs::PORT);
	else
		answer = wxString(SocketBase::LocalIP().c_str()) + "/" + Prefs::Instance()->Get(Prefs::PORT);

	//LogDebug(wxString().Format("My public address is:%s",answer.c_str()));

	return answer;
}

bool
Network::IsMe(const wxString& addr)
{
	wxString local = wxString(SocketBase::LocalIP().c_str()) + "/" + Prefs::Instance()->Get(Prefs::PORT);
	return (addr == local || addr == gateway_);
}

wxString
Network::ComputeNid(const wxString& address)
{
	//return crypto::Sha1Hash(wxString().Format("%s%d",address.c_str(),wxGetLocalTime));
	return crypto::Sha1Hash(address);
}

void
Network::Stop()
{;}

Network::~Network()
{
	/*
	Send a disconnect event.
	...I don't think this is really worth doing.
	*/
	DisconnectEvent();

	KillNetwork();

	delete hashTable;
}
	

