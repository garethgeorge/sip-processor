/* Copyright 1986 Gary Perlman */

#include "stat.h"

/*LINTLIBRARY*/
FUN(fiveplot,Plot Ordinal Statistics Fivenums,5.0,12/24/86)

#define	EPSILON (.000000001)
#define scale(x,minx,maxx,width) \
	((int) ((width)*((x)-(minx))/((maxx)-(minx)+EPSILON)))
#define	addch(line,i,c) (line[(i)] = (c))

#define	PLOT_MINCHAR      '<'
#define	PLOT_MAXCHAR      '>'
#define	PLOT_MEDCHAR      '#'
#define	PLOT_Q1Q3         '-'      /* quartile 1 to quartile 3 */

char *
fiveplot (xmin, q1, median, q3, xmax, width, minval, maxval)
double	xmin;    /* min */
double	q1;      /* quartile 1 */
double	median;  /* middle point */
double	q3;      /* quartile 3 */
double	xmax;    /* max */
int 	width;   /* width of plot */
double	minval;  /* global minimum */
double	maxval;  /* global maximum */
	{
	static	char	line[BUFSIZ];
	int 	i;
	int 	low, high;          /* range values in graph buffer */
	int 	minlow, maxhigh;    /* scaled versions of extrema */

	for (i = 0; i < width; i++)
		line[i] = ' ';
	line[width] = '\0';

	minlow = scale (xmin, minval, maxval, width);
	maxhigh = scale (xmax, minval, maxval, width);

	/* inter quartile range */
	low = scale (q1, minval, maxval, width);
	high = scale (q3, minval, maxval, width);
	if (low < minlow)
		low = minlow;
	if (high > maxhigh)
		high = maxhigh;
	for (i = low; i <= high; i++)
		addch (line, i, PLOT_Q1Q3);

	/* write in minimum and maximum, then median */
	addch (line, minlow, PLOT_MINCHAR);
	addch (line, maxhigh, PLOT_MAXCHAR);
	i = scale (median, minval, maxval, width);
	addch (line, i, PLOT_MEDCHAR);
	return (line);
	}
