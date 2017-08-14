/*  Copyright 1982 Gary Perlman */

/*LINTLIBRARY*/
#include "stat.h"
FUN(cor,correlation coefficient,5.0,1985)

#ifdef __STDC__
double cor (float *x, float *y, int n);
#endif

double          /* returns the correlation of n-long vectors x and y */
cor (x, y, n)
float	*x;     /* input vector may be null, use linear progression */
float	*y;
int 	n;
	{
	double sumx = 0.0;
	double sumy = 0.0;
	double ssx = 0.0;
	double ssy = 0.0;
	double sumxy = 0.0;
	double denom, r;
	extern double sqrt ();
	double	xval, yval;
	int	i;
	for (i = 0; i < n; i++)
		{
		if (x)
			xval = *x++;
		else
			xval = (double) i;
		yval = *y++;
		sumx += xval;
		sumy += yval;
		ssx  += xval * xval;
		ssy  += yval * yval;
		sumxy+= xval * yval;
		}
	if ((denom = (n*ssx-sumx*sumx)*(n*ssy-sumy*sumy)) < FZERO)
		return (0.0);
	r = (n*sumxy-sumx*sumy)/sqrt(denom);
	return (r);
	}
