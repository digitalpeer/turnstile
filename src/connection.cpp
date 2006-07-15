/* $Id: connection.cpp,v 1.2 2004/07/02 23:24:22 dp Exp $
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

/** @file connection.cpp
@brief Connection Definition
*/
#include "connection.h"
#include "buddytreectrl.h"
#include "mainframe.h"
#include "buddy.h"
#include "app.h"
#include "libjdhsocket/clientsocket.h"
#include "typeconv.h"
#include "prefs.h"
#include "utils.h"
#include "packet.h"
#include "files.h"
#include "lookupresponse.h"
#include "network.h"
#include "hashtable.h"
#include "hostcache.h"
#include "packetgen.h"
#include "imeventqueue.h"

/*#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif*/

#ifdef ENABLE_ENCRYPTION
#include "crypto.h"
#endif

#include <wx/event.h>
#include <wx/app.h>
#include <wx/tokenzr.h>
#include <wx/file.h>
#include <wx/timer.h>

#include <string>
using namespace std;
using namespace JdhSocket;

int Connection::idCounter_ = 9000;

Connection::Connection(wxEvtHandler* parent,int parentId, ClientSocket* socket)
	: 	parent_(parent), parentId_(parentId),id_(++idCounter_),
		packetsReceived_(0), packetsSent_(0),
		type_(TYPE_SERVER), status_(STATUS_DEAD), socket_(socket),
		sw_(new wxStopWatch)
#ifdef ENABLE_ENCRYPTION
		,symetric_(new crypto::TwoFishCipher)
#endif
{;}

Connection::Connection(wxEvtHandler* parent,int parentId, int type,
				wxString addr,wxString a)
	: 	parent_(parent), parentId_(parentId), id_(++idCounter_),
		packetsReceived_(0), packetsSent_(0),
		type_(type), status_(STATUS_DEAD), currentAddr_(addr),a_(a),
		socket_(new ClientSocket()),
		sw_(new wxStopWatch)
#ifdef ENABLE_ENCRYPTION
		,symetric_(new crypto::TwoFishCipher)
#endif
{;}

// this function does not send events
bool
Connection::Connect(/*bool create*/)
{
   if (currentAddr_ == wxT(""))
	{
	   LogError(wxString().Format(wxT("Blank address in con id %d with type %d"),id_,type_));
		return FALSE;
	}

	/*
	Here we are going to setup the socket, make a TCP connection, then
	do the higher level protocol handshaking.
	*/

   LogDebug(wxString().Format(wxT("Connecting to address %s in con id %d with type %d"),currentAddr_.c_str(),id_,type_));

	/*if (create)
	{
		if (!socket_->Create())
		{
			LogError(wxString().Format(wxT("Failed to create socket in con id %d"),id_));
			return FALSE;
		}

	}*/

	if (!socket_->Connect(ParseHost(currentAddr_).c_str(),ParsePort(currentAddr_),CONNECTION_WAIT))
	{
	   LogError(wxString().Format(wxT("Failed3 to connect to address %s in con id %d"),
				currentAddr_.c_str(),id_));
	   LogError(wxString().Format(wxT("Error: %s"), socket_->ErrorMessage().c_str()));
		return FALSE;
	}

	/*
	Now, lets do some booty shaking.  There are two ways we can handshake.
	*/
	if (type_ == TYPE_BOOTSTRAP)
		return InitWithJoin();
	else
		return InitWithHandshake();
}

// this function does not send events
bool
Connection::InitWithHandshake()
{
	/*
	Send the SYN packet.
	*/
	Packet snd = GenPacket_Syn(Network::Instance()->rsaPub);
	if (!Send(snd))
	{
	   LogError(wxString().Format(wxT("Send failed when trying to handshake on con id %d"),id_));
		return FALSE;
	}
	
	/*
	Get the ACK packet.
	*/
	Packet* rcv;
	if (!Receive(rcv) || rcv == 0)
	{
	   LogError(wxString().Format(wxT("Receive failed when trying to handshake on con id %d"), id_));
		return FALSE;
	}

	switch(rcv->Type())
	{
	case Packet::TYPE_ACK:
	{
		// this is the address and nid of the host we are working with, if
		//currentAddr_ = rcv->Get(Packet::HEADER_SRC_ADDR);
	   currentAddr_ = wxString().Format(wxT("%s/%d"),socket_->GetPeerHost().c_str(),ParsePort(rcv->Get(Packet::HEADER_SRC_ADDR)));
		currentNid_ = rcv->Get(Packet::HEADER_SRC_NID);

#ifdef ENABLE_ENCRYPTION
		wxString key = rcv->Get(Packet::HEADER_SYMETRIC_KEY);
		symetric_->Init(key.c_str()/*,(unsigned char*)"blah"*/);
#endif

		Network::Instance()->SetGateway(rcv->Get(Packet::HEADER_DEST_HOST));

		if (currentAddr_ == wxT("") || currentNid_ == wxT(""))
		{
		   LogError(wxString().Format(wxT("(1) Blank address or nid from node when trying to handshake on con id %d"),id_));
			delete rcv;
			return FALSE;
		}

		Packet snd = GenPacket_Confirm(wxString(socket_->GetPeerHost().c_str()));
		if (!Send(snd))
		{
		   LogError(wxString().Format(wxT("Send failed when trying to handshake on con id %d"),id_));
			return FALSE;
		}

		break;
	}
	default:
	{
	   LogError(wxString().Format(wxT("Invalid packet type %d when trying to handshake on con id %d"),rcv->Type(), id_));
		delete rcv;
		return FALSE;
	}
	}

	/*
	Lets store this node in the finger table and heck, even the hostcache.
	*/
	Network::Instance()->finger.Add(Node(currentNid_,currentAddr_));
	Prefs::Instance()->GetHostCache()->AddHost(currentAddr_);

	delete rcv;

	return TRUE;
}

