/* $Id: hashtable.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file hashtable.h
@brief Local Hash Table Definition
*/
#ifndef TURNSTILE_HASHTABLE_H
#define TURNSTILE_HASHTABLE_H

#include "files.h"

#include <wx/string.h>

#include <vector>
#include <map>

class wxCriticalSection;
class HashValue;

/** @class HashTable
Simply an array of HashValues, but with some extra functionality.

There may be replica hashes in the hash table if more than one node has
data pertaining to the hash.

@note Don't confuse this with a real hash table (like std::map or something).
	It's not one.

@see HashValue
*/
class HashTable
{
public:

	HashTable();

 	/** Add a hash to the table and atleast one node.
	*/
	void Add(HashValue value);

	/** Remove a hash from the table.
	*/
	void Remove(wxString hash);

	/** Get a node for a hash.
	@return Returns "" if the hash doesn't exist in the table
	*/
	HashValue GetNext(wxString hash, int cookie = 1);

	HashValue Get(int cookie = 1);

	/** Remove any values that have a hash greater than the
	one passed.
	*/
	//HashValue Pull(wxString prev);

	//HashValue PullLess(wxString hash1);

	//HashValue PullBetween(wxString hash1,wxString hash2);

	~HashTable();

#ifdef DEBUG
	void DumpTable();
#endif

private:
	/** Cleanup function to remove duplicates. 
	*/
	void Purge();

	wxCriticalSection* crit_;

	/** A simple array of hashes. */
	std::vector<HashValue> hashes_;
};

#endif
