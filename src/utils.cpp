/* $Id: utils.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

#include "utils.h"
#include "typeconv.h"

#include <wx/tokenzr.h>
#include <wx/file.h>
#include <wx/string.h>
#include <wx/log.h>
#include <wx/wx.h>

int
GetIndex(const wxString* array, int size, wxString value)
{
	int index = -1;
	for (int x = 0; x < size;x++)
	{
		if (array[x] == value)
		{
			index = x;
			break;
		}
	}
	return index;
}

wxString
extract(const char* raw, unsigned int& len, unsigned int& position, char del)
{
	const unsigned int start = position;
	for ( ; position < len; ++position)
	{
		if (raw[position] == del)
			break;
	}
	// go ahead and jump over delimiter
	++position;
	return wxString(raw+start,position-start-1);
}

bool
crawl(const char* raw, unsigned int& len, unsigned int& position, char del)
{
	int answer = FALSE;
	const unsigned int start = position;
	for ( ; position < len; ++position)
	{
		if (raw[position] == del)
		{
			answer = TRUE;
			break;
		}
	}
	return answer;
}

int
ParsePort(wxString value)
{
	int answer = 0;
	wxStringTokenizer tokens(value, "/");

	if (tokens.CountTokens() == 2)
	{
		const wxString host = tokens.GetNextToken();
		const wxString port = tokens.GetNextToken();

		answer = convert<int>(port.GetData());
	}
	return answer;
}

wxString
ParseHost(wxString value)
{
	wxString answer = "";
	wxStringTokenizer tokens(value, "/");

	if (tokens.CountTokens() == 2)
	{
		const wxString host = tokens.GetNextToken();
		const wxString port = tokens.GetNextToken();

		answer = host;
	}
	return answer;
}

//static wxMutex* logmutex = new wxMutex();

void
LogDebug(wxString text)
{
#ifdef DEBUG
#ifndef WIN32
//wxMutexGuiEnter();
	//logmutex->Lock();
	wxLogMessage(text);
	//logmutex->Unlock();
//wxMutexGuiLeave();
#endif
#endif
}

void
LogMsg(wxString text)
{
//#ifndef WIN32
//wxMutexGuiEnter();
	//logmutex->Lock();
	wxLogMessage("Message: " + text);
	//logmutex->Unlock();
//wxMutexGuiLeave();
//#endif
}

void
LogError(wxString text)
{
//#ifndef WIN32
//wxMutexGuiEnter();
	//logmutex->Lock();
	wxLogMessage("** Error: " + text);
	//logmutex->Unlock();
//wxMutexGuiLeave();
//#else
	//wxMessageDialog* diag = new wxMessageDialog(0,text,"Error",wxOK);
	//diag->ShowModal();
//#endif
}

unsigned int
FileSize(const wxString& path)
{
	wxFile file(path.c_str(),wxFile::read);
	unsigned int size = file.Length();
	file.Close();
	return size;
}

wxString
UnAliasEscape(const wxString& alias)
{
	wxString answer;
	for (unsigned int x = 0; x < alias.Length();x++)
	{
		switch (alias[x])
		{
		case '\\':
		{
			/*
			make sure it's not possible to crash with an
			escape on the end
			*/
			if (x == alias.Length()-1)
				return answer;

			switch (alias[++x])
			{
			case 's':
			{
				answer += " ";
				break;
			}
			}
			break;
		}
		default:
			answer += alias[x];
		}
	}
	return answer;
}

wxString
AliasEscape(const wxString& alias)
{
	wxString answer;
	for (unsigned int x = 0; x < alias.Length();x++)
	{
		switch (alias[x])
		{
		case ' ':
			answer += "\\s";
			break;
		default:
			answer += alias[x];
		}
	}
	return answer;
}

/*
Welcome to my cheap hack.
*/
wxString
HumanFileSize(unsigned int bytes)
{
	if (bytes > 1024)
	{
		if (bytes > 1024*1024)
		{
			if (bytes > 1024*1024*1024)
				return wxString().Format("%.2fGb",(double)(bytes)/1024/1024/1024);
			else
				return wxString().Format("%.2fMb",(double)(bytes)/1024/1024);
		}
		else
			return wxString().Format("%.2fKb",(double)(bytes)/1024);
	}
	else
		return wxString().Format("%.2fb",(double)(bytes));
}

/*
wxString comparison operators are not what I want.  This is.

strcmp()  function compares the two strings s1 and s2.  It returns
       an integer less than, equal to, or greater than zero if  s1  is  found,
       respectively, to be less than, to match, or be greater than s2.
*/

bool MYlte(const wxString& a, const wxString& b)
{
	return (strcmp(a.c_str(),b.c_str()) < 0) || (strcmp(a.c_str(),b.c_str()) == 0);
}

bool MYlt(const wxString& a, const wxString& b)
{
	return (strcmp(a.c_str(),b.c_str()) < 0);
}

bool MYne(const wxString& a, const wxString& b)
{
	return (strcmp(a.c_str(),b.c_str()) != 0);
}

bool MYe(const wxString& a, const wxString& b)
{
	return (strcmp(a.c_str(),b.c_str()) == 0);
}

bool MYgt(const wxString& a, const wxString& b)
{
	return (strcmp(a.c_str(),b.c_str()) > 0);
}


#if 0
BOOL IsInternetConnection()
{
 char szHostName[128];

 BOOL bPrivateAdr = false; // Private Address area
 BOOL bClassA = false;     // Class A definition
 BOOL bClassB = false;     // Class B definition
 BOOL bClassC = false;     // Class C definition
 BOOL bAutoNet = false;    // AutoNet definition
 CString str;

 if (gethostname(szHostName, 128) == 0 )
 {
  // Get host adresses
  struct hostent * pHost;
  int i;
  UINT ipb;
  pHost = gethostbyname(szHostName); 

  for (i = 0; 
       pHost!= NULL && pHost->h_addr_list[i]!= NULL; 
       i++ )
  {
   int j;
   str="";
   bClassA = bClassB = bClassC = false;
   for( j = 0; j < pHost->h_length; j++ )
   {
    CString addr;

    if( j > 0 )	str += ".";
    ipb = (unsigned int)((unsigned char*)pHost->h_addr_list[i])[j];

    // Define the IP range for exclusion
    if(j==0)
    {
     if(bClassA = (ipb == 10)) break; // Class A defined
     bClassB = (ipb == 172); 
     bClassC = (ipb == 192);
     bAutoNet = (ipb == 169);
    }
    else if (j==1)
    {
     // Class B defined
     if(bClassB = (bClassB && ipb >= 16 && ipb <= 31)) break;

     // Class C defined
     if(bClassC = (bClassC && ipb == 168)) break;

     //AutoNet pasibility defined
     if(bAutoNet = (bAutoNet && ipb == 254)) break;
    }

    addr.Format("%u", ipb );
    str += addr;
   }
   // If any address of Private Address 
   // area has been found bPrivateAdr = true
   bPrivateAdr = bPrivateAdr || bClassA || bClassB || bClassC;

   // If any address of Internet Address area 
   // has been found returns TRUE
   if (!bClassA 
   && !bClassB 
   && !bClassC 
   && !bAutoNet 
   && str != "127.0.0.1" 
   && !str.IsEmpty()) 
    return TRUE;
  }
 }

 if (bPrivateAdr)
 {
  // The system has IP address from Private Address 
  // area,only. Internet from the computer can be accessable 
  // via Proxy. If user turn on proxy connection flag, the 
  // function believe Internet accessable.
  return bProxyConnection;
 }
 
 return false;
}
#endif


