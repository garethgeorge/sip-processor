/*  Copyright 1980 Gary Perlman */

/*LINTLIBRARY*/
#include <ctype.h>
static char sccsfid[] = "@(#) strings.c 5.1 (|stat) 6/15/85";

/* strings reads from ioptr into abase, an array of most maxstrings strings,
   at most maxstrings strings, each of length at most maxchars-1 chars.
   It returns the number of strings read in, or maxstrings + 1 if some
   information is discarded.
*/
#ifndef ESCAPE
#define	ESCAPE '\\'
#endif

sstrings (line, abase, maxstrings, maxchars)
char	*line;
char	*abase;
	{
	int	nstrings = 0;
	int	nchars;
	while (isspace (*line)) line++;
	while (*line)
		{
		nchars = 0;
		while (*line && !isspace (*line) && nchars<maxchars-1)
			if ((abase[nchars++] = *line++) == ESCAPE)
				abase[nchars-1] = *line++;
		abase[nchars] = '\0';
		abase += maxchars;
		while (*line && !isspace (*line)) line++;
		while (isspace (*line)) line++;
		if (++nstrings == maxstrings)
			return (maxstrings + (*line ? 1 : 0));
		}
	return (nstrings);
	}

#ifdef STRINGS
#include <stdio.h>
#define	MAX 100
#define	MAXLEN 32
main ()
	{
	int 	ncols;
	char	line[BUFSIZ];
	char	array[MAX][MAXLEN];
	while (gets (line))
		{
		ncols = sstrings (line, array, MAX, MAXLEN);
		printf ("%d	%s ... %s\n", ncols, array[0], array[ncols-1]);
		}
	exit (0);
	}
#endif
