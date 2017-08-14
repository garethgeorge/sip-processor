/*  Copyright 1985 Gary Perlman */

#include "stat.h"
FUN(mdmat,MultiDimensional Matrix Routines,5.1,02/03/87)

/*
	mdmat: routines for reading and manipulating multidimensional arrays
	These routines assume each input datum is on a line by itself,
	preceded by strings encoding conditions under which it was obtained.
	The total of the combinations of conditions is used to create an
	array with software to support simulation of a multidimensional array.
	Accessible through global variables are the data, the names/number of
	factors, the names/number of levels of factors.  All these that are larger
	than a scalar are dynamically allocated, but not freed.

	Access into the mdarray is done with the functions mdaddr and mdnext.
	A level array is one of positive integers used as indexes into dimensions
	in the mdarray.  For example, if we had a 3d-array, we could access
	elements with a[i][j][k], but with these mdarrays, we do:
		level[0] = i; level[1] = j; level[2] = k;
		a[mdaddr (level)];
	mdaddr does all the multiplying necessary.

	Iteration through the array is somewhat automated with the mdnext
	function.  If we start with a level vector with 0's, successive
	calls to mdnext will count through each factor (dimension).
	For example, if factor 0 has 3 levels and factor 1 has 2, then we count:
		0 0,   0 1,   1 0,   1 1,   2 0,   2 1
	There is some subtlety in the parameters to mdnext because of the
	development use of mdmat: data analysis.  In data analysis, it is
	common to summarize a crossing of some factors, averaging or summing
	over all others.  This is done by providing a "source" parameter
	that tells mdnext the names of the factors of interest.  This
	is conceptually a set, but implemented as bits, them abstraction is
	hidden mostly in macros.  The last parameter to mdnext tells
	whether the next level of a factor of interest or one not of interest
	should be provided.  The expected use is to (1) cycle through all
	possible combinations of factors (called sources) and for each source,
	(2) cycle through all the levels of source factors, (3) summarizing
	over non-source factors.  This is coded as (T==1):

	for (source = 0; source < nsources; source++)
		do
			{
			do
				summarize over nonsources (e.g., sum += data[mdaddr(level)];)
			while (mdnext (level, source, F));
			report/store summary of this combination of source factor levels
			}
		while (mdnext (level, source, T));
*/


#ifndef R_DATA
#ifndef I_DATA
#define I_DATA
#endif
#endif
#include "mdmat.h"
static	Posint	mdfill ();     /* reads in data from temporary file */
static	char	*mdlevels ();  /* reads levels of factors into data */

#include <signal.h>
static	char	*MDtmpfile;
/* FUNCTION mdonint: remove temporary md file on interrupt */
void
mdonint ()
	{
	(void) signal (SIGINT, SIG_IGN);
	WARNING (...interrupted...removing tempfile)
	(void) unlink (MDtmpfile);
	exit (1);
	}

/* Global Data */
Posint	Nfactors;                 /* total number of factors */
char	**Factname;               /* names of factors + data in last */
Posint	*Nlevels;                 /* number of levels of each factor */
char	***Levelname;             /* level names */
DATUM	*Datax;                   /* will hold all the data */
short	*Nreplics;                /* number of replications in each cell */
Posint	Maxlev = MAXLEV;          /* maximum number of levels */
Posint	NAcount = 0;              /* number of missing points */


static
ncmp (sp1, sp2)
char	**sp1, **sp2;
	{
	return (numcmp (*sp1, *sp2));
	}
	
static
sortnames (vec, n)
char	**vec;
int 	n;
	{
	int 	i;
	int 	ncmp ();
		
	for (i = 0; i < n; i++)
		if (!number (vec[i]))
			return;
	
	qsort ((char *) vec, n, sizeof (char *), ncmp);
	}