bool
Connection::InitWithJoin()
{
	/*
	Send JOIN packet.
	*/
	Packet snd = GenPacket_Join(Network::Instance()->rsaPub);
	if (!Send(snd))
	{
	   LogError(wxString().Format(wxT("Send failed when trying to join on con id %d"),id_));
		return FALSE;
	}

	/*
	Receive JOIN_RESPONSE packet.
	*/
	Packet* rcv;
	if (!Receive(rcv) || rcv == 0)
	{
	   LogError(wxString().Format(wxT("Receive failed when trying to join on con id %d"), id_));
		return FALSE;
	}

	switch(rcv->Type())
	{
	case Packet::TYPE_JOIN_RESPONSE:
	{
		currentAddr_ = rcv->Get(Packet::HEADER_SRC_ADDR);
		currentNid_ = rcv->Get(Packet::HEADER_SRC_NID);

		wxString assign = rcv->Get(Packet::HEADER_ASSIGN_NID);

		wxString address = rcv->Get(Packet::HEADER_NODE);
		wxString nid = rcv->Get(Packet::HEADER_NID);

		Network::Instance()->SetGateway(rcv->Get(Packet::HEADER_DEST_HOST));

		delete rcv;

		if (currentAddr_ == wxT("") || currentNid_ == wxT(""))
		{
		   LogError(wxString().Format(wxT("(2) Blank address or nid from node when trying to handshake on con id %d"),id_));
			return FALSE;
		}

		if (assign == wxT(""))
		{
		   LogError(wxString().Format(wxT("Join response included a blank assign nid on con id %d"),id_));
			return FALSE;
		}

		Prefs::Instance()->Set(Prefs::NID,assign);

		LogDebug(wxString().Format(wxT("Found parent: %s (%s) on con id %d"),nid.c_str(),address.c_str(), id_));

		/*
		We found our new parent, so say hello.
		*/
		if (!HelloParent(Node(nid,address)))
		{
			PostConEvent(EVENT_NEXT_LOST);
			return FALSE;
		}
		else
		{
			PostConEvent(EVENT_NEXT_CONNECTED);
		}

		break;
	}
	default:
	{
		delete rcv;
		LogError(wxString().Format(wxT("Invalid packet type %d on con id %d"),rcv->Type(), id_));
		return FALSE;
	}
	}

	/*
	Lets save this node in the finger table.
	*/
	Network::Instance()->finger.Add(Node(currentNid_,currentAddr_));
	Prefs::Instance()->GetHostCache()->AddHost(currentAddr_);

	return TRUE;
}

