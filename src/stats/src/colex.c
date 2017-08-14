/*  Copyright 1984 Gary Perlman */

/*
	colex extracts columns from its input and prints them
	there are options to specify the printing format of columns
	which are specified in a form convenient for users:
		i integer      (d)
		n numerical    (f)
		a alphabetic   (s)
		e exponential  (g)
	these are converted to the printf format strings in parens.
*/

#include "stat.h"
PGM (colex,Column Extraction/Formatting,5.3,9/23/90)
/*HISTORY
	9/25/90	added 'L' option which disappeared somehow
	9/23/90	added -c option to support fixed character position columns 
	8/11/86	version 5.2
*/
#define	MAXCOLS     100           /* max number of input fields / requests */
#define MAXCHARS BUFSIZ           /* max number of chars in lines */
char	*NAstring = "NA";         /* string to print if Not Available */

Boole	Forcefill = FALSE;    /* empty fields are filled with NA */
Boole	Quotestrings = FALSE; /* strings are quoted */
Boole	Charcolumn = FALSE;   /* use character columns instead of fields */
Boole	Ignore = FALSE;       /* missing strings are ignored */
Boole	Dotab = TRUE;         /* put tab after every column */
Boole	Debug = FALSE;        /* print debugging information */
Boole	Validate = FALSE;     /* validate data type in columns */
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */

int 	Lineno = 0;           /* input line number */

char	*Format[MAXCOLS];     /* printf formats for columns */
char	*Dformat = "a";       /* default output format */
int 	Request[MAXCOLS];     /* requested columns */
int 	Nrequest = 0;         /* number of requested columns */
char 	Type[MAXCOLS];        /* data types = a e i n */

/* type specifiers */
#define	T_ALPHA     'a'
#define	T_INTEGER   'i'
#define	T_NUMBER    'n'
#define	T_EXPONENT  'e'

/* format specifiers */
#define	F_ALPHA     's'
#define	F_INTEGER   'd'
#define	F_NUMBER    'f'
#define	F_EXPONENT  'g'

#define	NTYPES       4


