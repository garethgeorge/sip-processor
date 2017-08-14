/*  Copyright 1985 Gary Perlman */

#include "stat.h"
PGM(contab,Contingency Tables and Chi-Square,5.4,02/03/87)

/*
	THINGS TO DO:
		n**k chisq test (see winer)

		Option to supply the expected values (complex!)
			changes df's

		Option to request specific tables and breakdowns
			-t "A B C"
	
	SOURCES:
		BMD P4F (1981++)
		Winer p. 857
		Goodman paper in JASA
		Siegel p. 96
		Bradley p. 195
*/


#ifndef	I_DATA
#define	I_DATA    /* using integer data in mdmat */
#endif	I_DATA
#define	MAXLEV       40           /* maximum number of levels of factors */
#define MAXCHARS BUFSIZ           /* maximum number of chars in lines */
#include "mdmat.h"

#define	percent(freq1,freq2) ((100.0 * (freq1)) / (freq2))
extern	double	pochisq ();

#define	SFORMAT "%8.8s"    /* format for string labels */
#define	LFORMAT "%-8.8s"   /* format for left justified labels */
#define	CFORMAT "%8d"      /* format for cell counts */
#define	EFORMAT "%8.2f"    /* format for expected frequencies */
#define	PFORMAT "%8.1f"    /* format for percentages */
static	char	*Sformat = SFORMAT;
static	char	*Lformat = LFORMAT;
static	char	*Cformat = CFORMAT;
static	char	*Eformat = EFORMAT;
static	char	*Pformat = PFORMAT;

DATUM	ConTable[MAXLEV][MAXLEV];  /* contingency table */
DATUM	ConRow[MAXLEV];            /* row totals */
DATUM	ConCol[MAXLEV];            /* column totals */
DATUM	Total;                     /* sum of all frequencies */
double	Expected[MAXLEV][MAXLEV];  /* expected frequencies */
Boole	Exp0;                      /* any expected frequencies == 0? */

/* OPTIONS */
Boole	PrExpected;           /* should expected cell frequencies be printed? */
Boole	PrDiffs;              /* should cell differences be printed? */
Boole	PrRows;               /* should row percentages be printed? */
Boole	PrCols;               /* should column percentages be printed? */
Boole	PrTotal;              /* should total percentages be printed? */
Boole	Blank = FALSE;        /* should blank lines be used in tables? */
int 	Interfact = MAXFACT;  /* maximum interaction to print */
Boole	Debug = FALSE;        /* should debugging information be printed? */
Boole	DoYates = TRUE;       /* do we apply Yates' correction? */
Boole	Sigtest = TRUE;       /* do we do significance test? */
#define	SMALLCELL   5.0       /* can't have too many of these */
#define	NFISHER     200       /* do exact test for N <= NFISHER */
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */

/*FUNCTION chisq1: comput and print chisq value for 1 way table */
void
chisq1 (source)
Posint	source;
	{
	/*FUTURE: add binomial exact test when df == 1 */
	Posint	fact;
	Posint	factor;
	Posint 	level[MAXFACT];
	int 	df;
	Boole	yates = FALSE;
	Posint	count;
	double	expect;
	double	diff;
	double	chival = 0.0;

	for (factor = 0; factor < Nfactors; factor++)
		if (member (factor, source)) /* identify the source factor numbers */
			{
			fact = factor;
			df = (Nlevels[fact] - 1);
			break;
			} 

	if (df < 1)
		{
		WARNING (no degrees of freedom for test)
		return;
		}

	if (DoYates && df == 1) /* correction not applied if df > 1 */
		yates = TRUE;

	for (factor = 0; factor < Nfactors; factor++)
		level[factor] = 0;

	expect = (double) Total / (double) Nlevels[fact];

	printf (Lformat, Factname[fact]);
	printf (Sformat, "count");
	if (PrExpected)
		printf (Sformat, "expect");
	if (PrDiffs)
		printf (Sformat, "diffs");
	if (PrRows || PrCols || PrTotal)
		printf (Sformat, "percent");
	putchar ('\n');

	do
		{
		count = 0;
		do
			count += Datax[mdaddr(level)];
		while (mdnext (level, source, FALSE));
		printf (Lformat, Levelname[fact][level[fact]]);
		printf (Cformat, count);
		if (PrExpected)
			printf (Eformat, expect);
		diff = count - expect;
		if (yates)
			{
			diff = fabs (diff) - 0.5;
			if (diff < 0.0) /* over-correction */
				diff = 0.0;
			}
		if (PrDiffs)
			printf (Eformat, diff);
		if (PrRows || PrCols || PrTotal)
			printf (Pformat, percent (count, Total));
		putchar ('\n');
		chival += diff*diff / expect;
		}
	while (mdnext (level, source, TRUE));

	printf (Lformat, "Total");
	printf (Cformat, Total);
	putchar ('\n');

	if (Sigtest)
		{
		if (yates)
			printf ("NOTE: Yates' correction for continuity applied\n");
		if (expect < SMALLCELL)
			printf ("WARNING: expected frequency is less than %g\n", SMALLCELL);
		chiprint (chival, df, pochisq (chival, df));
		/* if df == 1, then do one and two-tail binomial test */
		}
	}