// this function does not send events
bool
Connection::ReceiveHandShake()
{
	// TODO check the banned list!

	/*
	Get the SYN or JOIN packet.
	*/
	Packet* rcv;
	if (!Receive(rcv) || rcv == 0)
	{
	   LogError(wxString().Format(wxT("Receive failed on incomming con id %d"), id_));
		return FALSE;
	}

	switch(rcv->Type())
	{
	// node is a network node and wants to handshake
	case Packet::TYPE_SYN:
	{

#ifdef ENABLE_ENCRYPTION
		rsaPub_ = rcv->Get(Packet::HEADER_PUBLIC_KEY);

		if (rsaPub_ == wxT(""))
		{
		   LogError(wxString().Format(wxT("Got empty RSA public key on incomming con id %d"), id_));
			return false;
		}
#endif

		delete rcv;

#ifdef ENABLE_ENCRYPTION
		/*
		Init encryption.
		*/
		symetric_->Init(crypto::RandomString().c_str()/*,(unsigned char*)"blah"*/);
#endif
		/*
		Send the ACK packet.
		*/
#ifdef ENABLE_ENCRYPTION
		Packet snd = GenPacket_Ack(wxString(symetric_->Key().c_str()),wxString(socket_->GetPeerHost().c_str()));
#else
		Packet snd = GenPacket_Ack(wxString(),wxString(socket_->GetPeerHost().c_str()));
#endif
		if (!Send(snd))
		{
		   LogError(wxString().Format(wxT("ACK send failed on incomming con id %d"),id_));
			return FALSE;
		}

		if (!Receive(rcv) || rcv == 0)
		{
		   LogError(wxString().Format(wxT("Receive confirm packet failed on incomming con id %d"), id_));
			return FALSE;
		}

		if (rcv->Type() != Packet::TYPE_CONFIRM)
		{
		   LogError(wxString().Format(wxT("Did not get confirm packet on incomming con id %d, got packet type %d instead"), id_,rcv->Type()));
			return FALSE;
		}

		// TODO get the correct hostname if none is supplied
		currentAddr_ = rcv->Get(Packet::HEADER_SRC_ADDR);

		currentNid_ = rcv->Get(Packet::HEADER_SRC_NID);
		wxString wrkGrpKey = rcv->Get(Packet::HEADER_WRKGRPKEY);
		Network::Instance()->SetGateway(rcv->Get(Packet::HEADER_DEST_HOST));

		if (Prefs::Instance()->Get(Prefs::WRKGRPKEY) != wxT("")
			&&
			wrkGrpKey != Prefs::Instance()->Get(Prefs::WRKGRPKEY))
		{
		   LogError(wxString().Format(wxT("Node tried to connect with invalid workgroup %s key on incomming con id %d packet: %s"), wrkGrpKey.c_str(),id_,rcv->Raw()));
			delete rcv;
			return FALSE;
		}


		if (currentAddr_ == wxT("") || currentNid_ == wxT(""))
		{
		   LogError(wxString().Format(wxT("(3) Blank address or nid from node when trying to handshake on con id %d"),id_));
			delete rcv;
			return FALSE;
		}

		delete rcv;

		/*
		Now that handshaking is out of the way, lets figure out what this node wants.
		*/
		if (!Receive(rcv) || rcv == 0)
		{
		   LogError(wxString().Format(wxT("Receive failed on incomming con id %d"), id_));
			return FALSE;
		}

		switch(rcv->Type())
		{
		case Packet::TYPE_QUERY:
		{
			// get the hash we are being queried for
			wxString hash = rcv->Get(Packet::HEADER_HASH);

			LogDebug(wxString().Format(wxT("Responding to a query for '%s' on incomming con id %d"),hash.c_str(),id_));

			delete rcv;

			if (Network::Instance()->IsSuccessor(Prefs::Instance()->Get(Prefs::NID),
				Network::Instance()->Prev().Nid(),hash))
			{
				Packet answer = GenPacket_ResponseSuccess();
				bool found = FALSE;
				HashValue value;
				for (int x = 1; ;x++)
				{
					value = Network::Instance()->hashTable->GetNext(hash,x);

					if (value.hash == wxT(""))
						break;

					// responsible node information
					answer.Add(Packet::HEADER_NODE, value.node);
					answer.Add(Packet::HEADER_INFO, value.info);

					// yes, we are responsible for this mess
					found = TRUE;
				}

				if (found)	// we are responsible for the hash and we have it
				{
				   LogDebug(wxString().Format(wxT("We have the hash %s"),hash.c_str()));

					if (!Send(answer))
					{
						return FALSE;
					}
				}
				else	// we are responsible for the hash but don't have it
				{
				   LogDebug(wxString().Format(wxT("We have no entry matching hash %s"),hash.c_str()));

					Packet snd = GenPacket_ResponseEnd();
					if (!Send(snd))
					{
					   LogError(wxString().Format(wxT(" Send failed on incomming con id %d"),id_));
						return FALSE;
					}
				}
			}
			else	// not resposible, so tell them where to jump to next
			{
				// TODO use the finger table to find a jump node
				Node next;
				if (!Network::Instance()->GetNext(0,next))
				{
				   LogError(wxT("Could not get next to generate jump packet."));
				}

				Packet snd = GenPacket_ResponseJump(next.Address());
				if (!Send(snd))
				{
				   LogError(wxString().Format(wxT(" Send jump failed on incomming con id %d"),id_));
					return FALSE;
				}
			}

			break;
		}
		case Packet::TYPE_HELLO_NEXT:
		{
			delete rcv;

			LogDebug(wxString().Format(wxT("Got a hello next from %s"),currentNid_.c_str()));

			bool dojoin = FALSE;
			//if (Network::Instance()->Prev().Nid() == wxT("") &&
			//		Network::Instance()->seed_)
			Node next;
			Network::Instance()->GetNext(0,next);
			if (next.Nid() == Prefs::Instance()->Get(Prefs::NID))
			{
				dojoin = TRUE;
			}

			/*
			We now have a new previous!
			*/
			Network::Instance()->Prev(Node(currentNid_,currentAddr_));

			// TODO do a lookup and make sure we are the real successor!

			/*
			A new node just said it is my new child, so I have to:
			- send it all the hashes < than the new node's nid
			*/
			Packet insert = GenPacket_Insert();
			HashValue value;
			//bool found = FALSE;
			for (unsigned int x = 1;;++x)
			{
				value = Network::Instance()->hashTable->Get(x);

				if (value.hash == wxT(""))
					break;

				if (!Network::Instance()->IsSuccessor(Prefs::Instance()->Get(Prefs::NID),
					Network::Instance()->Prev().Nid(),value.hash))
				{

				   LogDebug(wxString().Format(wxT("Giving up hash %s to prev %s"),value.hash.c_str(),currentNid_.c_str()));

					insert.Add(Packet::HEADER_HASH,value.hash);
					insert.Add(Packet::HEADER_NODE,value.node);
					insert.Add(Packet::HEADER_INFO,value.info);
				}
				//found = TRUE;
			}

			// send an insert even if we have no hashes, this means "OK"
			if (!Send(insert))
			{
			   LogError(wxString().Format(wxT("Send insert failed on incomming con id %d"),id_));
				return FALSE;
			}

			/*
			If we don't have a next pointer then use the first new child (this will happen if
			we are a seed)
			*/
			if (dojoin)
			{
			   LogDebug(wxString().Format(wxT("Seed node, so joining parent %s"),currentNid_.c_str()));

				/*
				Now that we have a next pointer, lets say hey to it.
				*/
				if (!HelloParent(Node(currentNid_,currentAddr_)))
				{
					// if we can't connect to the node that just connected to us we lost our next
					PostConEvent(EVENT_NEXT_LOST);
					return FALSE;
				}
				else
				{
					//PostConEvent(EVENT_NEXT_CONNECTED);
				}
			}

			PostConEvent(EVENT_PREV_CONNECTED);

			break;
		}
		case Packet::TYPE_STAT:
		{
			delete rcv;

			Packet snd = GenPacket_StatResponse();
			if (!Send(snd))
			{
			   LogError(wxString().Format(wxT("STAT_RESPONSE send failed on incomming con id %d"),id_));
				return FALSE;
			}
			break;
		}
		case Packet::TYPE_REQUEST:
		{
			/*
			Client is requesting a file we have.
			... No need to check if we have the file or not here, the filetransferthread
			will fail if we don't have the file.
			*/

			// get the filename and an md5 of the file requested
			wxString md5 = rcv->Get(Packet::HEADER_FILEMD5);
			wxString filename = rcv->Get(Packet::HEADER_FILENAME);

			delete rcv;

			if (currentAddr_ == wxT("") || currentNid_ == wxT(""))
			{
			   LogError(wxString().Format(wxT("(4) Blank address or nid from node when trying to handshake on con id %d"),id_));
				delete rcv;
				return FALSE;
			}

			bool found = FALSE;
			for (unsigned int x = 0; x < Prefs::Instance()->sharedFiles.size();x++)
			{
				if (Prefs::Instance()->sharedFiles[x].Name() == filename &&
					Prefs::Instance()->sharedFiles[x].MD5() == md5)
				{
				   LogDebug(wxString().Format(wxT("Sending file %s with MD5 %s on incomming con id %d"), filename.c_str(),md5.c_str(),id_));
					ShareFile* file = new ShareFile(Prefs::Instance()->sharedFiles[x]);
					Put(file);
					found = TRUE;
					break;
				}
			}

			if (!found)
			{
			   LogError(wxString().Format(wxT(" Don't have request for file %s with MD5 %s on incomming con id %d"), filename.c_str(),md5.c_str(),id_));
				return FALSE;
			}

			break;
		}
		case Packet::TYPE_HELLO_BUDDY:
		{
			/*
			A buddy has signed on and is now saying hello to us.
			Set his status to online and save some info about him.
			*/
			wxString info = rcv->Get(Packet::HEADER_INFO);

			delete rcv;

			Buddy* buddy = 0;

			wxStringTokenizer tokens(info, wxT(":"));
			// TODO make sure the buddy info is valid (we have all the tokens)
			if (tokens.GetNextToken() == wxT("BUDDY"))
			{
				wxMutexGuiEnter();

				BuddyTreeCtrl* tree = wxGetApp().GetMainWindow()->GetTreeCtrl();
				wxString alias = tokens.GetNextToken();
				wxString publicKey = tokens.GetNextToken();
				wxString profile = tokens.GetNextToken();
				tree->SetBuddyData(publicKey, alias, profile, currentAddr_);

				buddy = tree->GetBuddy(publicKey);
				if (buddy != 0)
					buddy->SetStatusOn(Buddy::STATUS_ONLINE);

				wxMutexGuiLeave();

				LogMsg(wxString().Format(wxT("Found that buddy '%s' with id '%s' is online."),alias.c_str(),publicKey.c_str()));
			}
			else
			{
			   LogError(wxT("Got an invalid info header on buddy hello."));
			}

			break;
		}
		case Packet::TYPE_IM_CONNECT:
		{
			/*
			A buddy wants to connect to us.
			*/
			wxString info = rcv->Get(Packet::HEADER_INFO);

			delete rcv;

			Buddy* buddy = 0;

			wxStringTokenizer tokens(info, wxT(":"));
			// TODO make sure the buddy info is valid (we have all the tokens)
			if (tokens.GetNextToken() == wxT("BUDDY"))
			{
				wxString alias = tokens.GetNextToken();
				wxString publicKey = tokens.GetNextToken();
				wxString profile = tokens.GetNextToken();

				wxMutexGuiEnter();

				BuddyTreeCtrl* tree = wxGetApp().GetMainWindow()->GetTreeCtrl();
				buddy = tree->GetBuddy(publicKey);
				if (buddy != 0)
				{
					tree->SetBuddyData(publicKey, alias, profile, currentAddr_);

					LogMsg(wxString().Format(wxT("Buddy '%s' with id '%s' wants to connect."),alias.c_str(),publicKey.c_str()));
					//buddy->SetStatusOn(Buddy::STATUS_ONLINE);
				}
				else
				{
				   wxMessageDialog diag(0,wxT("Buddy '")+ alias + wxT("' wants to make an instant message connection to you. Do you want to accept this connection and add this buddy to your list?"),wxT("New Buddy Connection"),wxYES|wxNO);
					if (diag.ShowModal() == wxYES)
					{
					}
				}
				wxMutexGuiLeave();
			}
			else
			{
			   LogError(wxT("Got an invalid info header on buddy hello."));
			}

			if (buddy != 0)
			{
				parent_ = buddy;
				parentId_ = buddy->id();

				ImEventQueue* queue = new ImEventQueue();

				PostConEvent(ImEvent::EVENT_MADE,(void*)queue);

				/*
				Process the IM events.
				*/
				HandleImEvents(queue);

				PostConEvent(ImEvent::EVENT_LOST);

				//delete queue;
			}

			break;
		}
		case Packet::TYPE_INSERT:
		{
			// somebody wants to give us some hashes to be responsible for
			// TODO we should validate if we are responsible for them or not
			for (int x = 0; x < rcv->Count(Packet::HEADER_HASH); x++)
			{
				wxString hash = rcv->Get(Packet::HEADER_HASH,x);
				wxString node = rcv->Get(Packet::HEADER_NODE,x);
				wxString info = rcv->Get(Packet::HEADER_INFO,x);
				Network::Instance()->hashTable->Add(HashValue(node,hash,info));
				
				LogDebug(wxString().Format(wxT("Now responsible for hash: %s with owner %s and info %s"), hash.c_str(),node.c_str(),info.c_str()));
			}

			delete rcv;

			break;
		}
		default:
		{
		   LogError(wxString().Format(wxT("Invalid packet on incomming con id %d"), id_));
			delete rcv;
			return FALSE;
		}
		}

		break;
	}
	// node is not a network node and wants to join
	case Packet::TYPE_JOIN:
	{
		//currentAddr_ = rcv->Get(Packet::HEADER_SRC_ADDR);
	   currentAddr_ = wxString().Format(wxT("%s/%d"),socket_->GetPeerHost().c_str(),ParsePort(rcv->Get(Packet::HEADER_SRC_ADDR)));
		rsaPub_ = rcv->Get(Packet::HEADER_PUBLIC_KEY);
		wxString wrkGrpKey = rcv->Get(Packet::HEADER_WRKGRPKEY);

		if (Prefs::Instance()->Get(Prefs::WRKGRPKEY) != wxT("")
			&&
			wrkGrpKey != Prefs::Instance()->Get(Prefs::WRKGRPKEY))
		{
		   LogError(wxString().Format(wxT(" Node tried to connect with invalid workgroup %s key on incomming con id %d packet: %s"), wrkGrpKey.c_str(),id_,rcv->Raw()));
			delete rcv;
			return FALSE;
		}

		// compute an NID for the new node
		currentNid_ = Network::Instance()->ComputeNid(currentAddr_);

		delete rcv;

		if (currentAddr_ == wxT(""))
		{
		   LogError(wxString().Format(wxT("(5) Blank address or nid from node when trying to handshake on con id %d"),id_));
			delete rcv;
			return FALSE;
		}
#ifdef DEBUG
		// make sure the nid we computed is valid
		if (currentNid_.size() != 40)
		{
			// this should never happen, but whatever
		   LogError(wxT("Computed am invalid blank nid for joining node."));
			return FALSE;
		}
#endif
		/*
		Now, we need to do a lookup and find the parent of the node.
		*/
		LookupResponse answer = Lookup(currentNid_);

		switch(answer.type_)
		{
		case LookupResponse::SUCCESSOR:
		case LookupResponse::FOUND:
		{
			/*
			Found your successor.  If we get anything else from the lookup
			it means we can't do it and drop the connection (this should never happen!)
			*/
			Packet snd = GenPacket_JoinResponse(currentNid_,answer.successor,wxString(socket_->GetPeerHost().c_str()));
			if (!Send(snd))
			{
			   LogError(wxString().Format(wxT("JOIN_RESPONSE send failed on incomming con id %d"),id_));
				return FALSE;
			}
			break;
		}
		default:
		{
		   LogError(wxString().Format(wxT("Lookup failed when trying to find parent for new node on incomming con id %d"),id_));
			return FALSE;
		}
		}

		break;
	}
	default:
	{
	   LogError(wxString().Format(wxT("Invalid packet type %d on incomming con id %d"),rcv->Type(), id_));
		delete rcv;
		return FALSE;
	}
	}

	/*
	Save the node in the finger table.
	*/
	Network::Instance()->finger.Add(Node(currentNid_,currentAddr_));

	return TRUE;
}

