/* $Id: prefs.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

#include "prefs.h"
#include "utils.h"
#include "app.h"
#include "config.h"
#include "buddy.h"
#include "mainframe.h"
#include "hostcache.h"
#include "hashtable.h"
#include "crypto.h"

#include <wx/wx.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/dir.h>
#include <wx/utils.h>

#ifdef DEBUG
#include <iostream>
using namespace std;
#endif

#define TXT_FILEPROFILE		"turnstile.dat"
#define TXT_FILEHOSTCACHE	"hostcache.dat"

wxCriticalSection* Prefs::crit_ = new wxCriticalSection;
Prefs* Prefs::instance_ = 0;

const wxString Prefs::HEADERS[Prefs::COUNT] = 
			{
				"[publickey]",
				"[alias]",
				"[profile]",
				"[port]",
				"[sharedir]",
				"[group]",
				"[buddy]",
				"[nid]",
				"[maxuploads]",
				"[maxdownloads]",
				"[wrkgrpkey]",
				"[usefirewall]",
				"[regenrsa]",
				"[rsapriv]",
				"[rsapub]",
				"[showoffline]"
			};
		
Prefs*
Prefs::Instance()
{
	if (instance_ == 0)
	{
		instance_ = new Prefs();
		return instance_;
	}
	else
	{
		return instance_;
	}
}
	
Prefs::Prefs()
{
	// set root directory
	root_ = wxGetHomeDir() + "/.turnstile";
	hostCache_ = new HostCache(root_ + "/" + TXT_FILEHOSTCACHE);

//#ifdef DEBUG
//	LogDebug(root_);
//#endif

	InitDefaults();
}
	
bool
Prefs::Save()
{
	wxTextFile out;

	// remove any existing file
	wxRemoveFile(root_ + "/" + TXT_FILEPROFILE);

	if (!out.Create(root_ + "/" + TXT_FILEPROFILE))
	{
		LogDebug(wxString().Format("Error: Could not save prefs file: %s", (root_ + "/" + TXT_FILEPROFILE).c_str()));
		return FALSE;
	}

	out.AddLine(HEADERS[ALIAS] + " " + Get(ALIAS));
	out.AddLine(HEADERS[PROFILE] + " " + Get(PROFILE));
	out.AddLine(HEADERS[SHAREDIR] + " " + Get(SHAREDIR));
	out.AddLine(HEADERS[PUBLICKEY] + " " + Get(PUBLICKEY));
	out.AddLine(HEADERS[WRKGRPKEY] + " " + Get(WRKGRPKEY));
	out.AddLine(HEADERS[REGENRSA] + " " + Get(REGENRSA));
	out.AddLine(HEADERS[RSAPUB] + " " + Get(RSAPUB));
	out.AddLine(HEADERS[RSAPRIV] + " " + Get(RSAPRIV));
	out.AddLine(HEADERS[SHOWOFFLINE] + " " + Get(SHOWOFFLINE));

	out.AddLine(HEADERS[PORT] + " " + Get(PORT));
	out.AddLine(HEADERS[MAXUPLOADS] + " " + Get(MAXUPLOADS));
	out.AddLine(HEADERS[MAXDOWNLOADS] + " " + Get(MAXDOWNLOADS));

	for (unsigned int x = 0; x < bannedList.size();x++)
	{
		wxString newline = HEADERS[PORT];
		newline += " ";
		newline += ParseHost(*bannedList[x]);
		newline += "/";
		newline += ParsePort(*bannedList[x]);
		out.AddLine(newline);
	}

	SaveList(out,wxGetApp().GetMainWindow()->GetTreeCtrl()->GetRootItem(),wxTreeItemIdValue());

	return out.Write();
}

wxString
Prefs::Get(int id)
{
	wxCriticalSectionLocker locker(*crit_);

	return data_[id];
}

bool
Prefs::Set(int id, const wxString& value)
{
	wxCriticalSectionLocker locker(*crit_);

	bool answer = FALSE;

	if (data_[id] != value)
		answer = TRUE;

	data_[id] = value;

	return answer;
}
	
void
Prefs::InitDefaults()
{
	/*
	These will simply get replaced by real settings if they exist!
	*/
	Set(SHAREDIR,"");
	Set(PORT,"5000");
	Set(PROFILE,AliasEscape("New Turnstile User"));
	Set(ALIAS,"User0123456789");
	Set(PUBLICKEY,"0123456789012345678901234567890123456789");
	Set(WRKGRPKEY,"0123456789012345678901234567890123456789");
	Set(MAXUPLOADS,"2");
	Set(MAXDOWNLOADS,"5");
	Set(USEFIREWALL,"0");
	Set(REGENRSA,"0");
	Set(SHOWOFFLINE,"1");
}

