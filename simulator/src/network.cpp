#include "network.h"
#include "node.h"
#include "simutils.h"
#include "statistics.h"

#include "../../src/utils.h"

#include <wx/string.h>

using namespace std;

vector<Node*> Network::nodes;

bool Network::useCacheFinger = false;
bool Network::useExponentialFinger = false;

void
Network::FailNode()
{
	// pick a random index and kill that node
}

void
Network::TransferHashes(Node* first, Node* second)
{
	for (unsigned int x = 0; x >= 0 && x < first->hashes.size();x++)
	{
		if (MYlte(first->hashes[x],second->nid))
		{
			second->hashes.push_back(first->hashes[x]);
			first->hashes.erase(first->hashes.begin()+x);
			x--;
		}
	}
}

void
Network::AddNode(int max)
{
	// generate a nid for the new node
	wxString nid = GenerateRandomHash(nodes.size()+GetRandomNumber(nodes.size()+1));

	for (unsigned int x = 0; x < nodes.size();x++)
	{
		if (nodes[x]->nid == nid)
		{
			DEBUG_PRINT("tried to add a duplicate nid to the network")
			return;
		}
	}

	//DEBUG_PRINT("adding new node " << nid)

	// create the node
	Node* node = new Node(nid);

	if (nodes.size() == 0)
	{
		/*
		this is a hack
		force the first node on the network to be the smallest nid
		*/
		nid = "0000000000000000000000000000000000000000";
		node->nid = nid;
		node->next = nid;
	}
	else if (nodes.size() == 1)
	{
		/*
		this is a hack
		force the last node to be the highest
		*/
		nid = "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ";
		node->nid = nid;

		// stabalize
		node->next = nodes[0]->nid;
		node->previous = nodes[0]->nid;
		nodes[0]->previous = nid;
		nodes[0]->next = nid;

		TransferHashes(nodes[0],node);
	}
	else
	{
		// pick a random bootstrap node
		//int id = GetRandomNumber(nodes.size()-1);
		wxString successor = nodes[0]->Lookup(nid);

		if (successor == "")
		{
			DEBUG_PRINT("failed to lookup successor for new node")
			delete node;
			return;
		}

		// stabalize
		node->next = successor;
		node->previous = NidToNode(successor)->previous;
		NidToNode(NidToNode(successor)->previous)->next = nid;
		NidToNode(successor)->previous = nid;

		TransferHashes(NidToNode(successor),node);
	}

	// add the node to the network
	nodes.push_back(node);

	// assign the node some random hashes
	node->Init(max);

	// refresh finger tables of all nodes
	for (unsigned int x = 0; x < nodes.size();x++)
		nodes[x]->RefreshFinger();

	for (unsigned int z = 0; z < nodes.size();z++)
	{
		for (unsigned int x = 0; x < nodes.size();x++)
		{
			nodes[z]->AddNodupToFinger(nodes[x]->nid);
		}
	}

	// distribute it's hashes
	node->Distribute();

}


Node*
Network::NidToNode(const wxString& nid)
{
	for (unsigned int x = 0; x < nodes.size();x++)
	{
		if (nodes[x]->nid == nid)
		{
			return nodes[x];
		}
	}

	DEBUG_PRINT("could not convert a nid to a node:" << nid)

	return 0;
}