/*FUNCTION chisq2: compute and print chisq value for 2 way table */
chisq2 (source)
Posint	source;
	{
	Posint	fact1, fact2;
	Posint	lev1 = 0, lev2 = 0; /* number of levels of factors */
	Posint	row, col;
	Posint	factor;
	int 	df;             /* degrees of freedom for test */
	double	diff;           /* obtained - expected */
	double	p;
	double	value;          /* will be chi square value */
	Boole	yates = FALSE;  /* is yates' correction being applied? */
	Boole	fisher = FALSE; /* use fisher exact test? */
	double	fisher2x2 ();
	int 	smallcount;     /* count of cells with expected freqs < SMALLCELL */

	if (Blank)
		putchar ('\n');

	for (factor = 0; factor < Nfactors; factor++)
		if (member (factor, source)) /* identify the source factor numbers */
			if (!lev1)
				lev1 = Nlevels[fact1 = factor];
			else
				lev2 = Nlevels[fact2 = factor];

	df = (lev1 - 1) * (lev2 - 1);
	if (df < 1)
		{
		WARNING (no degrees of freedom for test)
		return;
		}
	
	if (DoYates && df == 1) /* correction not applied if df > 1 */
		yates = TRUE;

	smallcount = 0;
	for (row = 0; row < lev1; row++)
		for (col = 0; col < lev2; col++)
			if (Expected[row][col] < SMALLCELL)
				smallcount++;

	if (lev1 == 2 && lev2 == 2 && Total <= NFISHER)
		fisher = TRUE;

	value = 0.0;
	for (row = 0; row < lev1; row++)
		for (col = 0; col < lev2; col++)
			{
			diff = ConTable[row][col] - Expected[row][col];
			if (yates)
				{
				diff = fabs (diff) - 0.5;
				if (diff < 0.0) /* over-correction */
					diff = 0.0;
				}
			value += diff*diff / Expected[row][col];
			}
	
	p = pochisq (value, df);
	printf ("Analysis for %s x %s:\n", Factname[fact1], Factname[fact2]);
	if (yates)
		printf ("\tNOTE: Yates' correction for continuity applied\n");
	if (smallcount)
		printf ("\tWARNING: %d of %d cells had expected frequencies < %g\n",
			smallcount, lev1*lev2, SMALLCELL);
	chiprint (value, df, p);
	if (fisher)
		fishtest (ConTable[0][0], ConTable[0][1],
				  ConTable[1][0], ConTable[1][1]);
	if (lev1 == 2 && lev2 == 2)
		tprint ("phi Coefficient == Cramer's V", sqrt (value / Total));
	else
		tprint ("Cramer's V", sqrt (value / (Total * (min (lev1, lev2) - 1))));
	tprint ("Contingency Coefficient", sqrt (value / (value + Total)));
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
		"bDi:syc:LOV";

	while ((flag = getopt (argc, argv, optstring)) != EOF)
		switch (flag)
			{
			default:
				opterr++;
				break;
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			/* put option cases here */
			case 'b':
				Blank = TRUE;
				break;
			case 'c':
				for (; *optarg; optarg++)
					switch (*optarg)
						{
						case 'e': PrExpected = TRUE; break;
						case 'd': PrDiffs = TRUE; break;
						case 'p': PrRows = PrCols = PrTotal = TRUE; break;
						case 'c': PrCols = TRUE; break;
						case 'r': PrRows = TRUE; break;
						case 't': PrTotal = TRUE; break;
						default:
							fprintf (stderr, "%s: unknown cell entry '%c'\n",
								argv[0], *optarg);
							break;
						}
				break;
			case 'D':
				Debug = TRUE;
				break;
			case 'i':
				if (setint (Argv0, flag, optarg, &Interfact, 1, MAXFACT))
					opterr++;
				break;
			case 's':
				Sigtest = FALSE;
				break;
			case 'y':
				DoYates = FALSE;
				break;
			}

	if (opterr)
		USAGE ("[-bsy] [-c cell contents (edprct)] [-i nfactors] [factor names]")
	
	usinfo ();

	return (optind);
	}

