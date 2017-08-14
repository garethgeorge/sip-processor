/* Copyright 1980 Gary Perlman */

/*LINTLIBRARY*/
#include "stat.h"
FUN(centile,compute values given percentile rank,5.0,12/24/86)

/*returns the perc percentile of sorted v[n]*/
double
centile (perc, v, n)
int 	perc;
float	*v;
int 	n;
	{
	double	findex;     /* floating index to the perc centile */
	int 	pindex;
	
	findex = perc * .01 * n - 0.5;
	if (findex < 0.0)
		findex = 0.0;
	else if (findex > (n-1.0))
		findex = n - 1.0;
	pindex = (int) findex;
	return (v[pindex+1] * (findex - pindex) +
		v[pindex] * (1.0 - findex + pindex));
	}
