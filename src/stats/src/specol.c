/*  Copyright 1986 Gary Perlman */

#include <stdio.h>
#include <ctype.h>
#ifdef macintosh
#include <stdlib.h>
#endif

#define	Status	int
#define	SUCCESS 0
#define	FAILURE	1

/*LINTLIBRARY*/
#ifndef lint
static char sccsfid[] = "@(#) specol.c 5.1 (|stat) 8/11/86";
#endif

#define	RANGECHAR   '-'      /* ranges are of form M-N */
#define	OUT_OF_RANGE        (-1)
#define	TOO_MANY_COLUMNS    (-3)
#define	TOO_MUCH_INFO       (-4)
#define	ILLEGAL_RANGE       (-5)

char *
specerr (value_returned)
int 	value_returned;
	{
	switch (value_returned)
		{
		case OUT_OF_RANGE:
			return ("range value out of allowed range");
		case TOO_MANY_COLUMNS:
			return ("range value exceeds maximum");
		case TOO_MUCH_INFO:
			return ("extra information before range specification");
		case ILLEGAL_RANGE:
		default:
			return ("illegal range specification");
		}
	}

/*
	specify columns of the form M-N
	optionally preceded by a letter-ended info string
*/
int
specol (operand, request, info, nrequests, maxrequest, maxcols)
char	*operand;      /* input string */
int 	*request;      /* vector of requested columns */
char	**info;        /* info string for request[i] */
int 	nrequests;     /* number of requests so far */
int 	maxrequest;    /* max number of requests */
int 	maxcols;       /* maximum number of columns to read in */
	{
	int 	colno;
	char	*range;        /* a range request */
	int 	first, last;   /* extremes of ranges */

	for (range = operand; *range && !isalpha (*range); range++)
		continue;
	if (isalpha (*range))
		while (isalpha (*range))
			range++;
	else /* no special info on this operand */
		range = operand;
	if (getnums (range, &first, &last) == FAILURE)
		return (ILLEGAL_RANGE);
	if (first > maxcols || last > maxcols)
		return (TOO_MANY_COLUMNS);
	*range = '\0';
	for (colno = first;
		first <= last ? colno <= last : colno >= last;
		colno += first <= last ? 1 : -1)
		{
		if (nrequests < 0) /* already some bad parsing */
			return (nrequests);
		if (nrequests == maxrequest)
			return (maxrequest+1);
		if (colno <= 0)
			return (OUT_OF_RANGE);
		request[nrequests] = colno;
		if (info != NULL)
			info[nrequests] = operand;
		else if (*operand) /* illegal info string */
			return (TOO_MUCH_INFO);
		nrequests++;
		}
	return (nrequests);
	}

Status
getnums (s, first, last)
char	*s;        /* string of the form #[-#] */
int 	*first;    /* stuff first column number in here */
int 	*last;     /* stuff last column number in here */
	{
	char	*ptr;
	for (ptr = s; *ptr && *ptr != RANGECHAR; ptr++)
		continue;
	if (*ptr == RANGECHAR) /* range of columns */
		{
		*ptr++ = '\0';
		if (number (s) != 1)
			return (FAILURE);
		if (number (ptr) != 1)
			return (FAILURE);
		*first = atoi (s);
		*last = atoi (ptr);
		}
	else if (number (s) != 1)
		return (FAILURE);
	else
		*last = *first = atoi (s);
	if (*first < 1 || *last < 1)
		return (FAILURE);
	return (SUCCESS);
	}

#ifdef SPECOL

#define	MAXCOL 100

main (argc, argv) char **argv;
	{
	int 	request[MAXCOL];
	int 	nrequests = 0;
	char	*info[MAXCOL];
	int 	i;
	for (i = 1; i < argc; i++)
		{
		nrequests = specol (argv[i], request, info, nrequests, MAXCOL);
		printf ("%d requests\n", nrequests);
		}
	for (i = 0; i < nrequests; i++)
		{
		printf ("%4d %4d '%s'\n", i, request[i], info[i]);
		}
	}

#endif SPECOL
