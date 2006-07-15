#include "simutils.h"
#include "../../src/crypto.h"

#include <wx/timer.h>

wxString
GenerateRandomHash(int seed)
{
	wxString data = wxString().Format("%s%d%s%d%d%d%d%d",
						crypto::RandomString().c_str(),
						seed,
						crypto::RandomString().c_str(),
						wxGetLocalTime(),
						seed,
						GetRandomNumber(seed),
						GetRandomNumber(1000000000),
						GetRandomNumber(1000000000));
	return crypto::Sha1Hash(data);
}

/*
Random number generator from http://remus.rutgers.edu/~rhoads/Code/random.c
*/

/* linear congruential generator.  Generator x[n+1] = a * x[n] mod m */

#define RAND_INT(l,h) (((int)(Random() * ((double)(h)-(l)+1))) + (l))

static unsigned int SEED = 93186752;

double Random ()
{
/* The following parameters are recommended settings based on research
   uncomment the one you want. */


   static unsigned int a = 1588635695, m = 4294967291U, q = 2, r = 1117695901;
/* static unsigned int a = 1223106847, m = 4294967291U, q = 3, r = 625646750;*/
/* static unsigned int a = 279470273, m = 4294967291U, q = 15, r = 102913196;*/
/* static unsigned int a = 1583458089, m = 2147483647, q = 1, r = 564025558; */
/* static unsigned int a = 784588716, m = 2147483647, q = 2, r = 578306215;  */
/* static unsigned int a = 16807, m = 2147483647, q = 127773, r = 2836;      */
/* static unsigned int a = 950706376, m = 2147483647, q = 2, r = 246070895;  */

   SEED = a*(SEED % q) - r*(SEED / q);
   return ((double)SEED / (double)m);
 }


void RandSeed (unsigned int init)   {if (init != 0) SEED = init;}

int
GetRandomNumber(int max)
{
	return RAND_INT(0,max);
}
