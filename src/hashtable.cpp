/* $Id: hashtable.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file hashtable.cpp
@brief Local Hash Table Implimentation
*/
#include "hashtable.h"
#include "utils.h"
#include "dhtutils.h"

#include <wx/wx.h>

using namespace std;

/**
Comparison function for the hashes so we can order them nicely.
*/
/*struct ltstr
{
	bool operator()(const wxString s1, const wxString s2) const
	{
		return strcmp(s1, s2) < 0;
	}
};*/

HashTable::HashTable()
{
	crit_ = new wxCriticalSection;
}

void
HashTable::Add(HashValue value)
{
	crit_->Enter();
	hashes_.push_back(value);
	crit_->Leave();

	/*
	Warning! Purge locks the crit section so we need to make
	sure and leave it.
	*/
	Purge();
}

HashValue
HashTable::GetNext(wxString hash, int cookie)
{
	wxCriticalSectionLocker locker(*crit_);

	HashValue answer;
	int counter = 0;
	for (int x = 0; x < hashes_.size();x++)
	{
		if (hashes_[x].hash == hash)
		{
			counter++;
			if (counter == cookie)
			{
				answer = hashes_[x];
				break;
			}
		}
	}
	return answer;
}

HashValue
HashTable::Get(int cookie)
{
	wxCriticalSectionLocker locker(*crit_);

	HashValue answer;
	int counter = 0;
	for (int x = 0; x < hashes_.size();x++)
	{
		counter++;
		if (counter == cookie)
		{
			answer = hashes_[x];
			break;
		}
	}
	return answer;
}

void
HashTable::Remove(wxString hash)
{
	wxCriticalSectionLocker locker(*crit_);

	for (int x = 0; x < hashes_.size();x++)
	{
		if (hashes_[x].hash == hash)
		{
			hashes_.erase(hashes_.begin()+x);
			/*
			Make sure we get all of them (start over).
			*/
			x=0;
		}
	}
}

#ifdef DEBUG
void
HashTable::DumpTable()
{
	wxCriticalSectionLocker locker(*crit_);

	LogDebug("**** Current Local Hash Table Listing ****");
	for (unsigned int x = 0; x < hashes_.size(); x++)
	{
		LogDebug(wxString().Format("* Hash:%s",hashes_[x].hash.c_str()));
		LogDebug(wxString().Format("* Node:%s",hashes_[x].node.c_str()));
		LogDebug(wxString().Format("* Info:%s",hashes_[x].info.c_str()));
	}
	LogDebug("******************************************");
}
#endif
/*
HashValue
HashTable::Pull(wxString prev)
{
	wxCriticalSectionLocker locker(*crit_);

	//pull anything greater than me OR less than previous
	HashValue answer;
	for (int x = 0; x < hashes_.size();x++)
	{
		if (MYlte(hashes_[x].hash,prev))

		{
			answer = hashes_[x];
			hashes_.erase(hashes_.begin()+x);
			break;
		}
	}
	return answer;
}

HashValue
HashTable::PullLess(wxString hash1)
{
	wxCriticalSectionLocker locker(*crit_);

	HashValue answer;
	for (int x = 0; x < hashes_.size();x++)
	{
		if (MYlte(hashes_[x].hash,hash1))
		{
			answer = hashes_[x];
			hashes_.erase(hashes_.begin()+x);
			break;
		}
	}
	return answer;
}

HashValue
HashTable::PullBetween(wxString hash1,wxString hash2)
{
	wxCriticalSectionLocker locker(*crit_);

	HashValue answer;
	for (int x = 0; x < hashes_.size();x++)
	{
		if (MYlte(hashes_[x].hash,hash1) ||
			MYgt(hashes_[x].hash,hash2))
		{
			answer = hashes_[x];
			hashes_.erase(hashes_.begin()+x);
			break;
		}
	}
	return answer;
}
*/
void
HashTable::Purge()
{
	wxCriticalSectionLocker locker(*crit_);

	for (int x = 0; x < hashes_.size();x++)
	{
		for (int y = x+1; y < hashes_.size();y++)
		{
			if (hashes_[x] == hashes_[y])
			{
				hashes_.erase(hashes_.begin()+y);
				y=x+1;
			}
		}
	}
}

HashTable::~HashTable()
{
	delete crit_;
}



