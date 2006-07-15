/* $Id: packet.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file packet.h
@brief Packet Definition
*/
#ifndef TURNSTILE_PACKET_H
#define TURNSTILE_PACKET_H

#include <wx/string.h>

#include <vector>

class Header
{
public:

	Header(int id_, unsigned int start, unsigned int end)
			: id(id_),valueStart(start),valueEnd(end)
	{;}

	Header() : id(0)
	{;}

	int id;
	//wxString value;

	/*
	This is an optimization that just saves where the headers and values are in
	the raw data.
	*/
	unsigned int valueStart;
	unsigned int valueEnd;

};

/** @class Packet
@note Derived from wxObject for possible dump() usage.
*/
class Packet
{
public:
	/** Header ids.
	@note If you add more headers they need to go in the rest of this class
	also (parsing/validation). */
	enum  {
			HEADER_INVALID = -1,	// invalid header
			HEADER_PUBLIC_KEY,
			HEADER_SYMETRIC_KEY,
			HEADER_SRC_ALIAS,
			HEADER_SRC_AGENT,
			HEADER_SRC_ADDR,
			HEADER_DEST_HOST,
			HEADER_SRC_NID,
			HEADER_HASH,
			HEADER_PAYLOAD,
			HEADER_HOSTCACHE,
			HEADER_REQUESTTARGET,
			HEADER_FILEMD5,
			HEADER_FILENAME,
			HEADER_JUMP,
			HEADER_QUERYSTRING,
			HEADER_QUERYTARGET,
			HEADER_PREV_ADDR,
			HEADER_PREV_NID,
			HEADER_NEXT_ADDR,
			HEADER_NEXT_NID,
			HEADER_NID,
			HEADER_NODE,
			HEADER_ASSIGN_NID,
			HEADER_INFO,
			HEADER_ALIAS,
			HEADER_PRIVATE_KEY,
			HEADER_FILESEG_START,
			HEADER_FILESEG_END,
			HEADER_IM,
			HEADER_WRKGRPKEY,
			//
			HEADERS_COUNT
		};

	enum	{
			TYPE_INVALID = 0,	// invalid packet type
			TYPE_SYN,
			TYPE_ACK,
			TYPE_CONFIRM,
			TYPE_JOIN,
			TYPE_JOIN_RESPONSE,
			//
			TYPE_REQUEST,
			TYPE_CONT,
			TYPE_FILESEG,
			//
			TYPE_QUERY,
			TYPE_QUERYRESPONSE,
			TYPE_INSERT,
			TYPE_REMOVE,
			TYPE_RESPONSE_FOUND,
			TYPE_RESPONSE_JUMP,
			TYPE_RESPONSE_SUCCESSOR,
			TYPE_HELLO_NEXT,
			TYPE_HELLO_BUDDY,
			//
			TYPE_STAT,
			TYPE_STAT_RESPONSE,
			//
			TYPE_IM_CONNECT,
			TYPE_IM_CLOSED,
			TYPE_IM,
			//
			TYPES_COUNT
		};

	/*enum 	{	ENCRYPTION_TYPE_INVALID = -1,
			ENCRYPTION_TYPE_NONE,
			ENCRYPTION_TYPE_BLOWFISH,
			ENCRYPTION_TYPE_TRIPLEDES,
			//
			ENCRYPTION_TYPE_COUNT
		};*/

	static const wxString TYPES[TYPES_COUNT];
	static const wxString HEADERS[HEADERS_COUNT];

	Packet(unsigned int type = TYPE_INVALID, const char* data = 0);

	bool Valid()
	{
		if (parse_)
			Parse();

		return valid_;
	}

	/**  Get the raw data for this packet to be sent over the network.
	@warning Its is left up to the caller to delete the returned allocated memory! */
	char* Raw() const;

	void Raw(const char* raw);

	/** Get the length of the raw data.
	@note This *can* be a somewhat intensive call, so use sparingly.
	*/
	unsigned int RawLen() const;
	
	/** Set the value of a header.
	*/
	void Add(int header, const wxString& value);

	/** Get the value of a specific header type.  If there are more than one
	of the headers, use number to get that particular index. 
	*/
	const wxString Get(int header,int number = 0);
	unsigned int Count(int header);

	/** Get the packet type.
	*/
	unsigned int Type() const;
	void Type(unsigned int type);

	~Packet();

private:

	/** Parse the packet from the raw data. */	
	void Parse();

	/** All of the headers and their data. */
	std::vector<Header> headerdata_;

	//bool required_[HEADERS_COUNT];
	bool parse_;

	/** The raw data for this packet. */
	wxString raw_;

	unsigned int size_;

	/** variable used to check if the packet is valid in syntax. */
	bool valid_;

	unsigned int type_;

};

#endif