/*FUNCTION mdread: read multidimensional matrix */
Posint
mdread (argc, argv, firstname)
char	**argv;
	{
	char	*tmpdata;
	int 	i;
	Posint	ncells;
	tmpdata = mdlevels (argc, argv, firstname);
	for (i = 0; i < Nfactors; i++)
		sortnames (Levelname[i], Nlevels[i]);
	ncells = mdfill (tmpdata);
	return (ncells);
	}

/*FUNCTION mdlevels:	finds the number of levels of each factor */
/*
	For each line, it reads in the levels of each factor.
	It assumes that the number of levels equals the maximum levelnumber.
	The data is read from the stdin but is copied for further use.
	Returns the name of a temp file where data are stored.
*/
static
char *
mdlevels (argc, argv, firstop)
char	**argv;
int 	firstop;     /* first operand (factor names) */
	{
	register int factor;            /* looping variable */
	register int level;             /* looping variable */
	char	line[BUFSIZ];           /* each data line read in here */
	char	*column[MAXFACT+2];     /* data line separated in cols */
	char	*ptr;
	int 	ncols;                  /* number of columns in line */
	static	char tmpname[100];      /* temporary file */
	FILE	*datafile;              /* pointer to temporary file */
	MDtmpfile = tmpname;
	(void) signal (SIGINT, mdonint);

	if (mytmpfile (argv[0], tmpname))
		ERROPEN ("unique temporary file")
	if ((datafile = fopen (tmpname, "w")) == NULL)
		ERROPEN ("temporary file")

	while (fgets (line, BUFSIZ, stdin))
		{
		fputs (line, datafile); /* save data for next pass */
		ncols = parselin (line, column, MAXFACT+2);
		if (ncols == 0)
			continue;
		if (Nfactors == 0) /* initialize */
			{
			Nfactors = ncols - 1;
			if (Nfactors < 1 || Nfactors > MAXFACT)
				ERRMSG1 (must have between one and %d factors, MAXFACT)
			if (argc - firstop > Nfactors + 1)
				ERRMANY (factor names,Nfactors)

			Factname = myalloc (char *, Nfactors+1);
			if (Factname == NULL)
				ERRSPACE (factor names)
			Factname[Nfactors] = "DATA"; /* data name */
			for (factor = firstop; factor < argc; factor++)
				Factname[factor-firstop] = argv[factor];
			for (factor = factor-firstop; factor < Nfactors; factor++)
				{
				Factname[factor] = myalloc (char, 2);
				Factname[factor][0] = factor + 'A';
				Factname[factor][1] = '\0';
				}

			Nlevels = (Posint *) calloc (Nfactors, sizeof (Posint));
			if (Nlevels == NULL)
				ERRSPACE (numbers of levels of factors)
			Levelname = myalloc (char **, Nfactors);
			if (Levelname == NULL)
				ERRSPACE (level names)
			for (factor = 0; factor < Nfactors; factor++)
				{
				Levelname[factor] = myalloc (char *, Maxlev);
				if (Levelname[factor] == NULL)
					ERRSPACE (level names)
				}
			}
		if (ncols != Nfactors+1)
			ERRRAGGED

		/* check for new factor name */
		for (factor = 0; factor < Nfactors; factor++)
			{
			for (level = 0; level < Nlevels[factor]; level++)
				if (!strcmp (Levelname[factor][level], column[factor]))
					break;
			if (level == Maxlev)
				ERRMANY (levels, Maxlev)
			if (level == Nlevels[factor]) /* a new level */
				Levelname[factor][Nlevels[factor]++] = strdup (column[factor]);
			}
		
		if (isna (column[Nfactors]))
			{
			NAcount++;
			continue;
			}

#ifdef	R_DATA /* input must be numerical */
		if (!number (column[Nfactors]))
			ERRNUM (column[Nfactors],data value)
#endif	R_DATA
#ifdef	I_DATA /* input must be a frequency count */
		for (ptr = column[Nfactors]; isdigit (*ptr); ptr++)
			continue;
		if (*ptr) /* non digit -> not a frequency count */
			ERRMSG1 (datum (%s) is not a frequency count, column[Nfactors])
#endif	I_DATA
		}
	
	if (Nfactors == 0)
		ERRDATA
	for (factor = 0; factor < Nfactors; factor++)
		if (Nlevels[factor] < 2)
			ERRMSG1 (factor %s must have at least two levels, Factname[factor])

	(void) fclose (datafile);
	return (tmpname);
	}