LookupResponse
Connection::DirectQuery(wxString key)
{
	LookupResponse answer(LookupResponse::FAIL);

 	if (!Connect())
	{
	   LogError(wxString().Format(wxT("Connect() failed when doing a direct query in con id %d"),id_));
		KillSocket(-1);
		return answer;
	}

	/*
	Send QUERY.
	*/
	Packet snd = GenPacket_Query(key);
	if (!Send(snd))
	{
	   LogError(wxString().Format(wxT("Send failed when trying to direct query on con id %d"),id_));
		KillSocket(-1);
		return answer;
	}

	/*
	Receive some sort of query response.
	*/
	Packet* rcv;
	if (!Receive(rcv) || rcv == 0)
	{
	   LogError(wxString().Format(wxT("Receive failed when trying to direct query on con id %d"), id_));
		KillSocket(-1);
		return answer;
	}

	KillSocket(-1);

	switch(rcv->Type())
	{
	case Packet::TYPE_RESPONSE_FOUND:
	{
		answer = LookupResponse(LookupResponse::FOUND,Node(currentNid_,currentAddr_));

		for (int x = 0; x < rcv->Count(Packet::HEADER_NODE);x++)
		{
			wxString node = rcv->Get(Packet::HEADER_NODE,x);
			wxString info = rcv->Get(Packet::HEADER_INFO,x);
			answer.AddResponse(HashValue(node,key,info));
		}

		break;
	}
	case Packet::TYPE_RESPONSE_JUMP:
	{
	   answer = LookupResponse(LookupResponse::JUMP,Node(wxT("UNDEFINED"),rcv->Get(Packet::HEADER_JUMP)));
		break;
	}
	case Packet::TYPE_RESPONSE_SUCCESSOR:
	{
		answer = LookupResponse(LookupResponse::SUCCESSOR,Node(currentNid_,currentAddr_));
		break;
	}
	default:
	{
	   LogError(wxString().Format(wxT("Received an invalid lookup response from node on con id %d"), id_));
		break;
	}
	}

	delete rcv;
	return answer;
}

