/* $Id: finger.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

#include "finger.h"
#include "utils.h"

#include <wx/wx.h>

using namespace std;

FingerTable::FingerTable()
{
	crit_ = new wxCriticalSection;
}

Node FingerTable::BestStart(const wxString hash)
{
	if (hash == "")
		return Node();

	wxCriticalSectionLocker locker(*crit_);

	Node answer;
	for (unsigned int x = 0; x < nodes_.size();++x)
	{
		// if the node is less than the hash
		if (MYlte(nodes_[x].Nid(),hash))
		{
			answer = nodes_[x];
		}
	}

	return Node();
}

void 
FingerTable::Clear()
{
	wxCriticalSectionLocker locker(*crit_);

	nodes_.clear();
}

void 
FingerTable::Delete(const wxString& nid)
{
	wxCriticalSectionLocker locker(*crit_);

	for (unsigned int x = 0; x < nodes_.size();x++)
	{
		if (nid == nodes_[x].Nid())
		{
			nodes_.erase(nodes_.begin()+x);
			break;
		}
	}
}

bool
FingerTable::Add(const Node& node)
{
	wxCriticalSectionLocker locker(*crit_);

	bool answer = FALSE;
	/*
	Don't add it if it already exists.  This prevents us from basically looping
	around the circle and creating problems.
	*/
	bool found = false;
	for (unsigned int x = 0; x < nodes_.size();x++)
	{
		if (node.Nid() == nodes_[x].Nid())
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		answer = TRUE;
		nodes_.push_back(node);
	}
	
	return answer;
}

FingerTable::~FingerTable()
{
	delete crit_;
}

