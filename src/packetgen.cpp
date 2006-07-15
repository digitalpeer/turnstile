/** @file packetgen.cpp
@biref Functions responsible for generating packets for the most part.
*/
#include "config.h"
#include "packet.h"
#include "network.h"
#include "crypto.h"
#include "prefs.h"
#include "utils.h"
#include "dhtutils.h"

#ifdef DEBUG
#include "hashtable.h"
#endif

#include <wx/string.h>

#include <string>
using namespace std;

#define CONFIG_TXT_AGENT	wxT("TURNSTILE/0.0.1")

Packet GenPacket_Syn(const wxString& rsaPub)
{
	Packet packet(Packet::TYPE_SYN);
	packet.Add(Packet::HEADER_PUBLIC_KEY,rsaPub);
	return packet;
}

Packet GenPacket_Ack(const wxString& symKey, const wxString& peer)
{
	Packet packet(Packet::TYPE_ACK);
	packet.Add(Packet::HEADER_SRC_AGENT,CONFIG_TXT_AGENT);
	packet.Add(Packet::HEADER_SYMETRIC_KEY,symKey);
	packet.Add(Packet::HEADER_SRC_NID,Prefs::Instance()->Get(Prefs::NID));
	packet.Add(Packet::HEADER_SRC_ADDR, Network::Instance()->PublicAddress());
	packet.Add(Packet::HEADER_DEST_HOST, peer);
	return packet;
}

Packet GenPacket_Confirm(const wxString& peer)
{
	Packet packet(Packet::TYPE_CONFIRM);
	packet.Add(Packet::HEADER_SRC_AGENT,CONFIG_TXT_AGENT);
	packet.Add(Packet::HEADER_SRC_NID,Prefs::Instance()->Get(Prefs::NID));
	packet.Add(Packet::HEADER_SRC_ADDR,Network::Instance()->PublicAddress());
	packet.Add(Packet::HEADER_WRKGRPKEY,Prefs::Instance()->Get(Prefs::WRKGRPKEY));
	packet.Add(Packet::HEADER_DEST_HOST, peer);
	return packet;
}

Packet GenPacket_Join(const wxString& rsaPub)
{
	Packet packet(Packet::TYPE_JOIN);
	packet.Add(Packet::HEADER_PUBLIC_KEY,rsaPub);
	packet.Add(Packet::HEADER_SRC_AGENT, CONFIG_TXT_AGENT);
	packet.Add(Packet::HEADER_SRC_ADDR, Network::Instance()->PublicAddress());
	packet.Add(Packet::HEADER_WRKGRPKEY,Prefs::Instance()->Get(Prefs::WRKGRPKEY));
	return packet;
}

Packet GenPacket_JoinResponse(const wxString& nid, const Node& node, const wxString& peer)
{
	Packet packet(Packet::TYPE_JOIN_RESPONSE);
	packet.Add(Packet::HEADER_SRC_AGENT, CONFIG_TXT_AGENT);
	packet.Add(Packet::HEADER_SRC_NID,Prefs::Instance()->Get(Prefs::NID));
	packet.Add(Packet::HEADER_SRC_ADDR, Network::Instance()->PublicAddress());
	packet.Add(Packet::HEADER_ASSIGN_NID, nid);
	packet.Add(Packet::HEADER_NID, node.Nid());
	packet.Add(Packet::HEADER_NODE, node.Address());
	packet.Add(Packet::HEADER_DEST_HOST, peer);
	return packet;
}

Packet GenPacket_Request(const wxString& filename, const wxString& md5)
{
	Packet packet(Packet::TYPE_REQUEST);
	packet.Add(Packet::HEADER_FILEMD5, md5);
	packet.Add(Packet::HEADER_FILENAME, filename);
	return packet;
}

Packet GenPacket_Send(char* buffer, unsigned int size)
{
	Packet packet(Packet::TYPE_FILESEG);
	string raw = crypto::HexEncode(string(buffer,size));
	packet.Add(Packet::HEADER_PAYLOAD, wxString(raw.c_str(),raw.size()));
	return packet;
}

Packet GenPacket_Query(const wxString& hash)
{
	Packet packet(Packet::TYPE_QUERY);
	packet.Add(Packet::HEADER_HASH, hash);
	return packet;
}