/*FUNCTION mdaddr:	return unique index for each combination factor levels */
Posint
mdaddr (level)
Posint	*level;             /* levels (>= 0) of each factor */
	{
	register int factor;    /* looping variable */
	int 	aindex;         /* level of each factor read in here */
	int 	coeff = 1;      /* aindex multiplied by coeff */

	aindex = level[Nfactors-1];
	for (factor = Nfactors-2; factor >= 0; factor--)
		{
		coeff *= Nlevels[factor+1];
		aindex += coeff * level[factor];
		}
	return (aindex);
	}

/*FUNCTION mdfill:	read data from datafile and store it in data array */
/*
	Space is allocated for the data array and the number of replics per cell.
	For each line, it reads the levels of each factor and finds the location
	where the data is to be stored in data by calling mdaddr with the level
	numbers stored in the array called level.  Any space not used in data
	(because of nested design, for example) has nreplics == 0.
	Finally, it removes the temporary data file.
	returns the number of cells allocated.
*/
static
Posint
mdfill (tmpname)
char	*tmpname;
	{
	register Posint address;    /* where data will be added */
	register int factor;        /* looping variable */
	Posint	level[MAXFACT];     /* level of each factor */
	char	line[BUFSIZ];       /* each data input line read in here */
	char	*column[MAXFACT+1]; /* data line in columns */
	FILE	*datafile;
	Posint	ncells = 1;

	for (factor = 0; factor < Nfactors; factor++)
		ncells *= Nlevels[factor];
	if ((Datax = (DATUM *) calloc (ncells, sizeof (*Datax))) == NULL)
		ERRSPACE (data)
#ifdef	R_DATA /* allocate space for replications */
	if ((Nreplics = (short *) calloc (ncells, sizeof (*Nreplics))) == NULL)
		ERRSPACE (data)
#endif	R_DATA

	if ((datafile = fopen (tmpname, "r")) == NULL)
		ERROPEN ("temporary file");

	while (fgets (line, BUFSIZ, datafile))
		{
		if (parselin (line, column, MAXFACT+1) == 0) /* blank line */
			continue;
		for (factor = 0; factor < Nfactors; factor++)
			{
			level[factor] = 0;
			while (strcmp (column[factor], Levelname[factor][level[factor]]))
				level[factor]++;
			}
		address = mdaddr (level);
#ifdef	R_DATA
		Nreplics[address]++;
#endif	R_DATA
		Datax[address] += CONV (column[Nfactors]);
		}

#ifdef	R_DATA /* average all cells by number of replications */
		for (address = 0; address < ncells; address++)
			if (Nreplics[address] > 1)
				Datax[address] /= Nreplics[address];
#endif	R_DATA

	(void) fclose (datafile);

	(void) signal (SIGINT, SIG_DFL); /* really, this should reset to previous */
	(void) unlink (tmpname);

	return (ncells);
	}

/*FUNCTION mdnext:	simulate a counting system based on Nlevels[factors] */
Boole
mdnext (level, source, sourceflag)/* returns whether there are more levels */
Posint	level[MAXFACT];       /* the current levels of each factor */
Posint	source;               /* bit array of factors to (not) increment */
Boole	sourceflag;           /* incr source factor if TRUE, else non-source */
	{
	register int factor;

	for (factor = Nfactors-1; factor >= 0; factor--)
		if (sourceflag == member (factor, source))
			if (++level[factor] < Nlevels[factor])
				return (TRUE);
			else /* go to next `decimal' place */
				level[factor] = 0;
	return (FALSE);
	}

