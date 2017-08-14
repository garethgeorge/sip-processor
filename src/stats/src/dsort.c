/*  Copyright 1986 Gary Perlman */

/*
	things to add:
		option to ignore case in alpha comparisons
		way to deal with NA
*/
#include "stat.h"
PGM(dsort,Sort Data Lines by Multiple Key Columns,5.0,6/12/86)

#define	MAXCOLS     100
#define	MAXKEYS      10          /* sort by at most this many keys */
#define MAXCHARS BUFSIZ          /* maximum number of chars in lines */
int 	Nlines = 0;            /* number of data lines */
int 	Ncols = 0;             /* number of data columns/fields per line */
char	***Matrix;             /* will be malloc'd after all lines read */
char	*Info[MAXKEYS];        /* data types of each column to sort */
int 	Key[MAXKEYS];          /* sort by these columns */
int 	Nkeys = 0;             /* sort by this many keys */
Boole	Reverse[MAXKEYS];      /* should sorting order be reversed? */
Boole	Nocase[MAXKEYS];       /* should upper/lower case be ignored? */
int 	Maxcol;                /* max col to use as sort key */

#define	INFO_STRING  "a"
#define	INFO_INT     "i"
#define	INFO_EXP     "e"
#define	INFO_NUM     "n"
char	*getinfo ();           /* return data type of each column */
char	*parseinfo ();         /* return parsed info for a type spec */

unsigned	Maxlines = 100000;
Boole	Intsort;
Boole	Numsort;
Boole	Expsort;
Boole	Alphasort;
Boole	Revall;
Boole	Caseignore;


Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */
Boole	Debug;                /* secret option */


int
linecmp (a1, a2)
char	***a1, ***a2;
	{
	register	int 	keyno;
	register	int 	diff;
	register	char	*s1, *s2;
	int 	numcmp ();
	int 	sortcol;
	double	ddiff;

	for (diff = keyno = 0; keyno < Nkeys && diff == 0; keyno++)
		{
		sortcol = Key[keyno] - 1;
		s1 = a1[0][sortcol];
		s2 = a2[0][sortcol];
		switch (Info[keyno][0])
			{
			case 'i': /* int compare */
				diff = (atoi (s1) - atoi (s2));
				break;
			case 'n': /* numerical */
				diff = numcmp (s1, s2);
				break;
			case 'e': /* exponential notation compare */
				ddiff = atof (s1) - atof (s2);
				if (ddiff < 0.0)
					diff = (-1);
				else if (ddiff > 0.0)
					diff = 1;
				else
					diff = 0;
				break;
			default: /* assume single char type spec */
			case 'a': /* alpha compare */
				if (Nocase[keyno])
					diff = cistrcmp (s1, s2);
				else
					diff = strcmp (s1, s2);
				break;
			}
		if (diff)
			{
			if (Reverse[keyno])
				diff = (-diff);
			return (diff);
			}
		}
	return (0);
	}


main (argc, argv) char **argv;
	{
	int 	lineno;
	int 	keyno;
	int 	result;           /* result from readmatrix */
	char	*errmatrix ();    /* get error message from readmatrix */

	ARGV0;

	initial (argc, argv);

	checkstdin ();

	if (result = readmatrix (&Matrix, &Nlines, &Ncols, Maxlines, MAXCOLS))
		{
		bellmsg ();
		fprintf (stderr, "%s: %s\n", Argv0, errmatrix (result));
		exit (1);
		}

	if (Ncols == 0 || Nlines == 0)
		exit (0);

	if (Nkeys == 0)
		{
		for (Nkeys = 0; Nkeys < Ncols && Nkeys < MAXKEYS; Nkeys++)
			Key[Nkeys] = Nkeys+1;
		}

	for (keyno = 0; keyno < Nkeys; keyno++)
		{
		Info[keyno] = parseinfo (Info[keyno], keyno);
		if (Info[keyno] == NULL || Info[keyno][0] == '\0')
			Info[keyno] = getinfo (Key[keyno]-1);
		if (Debug)
			fprintf (stderr, "Info[%d] set to '%s'\n", keyno+1, Info[keyno]);
		}

	qsort ((char *) Matrix, Nlines, sizeof (*Matrix), linecmp);

	for (lineno = 0; lineno < Nlines; lineno++)
		printline (Matrix[lineno]);

	exit (SUCCESS);
	}