bool
Prefs::Load()
{
	// if they don't have a .turnstile directory then create one for them
	if (!wxDirExists(root_))
	{
		LogDebug("Creating new turnstile directory...");
		if (!wxMkdir(root_ + "/",0777))
		{
			LogDebug(wxString().Format("Failed to create turnstile directory: %s",root_.c_str()));
			return FALSE;
		}
	}
	else
	{
		/*
		Load everything!
		*/
		BuddyTreeCtrl* tree = wxGetApp().GetMainWindow()->GetTreeCtrl();
		wxTreeItemId lastGroupId = tree->AddRoot(wxT("root"),-1, -1,(Buddy*)0);
		wxString lastGroup;

		wxTextFile in;
		if (!in.Open(root_ + "/" + TXT_FILEPROFILE))
		{
			LogDebug(wxString().Format("Failed to open prefs file: %s", (root_ + "/" + TXT_FILEPROFILE).c_str()));
			return FALSE;
		}

		//LogDebug("Loading prefs file ...");

		const unsigned int size = in.GetLineCount();
		for (unsigned int x = 0; x < size;x++)
		{
			wxString buf = in[x];

			//if it's not a blank line
			if (buf.Len() != 0)
			{
				wxStringTokenizer tkz(buf);
				if (tkz.CountTokens() < 2)
				{
					//LogDebug(wxString().Format("Error:  Not enough info on line number %d in prefs file: %s",x,(root_ + "/" + TXT_FILEPROFILE).c_str()));
					//continue;
				}

				wxString header = tkz.GetNextToken();

				int index = GetIndex(HEADERS,COUNT,header);

				if (index == -1)
				{
					LogDebug(wxString().Format("Error:  Unknown header on line number %d in prefs file: %s",x,(root_ + "/" + TXT_FILEPROFILE).c_str()));
					continue;
				}

				switch (index)
				{
				case SHOWOFFLINE:
				case USEFIREWALL:
				case PUBLICKEY:
				case ALIAS:
				case PROFILE:
				case PORT:
				case MAXUPLOADS:
				case MAXDOWNLOADS:
				case WRKGRPKEY:
				case SHAREDIR:
				case REGENRSA:
				case RSAPRIV:
				case RSAPUB:
				{
					if (tkz.HasMoreTokens())
					{
						wxString value;
						value = tkz.GetNextToken();
						Set(index,value);
					}

					break;
				}
				case GROUP:
				{
					if (tkz.HasMoreTokens())
					{
						lastGroup = tkz.GetNextToken();
						lastGroup = UnAliasEscape(lastGroup);
						tree->AddGroup(lastGroup,false);
					}
					//cout << "Added group " << lastGroup << endl;
					break;
				}
				case BUDDY:
				{
					if (lastGroup == "")
					{
						LogDebug(wxString().Format("Error:  Unexpected buddy on line number %d in prefs file: %s",x,(root_ + "/" + TXT_FILEPROFILE).c_str()));
						continue;
					}

					Buddy* buddy = new Buddy();
					buddy->Set(Buddy::DATA_ALIAS,UnAliasEscape(tkz.GetNextToken()));

					if (tkz.HasMoreTokens())
						buddy->Set(Buddy::DATA_PUBLICKEY,tkz.GetNextToken());

					tree->AddBuddy(buddy,lastGroup,false);

					// make sure this item is visible now

					// could use this?
					//treeCtrl_->DoEnsureVisible();
					//cout << "Added buddy " << buddy->Get(Buddy::DATA_ALIAS) << endl;
					break;
				}
				}

				if (tkz.HasMoreTokens())
					LogDebug(wxString().Format("Error:  Too much info on line number %d in prefs file: %s",x,(root_ + "/" + TXT_FILEPROFILE).c_str()));
			}
		}// end for

		in.Close();

		LogDebug("Prefs file loaded.");
	}

	return Save();
}

void
Prefs::SaveList(wxTextFile& out, const wxTreeItemId idParent, wxTreeItemIdValue cookie)
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
	if (buddy == 0)
	{
		out.AddLine(Prefs::HEADERS[GROUP] + " " + AliasEscape(tree->GetItemText(id)));
	}
	else
	{
		out.AddLine(Prefs::HEADERS[BUDDY] + " " + AliasEscape(buddy->Get(Buddy::DATA_ALIAS)) + " " +
				buddy->Get(Buddy::DATA_PUBLICKEY));
	}

	if (tree->ItemHasChildren(id))
	   SaveList(out,id,wxTreeItemIdValue());

	SaveList(out, idParent, cookie);
}

bool Prefs::CreateDefaultProfile()
{
	return TRUE;	
}



/** This will insert each shared file into the DHT.  It does this in two ways:
	-#  Insert the full path name.
	-#  Tokenize the file name and insert.
*/
void Prefs::ReloadShareDir()
{
	if (Get(SHAREDIR) == "")
		return;

	// start fresh
	sharedFiles.clear();

	wxArrayString files;
	LogDebug("Refreshing share directory ...");
	wxDir::GetAllFiles(Get(SHAREDIR), &files, "", wxDIR_DIRS | wxDIR_FILES);

	ShareFile share;
	wxFileName filename;

	for (unsigned int x = 0; x < files.GetCount();x++)
	{
		filename.Assign(files.Item(x));

		share.Size(FileSize(files.Item(x)));
		share.MD5(crypto::Md5Hash(files.Item(x)));

		wxStringTokenizer tkz(filename.GetFullName().MakeLower()," _-.()~{}[]");
		while (tkz.HasMoreTokens())
		{
			::wxYield();

			wxString token = tkz.GetNextToken();/*.MakeLower();*/
			if (token.Length() > 3)
			{
				//file.Key(token);
				share.Key(token);
				share.Path(files.Item(x));


				/*
				Don't add the file if we already have this key
				for this file.
				*/
				bool found = false;
				for (unsigned int y = 0; y < sharedFiles.size();y++)
				{
					if (sharedFiles[y].Path() == share.Path() &&
						sharedFiles[y].Key() == share.Key())
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					LogDebug("adding token " + token);
					sharedFiles.push_back(share);
				}

				//delete newFile;
			}
		}

		LogDebug(wxString().Format("Indexed file %s",filename.GetFullName().c_str()));
	}
}

/*
void
Prefs::AddSharedToHashTable()
{
	for (unsigned int x = 0; x < sharedFiles.size();x++)
	{
		HashTable::Instance()->Add(sharedFiles[x].Hash(),GetHostName());
	}
}*/

Prefs::~Prefs()
{
	// delete the constructed hostcache
	delete hostCache_;
}
