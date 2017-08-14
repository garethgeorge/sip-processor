/*  Copyright 1986 Gary Perlman */

/* $link: linkml linex readlines+checkio+number+getopt+specol */

#include "stat.h"
PGM (linex,Line Extraction,5.0,12/01/86)
#define	MAXREQS    1000       /* max number number of line requests */
#define MAXCHARS BUFSIZ       /* maximum number of chars in lines */

Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */
Boole	Donumber = FALSE;     /* number output lines */
Boole	Silent = FALSE;       /* be silent about missing lines */

int 	Request[MAXREQS];     /* requested lines */
int 	Maxrequest;           /* maximum requested line */
int 	Nrequest = 0;         /* number of requested lines */
char	**Input;              /* input store */
int 	Nlines;               /* number of lines in input */


main (argc, argv) char **argv;
	{
	int 	optind;      /* local optind returned by initial() */
	char	*cs;         /* current lines specification */
	int 	reqno;       /* the request number */
	int 	reqline;     /* the requested line */
	char	*specerr (); /* get colspec error string */

	optind = initial (argc, argv);

	while (optind < argc)
		{
		cs = argv[optind++];
		Nrequest = specol (cs, Request, NULL, Nrequest, MAXREQS, MAXINT);
		if (Nrequest > MAXREQS)
			ERRMANY (line requests, MAXREQS)
		if (Nrequest < 0)
			{
			fprintf (stderr, "%s: %s\n", Argv0, specerr (Nrequest));
			exit (1);
			}
		}

	if (Nrequest == 0)
		ERRMSG0 (No line numbers were supplied)

	for (reqno = 0; reqno < Nrequest; reqno++)
		if (Request[reqno] > Maxrequest)
			Maxrequest = Request[reqno];

	checkstdin ();

	/*BUG? may have problems with broken pipe */
	Nlines = readlines (&Input, Maxrequest, stdin);
	if (Nlines < 0)
		ERRSPACE (lines)

	for (reqno = 0; reqno < Nrequest; reqno++)
		{
		reqline = Request[reqno];
		if (reqline <= Nlines)
			{
			if (Donumber)
				printf ("%7d ", reqline);
			puts (Input[reqline-1]);
			}
		else if (!Silent)
			fprintf (stderr, "%s: missing line %d\n", Argv0, reqline);
		}

	exit (0);
	}

/*FUNCTION initial: returns local optind */
int
initial (argc, argv)
int 	argc;
char	**argv;
	{
	extern	int 	optind;
	extern	char	*optarg;
	Boole	errflg = 0;
	int 	c;

	ARGV0;

	while ((c = getopt (argc, argv, "nsLOV")) != EOF)
		switch (c)
			{
			default:  errflg = TRUE;       break;
			case 'O': InfoOptions = TRUE;  break;
			case 'V': InfoVersion = TRUE;  break;
			case 'L': InfoLimits = TRUE;   break;
			case 'n': Donumber = TRUE;     break;
			case 's': Silent = TRUE;       break;
			}

	if (errflg)
		USAGE ("[-ns] line-numbers")

	usinfo ();

	return (optind);
	}


usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXREQS,  "maximum number of requested lines");
		const (MAXCHARS, "maximum number of characters in lines");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('n', "number lines", Donumber);
		lopt ('s', "be silent about missing lines", Silent);
		oper ("lines", "lines to extract", "");
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