/*FUNCTION printeffect:	print cell summary of an effect */
#ifdef	TRACE
printeffect ()
	{
	Posint	source;
	int 	factor;
	Posint	level[MAXFACT];
	Boole	sources, nonsources;
	DATUM	sum;
	Posint 	count;
	Posint	address;
	Posint	nsources = (1 << Nfactors);

	for (source = 0; source < nsources; source++)
		{
		for (factor = 0; factor < Nfactors; factor++)
			{
			level[factor] = 0;
			printf ("%s\t", Factname[factor]);
			}
		putchar ('\n');
		for (sources = TRUE; sources; sources = mdnext (level, source, TRUE))
			{
			sum = ZERO;
			count = 0;
			for (nonsources = TRUE; nonsources;
					nonsources = mdnext (level, source, FALSE))
				{
				address = mdaddr (level);
#ifdef	R_DATA /* only include cells with data in them */
				if (Nreplics[address])
					{
#endif	R_DATA
					sum += Datax[address];
					count++;
#ifdef	R_DATA
					}
#endif	R_DATA
				}

			if (count)
				{
				for (factor = 0; factor < Nfactors; factor++)
					if (member (factor, source))
						printf ("%s	", Levelname[factor][level[factor]]);
					else
						putchar ('\t');
				printf (FORMAT, sum);
				printf ("\t%d\n", count);
				}
			}
		}
	}
#endif	TRACE

/*FUNCTION printlevels:	print the levels of the factors */
#ifdef	TRACE
printlevels ()
	{
	int 	maxlev = 0;
	int 	factor, level;

	puts ("Levels of Factors:");
	for (maxlev = factor = 0; factor < Nfactors; factor++)
		{
		if (Nlevels[factor] > maxlev)
			maxlev = Nlevels[factor];
		printf ("%-7.7s%c",
			Factname[factor],
			factor == Nfactors-1 ? '\n' : '\t');
		}
	for (level = 0; level < maxlev; level++)
		for (factor = 0; factor < Nfactors; factor++)
			{
			printf ("%-7.7s%c",
				Nlevels[factor] > level ? Levelname[factor][level] : "",
				factor == Nfactors-1 ? '\n' : '\t');
			}
	}
#endif	TRACE

/*FUNCTION setsize: return the number of sources (bits) in a set */
int
setsize (set, maxsize)
Posint	set;
Posint 	maxsize;
	{
	int 	size = 0;
	Posint	bit;
	for (bit = 0; bit < maxsize; bit++)
		if (member (bit, set))
			size++;
	return (size);
	}

/*FUNCTION printsource:	print the names of factors in source */
Posint
printsource (source)
Posint	source;
	{
	int 	factor;
	int 	size = 0;
	printf ("SOURCE: ");
	for (factor = 0; factor < Nfactors; factor++)
		if (member (factor, source))
			{
			printf ("%s ", Factname[factor]);
			size++;
			}
	putchar ('\n');
	return (size);
	}

/*FUNCTION printdesign:	print names and levels of factors */
printdesign (ndata)
Posint	ndata;
	{
	int 	factor;
	char	*sformat = "%10.10s ";
	char	*dformat = "%10d ";

	printf ("FACTOR: ");
	for (factor = 0; factor <= Nfactors; factor++)
		printf (sformat, Factname[factor]);
	putchar ('\n');
	printf ("LEVELS: ");
	for (factor = 0; factor < Nfactors; factor++)
		printf (dformat, Nlevels[factor]);
	printf (dformat, ndata);
	putchar ('\n');
	if (NAcount)
		{
		printf ("NA    : ");
		for (factor = 0; factor < Nfactors; factor++)
			printf (sformat, "");
		printf (dformat, NAcount);
		putchar ('\n');
		}
	}