Packet GenPacket_ResponseSuccess()
{
	Packet packet(Packet::TYPE_RESPONSE_FOUND);
	// hashes found will be insrted by the caller
	return packet;
}

Packet GenPacket_ResponseJump(const wxString& addr)
{
	Packet packet(Packet::TYPE_RESPONSE_JUMP);
	packet.Add(Packet::HEADER_JUMP, addr);
	return packet;
}

Packet GenPacket_ResponseEnd()
{
	Packet packet(Packet::TYPE_RESPONSE_SUCCESSOR);
	return packet;
}

Packet GenPacket_Insert()
{
	Packet packet(Packet::TYPE_INSERT);
	return packet;
}

Packet GenPacket_HelloNext()
{
	Packet packet(Packet::TYPE_HELLO_NEXT);
	return packet;
}

Packet GenPacket_Stat()
{
	Packet packet(Packet::TYPE_STAT);
	return packet;
}

Packet GenPacket_StatResponse()
{
	Packet packet(Packet::TYPE_STAT_RESPONSE);

	// insert previous node
	packet.Add(Packet::HEADER_PREV_NID, Network::Instance()->Prev().Nid());
	packet.Add(Packet::HEADER_PREV_ADDR, Network::Instance()->Prev().Address());

	// insert our complete next table for them to grab
	Node next;
	for (unsigned int x = 0; x < Network::Instance()->NextSize();++x)
	{
		Network::Instance()->GetNext(x,next);
		packet.Add(Packet::HEADER_NEXT_NID, next.Nid());
		packet.Add(Packet::HEADER_NEXT_ADDR, next.Address());
	}

#ifdef DEBUG
	// insert all the hashes we own into the packet also
	for (unsigned int x = 1; ;x++)
	{
		HashValue value = Network::Instance()->hashTable->Get(x);
		if (value.hash == "")
			break;

		wxString info = value.hash + "(" + value.node + ")";
		packet.Add(Packet::HEADER_HASH, info);
	}

	// insert our successor list
	/*wxString list;
	for (unsigned int x = 0; x < successorList.size(); x++)
	{
		list += successorList[x] + " ";
	}
	packet.Add(Packet::HEADER_SUCCESSORLIST, info);*/

#endif

	return packet;
}

Packet GenPacket_HelloBuddy()
{
        Packet packet(Packet::TYPE_HELLO_BUDDY);
	wxString info = wxT("BUDDY:") + Prefs::Instance()->Get(Prefs::ALIAS) + wxT(":") +
	   Prefs::Instance()->Get(Prefs::PUBLICKEY) + wxT(":") +
						Prefs::Instance()->Get(Prefs::PROFILE);
	packet.Add(Packet::HEADER_INFO, info);
        return packet;
}

Packet GenPacket_Cont(unsigned int start, unsigned int end)
{
        Packet packet(Packet::TYPE_CONT);
	packet.Add(Packet::HEADER_FILESEG_START, wxString().Format(wxT("%i"),start));
	packet.Add(Packet::HEADER_FILESEG_END, wxString().Format(wxT("%i"),end));
        return packet;
}

Packet GenPacket_Im(const wxString& txt)
{
	Packet packet(Packet::TYPE_IM);
	string raw = crypto::HexEncode(string((char*)txt.c_str(),txt.size()));
	packet.Add(Packet::HEADER_IM, wxString(raw.c_str(),raw.size()));
	//packet.Add(Packet::HEADER_IM, txt);
        return packet;
}

Packet GenPacket_ImConnect()
{
	Packet packet(Packet::TYPE_IM_CONNECT);
	wxString info = wxT("BUDDY:") + Prefs::Instance()->Get(Prefs::ALIAS) + wxT(":") +
	   Prefs::Instance()->Get(Prefs::PUBLICKEY) + wxT(":") +
						Prefs::Instance()->Get(Prefs::PROFILE);
	packet.Add(Packet::HEADER_INFO, info);
        return packet;
}

Packet GenPacket_ImClose()
{
	Packet packet(Packet::TYPE_IM_CLOSED);
        return packet;
}