bool
Connection::HelloParent(const Node& node)
{
	Network::Instance()->Join(FALSE);

	Connection* helloNextCon = new Connection(0,-1,TYPE_MANUAL,node.Address());
	if (!helloNextCon->JoinParent())
	{
		delete helloNextCon;
		LogError(wxString().Format(wxT("Could not contact successor node %s"),node.Address().c_str()));
		return FALSE;
	}

	delete helloNextCon;

	/*
	Add first next.
	*/
	Network::Instance()->ClearNext();
	Network::Instance()->AddNext(node);

	/*
	Allow nodes to officially connect to me now.
	*/
	Network::Instance()->Join(TRUE);

	return TRUE;
}

bool
Connection::JoinParent()
{
	if (!Connect())
	{
	   LogError(wxString().Format(wxT(" Handshake failed in con id %d"),id_));
		KillSocket(-1);
		return FALSE;
	}

	/*
	Tell our parent hello and it has just born a new child.
	*/
	Packet packet = GenPacket_HelloNext();
	if (!Send(packet))
	{
	   LogError(wxString().Format(wxT(" Send failed in con id %d when trying to say hello to parent"),id_));
		KillSocket(-1);
		return FALSE;
	}

	/*
	Receive all the hashes we are responsible for.
	*/
	Packet* rcv;
	if (!Receive(rcv) || rcv == 0)
	{
	   LogError(wxString().Format(wxT(" Receive failed when getting responsible hashes from parent on con id %d"), id_));
		KillSocket(-1);
		return FALSE;
	}

	KillSocket(-1);

	// add all the hashes we are responsible for to our hash table
	for (int x = 0; x < rcv->Count(Packet::HEADER_HASH);x++)
	{
		wxString hash = rcv->Get(Packet::HEADER_HASH,x);
		wxString node = rcv->Get(Packet::HEADER_NODE,x);
		wxString info = rcv->Get(Packet::HEADER_INFO,x);
		Network::Instance()->hashTable->Add(HashValue(node,hash,info));
	}

	delete rcv;

	return TRUE;
}

bool
Connection::Send(Packet& packet)
{
	//unsigned int len = packet.RawLen();
	//char* raw = packet.Raw();
	//unsigned int type = packet.Type();

	//LogDebug(wxString().Format("Sending packet type %d (%s) on con id %d",packet.Type(), Packet::TYPES[packet.Type()].c_str(),id_));
	//LogDebug(wxString().Format("Sending packet type %d of size %d: %s",packet.Type(),len,raw));

	/*
	Depending on the type of packet this is, lets chose the encryption
	type and do it.
	*/
	if (packet.RawLen() > 0)
	{
		switch (packet.Type())
		{
		case Packet::TYPE_SYN:			// nothing!
		case Packet::TYPE_JOIN:			// nothing!
			break;
		case Packet::TYPE_ACK:			// rsa
		case Packet::TYPE_JOIN_RESPONSE:	// rsa
			RSAEncrypt(&packet);
			break;
		default:				//symetric
			SymEncrypt(&packet);
			break;
		}
		/*
		Encryption most likely changed some stuff.
		*/
		//len = packet.RawLen();
		//raw = packet.Raw();
	}

	//LogDebug(wxString().Format("Sending packet type %d of size %d: %s",packet.Type(),len,raw));

	unsigned int tmp = /*htonl(*/packet.RawLen()/*)*/;
	if (!socket_->ForceSend((char*)&tmp,4))
	{
	   LogError(wxString().Format(wxT("Failed to send packet size in con id %d"),id_));
		    LogError(wxString().Format(wxT("Error: %s"), socket_->ErrorMessage().c_str()));
		return FALSE;
	}

	tmp = /*htonl(*/packet.Type()/*)*/;
	if (!socket_->ForceSend((char*)&tmp,4))
	{
	   LogError(wxString().Format(wxT("Failed to send packet type in con id %d"),id_));
		    LogError(wxString().Format(wxT("Error: %s"), socket_->ErrorMessage().c_str()));
		return FALSE;
	}

	if (!socket_->ForceSend(packet.Raw(),packet.RawLen()))
	{
	   LogError(wxString().Format(wxT("Failed to send packet metadata in con id %d"),id_));
	   LogError(wxString().Format(wxT("Error: %s"), socket_->ErrorMessage().c_str()));
		return FALSE;
	}

	// keep track of the number of packets sent
	packetsSent_++;

	return TRUE;
}

