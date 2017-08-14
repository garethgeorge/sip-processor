/*  Copyright 1985 Gary Perlman */

/*LINTLIBRARY*/
#ifndef lint
static char sccsfid[] = "@(#) readlines.c 5.2 (|stat) 12/1/86";
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifdef __STDC__
#include "stdlib.h"
#else
extern	char	*malloc ();
#endif

#define	NOSPACE  (-1)

int
readlines (svecptr, maxlines, ioptr)
char	***svecptr;          /* string vector pointer */
unsigned	maxlines;        /* max lines to read */
FILE	*ioptr;              /* where to read lines from */
	{
	int 	nlines;          /* number of lines read */
	char	**lptr;          /* temporary svec space */
	char	linebuf[BUFSIZ]; /* individual lines */
	unsigned len;            /* input line length */

	lptr = (char **) malloc (maxlines * sizeof (char *));
	*svecptr = lptr;
	if (lptr == NULL)
		return (NOSPACE);

	for (nlines = 0; fgets (linebuf, sizeof (linebuf), ioptr); nlines++)
		{
		if (nlines == maxlines)         /* already full of lines */
			return (maxlines + 1);
		len = strlen (linebuf);
		if (linebuf[len-1] == '\n')     /* remove trailing newline if there */
			linebuf[--len] = '\0';
		if ((lptr[nlines] = malloc (len+1)) == NULL)
			return (NOSPACE);
		strcpy (lptr[nlines], linebuf);
		}
	return (nlines);
	}
