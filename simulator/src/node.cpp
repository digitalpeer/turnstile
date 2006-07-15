#include "node.h"
#include "simutils.h"
#include "network.h"
#include "statistics.h"
#include "../../src/utils.h"

#include <algorithm>
using namespace std;

Node::Node(const wxString& nid_)
	: crash(false)/*,id(++idCount)*/
{
	nid = nid_;
}

void
Node::Init(int maxHashes)
{
	// generate random hashes for this node
	GenerateRandomHashes(maxHashes);
}

void
Node::DoCrash()
{
	//
}

/*
"found"
"successor"
hash to jump to
*/
wxString
Node::Query(const wxString& hash)
{
	for (unsigned int x = 0; x < hashes.size();x++)
	{
		if (hashes[x] == hash)
		{
			return "found";
		}
	}

	if (MYgt(hash,nid))
		return BestStart(hash);

	return "successor";
}

wxString
Node::BestStart(const wxString& hash)
{
	/*
	this is an optimization/bug fix that is not in turnstile!
	*/
	if (IsSuccessor(nid, previous,hash))
	{
		return nid;
	}

	wxString current;
	if (MYgt(nid,next) || finger.size() == 0)
		current = next;
	else
	{
		current = finger[0];/*"0000000000000000000000000000000000000000";*/
		for (unsigned int x = 1; x < finger.size();x++)
		{
			if (MYgt(hash,finger[x]))
				break;

			//if (MYgt(finger[x],next))
				current = finger[x];
		}
	}


	//DEBUG_PRINT("beststart() returning " << current << " for hash " << hash)

	return current;
//#endif
//	return next;
}

void
Node::AddNodupToFinger(const wxString& nid_)
{
	if (nid_ == "")
	{
		DEBUG_PRINT("tried to add a blank node to finger!")
		return;
	}
	else if (nid == nid_)
	{
		return;
	}

	bool found = false;
	for (unsigned int x = 0; x < finger.size() && !found;x++)
	{
		if (finger[x] == nid_)
			found = true;
	}

	if (!found)
	{
		finger.push_back(nid_);
	}

	sort(finger.begin(),finger.end());
}

wxString
Node::Lookup(const wxString& hash)
{
	wxString successor;
	int hops = 0;

	wxString current = "0000000000000000000000000000000000000000";

	wxString start = current;
	bool end = false;
	while (!end)
	{
		hops++;

		Node* node = Network::NidToNode(current);
		wxString response = node->Query(hash);

		//DEBUG_PRINT("lookup response: " << response)

		// if we are using a catch finger then add this node to it
		if (Network::useCacheFinger)
		{
			AddNodupToFinger(current);
		}

		if (response == "found")
		{
			Statistics::foundLookups++;
			end = true;
			successor = current;
		}
		else if (response == "successor")
		{
			Statistics::goodLookups++;
			end = true;
			successor = current;
		}
		else
		{
			current = response;
		}

		/*if (current == start && !end)
		{
			DEBUG_PRINT("looped around circle during lookup for " << hash)
			Statistics::badLookups++;

			// don't use the stats for this lookup!
			return "";
		}*/

		if (hops > Network::nodes.size()*2)
		{
			DEBUG_PRINT("looped around circle TWICE during lookup for " << hash)
			Statistics::badLookups++;

			// don't use the stats for this lookup!
			return "";
		}
	}

	// update stats
	Statistics::AddLookup(hops);

	return successor;
}

void
Node::Crash()
{
	crash = true;
}

bool
Node::IsSuccessor(wxString nid_, wxString prev, wxString hash)
{
	bool answer = ((MYlte(hash,nid_) && MYgt(hash,prev) && prev != "")
			||
			(MYgt(hash,nid_) && MYgt(hash,prev) && MYgt(prev,nid_) && prev != "")
			||
			(MYlte(hash,nid_) && MYgt(prev,nid_) && prev != "")
			||
			prev == ""
			); // if we have no prev we are seed then ALWAYS successor

	return answer;
}

void
Node::GenerateRandomHashes(int max)
{
	// pick a random number of hashes between the
	int number = GetRandomNumber(max);

	// generate that many random hashes and add them to the local table
	for (int x = 0; x < number;x++)
	{
		// generate the random hash
		wxString hash = GenerateRandomHash(x);

		// generate random hash
		hashes.push_back(hash);
	}
}

void
Node::RefreshFinger()
{
	if (Network::useExponentialFinger)
	{
		finger.clear();

		// add in nodes an exponential distance away
	}
}

void
Node::Distribute()
{
	for (unsigned int x = 0; x >=0 && x < hashes.size();x++)
	{
		// generate the random hash
		wxString successor = Lookup(hashes[x]);

		if (successor == "")
		{
			DEBUG_PRINT("failed to lookup successor when distributing")
		}
		else
		{
			// if it's me then just keep it and move on
			if (successor != nid)
			{
				Network::NidToNode(successor)->hashes.push_back(hashes[x]);
				hashes.erase(hashes.begin()+x);
				x--;
			}
		}
	}
}

Node::~Node()
{;}



