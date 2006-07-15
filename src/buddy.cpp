/* $Id: buddy.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file buddy.cpp
@brief Buddy Implimentation
*/
#include "buddy.h"
#include "imframe.h"
#include "utils.h"
#include "connection.h"
#include "typeconv.h"
#include "app.h"
#include "crypto.h"
#include "packet.h"
#include "connectionthread.h"
//
//
#include "buddytreectrl.h"
#include "app.h"
#include "mainframe.h"
//
#include "imeventqueue.h"

#include <string>
using namespace std;

int Buddy::idCounter = 1000000;

Buddy::Buddy()
	: frame_(0), id_(++idCounter), queue_(0), status_(0), halfClose_(FALSE)
{
	//wxEvtHandler::wxEvtHandler();

	/*
	Handle connection events.
	*/
	wxEvtHandler::Connect(id_, wxEVT_COMMAND_MENU_SELECTED,
		(wxObjectEventFunction)
                (wxEventFunction)
                (wxCommandEventFunction)&Buddy::OnConnectionEvent,
		(wxObject*) NULL);

}

Buddy::Buddy(const Buddy& rhs)
	: frame_(0), id_(++idCounter), queue_(0), status_(0), halfClose_(FALSE)
{
	for (unsigned int x = 0; x < DATA_COUNT;x++)
	{
		data_[x] = rhs.data_[x];
	}

	/*
	Handle connection events.
	*/
	wxEvtHandler::Connect(id_, wxEVT_COMMAND_MENU_SELECTED,
		(wxObjectEventFunction)
                (wxEventFunction)
                (wxCommandEventFunction)&Buddy::OnConnectionEvent,
		(wxObject*) NULL);
}

void
Buddy::OpenWindow()
{
	if (frame_ == 0)
	{
		// create a new window
		frame_ = new ImFrame(this);
		frame_->Show(TRUE);
	}
	else
	{
		// unhide the window and raise it
		frame_->Show(TRUE);
		frame_->Raise();
	}
}

void Buddy::OnCloseWindow()
{
	if (queue_ != 0)
	{
  		ImEvent* event = new ImEvent(ImEvent::EVENT_CLOSE);
		queue_->AddSendEvent(event);
	}
	frame_ = 0;
}

void
Buddy::Disconnect()
{
	ImEvent* event = new ImEvent(ImEvent::EVENT_DISCONNECT);
	queue_->AddSendEvent(event);
}

void
Buddy::Connect()
{
	if (queue_ == 0)
	{
		/*
		This means that we need to initiate the connection before we
		can send the actual IM.
		*/

		ConnectionThread* thread = new ConnectionThread(this,id_,Connection::TYPE_IM_CONNECT,Get(DATA_ADDRESS));

		if (thread->Create() != wxTHREAD_NO_ERROR)
                   LogError(wxT("Failed to create IM connection thread!"));

		thread->Run();
	}
}

void
Buddy::SendIm(const wxString& text)
{
	if (queue_ == 0)
	{
		Connect();

		// queue the text that needs to be sent if there is something
		if (text != wxT(""))
			queueCurrent_ = text;
	}
	else
	{
		/*
		Simply generate the ImEvent and put it in the queue.
		*/
           if (text != wxT(""))
		{
			ImEvent* event = new ImEvent(ImEvent::EVENT_IM,text);
			queue_->AddSendEvent(event);
		}
	}
}

void
Buddy::ReceiveIm(ImEvent* data)
{
	// make sure the window is up and running for this buddy
	OpenWindow();

	string raw = crypto::HexDecode(data->data.c_str());

	// send the frame the data
	frame_->ReceiveIm(data->type,wxString(raw.c_str(),raw.size()));
	//frame_->ReceiveIm(data->type,data->data);

	// delete the data passed from the connection
	delete data;
}

void
Buddy::OnConnectionEvent(wxCommandEvent& event)
{
	switch(event.GetInt())
	{
	case ImEvent::EVENT_MADE:
	{
           LogDebug(wxT("Connection to buddy made."));

		// get the event queue from the connection
		queue_ = (ImEventQueue*)event.GetClientData();

		halfClose_ = FALSE;

		if (frame_ != 0)
			frame_->ConnectionMade();

		SetStatusOn(STATUS_CONNECTED);
		SetStatusOn(STATUS_ONLINE);

		if (queueCurrent_ != wxT(""))
		{
			SendIm(queueCurrent_);
			queueCurrent_ = wxT("");
		}

		break;
	}
	case ImEvent::EVENT_LOST:
	{
           LogDebug(wxT("Connection to buddy lost or could not be made."));

		if (frame_ != 0)
			frame_->ConnectionLost();

		if (queue_ != 0)
		{
			delete queue_;
			queue_ = 0;
		}

		//if (!GetStatus(STATUS_CONNECTED))
		//	SetStatusOff(STATUS_ONLINE);
		//else
			SetStatusOff(STATUS_CONNECTED);

		break;
	}
	case ImEvent::EVENT_SENT:
	{
           LogDebug(wxT("IM sent."));

		if (frame_ != 0)
			frame_->Sent();

		//SetStatusOn(STATUS_CONNECTED);

		break;
	}
	case ImEvent::EVENT_IM:
	{
           LogDebug(wxT("IM received."));
		/*
		Got an IM, so handle it.
		*/
		ImEvent* imData = (ImEvent*)event.GetClientData();
		ReceiveIm(imData);
		break;
	}
	case ImEvent::EVENT_CLOSE:
	{
           LogDebug(wxT("Other IM side closed."));

		halfClose_ = TRUE;

		if (frame_ == 0)
		{
			ImEvent* event = new ImEvent(ImEvent::EVENT_DISCONNECT);
			queue_->AddSendEvent(event);
		}

		break;
	}
	/*case EVENT_RCV_YES_TYPING:
	{
		break;
	}
	case EVENT_RCV_NO_TYPING:
	{
		break;
	}*/
	default:
           LogError(wxT("Unhandled IM event"));
	}
}

bool
Buddy::GetStatus(int status)
{
	return (status_ & status);
}

void
Buddy::SetStatusOn(int status)
{
	// make sure the bit we are after is on
	status_ = (status_ | status);
	BuddyTreeCtrl* tree = wxGetApp().GetMainWindow()->GetTreeCtrl();
	tree->RefreshBuddies();
}

void
Buddy::SetStatusOff(int status)
{
	// turn the bit we are after is off
	status_ = (status_ & ~status);
	BuddyTreeCtrl* tree = wxGetApp().GetMainWindow()->GetTreeCtrl();
	tree->RefreshBuddies();
}

void
Buddy::Set(int id, const wxString value)
{
	if (id >= DATA_COUNT || id < 0)
	{
	   LogError(wxT("Invalid call to Set()"));
		return;
	}

	data_[id] = value;
}

wxString
Buddy::Get(int id) const
{
	if (id >= DATA_COUNT || id < 0)
	{
           LogError(wxT("Invalid call to GetData()"));
		return wxString(wxT("%INVALID%"));
	}

	return data_[id];
}

Buddy::~Buddy()
{
	wxEvtHandler::Disconnect(id_, wxEVT_COMMAND_MENU_SELECTED);

	if (frame_ != 0)
		frame_->Destroy();

	if (queue_ != 0)
		delete queue_;
}