bool
Connection::Receive(Packet*& packet)
{
	packet = 0;

	unsigned int size = 0;
	unsigned int type = 0;

	// block until there is something to read
	while (!socket_->ReadReady());

	if (!socket_->ForceRecv((char*)&size,4,0))
	{
	   LogError(wxString().Format(wxT("Failed to get packet size in con id %d"),id_));
	   LogError(wxString().Format(wxT("Error: %s"), socket_->ErrorMessage().c_str()));
		return FALSE;
	}

	if (!socket_->ForceRecv((char*)&type,4,0))
	{
	   LogError(wxString().Format(wxT("Failed to get packet type in con id %d"),id_));
	   LogError(wxString().Format(wxT("Error: %s"), socket_->ErrorMessage().c_str()));
		return FALSE;
	}

	size = /*ntohl(*/size/*)*/;
	type = /*ntohl(*/type/*)*/;

	packet = new Packet(type);

	/*if (size > 20000|| size < 0)
	{
		LogError(wxString().Format(wxT("Invalid packet size in con id %d"),id_));
		return FALSE;
	}*/
	
	/*
	Depending on the packet type we got, we need to decrypt it.
	*/
	if (size > 0)
	{
		char* buffer = new char[size+1];
		memset(buffer, 0, size+1);

		if (!socket_->ForceRecv(buffer,size,0))
		{
		   LogError(wxString().Format(wxT("Failed to get packet metadata in con id %d"),id_));
		   LogError(wxString().Format(wxT("Error: %s"), socket_->ErrorMessage().c_str()));
			return FALSE;
		}

		//LogDebug(wxString().Format("Received raw packet type %d of size %d:%s",packet->Type(),size,buffer));

		switch (type)
		{
		case Packet::TYPE_SYN:			// nothing!
		case Packet::TYPE_JOIN:			// nothing!
			packet->Raw(buffer);
			break;
		case Packet::TYPE_ACK:			// rsa
		case Packet::TYPE_JOIN_RESPONSE:	// rsa
			RSADecrypt(buffer,packet);
			break;
		default:				//symetric
			SymDecrypt(buffer,packet);
			break;
		}

		delete [] buffer;
	}

	if (!packet->Valid())
	{
	   LogError(wxString().Format(wxT("Invalid packet parsed in con id %d"),id_));
		delete packet;
		return FALSE;
	}

	//LogDebug(wxString().Format("Received packet type %d (%s) on con id %d",packet->Type(), Packet::TYPES[packet->Type()].c_str(), id_));
	//LogDebug(wxString().Format("Received packet type %d of size %d:%s",packet->Type(),size,packet->Raw()));

	// keep track of the number of packets received
	packetsReceived_++;

	return TRUE;
}

#ifdef DEBUG
void
Connection::AnalyzeNetwork()
{
	Node next;
	Network::Instance()->GetNext(0,next);
	currentAddr_ = next.Address();
	currentNid_ = next.Nid();

	wxString firstAddr;

	/*
	Keep sending queries until we get something or fail.
	*/
	while (true)
	{
		if (firstAddr == currentAddr_)
			return;

		LogDebug(wxString().Format(wxT(">>Stating node %s (%s)"),currentAddr_.c_str(),currentNid_.c_str()));

		// save the first node we look at so we don't loop the circle!
		if (firstAddr == wxT(""))
			firstAddr = currentAddr_;

		if (!Connect())
		{
		   LogError(wxString().Format(wxT(" Handshaking failed in con id %d"),id_));
			return;
		}

		/*
		Send a stat request so we can get the predecessor.
		*/
		Packet snd = GenPacket_Stat();
		if (!Send(snd))
		{
		   LogError(wxString().Format(wxT(" Send failed when trying to stat on con id %d"),id_));
			return;
		}

		Packet* rcv;
		if (!Receive(rcv) || rcv == 0)
		{
		   LogError(wxString().Format(wxT(" Receive failed when trying to get the stat response on con id %d"), id_));
			delete rcv;
			return;
		}

		wxString prevNid = rcv->Get(Packet::HEADER_PREV_NID);
		wxString prevAddr = rcv->Get(Packet::HEADER_PREV_ADDR);

		currentNid_ = rcv->Get(Packet::HEADER_NEXT_NID);
		currentAddr_ = rcv->Get(Packet::HEADER_NEXT_ADDR);

		LogDebug(wxString().Format(wxT("<- %s (%s)"), prevAddr.c_str(),prevNid.c_str()));
			 LogDebug(wxString().Format(wxT("-> %s (%s)"), currentAddr_.c_str(),currentNid_.c_str()));

		for (int x = 1; ;x++)
		{
			wxString hash = rcv->Get(Packet::HEADER_HASH,x);
			if (hash == wxT(""))
				break;

			LogDebug(wxString().Format(wxT(">>    Hash: %s"), hash.c_str()));
		}

		delete rcv;

		KillSocket(-1);
	}
}
#endif

LookupResponse
Connection::Lookup(wxString key,wxString target)
{
	// if we were given nothing to start with, then use the finger table
   if (target == wxT(""))
	{
		// TODO uncomment this line to use the finger table
		//target = Network::Instance()->finger.BestStart(key).Address();

		// if the finger table is useless, then just use our next
	   if (target == wxT("") || Network::Instance()->IsMe(target))
		{
			//target = Network::Instance()->SelfAddress();
			Node next;
			Network::Instance()->GetNext(0,next);
			target = next.Address();
		}
	}

	// the only time this will happen is if i am a seed, so use myself
   if (target == wxT(""))
	{
	   LogError(wxT("Target is null for lookup."));
		return LookupResponse(LookupResponse::FAIL);
	}

	wxString firstAddr;

	/*
	Keep sending queries until we get something or fail.
	*/
	while (!Network::Instance()->KillAll())
	{
	   LogDebug(wxString().Format(wxT("Looking up '%s' on hop '%s'"),key.c_str(),target.c_str()));

		if (firstAddr == target)
		{
		   LogError(wxString().Format(wxT("Went in a complete circle during lookup for %s"),key.c_str()));
			return LookupResponse(LookupResponse::FAIL);
		}

		Connection* queryCon = new Connection(0,-1,TYPE_QUERY,target);
		LookupResponse answer = queryCon->DirectQuery(key);

		// save the first node we look at so we don't loop the circle!
		if (firstAddr == wxT(""))
			firstAddr = target;

		switch(answer.type_)
		{
		case LookupResponse::JUMP:
		{
			delete queryCon;
			target = answer.successor.Address();
			break;
		}
		default:
		{
			delete queryCon;
			return answer;
		}
		}
	}

	// this will never be used
	return LookupResponse(LookupResponse::FAIL);
}



