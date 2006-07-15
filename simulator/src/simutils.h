#ifndef YAWPSIM_UTILS
#define YAWPSIM_UTILS

#include <wx/string.h>

#ifdef DEBUG
#include <iostream>
#define DEBUG_PRINT(text) \
	std::cout << "Debug: " << __FILE__ << ":" << __LINE__; \
	std::cout << " " << text << std::endl;
#else
#define DEBUG_PRINT(text)
#endif

wxString GenerateRandomHash(int seed);

int GetRandomNumber(int max);

double Random (void);
void RandSeed (unsigned int);

#endif
