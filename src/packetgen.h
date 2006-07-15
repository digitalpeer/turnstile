#ifndef TURNSTILE_PACKETGEN_H
#define TUENSTILE_PACKETGEN_H 

class wxString;
class Node;
class Packet;

Packet GenPacket_Syn(const wxString& rsaPub);
Packet GenPacket_Ack(const wxString& symKey, const wxString& peer);
Packet GenPacket_Confirm(const wxString& peer);
Packet GenPacket_Join(const wxString& rsaPub);
Packet GenPacket_JoinResponse(const wxString& nid, const Node& node, const wxString& peer);
Packet GenPacket_Request(const wxString& filename, const wxString& md5);
Packet GenPacket_Cont(unsigned int start, unsigned int end);
Packet GenPacket_Send(char* buffer, unsigned int size);
Packet GenPacket_Query(const wxString& hash);
Packet GenPacket_ResponseSuccess();
Packet GenPacket_ResponseJump(const wxString& addr);
Packet GenPacket_ResponseEnd();
Packet GenPacket_Insert();
Packet GenPacket_InsertSuccess();
Packet GenPacket_Remove();
Packet GenPacket_HelloNext();
Packet GenPacket_Stat();
Packet GenPacket_StatResponse();
Packet GenPacket_HelloBuddy();
Packet GenPacket_Im(const wxString& txt);
Packet GenPacket_ImConnect();
Packet GenPacket_ImClose();


#endif
