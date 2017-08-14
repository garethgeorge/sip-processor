#include "stat.h"
PGM(features,Tabulate Features of Items,3.1,02/12/90)
/*HISTORY
	03/10/89	3.0	initial version
	02/12/90	3.1	incorporated stat.h
*/

/*
	Special codes such as Consumer Reports advantages and disadvantages
		can be done by:
			Item   Advantages=A,B,C  Disadvantages=a,h
		and then inserting the meanings of the codes at the end of the table.
	Tabs can be inserted between columns with the -c option.
*/

Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */

#ifdef __STDC__
int printheader (void);
void getfeatures (int argc, char **argv, int optind);
void initial (int argc, char **argv);
int substring (char *s1, char *s2);
int begins (char *s1, char *s2);
int printtable (char *filename);
void printoptions (int argc, char **argv);
void printfield (int fieldno, char *contents);
#else
int printheader ();
void getfeatures ();
void initial ();
int substring ();
int begins ();
int printtable ();
void printoptions ();
void printfield ();
#endif

#define	MAXFEAT    100           /* max number of features */
char	*HasFeature[MAXFEAT];    /* string to indicate having feature */
int 	Featwidth[MAXFEAT];      /* width of each feature column */
char	**Feature;               /* names of features, from cmd line */
int 	Nfeatures;               /* number of features */

char	*Infile = "-";           /* input file name ("-" -> stdin) */

int 	Itemwidth = 15;          /* width of leading field */
int 	Width = 1;               /* width of features */
int 	Pagesize  = 60;          /* lines per page */
char	*Coldelim = "|";         /* used to delimit feature columns */
char	Leader    = '_';         /* leader from end of leading field */
char	*Featyes   = "#";        /* code for having a feature */
char	*Featno    = " ";        /* code for non-membership */
#define	ASSIGN      '='          /* assigned code */
int 	Horizontal = 0;          /* use a one line horizontal header */

main (argc, argv) char **argv;
	{
	extern	int 	optind;
	
	ARGV0;
	initial (argc, argv);
	getfeatures (argc, argv, optind);
	if (printtable (Infile))
		fprintf (stderr, "%s: Can't read from file: '%s'\n", argv[0], Infile);
	exit (0);
	}

/*FUNCTION getfeatures: get features from the command line or file */
void
getfeatures (argc, argv, optind) char **argv;
	{
	char	*ptr;
	Feature = argv + optind;
	Nfeatures = argc - optind;
	
	/* check for width parameters for features */
	for (optind = 0; optind < Nfeatures; optind++)
		{
		if (ptr = strchr (Feature[optind], ASSIGN))
			{
			if (!isinteger (ptr+1))
				ERRNUM(ptr+1,individual column width)
			Featwidth[optind] = atoi (ptr+1);
			*ptr = '\0';
			}
		}
	}

/*FUNCTION initial: initialize program by setting options */
void
initial (argc, argv) char **argv;
	{
	extern	int 	optind;
	extern	char	*optarg;
	char	*optstring = "LOVhc:f:i:l:n:p:w:y:";
	int 	c;
	int 	opterr = 0;
	while ((c = getopt (argc, argv, optstring)) != EOF)
		{
		switch (c)
			{
			case '?':
				opterr++;
				break;
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			case 'c': /* column delimiter */
				Coldelim = optarg;
				break;
			case 'f': /* input file */
				Infile = optarg;
				break;
			case 'h': /* horizontal header */
				Horizontal = 1;
				break;
			case 'i': /* item width */
				if (setint (Argv0, c, optarg, &Itemwidth, 1, 100))
					opterr++;
				break;
			case 'l': /* leader character */
				if (*optarg)
					Leader = *optarg;
				break;
			case 'n': /* feature "no" indicator */
				Featno = optarg;
				break;
			case 'p': /* page size */
				if (setint (Argv0, c, optarg, &Pagesize, 10, 200))
					opterr++;
				break;
			case 'w': /* feature column width */
				if (setint (Argv0, c, optarg, &Width, 1, 100))
					opterr++;
				break;
			case 'y': /* yes string */
				Featyes = optarg;
				break;
			}
		}
	if (opterr)
		USAGE ("[-h] [-c col-delim] [-f inputfile] [-i itemwidth]\n\t[-l leader] [-n no-str] [-p pagesize] [-w feat-width] [-y yes-str]")
	usinfo ();
	}

/*FUNCTION getfirst: returns the first field on a line */
char *
getfirst (line)
char	*line;
	{
	static char buffer[100];
	char	*ptr = buffer;
	while (isspace (*line))
		line++;
	while (*line && !isspace (*line))
		*ptr++ = *line++;
	*ptr = '\0';
	if (ptr == buffer)
		return (NULL);
	return (buffer);
	}

/*FUNCTION getrest: returns the rest of a line after the first field */
char *
getrest (line)
char	*line;
	{
	while (isspace (*line))
		line++;
	while (*line && !isspace (*line))
		line++;
	return (line);
	}

