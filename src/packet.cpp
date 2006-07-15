/* $Id: packet.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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
@brief Packet Implimentation
*/
#include "packet.h"
#include "app.h"
#include "typeconv.h"
#include "utils.h"
#include "crypto.h"

#ifdef DEBUG
#include <iostream>
#endif

using namespace std;

#include <wx/tokenzr.h>

#define HEADER_DEL 	':'
#define VALUE_DEL	'^'

const wxString Packet::TYPES[Packet::TYPES_COUNT] = {
   wxT(""),
   wxT("TYPE_SYN"),
       wxT("TYPE_ACK"),
	   wxT("TYPE_CONFIRM"),
	       wxT("TYPE_JOIN"),
		   wxT("TYPE_JOIN_RESPONSE"),
		       wxT("TYPE_REQUEST"),
			   wxT("TYPE_CONT"),
   wxT("TYPE_FILESEG"),
   wxT("TYPE_QUERY"),
   wxT("TYPE_QUERYRESPONSE"),
   wxT("TYPE_INSERT"),
   wxT("TYPE_REMOVE"),
		wxT("TYPE_RESPONSE_FOUND"),
		wxT("TYPE_RESPONSE_JUMP"),
		wxT("TYPE_RESPONSE_SUCCESSOR"),
		wxT("TYPE_HELLO_NEXT"),
		wxT("TYPE_HELLO_BUDDY"),
		wxT("TYPE_STAT"),
		wxT("TYPE_STAT_RESPONSE"),
		wxT("TYPE_IM_CONNECT"),
		wxT("TYPE_IM_CLOSED"),
		wxT("TYPE_IM")
		};

const wxString Packet::HEADERS[Packet::HEADERS_COUNT] = {
		//
		//wxT("enc-type"),
   wxT("public-key"),
   wxT("symetric-key"),
   wxT("src-alias"),
		wxT("src-agent"),
		wxT("src-addr"),
		wxT("dest-host"),
		wxT("src-nid"),
		//
		wxT("hash"),
		wxT("payload"),
		//
		wxT("hostcache"),
		//
		wxT("request-target"),
		//
		wxT("file-md5"),
		wxT("file-name"),
		//
		wxT("jump"),
		//
		wxT("query-string"),
		wxT("query-target"),
		//
		wxT("prev-addr"),
		wxT("prev-nid"),
		wxT("next-addr"),
		wxT("next-nid"),
		wxT("nid"),
		wxT("node"),
		wxT("assign-nid"),
		wxT("info"),
		wxT("alias"),
		wxT("private-key"),
		wxT("fileseg-start"),
		wxT("fileseg-end"),
		wxT("im"),
		wxT("wrkgrpkey")
		};

Packet::Packet(unsigned int type,const char* data) : type_(type)
{
	if (data != 0)
		Raw(data);

	parse_ = TRUE;
	valid_ = FALSE;
}

void
Packet::Raw(const char* raw)
{
	raw_ = raw;
}

unsigned int
Packet::RawLen() const
{
	return raw_.size();
}

char*
Packet::Raw() const
{
	return (char*)raw_.c_str();
}

void
Packet::Add(int header, const wxString& value)
{
	raw_ += HEADERS[header];
	raw_ += wxString(HEADER_DEL,1);
	raw_ += value;
	raw_ += wxString(VALUE_DEL,1);

	parse_ = TRUE;
}

unsigned int
Packet::Count(int header)
{
	unsigned int answer = 0;
	for (unsigned int x = 0; x < headerdata_.size();x++)
	{
		if (headerdata_[x].id == header)
			answer++;
	}
	return answer;
}

const wxString
Packet::Get(int header,int number)
{
	static wxString BLANK;

	if (parse_)
		Parse();

	int counts[HEADERS_COUNT];
	for (int y = 0; y < HEADERS_COUNT;y++)
		counts[y] = 0;

	for (unsigned int x = 0; x < headerdata_.size();x++)
	{
		if (headerdata_[x].id == header && counts[header] == number)
		{
			return wxString(raw_.c_str()+headerdata_[x].valueStart,headerdata_[x].valueEnd-headerdata_[x].valueStart);
		}
		++(counts[headerdata_[x].id]);
	}

	LogDebug(wxString().Format("Returned blank value for packet header %s on packet type %d",HEADERS[header].c_str(),Type()));

	return BLANK;
}

/**
This is an internal function that will only get called when necessary.
*/
void
Packet::Parse()
{
	unsigned int position = 0;
	unsigned int len = RawLen();

	while (position < len)
	{
		// extract the header
		wxString header = extract(Raw(),len,position,HEADER_DEL);
		// save this position (it's first char in value)
		int valueStart = position;
		// crawl up to value delimiter
		if (!crawl(Raw(),len,position,VALUE_DEL))
		{
			LogError(wxString().Format("Invalid packet value received for header: '%s'", header.c_str()));
			LogError(wxString().Format("Packet data of length %d: %s",RawLen(),Raw()));
			valid_=FALSE;
			return;
		}

		int index = GetIndex(HEADERS,HEADERS_COUNT,header);

		if (index == -1 || header.size() == 0)
		{
			LogError(wxString().Format("Unknown packet header received: '%s'", header.GetData()));
			LogError(wxString().Format("%s",Raw()));
			valid_=FALSE;
			return;
		}

		switch (index)
		{
		case HEADER_PAYLOAD:
		case HEADER_FILENAME:
		case HEADER_FILEMD5:
		case HEADER_SRC_ALIAS:
		case HEADER_SRC_AGENT:
		case HEADER_SRC_NID:
		case HEADER_SRC_ADDR:
		case HEADER_DEST_HOST:
		case HEADER_HOSTCACHE:
		case HEADER_REQUESTTARGET:
		case HEADER_QUERYSTRING:
		case HEADER_QUERYTARGET:
		case HEADER_JUMP:
		case HEADER_NID:
		case HEADER_HASH:
		case HEADER_ASSIGN_NID:
		case HEADER_INFO:
		case HEADER_NODE:
		case HEADER_PREV_ADDR:
		case HEADER_PREV_NID:
		case HEADER_NEXT_ADDR:
		case HEADER_NEXT_NID:
		case HEADER_SYMETRIC_KEY:
		case HEADER_ALIAS:
		case HEADER_PRIVATE_KEY:
		case HEADER_PUBLIC_KEY:
		case HEADER_FILESEG_START:
		case HEADER_FILESEG_END:
		case HEADER_IM:
		case HEADER_WRKGRPKEY:
		{

			headerdata_.push_back(Header(index,valueStart,position));
			break;
		}
		default:
		{
			LogError(wxString().Format("Unhandled packet header: %s",HEADERS[index].c_str()));
			valid_=FALSE;
			return;
		}
		}
		// jump off value delimiters
		++position;
	}
	valid_=TRUE;
	parse_=FALSE;
}

unsigned int
Packet::Type() const
{
	return type_;
}

void
Packet::Type(unsigned int type)
{
	type_ = type;
}

Packet::~Packet()
{;}

