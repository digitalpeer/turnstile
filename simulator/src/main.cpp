
#include "node.h"
#include "statistics.h"
#include "simutils.h"
#include "network.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <unistd.h>
#include <signal.h>

#include <wx/string.h>
#include <wx/timer.h>

using namespace std;

static bool doAbort = false;

void SigHandler(int signum)
{
	doAbort = true;

	// re-activate for the fun of it
	signal(SIGINT, SIG_DFL);
}

void
PrintLookupSummary(int latency)
{
	Statistics::Finalize();

	if (Statistics::hopCount.size() == 0)
	{
		DEBUG_PRINT("no successful lookups have been performed to generate stats")
		return;
	}

	cout << "Max lookup time................: " << latency*Statistics::TotalHops() << "ms" << endl;
	cout << "Min lookup time................: " << latency*Statistics::hopCount[0] << "ms" << endl;
	cout << "Average lookup time............: " << (double)(latency*Statistics::TotalHops())/(double)Statistics::TotalHops() << "ms" << endl;
	cout << "Total hops.....................: " << Statistics::TotalHops() << endl;
	cout << "Max hops during lookup.........: " << Statistics::hopCount[Statistics::hopCount.size()-1] << endl;
	cout << "Min hops during lookup.........: " << Statistics::hopCount[0] << endl;
	cout << "Average hops during lookup.....: " << Statistics::AverageHops() << endl;
	cout << "Mean hops during lookup........: " << Statistics::MeanHops() << endl;
	cout << "Total lookups..................: " << Statistics::goodLookups + Statistics::foundLookups << endl;
	cout << "Number of successful lookups...: " << Statistics::goodLookups << endl;
	cout << "Number of found lookups........: " << Statistics::foundLookups << endl;
	cout << "Number of failed lookups.......: " << Statistics::badLookups << " *" << endl;
	cout << "* These are not included in other stats" << endl;
}

int
main(int argc, char** argv)
{
	RandSeed(wxGetLocalTime());

	if (argc != 2)
	{
		cerr << "Usage: " << argv[0] << " [input_file]" << endl;
		return -1;
	}

	const char* INPUT = argv[1];

	ifstream in(INPUT);
	if (!in)
	{
		cerr << "Failed to load input file." << endl;
		return -1;
	}

	Network network;

	int initialNodes;
	int latency;
	int maxHashes;
	char exponentialFinger;
	char cacheFinger;
	int failInterval;
	int addInterval;

	cout << "Loading configuration ..." << endl;

	in >> initialNodes;
	in >> latency;
	in >> maxHashes;
	in >> exponentialFinger;
	in >> cacheFinger;
	in >> failInterval;
	in >> addInterval;

	if (cacheFinger == '1')
		Network::useCacheFinger = true;

	if (exponentialFinger == '1')
		Network::useExponentialFinger = true;

	// close the input file
	in.close();

	cout << "Initial nodes.............: " << initialNodes << endl;
	cout << "Network latency...........: " << latency << endl;
	cout << "Maximum hashes per node...: " << maxHashes << endl;
	cout << "Use exponential finger....: " << exponentialFinger << endl;
	cout << "Use catch finger..........: " << cacheFinger << endl;
	cout << endl;

	cout << "Initializing ..." << endl << endl;

	// create initial network
	for (int x = 0; x < initialNodes;x++)
	{
		Network::AddNode(maxHashes);
	}

	int choice = -1;

	while (choice != 9)
	{
		cout << "-- Menu --" << endl;
		cout << setw(4) << "1." << "Lookup Hash" << endl;
		cout << setw(4) << "2." << "Lookup Random Hashes" << endl;
		cout << setw(4) << "3." << "Fail Node" << endl;
		cout << setw(4) << "4." << "Fail Concurrent Nodes" << endl;
		cout << setw(4) << "5." << "Add Concurrent Nodes" << endl;
		cout << setw(4) << "6." << "Add Nodes" << endl;
		cout << setw(4) << "7." << "Print Network Stats" << endl;
		cout << setw(4) << "8." << "Print Network" << endl;
		cout << setw(4) << "9." << "Quit" << endl;
		cout << "Choice:" << flush;

		cin >> choice;

		cout << endl << endl;

		switch (choice)
		{
		case 1:	// lookup hash
		{
			cout << "NOT IMPLIMENTED" << endl;
			break;
		}
		case 2:	// lookup random hash
		{
			int howManyHashes = 0;
			cout << "How many random hashes do you want to lookup: " << flush;
			cin >> howManyHashes;

			int howManyNodes = 0;
			cout << "How many random nodes do you want to do the lookups from: " << flush;
			cin >> howManyNodes;

			Statistics::Reset(initialNodes);

			cout << endl << "Performing lookup of " << howManyHashes << " random hashes from " << howManyNodes << " random nodes ..." << endl << endl;

			/*
			go through the number of nodes we want to try
			pick a random node to do the lookup from
			lookup a random hash (the hash isn't expected to actually be on the network
			*/
			for (int x = 0; x < howManyNodes;x++)
			{
				// lookup all a different set of random hashes from this node
				for (int y = 0; y < howManyHashes;y++)
				{
					// generate a unique random hash to lookup
					wxString hash  = GenerateRandomHash(x+y);

					// generate random hash
					Network::nodes[x]->Lookup(hash);
				}
			}

			PrintLookupSummary(latency);

			break;
		}
		case 3:	// fail node
		{
			cout << "NOT IMPLIMENTED" << endl;
			break;
		}
		case 4:	// fail concurrent nodes
		{
			cout << "NOT IMPLIMENTED" << endl;
			break;
		}
		case 5:	// add concurrent nodes
		{
			cout << "NOT IMPLIMENTED" << endl;
			break;
		}
		case 6:	// add nodes
		{
			Network::AddNode(maxHashes);
			break;
		}
		case 7:	// print network details
		{
			cout << "Number of nodes: " << Network::nodes.size() << endl;

			unsigned int totalHashes = 0;
			unsigned int maxHashes = 0;
			unsigned int minHashes = 0;
			for (unsigned int x = 0; x < Network::nodes.size();x++)
			{
				totalHashes += Network::nodes[x]->hashes.size();
				if (Network::nodes[x]->hashes.size() > maxHashes)
					maxHashes = Network::nodes[x]->hashes.size();
				if (Network::nodes[x]->hashes.size() < minHashes)
					minHashes = Network::nodes[x]->hashes.size();
			}

			cout << "Number of hashes in network........: " << totalHashes << endl;
			cout << "Maximum hashes per node............: " << maxHashes << endl;
			cout << "Minimum hashes per node............: " << minHashes << endl;
			cout << "Average number of hashes per node..: " << totalHashes/Network::nodes.size() << endl;

			break;
		}
		case 8:
		{
			for (unsigned int x = 0; x < Network::nodes.size();x++)
			{
				cout << Network::nodes[x]->nid << endl;
				cout << "   next: " << Network::nodes[x]->next << endl;
				cout << "   prev: " << Network::nodes[x]->previous << endl;

				for (unsigned int y = 0; y < Network::nodes[x]->finger.size();y++)
				{
					cout << "      finger: " << Network::nodes[x]->finger[y] << endl;
				}

				for (unsigned int y = 0; y < Network::nodes[x]->hashes.size();y++)
				{
					cout << "         hash: " << Network::nodes[x]->hashes[y] << endl;
				}


			}
			break;
		}
		case 9:	// quit
			break;
		default:
			cout << "Unknown menu option" << endl;
		}

		cout << endl << endl;
	}
}