main (argc, argv) char **argv;
	{
	char	line[MAXCHARS];     /* input line */
	char	*input[MAXCOLS];    /* pointers to fields in input line */
	int 	ncols;              /* number of columns in input line */
	int 	reqno;              /* request number */
	int 	colno;              /* column number in line */
	int 	optind;             /* local optind returned by initial() */
	char	*cs;                /* current column specification */
	int 	errcount = 0;       /* count of errors in parsing requests */
	char	*specerr ();        /* get column specification error message */

	optind = initial (argc, argv);

	while (optind < argc)
		{
		cs = argv[optind++];
		Nrequest = specol (cs, Request, Format, Nrequest, MAXCOLS, MAXCOLS);
		if (Nrequest > MAXCOLS)
			ERRMANY (column Requests, MAXCOLS)
		if (Nrequest < 0)
			{
			fprintf (stderr, "%s: problem with column specification: '%s', %s\n",
				Argv0, cs, specerr (Nrequest));
			errcount++;
			}
		}

	if (Nrequest == 0)
		ERRMSG0 (No column numbers were supplied)
	if (errcount > 0)
		ERRMSG1 (%d illegal column specifications were detected, errcount)

	settypes ();

	checkstdin ();

	while (gets (line))
		{
		Lineno++;
		if (Charcolumn)
			ncols = strlen (line);
		else
			if ((ncols = parselin (line, input, MAXCOLS)) > MAXCOLS)
				ERRMANY (columns in input line, MAXCOLS)
		for (reqno = 0; reqno < Nrequest; reqno++)
			{
			colno = Request[reqno];
			if (colno <= ncols)
				if (Charcolumn)
					putchar (line[colno-1]);
				else
					printstring (input[colno-1], Type[reqno], Format[reqno]);
			else if (Forcefill)
				if (Charcolumn)
					putchar (' ');
				else
					printstring (NAstring, T_ALPHA, "s");
			else if (!Ignore)
				ERRMSG2 (missing column %d in short input line %d, colno, Lineno)
			else if (Quotestrings) /* not possible with Charcolumn */
				printstring ("", T_ALPHA, "s");
			if (Dotab)
				putchar ('\t');
			}
			
		putchar ('\n');
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

	while ((c = getopt (argc, argv, "cqiftvF:DLOV")) != EOF)
		switch (c)
			{
			default:  errflg = TRUE;       break;
			case 'c': Charcolumn = TRUE;   break;
			case 'i': Ignore = TRUE;       break;
			case 'f': Forcefill = TRUE;    break;
			case 'q': Quotestrings = TRUE; break;
			case 't': Dotab = FALSE;       break;
			case 'v': Validate = TRUE;     break;
			case 'F':
				Dformat = optarg;
				break;
			case 'D': Debug = TRUE;        break;
			case 'O': InfoOptions = TRUE;  break;
			case 'L': InfoLimits = TRUE;   break;
			case 'V': InfoVersion = TRUE;  break;
			}

	if (errflg)
		USAGE ("[-cfiqt] [-F format] column-numbers")

	if (Charcolumn)
		{
		Validate = FALSE;
		Quotestrings = FALSE;
		}

	usinfo ();

	return (optind);
	}


printstring (string, type, fmt)
char	*string;
int 	type;
char	*fmt;
	{
	char	qchar = '"';
	double	dtmp;
	int 	itmp;
	char	printbuf[128];

	switch (type)
		{
		case T_ALPHA:
			if (Quotestrings)
				{
				if (strchr (string, qchar))
					qchar = '\'';
				putchar (qchar);
				}
			sprintf (printbuf, "%c%s", '%', fmt);
			printf (printbuf, string);
			if (Quotestrings)
				putchar (qchar);
			break;
		case T_INTEGER:
			if (Validate && number (string) != 1)
				fprintf (stderr, "%s: %s on line %d is not an integer\n",
					Argv0, string, Lineno);
			itmp = atoi (string);
			sprintf (printbuf, "%c%s", '%', fmt);
			printf (printbuf, itmp);
			break;
		case T_NUMBER:
		case T_EXPONENT:
			if (Validate && !number (string))
				fprintf (stderr, "%s: %s on line %d is not a number\n",
					Argv0, string, Lineno);
			dtmp = atof (string);
			sprintf (printbuf, "%c%s", '%', fmt);
			printf (printbuf, dtmp);
			break;
		}
	}

/*FUNCTION settypes: set up (user supplied) type and format information */
Status
settypes ()
	{
	int 	colno;
	char	*ptr;
	for (colno = 0; colno < Nrequest; colno++)
		{
		if (*Format[colno] == '\0')
			strcpy (Format[colno], Dformat);
		for (ptr = Format[colno]; *ptr && !isalpha (*ptr); ptr++)
			continue;
		switch (*ptr)
			{
			default:
				ERRMSG1 (unknown data type specification: %s, Format[colno])
				/*NOTREACHED*/
				break;
			case T_ALPHA:
			case F_ALPHA:
				Type[colno] = T_ALPHA;
				*ptr++ = F_ALPHA;
				*ptr = '\0';
				break;
			case T_INTEGER:
			case F_INTEGER:
				Type[colno] = T_INTEGER;
				*ptr++ = F_INTEGER;
				*ptr = '\0';
				break;
			case T_NUMBER:
			case F_NUMBER:
				Type[colno] = T_NUMBER;
				*ptr++ = F_NUMBER;
				*ptr = '\0';
				break;
			case T_EXPONENT:
			case F_EXPONENT:
				Type[colno] = T_EXPONENT;
				*ptr++ = F_EXPONENT;
				*ptr = '\0';
				break;
			}

		if (Format[colno][0] == '%')
			Format[colno]++;

		if (Debug)
			fprintf (stderr, "%3d %3d %c %s\n",
				colno+1, Request[colno], Type[colno], Format[colno]);
		}
	return (SUCCESS);
	}


usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXCOLS, "maximum number of columns/fields");
		const (MAXCHARS, "maximum number of characters in input lines");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('c', "use character columns instead of fields", Charcolumn);
		lopt ('f', "force-fill columns with NA", Forcefill);
		sopt ('F', "format", "default column output format (aine)", Dformat);
		lopt ('i', "ignore missing columns", Ignore);
		lopt ('q', "quote output columns", Quotestrings);
		lopt ('t', "place tab after every column", Dotab);
		lopt ('v', "validate data types in columns", Validate);
		oper ("columns", "columns to extract", "");
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
