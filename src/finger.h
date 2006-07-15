/* $Id: finger.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

#ifndef TURNSTILE_FINGER_H
#define TURNSTILE_FINGER_H

#include "dhtutils.h"

#include <vector>

class wxCriticalSection;
class wxString;

/** @class FingerTable

Alright, basically this is yet another cool container.  How nodes are added to this
container is where the cool shit happens.  BestStart() is the only thing that is used
except when building the finger table.  Which, by the way, gets built when a node
re-stabalizes and finds a new parent.  This event driven process really has nothing to do
with re-doing the finger table, but whatever.  Make it better or something.  It's pretty
expensive to build the finger table.
*/
class FingerTable
{
public:
	FingerTable();

	/** Give me the node with the highest nid less than the hash.
	If a blank hash is supplied, it will simply return a random node.
	*/
   Node BestStart(const wxString hash = wxT(""));

	/** Remove a node from the hash table.
	*/
	void Delete(const wxString& nid);

	/** Add a node to the hash table.  Returns false if the node already
	exists in the table (wasn't added).
	*/
	bool Add(const Node& node);

	void Clear();

	~FingerTable();

private:
	/** Simple array of nodes in the finger table. */
	std::vector<Node> nodes_;

	wxCriticalSection* crit_;
};

#endif