/*FUNCTION main */
main (argc, argv) char **argv;
	{
	int 	firstname;
	Posint	ncells;
	Posint	source;
	Posint	nsources;
	Posint	nfactors;
	int 	i;
	ARGV0;

	Maxlev = MAXLEV;
	firstname = initial (argc, argv);
	checkstdin ();
	ncells = mdread (argc, argv, firstname);
	nsources = 1 << Nfactors;
	for (i = 0; i < ncells; i++)
		Total += Datax[i];
	if (Total == 0)
		ERRMSG0 (No expected cell frequencies were read in)

	printdesign (Total);

	for (source = 0; source < nsources; source++)
		if ((nfactors = setsize (source, Nfactors)) == 1)
			{
			putchar ('\n');
			chisq1 (source);
			}

	for (source = 0; source < nsources; source++)
		if ((nfactors=setsize (source, Nfactors)) == 2 && nfactors <= Interfact)
			{
			putchar ('\n');
			printsource (source);
			sumtab (source);
			if (Sigtest)
				{
				if (Exp0 == TRUE)
					WARNING (no chi-square: expected cell frequency == 0)
				else
					chisq2 (source);
				}
			}

	for (source = 0; source < nsources; source++)
		if ((nfactors=setsize (source, Nfactors)) > 2 && nfactors <= Interfact)
			{
			putchar ('\n');
			printsource (source);
			summary (source);
			}

	exit (0);
	}

/*FUNCTION summary: print summary of higher order design */
summary (source)
Posint	source;
	{
	Posint	level[MAXFACT];
	Posint 	count;
	Posint	factor;

	for (factor = 0; factor < Nfactors; factor++)
		{
		level[factor] = 0;
		printf (Sformat, Factname[factor]);
		}
	putchar ('\n');

	do	{
		count = 0;
		do
			count += Datax[mdaddr(level)];
		while (mdnext (level, source, FALSE));
		for (factor = 0; factor < Nfactors; factor++)
			if (member (factor, source))
				printf (Sformat, Levelname[factor][level[factor]]);
			else
				printf (Sformat, "");
		printf (Cformat, count);
		putchar ('\n');
		}
	while (mdnext (level, source, TRUE));
	}

/*FUNCTION sumtab: summarize contingency table */
sumtab (source)
Posint	source;
	{
	Posint	factor, level[MAXFACT];
	DATUM	count;
	Posint	row, col;
	Posint	fact1, fact2;
	Posint	lev1 = 0, lev2;

	/* find factors in source and their levels */
	for (factor = 0; factor < Nfactors; factor++)
		{
		level[factor] = 0;
		if (member (factor, source)) /* identify the source factor numbers */
			{
			if (!lev1)
				lev1 = Nlevels[fact1 = factor];
			else
				lev2 = Nlevels[fact2 = factor];
			}
		}

	/* initialize contingency table */
	for (row = 0; row < lev1; row++)
		ConRow[row] = 0;
	for (col = 0; col < lev2; col++)
		ConCol[col] = 0;
	for (row = 0; row < lev1; row++)
		for (col = 0; col < lev2; col++)
			ConTable[row][col] = 0;

	do
		do
			ConTable[level[fact1]][level[fact2]] += Datax[mdaddr (level)];
		while (mdnext (level, source, FALSE));
	while (mdnext (level, source, TRUE));

	for (row = 0; row < lev1; row++)
		for (col = 0; col < lev2; col++)
			{
			count = ConTable[row][col];
			ConCol[col] += count;
			ConRow[row] += count;
			}

	doexpect (fact1, fact2);
	table (fact1, fact2);
	}

