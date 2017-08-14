/* Copyright 1992 Gary Perlman */

/*LINTLIBRARY*/
#include "stat.h"
FUN(errplot,display error bar plot,5.0,08/22/92)

#define	EPSILON (.000000001)
#define scale(x,minx,maxx) ((int) (width*((x)-(minx))/((maxx)-(minx)+EPSILON)))

#define	PLOT_MINCHAR      '<'   /* char for the minimum value */
#define	PLOT_MAXCHAR      '>'   /* char for the maximum value */
#define	PLOT_MEANCHAR     '#'   /* char for the measure of centrality */
#define	PLOT_MINSE        '('   /* char for low standard error */
#define	PLOT_MAXSE        ')'   /* char for high standard error */
#define	PLOT_STANDEV      '-'   /* char for range of one deviation */

char *
errplot (width, globmin, globmax, minval, maxval, n, center, deviation, seflag)
int 	width;                /* width of the plot */
double	globmin, globmax;     /* global min and max values */
double	minval, maxval;       /* condition min and max values */
int 	n;                    /* number of cases */
double	center, deviation;    /* e.g., mean and sd */
int 	seflag;               /* should standard error be shown? */
	{
	static	char	line[BUFSIZ];
	int 	i;
	int 	low, high;          /* range values in graph buffer */
	int 	minlow, maxhigh;    /* scaled versions of condition's extrema */
	double	se;
	
	for (i = 0; i < width; i++)
		line[i] = ' ';
	line[width] = '\0';

	minlow = scale (minval, globmin, globmax);
	maxhigh = scale (maxval, globmin, globmax);

	/* one standard deviation above and below the mean */
	low = scale (center-deviation, globmin, globmax);
	high = scale (center+deviation, globmin, globmax);
	if (low < minlow)
		low = minlow;
	if (high > maxhigh)
		high = maxhigh;
	for (i = low; i <= high; i++)
		line[i] = PLOT_STANDEV;

	/* one standard error around the mean */
	if (seflag && (n > 0))
		{
		se = deviation / sqrt ((double) n);
		low = scale (center - se, globmin, globmax);
		high = scale (center + se, globmin, globmax);
		if (low < minlow)
			low = minlow;
		if (high > maxhigh)
			high = maxhigh;
		line[low] = PLOT_MINSE;
		line[high] = PLOT_MAXSE;
		}
	
	/* write in minimum and maximum, then mean */
	line[minlow] = PLOT_MINCHAR;
	line[maxhigh] = PLOT_MAXCHAR;
	i = scale (center, globmin, globmax);
	line[i] = PLOT_MEANCHAR;
	return (line);
	}


#ifdef ERRPLOT

main (argc, argv)
char	**argv;
	{
	int 	w = 30;
	double	gmin = 0.0;
	double	gmax = 10.0;
	char	*s;
	if (argc > 1)
		if ((w = atoi (argv[1])) < 5)
			w = 5;

	s = errplot (w, gmin, gmax, 3.0, 8.0, 12, 5.5, 1.0, 1);
	printf ("|%s|\n", s);
	s = errplot (w, gmin, gmax, 3.0, 8.0, 100, 5.5, 1.0, 1);
	printf ("|%s|\n", s);
	s = errplot (w, gmin, gmax, 0.0, 10.0, 44, 8.9, 3.5, 1);
	printf ("|%s|\n", s);
	s = errplot (w, gmin, gmax, 4.0, 6.0, 222, 5.0, 0.5, 1);
	printf ("|%s|\n", s);
	s = errplot (w, gmin, gmax, 4.0, 6.0, 2, 5.0, 0.5, 0);
	printf ("|%s|\n", s);
	/* errplot (w, gmin, gmax, minval, maxval, n, center, deviation, 1); */
	}

#endif
