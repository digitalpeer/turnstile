/* $Id: prefs.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

#ifndef TURNSTILE_PREFS_H
#define TURNSTILE_PREFS_H

#include "files.h"
#include "buddytreectrl.h"

class wxCriticalSection;
class wxTreeItemId;
class wxTextFile;
class HostCache;
class wxString;

#include <wx/string.h>

#include <vector>

class Prefs
{
public:

	static Prefs* Instance();

	enum 	{ 		
			PUBLICKEY,
			ALIAS,
			PROFILE,
			PORT,
			SHAREDIR,
			GROUP,
			BUDDY,
			//BANNED,
			NID,
			MAXUPLOADS,
			MAXDOWNLOADS,
			WRKGRPKEY,
			USEFIREWALL,
			REGENRSA,
			RSAPRIV,
			RSAPUB,
			SHOWOFFLINE,
			//
			COUNT
		};
	
	static const wxString HEADERS[COUNT];
	
	bool Save();
	bool Load();

	void SaveList(wxTextFile& out, const wxTreeItemId idParent, wxTreeItemIdValue cookie);

	void InitDefaults();
	
	bool CreateDefaultProfile();
	
	wxString Get(int id);

	/** Set a value and return true if the value was changed.
	*/
	bool Set(int id, const wxString& value);

	wxString data_[COUNT];
	
	void ReloadShareDir();
	
	std::vector<wxString> bannedList;
	std::vector<ShareFile> sharedFiles;
	
	HostCache* GetHostCache()
	{
		return hostCache_;
	}
	
	//void AddSharedToHashTable();
	
	~Prefs();
	
private:

	Prefs();

	static Prefs* instance_;
	static wxCriticalSection* crit_;

	/** Current hostcache. */
	HostCache* hostCache_;
	wxString root_;

};


#endif