void
Connection::Put(ShareFile* file)
{
	char buffer[FILE_SEG_BUFFER_SIZE+1];
	// the total bytes transfered in this session
	unsigned int totalBytesTransfered = 0;

	LogMsg(wxString().Format(wxT("Uploading file %s ..."),file->Path().c_str()));

	Network::Instance()->AddUpload(file);

	wxFile data(file->Path(), wxFile::read);

	if (!data.IsOpened())
	{
	   LogError(wxT("Failed to open file for reading to upload."));
		file->Status(ShareFile::STATUS_FAILED);
		data.Close();
		return;
	}

	file->Status(ShareFile::STATUS_INPROGRESS);

	memset(buffer, 0, sizeof(*buffer));

	sw_->Start(0);

	while (true)
	{

		if (Network::Instance()->KillAll() ||
			file->Status() == ShareFile::STATUS_CANCELLED)
		{
		   LogDebug(wxT("File transfer thread got signal to exit"));
			file->Status(ShareFile::STATUS_PAUSED);
			data.Close();
			return;
		}

		unsigned int start = 0;
		unsigned int end = 0;

		/*
		Remember, this session does not mean we are transfering a file.  The
		remote host can request any file segment as long as it is from the
		same file requested in any order he wants.  So if he drops the
		connection then he's done asking.
		*/
		Packet* rcv;
		if (!Receive(rcv) || rcv == 0)
		{
			file->Status(ShareFile::STATUS_FINISHED);
			data.Close();
			delete rcv;
			return;
		}

		switch(rcv->Type())
		{
		case Packet::TYPE_CONT:
		{
			wxString startTxt = rcv->Get(Packet::HEADER_FILESEG_START);
			wxString endTxt = rcv->Get(Packet::HEADER_FILESEG_END);
			start = atoi(startTxt.c_str());
			end = atoi(endTxt.c_str());

			if (start < 0 || start >= end || end > file->Size()
				||
			    startTxt == wxT("") || endTxt == wxT("")
				|| end-start > FILE_SEG_BUFFER_SIZE)
			{
			   LogError(wxString().Format(wxT("Invalid start %d or end %d range requested on con id %d"),start,end,id_));
				file->Status(ShareFile::STATUS_FAILED);
				data.Close();
				delete rcv;
				return;
			}
			break;
		}
		default:
		   LogError(wxString().Format(wxT("Did not receive CONT packet when expected on con id %d"), id_));
		}

		if (data.Seek(start, wxFromStart) != start)
		{
			file->Status(ShareFile::STATUS_FAILED);
			data.Close();
			LogError(wxString().Format(wxT("Could not seek to requested file byte: %d"),start));
			return;
		}

		delete rcv;

		unsigned int count = data.Read(buffer,end-start);

		if (count != end-start)
		{
		   LogError(wxString().Format(wxT("Could not read requested file segment from local file on con id %d"),id_));
			file->Status(ShareFile::STATUS_FAILED);
			return;
		}

		//LogDebug(wxString().Format("Sending %d (%d) bytes ...",end-start,strlen(buffer)));

		Packet snd = GenPacket_Send(buffer,count);
		if (!Send(snd))
		{
		   LogError(wxString().Format(wxT("Failed when sending file on con id %d"),id_));
			file->Status(ShareFile::STATUS_FAILED);
			return;
		}

		totalBytesTransfered += count;
		file->CompletedSize(totalBytesTransfered);

		file->Kbs((double)(totalBytesTransfered/1024)/(double)(sw_->Time()/1000));
	}
}

void
Connection::KillSocket(int event)
{
	socket_->Close();

	status_ = STATUS_DEAD;

	// if we have an event, then send it
	if (event >= 0)
	{
		PostConEvent(event);
	}
}


void Connection::PostConEvent(int type,void* data)
{
	if (parent_ != 0 && type != -1)
	{
		wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED,parentId_);
		event.SetInt(type);
		event.SetClientData(data);
		wxPostEvent(parent_,event);
	}
}

void Connection::HandleImEvents(ImEventQueue* queue)
{
	ImEvent* event;

	while(true/*!(Network::Instance()->KillAll())*/)
	{
		// see if there are any events to send
		event = queue->PopSendEvent();
		if (event != (ImEvent*)0)
		{
			switch (event->type)
			{
			case ImEvent::EVENT_IM:
			{
				Packet snd = GenPacket_Im(event->data);
				if (!Send(snd))
				{
				   LogError(wxString().Format(wxT("Sending im %s on con id %d"),event->data.c_str(),id_));
					PostConEvent(ImEvent::EVENT_LOST);
					delete event;
					return;
				}

				PostConEvent(ImEvent::EVENT_SENT);

				break;
			}
			case ImEvent::EVENT_CLOSE:
			{
				Packet snd = GenPacket_ImClose();
				if (!Send(snd))
				{
				   LogDebug(wxString().Format(wxT("Sending close im event on con id %d"),id_));
					PostConEvent(ImEvent::EVENT_LOST);
					delete event ;
					return;
				}

				break;
			}
			case ImEvent::EVENT_DISCONNECT:
			{
				PostConEvent(ImEvent::EVENT_LOST);
				delete event;
				return;

				break;
			}
			}

			delete event;
		}

		// see if there are any packets to read
		if (socket_->ReadReady())
		{
			Packet* rcv;
			if (Receive(rcv/*,FALSE*/))
			{
				if (rcv != (Packet*)0)
				{
					switch (rcv->Type())
					{
					case Packet::TYPE_IM_CLOSED:
					{
						PostConEvent(ImEvent::EVENT_CLOSE);
						break;
					}
					case Packet::TYPE_IM:
					{
						event = new ImEvent(ImEvent::EVENT_IM,rcv->Get(Packet::HEADER_IM));
						PostConEvent(ImEvent::EVENT_IM,event);
						break;
					}
					}
				}
			}
			else
			{
				PostConEvent(ImEvent::EVENT_LOST);
				return;
			}
		}

		wxThread::Sleep(1);
	}

	PostConEvent(ImEvent::EVENT_LOST);
}

