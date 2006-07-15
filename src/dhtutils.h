/* $Id: dhtutils.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file dhtutils.h
@brief Distributed Hash Table Utils
*/
#ifndef TURNSTILE_DHT_UTILS_H
#define TURNSTILE_DHT_UTILS_H

#include <wx/string.h>

class Key;

/** @class Node
@brief A single node in the DHT.
*/
class Node
{
public:
	Node();
	explicit Node(const wxString& address);
	explicit Node(const wxString& nid, const wxString& address);

	void Address(const wxString& address);
	wxString Address() const;
	void Nid(const wxString& nid);
	wxString Nid() const;

	~Node();

private:

	/** IP address of the node. */
	wxString address_;

	/** Node hash. */
	wxString nid_;
};

/** @class HashValue
A single hash value that's in the distributed hash table.  It contains
the hash and more information about the hash.
@see HashTable
@see LookupResponse
*/
class HashValue
{
public:
	HashValue()
	{;}

   HashValue(const wxString& node_, const wxString& hash_, const wxString& info_ = wxT(""))
		: node(node_),hash(hash_),info(info_)
	{;}

	bool operator!=(const HashValue& rhs) const
	{
		return (hash != rhs.hash || node != rhs.node || info != rhs.info);
	}

	bool operator==(const HashValue& rhs) const
	{
		return (hash == rhs.hash && node == rhs.node && info == rhs.info);
	}

	/** The hash */
	wxString hash;
	/** The node that has the data for the hash */
	wxString node;
	/** Some extra information about the data */
	wxString info;
};


#endif