/*FUNCTION table: print the contingency table */
table (fact1, fact2)
Posint	fact1;
Posint	fact2;
	{
	Posint	row, col;
	Posint	lev1 = Nlevels[fact1];
	Posint	lev2 = Nlevels[fact2];

	printf (Sformat, "");
	for (col = 0; col < lev2; col++)
		printf (Sformat, Levelname[fact2][col]);
	printf (Sformat, "Totals");
	putchar ('\n');

	for (row = 0; row < lev1; row++)
		{
		if (Blank)
			putchar ('\n');

		printf (Lformat, Levelname[fact1][row]);
		for (col = 0; col < lev2; col++)
			printf (Cformat, ConTable[row][col]);
		printf (Cformat, ConRow[row]);
		putchar ('\n');

		if (PrExpected)
			{
			printf (Lformat, "  expect");
			for (col = 0; col < lev2; col++)
				printf (Eformat, Expected[row][col]);
			putchar ('\n');
			}

		if (PrDiffs)
			{
			printf (Lformat, "  diffs");
			for (col = 0; col < lev2; col++)
				printf (Eformat, ConTable[row][col] - Expected[row][col]);
			putchar ('\n');
			}

		if (PrRows)
			{
			printf (Lformat, "  row %");
			for (col = 0; col < lev2; col++)
				printf (Pformat, percent (ConTable[row][col], ConRow[row]));
			printf (Pformat, percent (ConRow[row], Total));
			putchar ('\n');
			}

		if (PrCols)
			{
			printf (Lformat, "  col %");
			for (col = 0; col < lev2; col++)
				printf (Pformat, percent (ConTable[row][col], ConCol[col]));
			putchar ('\n');
			}

		if (PrTotal)
			{
			printf (Lformat, "  % tot");
			for (col = 0; col < lev2; col++)
				printf (Pformat, percent (ConTable[row][col], Total));
			putchar ('\n');
			}
		}

	if (Blank)
		putchar ('\n');

	printf (Lformat, "Totals");
	for (col = 0; col < lev2; col++)
		printf (Cformat, ConCol[col]);
	printf (Cformat, Total);
	putchar ('\n');

	if (PrCols)
		{
		printf (Lformat, "  col %");
		for (col = 0; col < lev2; col++)
			printf (Pformat, percent (ConCol[col], Total));
		printf (Pformat, 100.0);
		putchar ('\n');
		}
	}

/*FUNCTION doexpect: fill expected cell frequency table with value */
doexpect (fact1, fact2)
Posint	fact1;
Posint	fact2;
	{
	Posint	lev1 = Nlevels[fact1];
	Posint	lev2 = Nlevels[fact2];
	int 	row, col;
	double	ftotal = Total;   /* assumes this is non-zero */

	Exp0 = FALSE;
	for (row = 0; row < lev1; row++)
		for (col = 0; col < lev2; col++)
			{
			Expected[row][col] = (ConCol[col] / ftotal) * ConRow[row];
			if (fzero (Expected[row][col]))
				Exp0 = TRUE;
			}
	}

/*FUNCTION usinfo: print info about contab */
usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXFACT, "maximum number of factors");
		const (Maxlev, "maximum number of levels");
		const (MAXCHARS, "maximum number of characters in lines");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('b', "insert blank lines in tables", Blank);
		sopt ('c', "contents", "request cell contents (edprct)", "");
		iopt ('i', "factors",  "maximum number of factors in interactions", Interfact);
		lopt ('s', "print significance tests", Sigtest);
		lopt ('y', "use Yates correction when df == 1", DoYates);
		oper ("[names]", "factor names", "A B ... DATA");
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
