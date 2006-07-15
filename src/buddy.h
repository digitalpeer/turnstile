/* $Id: buddy.h,v 1.2 2004/08/29 23:16:25 dp Exp $
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

/** @file buddy.h
@brief Buddy Definition
*/
#ifndef TURNSTILE_BUDDY_H
#define TURNSTILE_BUDDY_H

#include "imevent.h"

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/event.h>

class ImFrame;
class Packet;
class ImEventQueue;

/** @class Buddy
Not only is this meant to be an actual tree item, but it
also houses info for each buddy.
*/
class Buddy : public wxTreeItemData, public wxEvtHandler
{
	friend class ImFrame;

public:
	/** Buddy Statuses */
	enum	{
			STATUS_ONLINE = 	0x00000001,
			STATUS_CONNECTED = 	0x00000010,
			STATUS_IDLE = 		0x00000100,
			STATUS_AWAY = 		0x00001000
		};

	/** Buddy Data Ids */
	enum 	{
			DATA_ALIAS,
			DATA_ADDRESS,
			DATA_PUBLICKEY,
			DATA_PROFILE,
			//
			DATA_COUNT
		};

	Buddy();
	Buddy(const Buddy& rhs);

	//void ShowProfile() const;

	/** Set some buddy info.
	*/
	void Set(int id, const wxString value);

	/** Get some buddy info.
	*/
	wxString Get(int id) const;

	bool GetStatus(int status);
	void SetStatusOn(int status);
	void SetStatusOff(int status);

	/** Handle connection events.
	*/
	void OnConnectionEvent(wxCommandEvent& event);

	/** Open the window of this buddy (show it if it already is open).
	*/
	void OpenWindow();

	/** Called by the closing window to alert the buddy it is closing.
	*/
	void OnCloseWindow();
	void Disconnect();

	void Connect();

	inline int id() const
	{ return id_; }

	~Buddy();

private:

	/** Send an IM, however this may be required to create
	the connection.
	*/
	void SendIm(const wxString& text);

	static int idCounter;

	/** Data container for the buddy. */
	wxString data_[DATA_COUNT];

	/** Got an IM event.
	*/
	void ReceiveIm(ImEvent* data);


	/** IM window for this buddy.
	*/
	ImFrame* frame_;

	/** Unique buddy id for handling connection events.
	*/
	int id_;

	/** The event queue assigned by a connection for us to send
	events to the connection */
	ImEventQueue* queue_;

	/** Current status of this buddy.
	*/
	int status_;

	wxString queueCurrent_;

	bool halfClose_;
};

#endif
