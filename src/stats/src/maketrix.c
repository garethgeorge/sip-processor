/*  Copyright 1982 Gary Perlman */

#include "stat.h"
PGM(maketrix,Make A Matrix Format File,5.0,2/27/85)

#define MAXCHARS 100           /* maximum number of chars in input fields */

main (argc, argv) char **argv;
	{
	ARGV0;
	initial (argc, argv);
	checkstdin ();
	maketrix (stdin);
	exit (0);
	}

/* OPTIONS */
int 	Ncols = 2;
int 	Silent = 0;
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */

initial (argc, argv) char **argv;
	{
	extern	char	*optarg;
	extern	int 	optind;
	int 	errflg = 0;
	int 	C;
	while ((C = getopt (argc, argv, "sLOV")) != EOF)
		switch (C)
			{
			case 's': Silent = 1; break;
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			default: errflg++; break;
			}
	if (optind < argc)
		{
		if (optind < argc-1) /* too many args; some will be ignored */
			errflg++;
		if (!number (argv[optind]))
			ERRNUM (argv[optind],number of columns);
		Ncols = atoi (argv[optind]);
		if (Ncols < 1)
			ERRMSG0 (Number of columns must be positive)
		}
	if (errflg)
		USAGE ("[-s] [ncols]")
	usinfo ();
	}

maketrix (ioptr)
FILE	*ioptr;
	{
	char	string[MAXCHARS];
	int 	nstrings = 0;
	while (getword (string, ioptr))
		{
		fputs (string, stdout);
		if (++nstrings == Ncols)
			{
			putchar ('\n');
			nstrings = 0;
			}
		else
			putchar ('\t');
		}
	if (nstrings != 0)
		{
		putchar ('\n');
		if (!Silent)
			WARNING (last line does not have expected number of columns)
		}
	}

usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXCHARS, "maximum number of characters in input words");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('s', "be silent about uneven last line", Silent);
		oper ("ncols", "number of columns of output", "2");
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
