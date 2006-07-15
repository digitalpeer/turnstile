#ifndef YAWPSIM_STATISTICS
#define YAWPSIM_STATISTICS

#include <vector>

class Statistics
{
public:
	static int badLookups;
	static int goodLookups;
	static int foundLookups;
	static std::vector<int> hopCount;

	static void Reset(int maxNodes);
	static double AverageHops();
	static void AddLookup(int hops);

	static void Finalize();

	static int MeanHops();
	static unsigned int TotalHops();

};

#endif
