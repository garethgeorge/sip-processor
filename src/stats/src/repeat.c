/*  Copyright 1982 Gary Perlman */

#include "stat.h"
PGM(repeat,Repeat a File or String,5.1,10/13/86)

#define MAXCHARS BUFSIZ           /* maximum number of chars in lines */

int 	Maxlines = 1000;
int 	Count = 1;            /* number of times to repeat */
Boole	Formfeeds = FALSE;    /* begin copies with ^L */
Boole	Inside = FALSE;       /* repeat lines within files */
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */

main (argc, argv) char **argv;
	{
	int 	optind;
	int 	repeat ();
	
	ARGV0;
	optind = initial (argc, argv);
	exit (filter (argc, argv, optind, repeat));
	}

repeat (name, ioptr)
char	*name;
FILE	*ioptr;
	{
	int 	nlines;
	char	**lptr;
	register	int 	i;
	int 	repcount;

	nlines = readlines (&lptr, Maxlines, ioptr);
	if (nlines > Maxlines)
		{
		fprintf (stderr, "%s: problem repeating file '%s'\n", Argv0, name);
		ERRMANY (input lines, Maxlines);
		}
	if (nlines < 0)
		ERRSPACE (lines)

	if (Inside)
		{
		for (i = 0; i < nlines; i++)
			for (repcount = 0; repcount < Count; repcount++)
				puts (lptr[i]);
		}
	else
		{
		for (repcount = 0; repcount < Count; repcount++)
			{
			if (Formfeeds)
				putchar ('\f');
			for (i = 0; i < nlines; i++)
				puts (lptr[i]);
			}
		}
	for (i = 0; i < nlines; i++)
		free (lptr[i]);
	free (lptr);
	return (SUCCESS);
	}

/*FUNCTION initial: returns local version of optind, index to first operand */
int
initial (argc, argv) char **argv;
	{
	extern	char *optarg;    /* option value accessed through this by getopt */
	extern	int  optind;     /* will be index to first operand */
	int 	opterr = 0;      /* count of number of errors */
	int 	flag;            /* option flag characters read in here */
	char	*optstring =     /* getopt string to be filled in */
		"fil:n:LOV";
	char	*usage =         /* part of usage summary to match optstring */
		"[-fi] [-l maxlines] [-n count] [-] [files]";

	while ((flag = getopt (argc, argv, optstring)) != EOF)
		switch (flag)
			{
			default:
				opterr++;
				break;
			/* put option cases here */
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			case 'f':
				Formfeeds = TRUE;
				break;
			case 'i':
				Inside = TRUE;
				break;
			case 'l':
				if (setint (Argv0, flag, optarg, &Maxlines, 1, MAXINT))
					opterr++;
				break;
			case 'n':
				if (setint (Argv0, flag, optarg, &Count, 0, MAXINT))
					opterr++;
				break;
			}

	if (opterr) /* print usage message and bail out */
		{
		fprintf (stderr, "Usage: %s %s\n", ARGV0, usage);
		exit (1);
		}
	
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
		const (Maxlines, "maximum number of lines");
		const (MAXCHARS, "maximum number of characters in lines");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('f',          "insert formfeeds before each copy", Formfeeds);
		lopt ('i',          "repeat lines inside files", Inside);
		iopt ('l', "lines", "maximum number of input lines", Maxlines);
		iopt ('n', "count", "number of times to repeat input files", Count);
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
