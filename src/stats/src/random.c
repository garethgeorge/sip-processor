/*  Copyright 1982 Gary Perlman */

/*LINTLIBRARY*/
#include "stat.h"
#ifdef __TURBOC__
#include <process.h>
#endif
FUN(random,random number initialization,5.3,04/14/93)

double	Maxrand;
#define	MAX32 2147483648.0
#define	MAX16 32768.0
#ifdef hpux
#define	MAXRAND MAX16
#else
#define MAXRAND (sizeof (int) == 4 ? MAX32 : MAX16)
#endif

initrand (seed)
int 	seed;
	{
	long 	clock;
	if (seed == 0)
		{
		time (&clock);
		seed = clock + getpid ();
		}
	srand (seed);
#ifdef RAND_MAX /* might be defined in stdlib.h */
	Maxrand = (double) RAND_MAX + 1.0;
#else
	Maxrand = MAXRAND;
#endif
	}
