/* $Id: hostcache.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

#ifndef TURNSTILE_HOSTCACHE_H
#define TURNSTILE_HOSTCACHE_H

#include <wx/string.h>

#include <vector>

class HostCacheNode
{
public:
	HostCacheNode(wxString address_ = "",unsigned int count_ = 0)
		: address(address_),count(count_)
	{;}

	wxString address;
	unsigned int count;

};

/** @class HostCache
*/
class HostCache
{
public:

	HostCache(const wxString& file);

	/** Load a hostcache file.
	*/
	bool Load();
	
	/** Save the currently loaded cache.
	@note It will *only* save if the loaded data has changed sinse the last save.
	*/
	bool Save();
	
	/** Add a host to the loaded cache.
	*/
	bool AddHost(const wxString& host, unsigned int count = 0);
	
	/** Delete a host from the loaded cache.
	@return TRUE if the host was found.
	*/
	bool DeleteHost(const wxString& host);
	
	/** Get a host my index.
	*/
	wxString operator[](unsigned int index);
	
	wxString* GetArray()
	{
		wxString* array = new wxString[Size()];
		for (unsigned int x = 0; x < Size();x++)
			array[x] = hosts_[x].address;
		return array;
	}
	
	/** Get the number of hosts.
	*/
	unsigned int Size() const;
	
	wxString GetNext() const;

	~HostCache();
	
private:

	/** Array of addresses that are currently loaded. */
	std::vector<HostCacheNode> hosts_;
	
	wxString file_;
	
	bool changed_;
	
	static int currentHost_;
	
};

#endif
