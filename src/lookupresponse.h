/* $Id: lookupresponse.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file lookupresponse.h
@brief Lookup Response Definition
*/
#ifndef TURNSTILE_LOOKUPRESPONSE_H
#define TURNSTILE_LOOKUPRESPONSE_H

#include "dhtutils.h"

#include <wx/string.h>

#include <vector>

class Node;

/** @class LookupResponse
This is an object that gets passed around which holds the results of
a lookup on the network.  It's type defines the results.
*/
class LookupResponse
{
public:
	enum	{
			JUMP,		// try this node
			FOUND,		// found the successor and found what you were looking for
			SUCCESSOR,	// didn't find what you were looking for, but here's the successor
			FAIL		// lookup failed, unstable network
		};

	LookupResponse();
	LookupResponse(int type,Node successor);
	LookupResponse(int type);

	void AddResponse(HashValue value);

	int type_;
	Node successor;
	std::vector<HashValue> responses_;

private:

};

#endif