int
initial (argc, argv) char **argv;
	{
	extern	char *optarg;    /* option value accessed through this by getopt */
	extern	int  optind;     /* will be index to first operand */
	int 	opterr = 0;      /* count of number of errors */
	int 	flag;            /* option flag characters read in here */
	int 	col;

	while ((flag = getopt (argc, argv, "aceil:nrDLOV")) != EOF)
		switch (flag)
			{
			default:
				opterr++;
				break;
			/* put option cases here */
			case 'a':
				Alphasort = TRUE;
				Expsort = Intsort = Numsort = FALSE;
				for (col = 0; col < MAXKEYS; col++)
					Info[col] = INFO_STRING;
				break;
			case 'c':
				Caseignore = TRUE;
				for (col = 0; col < MAXKEYS; col++)
					Nocase[col] = TRUE;
				break;
			case 'e':
				Expsort = TRUE;
				Alphasort = Intsort = Numsort = FALSE;
				for (col = 0; col < MAXKEYS; col++)
					Info[col] = INFO_EXP;
				break;
			case 'i':
				Intsort = TRUE;
				Alphasort = Expsort = Numsort = FALSE;
				for (col = 0; col < MAXKEYS; col++)
					Info[col] = INFO_INT;
				break;
			case 'l':
				if (setint (Argv0, 'l', optarg, &Maxlines, 1, MAXINT))
					opterr++;
				break;
			case 'n':
				Numsort = TRUE;
				Intsort = Expsort = Numsort = FALSE;
				for (col = 0; col < MAXKEYS; col++)
					Info[col] = INFO_NUM;
				break;
			case 'r':
				Revall = TRUE;
				for (col = 0; col < MAXKEYS; col++)
					Reverse[col] = TRUE;
				break;
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			case 'D': Debug = TRUE; break;
			}

	if (opterr) /* print usage message and bail out */
		USAGE ("[-aceinr] [-l lines] [[type][column-range]] ...")

	usinfo ();

	while (optind < argc)
		{
		Nkeys = specol (argv[optind], Key, Info, Nkeys, MAXKEYS, MAXCOLS);
		if (Nkeys < 0)
			ERRMSG0 (illegal sorting key format)
		if (Nkeys > MAXKEYS)
			ERRMANY (sorting keys, MAXKEYS)
		optind++;
		}
	
	for (col = 0; col < Nkeys; col++)
		if (Key[col] > Maxcol)
			Maxcol = Key[col]; /* will compare Maxcol to Ncols */
	}



printline (matline)
char	**matline;
	{
	int 	col;
	char	*ptr;
	for (ptr = matline[0]; *ptr; ptr++)
		putchar (*ptr);
	for (col = 1; col < Ncols; col++)
		{
		putchar ('\t');
		for (ptr = matline[col]; *ptr; ptr++)
			putchar (*ptr);
		}
	putchar ('\n');
	}


usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXCOLS,  "maximum number of columns");
		const (MAXKEYS,  "maximum number of sorting keys");
		const (MAXCHARS, "maximum number of characters in lines");
		const (Maxlines, "maximum number of input lines");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('a', "order fields alphabetically", Alphasort);
		lopt ('c', "ignore case in alphabetical comparisons", Caseignore);
		lopt ('e', "order fields as xxx.yyyEzzz numbers", Expsort);
		lopt ('i', "order fields as integers", Intsort);
		iopt ('l', "lines", "maximum number of input lines", Maxlines);
		lopt ('n', "order fields as xxx.yyy numbers", Numsort);
		lopt ('r', "reverse sorting order of lines", Revall);
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (SUCCESS);
	}


char *
parseinfo (infoptr, keyno)
char	*infoptr;
int 	keyno;
	{
	char	*type = NULL;
	if (infoptr == NULL)
		return (infoptr);
	while (*infoptr)
		{
		switch (*infoptr)
			{
			case 'I': case 'i': /* integer */
				type = INFO_INT;
				break;
			case 'N': case 'n': /* numerical */
				type = INFO_NUM;
				break;
			case 'E': case 'e': /* exponential notation */
				type = INFO_EXP;
				break;
			case 'C': case 'c': /* case insensitive */
				Nocase[keyno] = TRUE;
				/*FALLTHROUGH*/
			case 'A': case 'a': /* alphabetical */
				type = INFO_STRING;
				break;
			case 'R': case 'r': /* reverse */
				Reverse[keyno] = TRUE;
				break;
			default:
				/* IGNORE */
				break;
			}
		infoptr++;
		}
	return (type);
	}

char *
getinfo (column)
int 	column;
	{
	int 	lineno;
	char 	*type = NULL;
	for (lineno = 0; lineno < Nlines; lineno++)
		{
		switch (number (Matrix[lineno][column]))
			{
			case 0: /* not a number, use alpha sort */
				return (INFO_STRING);
			case 1: /* integer */
				/* wait and hope for such an easy sort */
				break;
			case 2: /* xxx.yyy number, use at least numcmp if all numbers */
				if (type == NULL)
					type = INFO_NUM; /* sort will be at least this hard */
				break;
			case 3: /* exp notation, use slow atof to compare */
				type = INFO_EXP;   /* will have to go all the way */
				break;
			default:
				ERRMSG0 (|STAT type checking has a bug)
			}
		}
	if (type == NULL)
		type = INFO_INT;
	return (type);
	}
