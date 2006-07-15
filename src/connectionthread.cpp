/* $Id: connectionthread.cpp,v 1.2 2004/07/02 23:24:22 dp Exp $
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
#include "config.h"
#include "connectionthread.h"
#include "utils.h"
#include "typeconv.h"
#include "lookupresponse.h"
#include "network.h"
#include "packet.h"
#include "packetgen.h"
#include "prefs.h"
#include "buddy.h"
#include "imeventqueue.h"
#include "libjdhsocket/clientsocket.h"
//
#include "buddytreectrl.h"
#include "app.h"
#include "mainframe.h"

#include <wx/tokenzr.h>

#ifdef DEBUG
#include <iostream>
using namespace std;
#endif

using namespace JdhSocket;

ConnectionThread::ConnectionThread(wxEvtHandler* parent,int parentId, JdhSocket::ClientSocket* socket)
	: /*wxThread(),*/ Connection(parent,parentId,socket)
{;}

ConnectionThread::ConnectionThread(wxEvtHandler* parent,int parentId, int type,
				wxString addr,wxString a,wxString b)
	: /*wxThread(),*/ Connection(parent,parentId, type, addr,a), b_(b)
{;}

/*
*/
void
RecoverLostParent()
{
}


void* ConnectionThread::Entry()
{
	//LogDebug(wxString().Format("Connection type %d thread id %d starting",type_,id_));

	/*
	If we are initiating the connection, then do the connect.
	Otherwise, we should be expecting the other side to do it.
	*/
	switch (type_)
	{
	case TYPE_HELLOBUDDY:
	{
		LookupResponse answer = Lookup(a_);

		switch(answer.type_)
		{
		case LookupResponse::FOUND:
		{
			// TODO we are just going to pick the first one we find,
			// may want to do more later with replica usernames
			for (unsigned int x = 0; x < answer.responses_.size();x++)
			{
				currentAddr_ = answer.responses_[x].node;
				wxString info = answer.responses_[x].info;

				if (!Connect())
				{
					LogError(wxString().Format(" Handshaking failed when trying to connect to say hello to buddy in con id %d",id_));
					KillSocket(EVENT_FAILED);
					return 0;
				}

				Packet snd = GenPacket_HelloBuddy();
				if (!Send(snd))
				{
					LogError(wxString().Format(" Send failed when trying to say hey to buddy on con id %d",id_));
					KillSocket(EVENT_FAILED);
					return 0;
				}

				/*
				Now, because we know this buddy is online, we can alert the local buddy
				list of it.
				*/
				wxStringTokenizer tokens(info, ":");
				if (tokens.GetNextToken() == "BUDDY")
				{
					wxMutexGuiEnter();

					// TODO this can be taken elsewhere for optimization
					// TODO validate that a_ is == publicKey
					BuddyTreeCtrl* tree = wxGetApp().GetMainWindow()->GetTreeCtrl();
					wxString alias = tokens.GetNextToken();
					wxString publicKey = tokens.GetNextToken();
					wxString profile = tokens.GetNextToken();
					//tree->SetBuddyStatus(publicKey, Buddy::STATUS_ONLINE);
					tree->SetBuddyData(publicKey, alias, profile, currentAddr_);

					Buddy* buddy = tree->GetBuddy(publicKey);
					if (buddy != 0)
					{
						buddy->SetStatusOn(Buddy::STATUS_ONLINE);
						LogMsg(wxString().Format("Noticed that buddy %s is online!",a_.c_str()));
					}

					wxMutexGuiLeave();
				}

				KillSocket(EVENT_EXIT);
			}

			break;
		}
		default:
		{
			LogDebug(wxString().Format("Buddy %s is offline.",a_.c_str()));
			break;
		}
		}

		break;
	}
	case TYPE_INSERT:
	{
		LookupResponse answer = Lookup(a_);

		switch(answer.type_)
		{
		/*
		All we want is the successor, we don't care if we actually found
		something.
		*/
		case LookupResponse::SUCCESSOR:
		case LookupResponse::FOUND:
		{

			currentAddr_ = answer.successor.Address();

			//LogDebug(wxString().Format("Connecting to address %s after lookup to insert hash in con id %d",currentAddr_.c_str(),id_));

			if (!Connect())
			{
				LogError(wxString().Format(" Handshaking failed in con id %d",id_));
				KillSocket(EVENT_INSERT_FINISHED);
				return 0;
			}

			Packet insert = GenPacket_Insert();
			insert.Add(Packet::HEADER_HASH,a_);
			insert.Add(Packet::HEADER_INFO,b_);
			insert.Add(Packet::HEADER_NODE,Network::Instance()->PublicAddress());
			if (!Send(insert))
			{
				LogError(wxString().Format(" Send failed when trying to stat on con id %d",id_));
				KillSocket(EVENT_INSERT_FINISHED);
				return 0;
			}

			break;
		}
		default:
		{
			LogError(wxString().Format(" Failed to insert hash %s in con id %d with response type %d",a_.c_str(),id_,answer.type_));
			break;
		}
		}

		KillSocket(EVENT_INSERT_FINISHED);

		break;
	}
	case TYPE_QUERY:
	{
		LookupResponse answer = Lookup(a_);

		switch(answer.type_)
		{
		case LookupResponse::FOUND:
		{
			LogDebug(wxString().Format("Found values for hash %s (%s) in con id %d with response type %d",a_.c_str(),answer.successor.Address().c_str(),id_,answer.type_));
			LookupResponse* stuff = new LookupResponse(answer);
			PostConEvent(EVENT_QUERYFOUND,(void*)stuff);
			break;
		}
		case LookupResponse::SUCCESSOR:
		{
			LogDebug(wxString().Format("Found successor for hash %s (%s) in con id %d with response type %d",a_.c_str(),answer.successor.Address().c_str(),id_,answer.type_));
			PostConEvent(EVENT_QUERYNOTFOUND);
			break;
		}
		default:
		{
			LogError(wxString().Format("Found nothing for hash %s in con id %d with response type %d",a_.c_str(),id_,answer.type_));
			PostConEvent(EVENT_QUERYNOTFOUND);
			break;
		}
		}

		//KillSocket(-1);

		break;
	}
	case TYPE_SPREAD:
	{
		/*
		"SPREAD" simply means contact a next and populate the next table.
		If it can't find a valid next and populate the next table, the
		connection to the network is considered lost.

		Step through the list, connect, and get the next node to
			append to the table.
		If any node fails, delete it and continue with the next.

		If the node is myself, don't connect and assume list is populated.
		If the list dwindles to nothing, we have lost connection to the network.
  		*/

		LogDebug(wxString().Format("Starting spread with %d nodes in nexttable",Network::Instance()->NextSize()));

		Node next;
		for (int x = 0; x >= 0 && x < Network::Instance()->NextSize();++x)
		{

			Network::Instance()->GetNext(x,next);

			currentAddr_ = next.Address();
			currentNid_ = next.Nid();

			// if it's me, just quit and assume everything is ok
			if (Network::Instance()->IsMe(currentAddr_))
			{
				LogDebug(wxString().Format("Skipping myself in next table in con id %d",id_));
				//KillSocket(-1);
				return 0;
			}

			LogDebug(wxString().Format("Testing node %s in con id %d",currentAddr_.c_str(),id_));

			if (!Connect())
			{
				LogDebug(wxString().Format("Failed to contact next in con id %d",id_));

				// erase the bad node
				Network::Instance()->EraseNext(x);
				// continue on with next one
				//--x;
				x=-1;
				KillSocket(-1);
				continue;
			}

			/*
			Send a stat request so we can get the predecessor.
			*/
			Packet snd = GenPacket_Stat();
			if (!Send(snd))
			{
				LogDebug(wxString().Format("Could not send stat to next on con id %d",id_));

				// erase the bad node
				Network::Instance()->EraseNext(x);
				// continue on with next one
				//--x;
				x=-1;
				KillSocket(-1);
				continue;
			}

			/*
			Get the stat response.
			*/
			Packet* rcv;
			if (!Receive(rcv) || rcv == 0)
			{
				LogDebug(wxString().Format("Receive failed when trying to get the stat response on con id %d", id_));

				// erase the bad node
				Network::Instance()->EraseNext(x);
				// continue on with next one
				//--x;
				x=-1;
				KillSocket(-1);
				continue;
			}

			switch(rcv->Type())
			{
			case Packet::TYPE_STAT_RESPONSE:
			{
				LogDebug("Got stat response.");

				// this should be us
				wxString nextNid = rcv->Get(Packet::HEADER_PREV_NID);
				wxString nextAddr = rcv->Get(Packet::HEADER_PREV_ADDR);

				// this is 2 nodes away
				wxString nextNextNid = rcv->Get(Packet::HEADER_NEXT_NID);
				wxString nextNextAddr = rcv->Get(Packet::HEADER_NEXT_ADDR);

				delete rcv;

				// the first node in the next table is our next, so this
				// is a special case to verify that we are indeed the previous of
				// that node
				// if we arn't, it means we need to jump to another node
				if (x == 0)
				{
					// uh oh, somebody snuck in, we have a new parent
					if (nextNid == "" || nextNid != Prefs::Instance()->Get(Prefs::NID))
					{
						LogDebug(wxString().Format("We have a new parent on con id %d",rcv->Type(), id_));
						KillSocket(-1);

						if (!HelloParent(Node(nextNid,nextAddr)))
						{
							/*
							There is a be better ways of handling this, but I am just going to abort.
							... yet another weakness in the client.
							*/
							LogError("Could not change to new node in spread");
							KillSocket(EVENT_NEXT_LOST);
							return 0;
						}

						Network::Instance()->AddNext(Node(currentNid_,currentAddr_));
						//Network::Instance()->AddNext(Node(nextNextNid,nextNextAddr));

						// JoinParent() modifies next table correctly, so we don't
						// need to do it here
						x=-1;
						//continue;
					}
					else
						Network::Instance()->AddNext(Node(nextNextNid,nextNextAddr));
				}
				else
				{
					Network::Instance()->AddNext(Node(nextNextNid,nextNextAddr));

					/* ** Successful Completion ***
					if the next table is "FULL" or we have started to loop around the
					circle again then we are done
					*/
					Node first;
					Network::Instance()->GetNext(0,first);
					if (Network::Instance()->NextSize() >= NEXT_TABLE_SIZE ||
						first.Nid() == nextNextNid)
					{
						KillSocket(-1);
						return 0;
					}
				}

				break;
			}
			default:
			{
				/*
				The node has lost its mind and sent us something we
				didn't expect.
				*/

				delete rcv;
				// erase the bad node
				Network::Instance()->EraseNext(x);
				// continue on with next one
				--x;
				LogDebug(wxString().Format("Invalid response to a stat on con id %d", id_));
				KillSocket(-1);
			}
			}
		}

		if (Network::Instance()->NextSize() == 0)
		{
			LogDebug("Could not complete nexttable");

			/*
			If we ever make it out of the loop then we have failed to
			contact a next.
			*/
			KillSocket(EVENT_NEXT_LOST);
		}

		break;
	}
	case TYPE_BOOTSTRAP:
	{
		/*
		Initiate the connection.  This takes care of handshaking which is standard
		for absolutely all connections.
		*/
		if (!Connect())
		{
			LogError(wxString().Format("Handshaking failed in con id %d",id_));
			KillSocket(EVENT_NEXT_LOST);
			return 0;
		}

		KillSocket(EVENT_NEXT_CONNECTED);

		break;
	}
	case TYPE_SERVER:
	{
		/*
		Simple slave to an incomming connection.
		*/
		if (!ReceiveHandShake())
		{
			LogError(wxString().Format("Incomming connection handshake failed in con id %d",id_));
		}

		// kill socket and don't send any events
		KillSocket(-1);

		break;
	}
	case TYPE_IM_CONNECT:
	{
		if (!Connect())
		{
			LogError(wxString().Format(" Handshaking failed in con id %d",id_));
			KillSocket(-1);
			PostConEvent(ImEvent::EVENT_LOST);
			return 0;
		}

		Packet snd = GenPacket_ImConnect();
		if (!Send(snd))
		{
			LogError(wxString().Format(" Send failed when trying to say hey to buddy on con id %d",id_));
			KillSocket(-1);
			PostConEvent(ImEvent::EVENT_LOST);
			return 0;
		}

		ImEventQueue* queue = new ImEventQueue();

		PostConEvent(ImEvent::EVENT_MADE,(void*)queue);

		/*
		Handle events until the user closes the connection or remote
		closes the connection.
		*/
		try
		{
		HandleImEvents(queue);
		}
		catch(...)
		{
			LogMsg("Failed to handle IM events!");
		}

		//delete queue;
		PostConEvent(ImEvent::EVENT_LOST);

		KillSocket(-1);

		break;
	}
	default:
	{
		LogError(wxString().Format("Programmer: Unhandled connection thread type in con id %d",id_));
		break;
	}
	}

	return 0;
}

void ConnectionThread::OnExit()
{;}