/*FUNCTION substring: true if first string is a substring of second */
int
substring (s1, s2)
char	*s1, *s2;
	{
	if (s1 == NULL || *s1 == '\0')
		return (1);
	if (s2 == NULL || *s2 == '\0')
		return (0);
	while (*s2)
		if (begins (s1, s2))
			return (1);
		else
			s2++;
	return (0);
	}

/*FUNCTION begins: true if first string begins the second */
int
begins (s1, s2)
char	*s1, *s2;
	{
	if (s1 == NULL)
		return (1);
	if (s2 == NULL)
		return (0);
	while (*s1)
		{
#		define	makelower(c)  (isupper(c)?tolower(c):(c))
		if (*s1 != *s2 && makelower (*s1) != makelower (*s2))
			return (0);
		s1++;
		s2++;
		}
	return (1);
	}

/*FUNCTION printtable: print the contents of a feature table */
int
printtable (filename)
char	*filename;
	{
	int 	i;
	FILE	*ioptr;
	int 	lineno = Pagesize;     /* will trigger new header */
	char	*item;                 /* the first thing on the line */
	char	*lineptr, *featptr;
	char	line[BUFSIZ];          /* the input line */
	char	*itemvalue;            /* "red" in color=red */

	if (strcmp (filename, "-") == 0)
		{
		ioptr = stdin;
		checkstdin();
		}
	else if ((ioptr = fopen (filename, "r")) == NULL)
		return (1);
	while (fgets (line, sizeof (line), ioptr))
		{
		lineptr = getrest (line);
		if (*lineptr == '\0')
			continue;
		if (lineno == Pagesize)
			lineno = printheader ();
		lineno++;
		item = getfirst (line);
		printf ("%s", item);
		for (i = strlen (item); i < Itemwidth; i++)
			putchar (Leader);
		for (i = 0; i < Nfeatures; i++)
			HasFeature[i] = Featno;
		while (featptr = getfirst (lineptr))
			{
			/*	First strip off item value from any name=value pair
				to will allow the line item to match the Feature name. */
			if (itemvalue = strchr (featptr, ASSIGN))
				*itemvalue++ = '\0';
			else
				itemvalue = Featyes;
			/* Now check for substrings, stop on first match.  */
			for (i = 0; i < Nfeatures; i++)
				{
				if (substring (featptr, Feature[i]))
					{
					if (itemvalue != Featyes)
						itemvalue = strdup (itemvalue);
					HasFeature[i] = itemvalue;
					break;
					}
				}
			lineptr = getrest (lineptr);
			}
		for (i = 0; i < Nfeatures; i++)
			{
			printfield (i, HasFeature[i]);
			if (HasFeature[i] != Featyes && HasFeature[i] != Featno)
				free (HasFeature[i]);
			}
		printf ("%s\n", Coldelim);
		}
	if (ioptr && ioptr != stdin)
		fclose (ioptr);
	return (0);
	}

/*FUNCTION printheader: print a header for a feature table */
/*DESCRIPTION
	This routine prints the Feature labels, at staggered columns
	to create a compact form.
*/
int              /* return number of lines in header */
printheader ()
	{
	int	i, j;
	if (Horizontal)
		{
		for (j = 0; j < Itemwidth; j++)
			putchar (' ');
		for (j = 0; j < Nfeatures; j++)
			printfield (j, Feature[j]);
		printf ("%s\n", Coldelim);
		}
	else
		for (i = 0; i < Nfeatures; i++)
			{
			for (j = 0; j < Itemwidth; j++)
				putchar (' ');
			for (j = 0; j < i; j++)
				printfield (j, " ");
			printf ("%s%s\n", Coldelim, Feature[i]);
			}
	return (Horizontal ? 1 : Nfeatures);
	}

/*FUNCTION printfield: print a field padding or truncating specified contents */
void
printfield (fieldno, contents)
int 	fieldno;     /* the field number to print */
char	*contents;   /* what to place in the field */
	{
	int 	width;
	printf ("%s", Coldelim);
	if (contents == NULL)
		contents = "";
	if (Featwidth[fieldno] || Width != 1)
		{
		if (Featwidth[fieldno])
			width = Featwidth[fieldno];
		else
			width = Width;
		printf ("%-*.*s", width, width, contents);
		}
	else
		printf ("%s", contents);
	}

usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (BUFSIZ, "maximum number of characters in lines");
		const (MAXFEAT, "maximum number of features");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		sopt ('c', "colchar", "Column Separator String", Coldelim);
		sopt ('f', "input", "Input File Name", Infile);
		iopt ('i', "width", "Item Label Width", Itemwidth);
		lopt ('h', "Horizontal Format for Header", Horizontal);
		copt ('l', "leader", "Leader Character After Label", Leader);
		sopt ('n', "no", "String Indicating 'NO'", Featno);
		iopt ('p', "lines", "Page Size", Pagesize);
		iopt ('w', "width", "Width of Feature Columns", Width);
		sopt ('y', "yes", "String Indicating 'YES'", Featyes);
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
