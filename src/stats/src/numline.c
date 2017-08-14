/*  Copyright 1986 Gary Perlman */

#include "stat.h"
FUN(numline,print min and max values,5.0,1/13/86)

numline (minx, maxx, padwidth)
double	minx;
double	maxx;
int 	padwidth;  /* amount of padding needed */
	{
	char	buf[BUFSIZ];

	(void) sprintf (buf, "%-.3f", minx);
	padwidth -= strlen (buf);
	(void) sprintf (buf, "%.3f", maxx);
	padwidth -= strlen (buf);

	printf ("%-.3f", minx);
	while (padwidth-- > 0)
		putchar (' ');
	printf ("%.3f", maxx);
	putchar ('\n');
	}
