/*  Copyright 1980 Gary Perlman */

/* use colspec to assign types to columns */

#include "stat.h"
PGM(validata,Data Validation and Consistency Checking,5.1,02/03/87)

/*
	validata looks at a data file to see what sort of columns it has.
	It complains if there are not an equal number of columns
	for each line in the input.  At the end, it prints the type
	of each column depending on what types of data appeared in them.
*/

#define	strint(s)    (number(s)==1)
#define	strfloat(s)  (number(s))

#define	MAXCOLS		100
#define MAXCHARS BUFSIZ           /* maximum number of chars in lines */
int 	Nalnum[MAXCOLS];
int 	Nalpha[MAXCOLS];
int 	Nint[MAXCOLS];
int 	Nfloat[MAXCOLS];
int 	Nother[MAXCOLS];
int 	Ntotal[MAXCOLS];
int 	NNA[MAXCOLS];             /* number of NA missing fields */
double	Minvalue[MAXCOLS];
double	Maxvalue[MAXCOLS];
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */

char	*T_int   = "int";
char	*T_float = "float";
char	*T_alpha = "alpha";
char	*T_alnum = "alnum";
char	*T_other = "other";
int 	Maxcols = 0;
int 	Linecount = 0;

main (argc, argv)
int 	argc;
char	**argv;
	{
	ARGV0;
	initial (argc, argv);
	checkstdin ();
	readdata ();
	report ();
	
	exit (0);
	}

readdata ()
	{
	char	line[MAXCHARS];
	char	*col[MAXCOLS];
	Boole	isother;
	int 	ncols;
	int 	colno;
	int 	colcount = (-1);
	double	value;
	char	*s;
		
	while (gets (line))
		{
		Linecount++;
		ncols = parselin (line, col, MAXCOLS);
		if (ncols > MAXCOLS)
			ERRMANY (columns, MAXCOLS)
		if (ncols > Maxcols)
			Maxcols = ncols;
		if (ncols != colcount)
			{
			if (Linecount != 1)
				fprintf (stderr, "%s: Variable number of columns at line %d\n",
					Argv0, Linecount);
			colcount = ncols;
			}
		if (ncols == 0)
			fprintf (stderr, "%s: Line %d is empty\n", Argv0, Linecount);
	
		isother = TRUE;
		for (colno = 0; colno < ncols; colno++)
			{
			s = col[colno];
			Ntotal[colno]++;
			if (isna (s))
				{
				NNA[colno]++;
				continue;
				}
			if (stralnum (s))
				{
				isother = FALSE;
				Nalnum[colno]++;
				if (stralpha (s))
					Nalpha[colno]++;
				}
			if (strfloat (s))
				{
				isother = FALSE;
				Nfloat[colno]++;
				if (strint (s))
					Nint[colno]++;
				value = atof (s);
				if (Nfloat[colno] == 1)
					{
					Maxvalue[colno] = value;
					Minvalue[colno] = value;
					}
				if (value > Maxvalue[colno])
					Maxvalue[colno] = value;
				else if (value < Minvalue[colno])
					Minvalue[colno] = value;
				}
			if (isother)
				Nother[colno]++;
			else
				isother = TRUE;
			}
		}
	}

report ()
	{
	char	*shortstr = "%3s ";
	char	*longstr  = "%5s ";
	char	*shortint = "%3d ";
	char	*longint  = "%5d ";
	int 	colno;
		
	if (isatty (1))
		printf ("%s: %d lines read\n", Argv0, Linecount);
	
#define	vprint(var, fmt) printf (fmt, var)
		vprint ("Col",    shortstr);
		vprint ("N",      shortstr);
		vprint ("NA",     shortstr);
		vprint (T_alnum,  longstr);
		vprint (T_alpha,  longstr);
		vprint (T_int,    longstr);
		vprint (T_float,  longstr);
		vprint (T_other,  longstr);
		vprint ("type",   longstr);
		vprint ("min",    longstr);
		vprint ("max",    longstr);
		putchar ('\n');
	
	for (colno = 0; colno < Maxcols; colno++)
		{
		vprint (colno+1,        shortint);
		vprint (Ntotal[colno],  shortint);
		vprint (NNA[colno],     shortint);
		vprint (Nalnum[colno],  longint);
		vprint (Nalpha[colno],  longint);
		vprint (Nint[colno],    longint);
		vprint (Nfloat[colno],  longint);
		vprint (Nother[colno],  longint);
		if (Nint[colno] + NNA[colno] == Ntotal[colno])
			vprint (T_int, longstr);
		else if (Nfloat[colno] + NNA[colno] == Ntotal[colno])
			vprint (T_float, longstr);
		else if (Nalnum[colno] + NNA[colno] == Ntotal[colno])
			vprint (T_alnum, longstr);
		else
			vprint (T_other, longstr);
		vprint (Minvalue[colno],"%5g " );
		vprint (Maxvalue[colno],"%5g ");
		putchar ('\n');
		}
	}

stralnum (s) char *s;
	{
	while (isspace (*s))
		s++;
	while (*s)
		if (!isalnum (*s))
			return (0);
		else
			s++;
	return (1);
	}

stralpha (s) char *s;
	{
	while (isspace (*s))
		s++;
	while (*s)
		if (!isalpha (*s))
			return (0);
		else
			s++;
	return (1);
	}

/*FUNCTION initial: returns local version of optind, index to first operand */
int
initial (argc, argv) char **argv;
	{
	extern	char *optarg;    /* option value accessed through this by getopt */
	extern	int  optind;     /* will be index to first operand */
	int 	opterr = 0;      /* count of number of errors */
	int 	flag;            /* option flag characters read in here */

	while ((flag = getopt (argc, argv, "LOV")) != EOF)
		switch (flag)
			{
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			default:
				opterr++;
				break;
			}

	if (opterr) /* print usage message and bail out */
		USAGE ("-LOV")

	usinfo ();

	ERROPT (optind);

	return (optind);
	}

usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXCOLS, "maximum number of input columns");
		const (MAXCHARS, "maximum number of characters in lines");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
