#ifndef YAWPSIM_NETWORK
#define YAWPSIM_NETWORK

#include <vector>

class Node;
class wxString;

class Network
{
public:
	/** remove a node from the network imporperly. */
	static void FailNode();

	/** Add a node to the circle. */
	static void AddNode(int max);

	/** Get a node with an nid. */
	static Node* NidToNode(const wxString& nid);

	static void TransferHashes(Node* first, Node* second);

	/** This is my array of nodes on the network.
	It does not depict the order of nodes in the circle. */
	static std::vector<Node*> nodes;

	static bool useCacheFinger;
	static bool useExponentialFinger;
};

#endif
