/* $Id: hostcache.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

#include "hostcache.h"
#include "app.h"
#include "utils.h"

#include <wx/textfile.h>
#include <wx/tokenzr.h>

using namespace std;

int HostCache::currentHost_ = -1;

HostCache::HostCache(const wxString& file)
	: file_(file),changed_(FALSE)
{;}

bool HostCache::Load()
{
	currentHost_ = -1;

	wxTextFile in;
	if (!in.Open(file_))
	{
		LogError(wxString().Format("Loading hostcache file %s ...",file_.c_str()));
		return FALSE;
	}

	LogDebug(wxString().Format("Loading hostcache file %s ...",file_.c_str()));

	const unsigned int size = in.GetLineCount();

	for (unsigned int x = 0; x < size;x++)
	{
		wxString buf = in[x];
		if (buf.Length() != 0)
		{
			wxStringTokenizer tkz(buf);
			if (tkz.CountTokens() == 2)
			{
				wxString address = tkz.GetNextToken();
				wxString count = tkz.GetNextToken();
				//hosts_.push_back(HostCacheNode(address,atoi(count.c_str())));
				AddHost(address,atoi(count.c_str()));
			}
			else
			{
				LogError(wxString().Format("Invalid line %d in hostcache file",x));
			}
		}
	}

	currentHost_=0;

	in.Close();

	changed_ = FALSE;

	return TRUE;
}

bool HostCache::Save()
{
	if (!changed_)
		return FALSE;

	wxTextFile out;

	// remove any existing file
	wxRemoveFile(file_);

	if (!out.Create(file_))
	{
		LogError("Could not save hostcache file.");
		return FALSE;
	}

	for (unsigned int x = 0; x < hosts_.size();x++)
	{
		out.AddLine(wxString().Format("%s %d",hosts_[x].address.c_str(),hosts_[x].count));
	}
	return out.Write();
}

bool HostCache::AddHost(const wxString& host, unsigned int count)
{
	LogDebug(wxString().Format("Adding host to hostcache: %s",host.c_str()));

	bool found = FALSE;
	for (unsigned int x = 0; x < hosts_.size();x++)
	{
		if (hosts_[x].address == host)
		{
			hosts_[x].count++;
			found = TRUE;
			changed_ = TRUE;
			break;
		}
	}

	if (!found)
	{
		hosts_.push_back(HostCacheNode(host,count));
	}

	return found;
}

bool HostCache::DeleteHost(const wxString& host)
{
	bool found = FALSE;
	for (unsigned int x = 0; x < hosts_.size();x++)
	{
		if (hosts_[x].address == host)
		{
			hosts_.erase(hosts_.begin()+x);
			found = TRUE;
			changed_ = TRUE;
			break;
		}
	}
	return found;
}

unsigned int HostCache::Size() const
{
	return hosts_.size();
}

wxString HostCache::GetNext() const
{
	if (currentHost_ < 0 || currentHost_ >= hosts_.size())
		return "";
	
	return hosts_[currentHost_++].address;
}

wxString HostCache::operator[](unsigned int index)
{
	return hosts_[index].address;
}

HostCache::~HostCache()
{;}
