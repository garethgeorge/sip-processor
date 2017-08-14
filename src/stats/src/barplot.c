/*  Copyright 1982 Gary Perlman */

/*LINTLIBRARY*/
#include "stat.h"
FUN(barplot,horizontal histogram,5.0,1985)

#define	EPSILON (.000000001)
#define scale(x,minx,maxx,width) ((int) (width*((x)-(minx))/((maxx)-(minx)+EPSILON)))

meanplot (mean, sd, minx, maxx, width)
double	mean;
double	sd;
double	minx;
double	maxx;
	{
	char	line[BUFSIZ];
	int	ii;
	for (ii = 0; ii < width; ii++)
		line[ii] = '-';
	line[width] = '\0';
	line[scale (mean, minx, maxx, width)] = '|';
	line[scale (mean+sd, minx, maxx, width)] = '+';
	line[scale (mean-sd, minx, maxx, width)] = '+';
	puts (line);
	}

barplot (vec, n, style, axes, donum, width, base, interval)
float	*vec;
double	base;
double	interval;
	{
	int	elt;
	double	sum = 0.0;
	double	ss  = 0.0;
	double	minx = *vec;
	double	maxx = *vec;
	double	mean;
	double	sd;
	int	w;
	int	midwidth;
	for (elt = 0; elt < n; elt++)
		{
		sum += vec[elt];
		ss  += vec[elt] * vec[elt];
		if (vec[elt] < minx) minx = vec[elt];
		if (vec[elt] > maxx) maxx = vec[elt];
		}
	mean = sum/n;
	sd = n <= 1 ? 0.0 : sqrt ((ss-sum*sum/n)/(n-1));
	midwidth = scale (mean, minx, maxx, width);
	if (axes)
		{
		nlabel (-1, donum, base, interval);
		meanplot (mean, sd, minx, maxx, width);
		}
	for (elt = 0; elt < n; elt++)
		{
		w = scale (vec[elt], minx, maxx, width);
		nlabel (elt, donum, base, interval);
		barline (w, midwidth, style);
		}
	if (axes)
		{
		nlabel (-1, donum, base, interval);
		meanplot (mean, sd, minx, maxx, width);
		nlabel (-1, donum, base, interval);
		numline (minx, maxx, width);
		}
	}

barline (n, mid, style)
	{
	switch (style)
		{
		case 2:
			if (n < mid)
				{
				repeat ('-', n);
				}
			else if (n == mid)
				{
				repeat ('-', n);
				repeat ('+', 1);
				}
			else
				{
				repeat ('-', mid);
				repeat ('|', 1);
				repeat ('-', n-mid);
				}
			break;
		default:
			if (n < mid)
				{
				repeat (' ', n);
				repeat ('-', mid-n);
				repeat ('|', 1);
				}
			else if (n == mid)
				{
				repeat (' ', n);
				repeat ('+', 1);
				}
			else
				{
				repeat (' ', mid);
				repeat ('|', 1);
				repeat ('-', n-mid);
				}
			break;
		}
		putchar ('\n');
	}

repeat (c, n)
	{
	while (n-- > 0)
		putchar (c);
	}

nlabel (n, donum, base, interval)
int 	n;        /* label number */
int 	donum;    /* should numbering be done? */
double	base;     /* the base label to be numbered */
double	interval; /* the size of the interval between labels */
	{
	if (!donum) return;
	if ((n >= 0) && ((n%donum) == 0))
		printf ("%12.4f	", n*interval + base);
	else
		printf ("%12s	", "");
	}
