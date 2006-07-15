#include "statistics.h"

#include <algorithm>
using namespace std;

int Statistics::badLookups = 0;
int Statistics::goodLookups = 0;
int Statistics::foundLookups = 0;
std::vector<int> Statistics::hopCount;

void
Statistics::Reset(int maxNodes)
{
	badLookups = 0;
	goodLookups = 0;
	foundLookups = 0;
	hopCount.clear();
}

unsigned int
Statistics::TotalHops()
{
	unsigned int total = 0;
	for (unsigned int x = 0; x < hopCount.size();x++)
		total += hopCount[x];

	return total;
}

double
Statistics::AverageHops()
{
	return (double)TotalHops()/(double)(goodLookups + foundLookups);
}

void
Statistics::Finalize()
{
	sort(hopCount.begin(),hopCount.end());
}

int
Statistics::MeanHops()
{
	return hopCount[(hopCount.size()/2)-1];
}

void
Statistics::AddLookup(int hops)
{
	hopCount.push_back(hops);
}