void
Connection::RSAEncrypt(Packet* packet)
{
#ifdef ENABLE_ENCRYPTION
	//LogDebug(wxString().Format("Encrypting %s with rsa key %s",packet->Raw(), rsaPub_.c_str()));
	//string encrypted = crypto::RSAEncryptString(rsaPub_.c_str(),packet->Raw());
	//LogDebug(wxString().Format("Encrypted: %s",encrypted.c_str()));
	try
	{
		packet->Raw(crypto::RSAEncryptString(rsaPub_.c_str(),packet->Raw()).c_str());
	}
	catch (...)
	{
	   LogError(wxString().Format(wxT("Failed to RSA encrypt with key '%s' and text '%s' of length %d"),rsaPub_.c_str(),packet->Raw(),strlen(packet->Raw())));
		exit(1);
	}
#endif
}

void
Connection::RSADecrypt(const char* raw, Packet* packet)
{
#ifdef ENABLE_ENCRYPTION
	//LogDebug(wxString().Format("Decrypting %s with rsa key %s",raw,rsaPriv_.c_str()));
	//string decrypted = crypto::RSADecryptString(Network::Instance()->rsaPriv.c_str(),raw);
	//LogDebug(wxString().Format("Decrypted: %s",decrypted.c_str()));

	try
	{
		packet->Raw(crypto::RSADecryptString(Network::Instance()->rsaPriv.c_str(),raw).c_str());
	}
	catch (...)
	{
		LogError(wxString().Format("Failed to RSA decrypt with key '%s' and text '%s' of length %d",Network::Instance()->rsaPriv.c_str(),raw,strlen(raw)));
		exit(1);
	}
#else
	packet->Raw(raw);
#endif
}

void
Connection::SymEncrypt(Packet* packet)
{
#ifdef ENABLE_ENCRYPTION
	//LogDebug(wxString().Format("Encrypting with key %s",symKey_.c_str()));

	//packet->Raw(symetric_->Encrypt(packet->Raw()));
	packet->Raw(crypto::EncryptString(packet->Raw(), symetric_->Key().c_str()).c_str());

#endif
}

void
Connection::SymDecrypt(const char* raw, Packet* packet)
{
#ifdef ENABLE_ENCRYPTION
	//LogDebug(wxString().Format("Decrypting with key %s",symKey_.c_str()));

	//packet->Raw(symetric_->Decrypt(raw));
	packet->Raw(crypto::DecryptString(raw, symetric_->Key().c_str()).c_str());
#else
	packet->Raw(raw);
#endif
}

Connection::~Connection()
{
	delete socket_;
#ifdef ENABLE_ENCRYPTION
	delete symetric_;
#endif
	delete sw_;
}



#if 0

/**
This is a simple wrapper around a Connection and puts it in a thread.
*/
class ConnectionThread
{
public:
	switch(type_)
	{
	case CONNECTIONTHREAD_QUERY:
	case CONNECTIONTHREAD_IM:
	case CONNECTIONTHREAD_FILE:
	}
};

/** @class ConnectionQuery
Non-Persistent connection to query the network for a hash.
*/
class ConnectionQuery
{
public:
	ConnectionInstantMessage();
	virtual ~ConnectionInstantMessage();
};

/** @class ConnectionInstantMessage
Persistent connection to communicate with a buddy.
*/
class ConnectionInstantMessage
{
public:
	ConnectionInstantMessage();
	virtual ~ConnectionInstantMessage();
};

/** @class ConnectionFileTransfer
Non-Persistent connection to download or upload a file.
*/
class ConnectionFileTransfer
{
public:
	ConnectionFileTransfer();

	bool PutFile();
	bool GetFile();

	virtual ~ConnectionFileTransfer();
};

/**
*/
class Connection
{
public:
	Connection();
	virtual ~Connection();
};

/**
This class includes the actual algorithms and the general "do and respond"
archtitecture of the protocol.
*/
class ConnectionSessionLayer
{
public:
	ConnectionSession();

	bool InitWithJoin();
	bool InitWithHandShake();

	bool GetHelloNext();
	bool PutHelloNext();
	bool GetQuery();
	bool PutQuery();



	virtual ConnectionSession();
};

/**
This class involves the actual sending and receiving of packets as
well as standard things like handshaking, encryption, and the PKI
infrastructure.  This does everything leading up to a connection
session. Basically, this is what starts and stops a connection, as
well as handles any network I/O.
*/
class ConnectionNetworkLayer
{
public:
	ConnectionNetwork(ClientSocket* socket = 0);

	bool Connect(const Node& node);
	bool Disconnect(const Node& node);

	/** Send a packet. */
	bool Send(Packet& packet);

	/** Read a packet (blocks).
	@return Returns NULL on failure. */
	Packet* Receive();

	virtual ConnectionNetwork();

private:

	void RSAEncrypt(Packet* packet);
	void RSADecrypt(const char* raw,Packet* packet);

	void SymEncrypt(Packet* packet);
	void SymDecrypt(const char* raw,Packet* packet);

	ClientSocket* socket_;
};

class ConnectionDataLayer
{
public:
	ConnectionDataLayer();

	static Packet GenPacket_Syn(const wxString& rsaPub);
	static Packet GenPacket_Ack(const wxString& symKey, const wxString& peer);
	static Packet GenPacket_Confirm(const wxString& peer);
	static Packet GenPacket_Join(const wxString& rsaPub);
	static Packet GenPacket_JoinResponse(const wxString& nid, const Node& node, const wxString& peer);
	static Packet GenPacket_Request(const wxString& filename, const wxString& md5);
	static Packet GenPacket_Cont(unsigned int start, unsigned int end);
	static Packet GenPacket_Send(char* buffer, unsigned int size);
	static Packet GenPacket_Query(const wxString& hash);
	static Packet GenPacket_ResponseSuccess();
	static Packet GenPacket_ResponseJump(const wxString& addr);
	static Packet GenPacket_ResponseEnd();
	static Packet GenPacket_Insert();
	static Packet GenPacket_InsertSuccess();
	static Packet GenPacket_Remove();
	static Packet GenPacket_HelloNext();
	static Packet GenPacket_Stat();
	static Packet GenPacket_StatResponse();
	static Packet GenPacket_HelloBuddy();
	static Packet GenPacket_Im(const wxString& txt);
	static Packet GenPacket_ImConnect();
	static Packet GenPacket_ImClose();

	virtual ~ConnectionDataLayer();
};
#endif


