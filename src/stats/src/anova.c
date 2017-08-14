/*  Copyright 1980 Gary Perlman */
#include "stat.h"
PGM(anova,Multi-Factor Analysis of Variance,5.4,08/22/92)

/*
	This program does a general analysis of variance.
	It was written by Gary Perlman at UCSD April 1980.
	Major rewrite November 1985
		Thanks to Derek Austin for helping locate several problems
*/

#include <signal.h>

#define MAXFACT      10           /* the maximum number of factors */
#define MAXLEV      500           /* maximum number of levels per factor */
#define MAXCHARS BUFSIZ           /* maximum number of chars in lines */
#define	MAXBET       20           /* max for between factor levels */
#define RANDOM        0           /* zeroeth column is RANDOM factor */

#define	member(factor,source)     (((1 << (factor)) & (source)) != 0)
#define	join(a,b)                 ((a) | (b))
#define	subset(a,b)               (((a)&(b)) == (a))
#define	enter(factor,source)      ((source) |= (1 << (factor)))
int
setsize (set, maxsize)
int 	set;
Posint 	maxsize;
	{
	int 	size = 0;
	Posint	bit;
	for (bit = 0; bit < maxsize; bit++)
		if (member (bit, set))
			size++;
	return (size);
	}

Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */
Boole	Debug;                /* print debugging information */
int 	Plot = 0;             /* number of levels to show plots */
int 	Width = 35;           /* width of plots */
int 	NAcount = 0;          /* number of NA missing values */

Posint	Nfactors;                 /* total number of factors */
Posint	Nsources;                 /* number of sources */
Posint	Nlevels[MAXFACT];         /* number of levels of each factor */
int 	Between;                  /* holds between subject factors */
char	*Factorname[MAXFACT+1]    /* default names for factors */
	= {"RANDOM", "A", "B", "C", "D", "E", "F", "G", "H", "I"};
char	*Levelname[MAXFACT][MAXLEV];	/* level names */
float	*Datax;                   /* will hold all the data */
short	*Nreplics;                /* number of replications in each cell */
double	*Bracket;                 /* will hold bracket values */
FILE	*Datafile;                /* input file */
char	Tmpname[100];             /* temporary file where data are stored */
Boole	Errorflag = FALSE;        /* set to TRUE in case of fatal error */

void
onintr ()
	{
	(void) signal (SIGINT, SIG_IGN);
	WARNING (...interrupted...removing tempfile)
	(void) unlink (Tmpname);
	exit (1);
	}

/*FUNCTION main:	very high level calls only */
main (argc, argv) int argc; char *argv[];
	{
	(void) signal (SIGINT, onintr); /* should really check if ignored */

	ARGV0;
	initial (argc, argv);
	checkstdin ();
	getlevels (argc, argv);
	readdata ();
	cellmeans ();
	anova ();
	exit (0);
	}

/*FUNCTION initial: returns local version of optind, index to first operand */
int
initial (argc, argv) char **argv;
	{
	extern	char *optarg;    /* option value accessed through this by getopt */
	extern	int  optind;     /* will be index to first operand */
	int 	opterr = 0;      /* count of number of errors */
	int 	flag;            /* option flag characters read in here */
	int 	factor;
	Boole	wantplot = FALSE;

	while ((flag = getopt (argc, argv, "w:pDLOV")) != EOF)
		switch (flag)
			{
			default:
				opterr++;
				break;
			/* put option cases here */
			case 'D': Debug = TRUE; break;
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			case 'p': Plot++; break;
			case 'w':
				if (setint (Argv0, 'w', optarg, &Width, MIN_PLOT, MAX_WIDTH))
					opterr++;
				wantplot = TRUE;
				break;
			}

	if (opterr) /* print usage message and bail out */
		USAGE ("[-p] [-w width] [factor names]")

	if (wantplot && (Plot == 0))
		Plot++;
	usinfo ();

	if (argc-optind > MAXFACT)
		ERRMANY (factor names,MAXFACT)
	for (factor = optind; factor < argc; factor++)
		Factorname[factor-optind] = strdup (argv[factor]);

	return (optind);
	}

/*FUNCTION getlevels:	finds the number of levels of each factor */
/*
	For each line, it reads in the levels of each factor.
	It assumes that the number of levels equals the maximum levelnumber.
	The data is read from the stdin but is copied for further use in Datafile.
*/
getlevels (argc, argv) int argc; char **argv;
	{
	register int factor;            /* looping variable */
	register int level;             /* looping variable */
	char	line[MAXCHARS];         /* each data line read in here */
	char	*column[MAXFACT+2];     /* data line separated in cols */
	int 	ncols;                  /* number of columns in line */

	if (mytmpfile ("anova", Tmpname))
		ERROPEN ("unique temporary file");
	if ((Datafile = fopen (Tmpname, "w")) == NULL)
		ERROPEN ("temporary file")

	while (fgets (line, sizeof (line), stdin))
		{
		fputs (line, Datafile); /* save data for next pass */
		ncols = parselin (line, column, MAXFACT+2);
		if (ncols == 0)
			continue;
		if (Nfactors == 0) /* initialize */
			{
			Nfactors = ncols - 1;
			Nsources = 1 << Nfactors;
			if (Nfactors < 1 || Nfactors > MAXFACT)
				ERRMSG1 (must have at between one and %d factors, MAXFACT)
			if (argc < Nfactors+2)
				Factorname[Nfactors] = "DATA";
			}
		if (ncols != Nfactors+1)
			ERRRAGGED
	
		/* check for new factor name */
		for (factor = 0; factor < Nfactors; factor++)
			{
			for (level = 0; level < Nlevels[factor]; level++)
				if (!strcmp (Levelname[factor][level], column[factor]))
					break;
			if (level == MAXLEV)
				ERRMANY (levels, MAXLEV)
			if (level == Nlevels[factor]) /* a new level */
				{
				Levelname[factor][Nlevels[factor]] = strdup (column[factor]);
				Nlevels[factor]++;
				}
			}
	
		if (isna (column[Nfactors]))
			{
			NAcount++;
			continue;
			}
	
		if (!number (column[Nfactors]))
			ERRNUM (column[Nfactors],data value)
		}
			
	if (Nfactors == 0)
		ERRDATA

	for (factor = 0; factor < Nfactors; factor++)
		sortnames (Levelname[factor], Nlevels[factor]);
	if (Debug)
		printlevels (stderr);
	}

ncmp (sp1, sp2)
char	**sp1, **sp2;
	{
	return (numcmp (*sp1, *sp2));
	}
	
sortnames (vec, n)
char	**vec;
int 	n;
	{
	int 	i;
	int 	ncmp ();
		
	if (Debug)
		{
		fprintf (stderr, "anova: sorting %d names\n", n);
		for (i = 0; i < n; i++)
			fprintf (stderr, "%s%c", vec[i], i == n-1 ? '\n' : ' ');
		}
	
	for (i = 0; i < n; i++)
		if (!number (vec[i]))
			return;
	
	qsort ((char *) vec, n, sizeof (char *), ncmp);
	
	if (Debug)
		for (i = 0; i < n; i++)
			fprintf (stderr, "%s%c", vec[i], i == n-1 ? '\n' : ' ');
	}

/*FUNCTION readdata:	read data from Datafile and store it in data array */
/*
	Space is allocated for the data array and the number of replics per cell.
	For each line, it reads the levels of each factor and finds the location
	where the data is to be stored in data by calling offset with the level
	numbers stored in the array called level.  Any space not used in data
	(because of nested design, for example) has nreplics == 0.
	Finally, it removes the temporary data file.
*/
readdata ()
	{
	register int address;       /* where data will be added */
	register int factor;        /* looping variable */
	int 	level[MAXFACT];     /* level of each factor */
	char	line[MAXCHARS];     /* each data input line read in here */
	char	*column[MAXFACT+1]; /* data line in columns */
	Posint	ncells = 1;         /* product of all levels */

	for (factor = 0; factor < Nfactors; factor++)
		ncells *= Nlevels[factor];
	if ((Datax = (float *) calloc (ncells, sizeof (float))) == NULL)
		ERRSPACE (data)
	if ((Nreplics = (short *) calloc (ncells, sizeof (*Nreplics))) == NULL)
		ERRSPACE (data)

	Datafile = freopen (Tmpname, "r", Datafile);
	if (Datafile == NULL)
		ERROPEN ("temporary file");

	while (fgets (line, sizeof (line), Datafile))
		{
		if (parselin (line, column, MAXFACT+1) == 0) /* blank line */
			continue;
		if (isna (column[Nfactors]))
			continue;
		for (factor = 0; factor < Nfactors; factor++)
			{
			level[factor] = 0;
			while (strcmp (column[factor], Levelname[factor][level[factor]]))
				{
				level[factor]++;
				}
			}
		address = offset (level);
		Nreplics[address]++;
		Datax[address] += atof (column[Nfactors]);
		}

	for (address = 0; address < ncells; address++)
		if (Nreplics[address] > 1)
			Datax[address] /= (float)Nreplics[address];
	(void) unlink (Tmpname); /* done with Datafile */
	}

/*FUNCTION offset:	return unique index for each combination factor levels */
int
offset (level)
int 	level[MAXFACT];     /* levels (>= 0) of each factor */
	{
	register int factor;    /* looping variable */
	int 	aindex;         /* level of each factor read in here */
	int 	coeff = 1;      /* aindex multiplied by coeff */

	aindex = level[Nfactors-1];
	for (factor = Nfactors-2; factor >= 0; factor--)
		{
		coeff *= Nlevels[factor+1];
		aindex += coeff * (level[factor]);
		}
	return (aindex);
	}

/*FUNCTION cellmeans:	heavy computation of sums of squares an cell means */
/*
	The input to cellmeans is a set
	of arrays holding design information, and the data array which has to be
	in a particular format as defined by the program, offset.
	cellmeans also determines the type of each factor.

The algorithm proceeds as follows:
	for each source (from 0 to 2**Nfactors), the sums and sums of squares
	for each level of that source are computed by looping through all
	the non-sources.  From this information, the mean and sd are
	printed for each level, and a term is added into Bracket[source],
	an array of sums of squares of sums.  This array will be used for anova.
*/
cellmeans ()
	{
	register int factor;        /* looping variable */
	int 	level[MAXFACT];     /* level of each factor */
	int 	source;             /* source number/set */
	int 	nterms;             /* number of terms in source */
	double	datum;              /* each datum read into here */
	int 	address;            /* result from ofset */
	double	sum;                /* sum for each level of each factor */
	double	sumsq;              /* for each level of each factor */
	double	minval;             /* min for each level */
	double	maxval;             /* max for each level */
	double	grandmin;           /* minimum for all data */
	double	grandmax;           /* maximum for all data */
	int 	count;              /* count used along with sum */
	int 	sumcount[MAXFACT];  /* sum of counts of all levels of fact */
	int 	withprod = 1;       /* product of Nlevels[WITHIN] */
	Boole	sources, nonsources;/* while loop variables */

	for (factor = 0; factor < Nfactors; factor++)
		sumcount[factor] = 0;

	if ((Bracket = (double *) calloc (Nsources, sizeof (double))) == NULL)
		ERRSPACE (computations)

	for (source = 0; source < Nsources; source++)
		{
		nterms = setsize (source, Nfactors);

		pcellheader (source, TRUE);

		for (factor = 0; factor < Nfactors; factor++)
			level[factor] = 0;

		for (sources = TRUE; sources; sources = nextlevel (level, source, TRUE))
			{
			sum = sumsq = 0.0;
			count = 0;
			for (nonsources = TRUE; nonsources;
					nonsources = nextlevel (level, source, FALSE))
				{
				address = offset (level);
				if (Nreplics[address])
					{
					datum = Datax[address];
					sum += datum;
					sumsq += datum*datum;
					if (count == 0)
						minval = maxval = datum;
					else if (datum < minval)
						minval = datum;
					else if (datum > maxval)
						maxval = datum;
					count++;
					}
				}
			if (source == 0) /* grand mean */
				{
				grandmin = minval;
				grandmax = maxval;
				}

			if (! member (RANDOM, source))
				{
				for (factor = 1; factor < Nfactors; factor++)
					if (member (factor, source))
						{
						printf ("%-7.7s ", Levelname[factor][level[factor]]);
						if (nterms == 1) /* main effect */
							sumcount[factor] += count;
						}
					else printf ("%-7.7s ", "");
				pcellstats (count, sum, sumsq, minval, maxval);
				}

			if (count)
				Bracket[source] += sum*sum/count;
			else if (nterms == 2 && member (RANDOM, source))
				Between = join (Between, source-1);
			}

		if (Debug)
			printbracket (source, Bracket[source]);

		if (! member (RANDOM, source))
			putchar ('\n');

		if (Plot > 0 && (Plot >= nterms) && (! member (RANDOM, source)))
			{
			pcellheader (source, FALSE);

			for (factor = 0; factor < Nfactors; factor++)
				level[factor] = 0;
			
			for (sources = TRUE; sources;
				sources = nextlevel (level, source, TRUE))
				{
				sum = sumsq = 0.0;
				count = 0;
				for (nonsources = TRUE; nonsources;
					nonsources = nextlevel (level, source, FALSE))
					{
					address = offset (level);
					if (Nreplics[address])
						{
						datum = Datax[address];
						sum += datum;
						sumsq += datum*datum;
						if (count == 0)
							minval = maxval = datum;
						else if (datum < minval)
							minval = datum;
						else if (datum > maxval)
							maxval = datum;
						count++;
						}
					}
				effectname (source, level);
				pcellplot (count, sum, sumsq, minval, maxval, grandmin, grandmax);
				}
			effectname (0, level);
			putchar (' ');
			numline (grandmin, grandmax, Width);
			putchar ('\n');
			}
		}

	if (Errorflag == FALSE && nonprop ())
		ERRMSG0 (Unequal cell design did not have proportional cell sizes)

	for (factor = 0; factor < Nfactors; factor++)
		if (! member (factor, Between))
			withprod *= Nlevels[factor];

	for (factor = 1; factor < Nfactors; factor++)
		if (sumcount[factor] != withprod)
			{
			WARNING (Unbalanced factor)
			Errorflag = TRUE;
			}

	/* why bother freeing?
	free ((char *) Datax);
	free ((char *) Nreplics);
	*/
	}

/*FUNCTION nonprop:	check cell size proportions across between factors */
Boole
nonprop ()
	{
	int 	source;
	Boole	sources, nonsources;
	Posint	factor;
	Posint	count[MAXBET][MAXBET]; /* counts for first row of factor */
	int 	level[MAXFACT];        /* levels of each factor used to index */
	double	coeff, test;           /* coefficient constant? across rows */
	Posint	fact1, fact2;
	Posint	lev1, lev2;

	if (setsize (Between, Nfactors) <= 1) /* no possible problems */
		return (FALSE);

	for (source = 0; source < Nsources; source += 2)
		{
		if (!subset (source, Between) || setsize (source, Nfactors) != 2)
			continue;

		for (fact1 = fact2 = factor = 0; factor < Nfactors; factor++)
			{
			level[factor] = 0;
			if (member (factor, source))
				if (fact1)
					fact2 = factor;
				else
					fact1 = factor;
			}
		for (lev1 = 0; lev1 < Nlevels[fact1]; lev1++)
			for (lev2 = 0; lev2 < Nlevels[fact2]; lev2++)
				count[lev1][lev2] = 0;

		for (sources = TRUE; sources; sources = nextlevel (level, source, TRUE))
			{
			for (nonsources = TRUE; nonsources;
							nonsources = nextlevel (level, source, FALSE))
				{
				if (Nreplics[offset (level)])
					count[level[fact1]][level[fact2]]++;
				}
			}

		for (lev1 = 1; lev1 < Nlevels[fact1]; lev1++)
			{
			coeff = (double) count[lev1][0] / (double) count[0][0];
			for (lev2 = 1; lev2 < Nlevels[fact2]; lev2++)
				{
				test = (double) count[lev1][lev2] / (double) count[0][lev2];
				if (!fzero (test - coeff))
					return (TRUE);
				}
			}
		}

	return (FALSE); /* no problems here */
	}

/*FUNCTION nextlevel:	simulate a counting system based on Nlevels[factors] */
Boole
nextlevel (level, source, sourceflag)/* returns whether there are more levels */
int 	level[MAXFACT];       /* the current levels of each factor */
int 	source;               /* bit array of factors to (not) increment */
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

/*FUNCTION anova:	compute F statistics and print table */
/*
	anova assumes that the Bracket array has been allocated and set up with
	Bracket values as described in Keppel.  anova's task is to compute from
	these Bracket values the SS for each source, and then, with the array
	Between (which holds the type of each factor (WITHIN or BETWEEN)) gets
	the error term for each source.  anova prints for each testable
	source, a mini-F-table with its particular error term.
*/
#define	signedterm(nterms,sub,value) \
	((((nterms)-setsize(sub,Nfactors))%2) ? -(value) : (value))
anova ()
	{
	register int factor;          /* looping variable */
	int 	source;               /* source number/set */
	int 	nterms;               /* number of terms in source */
	register int subsource;
	int 	error;                /* will hold factors in error term */
	int 	nerror;               /* number of terms in error source */
	int 	betprod = 1;          /* product nlevels of between factors */
	int 	withprod = 1;         /* product nlevels within subj facts */
	double	sseffect, sserror;    /* sum of squares */
	int 	dfeffect, dferror;    /* degrees of freedom */
	double	mseffect, mserror;    /* mean square */
	double	F, p;                 /* F ratio and probability */
	double	omega2;               /* strength of effect */
	double	pre;                  /* proporational reduction in error */
	double	sstotal;              /* total sum of squares */
	double	pof ();               /* probability of F ratio */

	for (factor = 0; factor < Nfactors; factor++)
		if (Nlevels[factor] <= 1)
			ERRMSG1 (Too few levels of factor %s, Factorname[factor])
		else if (member (factor, Between))
			betprod *= Nlevels[factor];
		else
			withprod *= Nlevels[factor];
	
	sumdesign (withprod);

	if (Errorflag)
		ERRMSG0 (No F table due to previous fatal error)

	putchar ('\n');
	printf ("SOURCE                SS     df             MS         F      p");
	if (Debug)
		printf ("    PRE omega2");
	putchar ('\n');
	printf ("===============================================================");
	if (Debug)
		printf ("==============");
	putchar ('\n');

	/* if source is odd, then RANDOM will be in its source,
	   so we only want even sources for the main effects */
	sstotal = Bracket[Nsources-1] - Bracket[0];
	for (source = 0; source < Nsources; source += 2)
		{
		nterms = setsize (source, Nfactors);

		/* COMPUTE sseffect by adding brackets, alternating signs */
		sseffect = 0.0;
		for (subsource = 0; subsource <= source; subsource += 2)
			if (subset (subsource, source))
				sseffect += signedterm (nterms, subsource, Bracket[subsource]);
		if (fzero (sseffect))
			sseffect = 0.0;
		else if (sseffect <= -FZERO)
			ERRMSG1 (invalid SSeffect: %g, sseffect)

		dfeffect = 1;
		for (factor = 1; factor < Nfactors; factor++)
			if (member (factor, source))
				dfeffect *= Nlevels[factor] - 1;
		if (dfeffect <= 0)
			ERRMSG1 (invalid dfeffect: %d, dfeffect)

		/* the error term for a source factor is WxS/B,
		   where W is the set of all within subjects factors
		   IN THE SOURCE, and B is the set of ALL between
		   subject factors in the WHOLE design. S is the
		   subjects or RANDOM factor.  A bracket term is used
		   int the error term if it includes all between subject
		   factors, and if the only other factors it includes are
		   within subject factors or RANDOM.
		*/

		error = 0;
		enter (RANDOM, error); /* random factor always in the error term */
		error = join (error, Between);
		error = join (error, source);
		nerror = setsize (error, Nfactors);

		sserror = 0.0;
		for (subsource = 0; subsource < Nsources; subsource++)
			if (subset (subsource, error) && subset (Between, subsource))
				sserror += signedterm (nerror, subsource, Bracket[subsource]);
		if (sserror < FZERO && Debug == FALSE) /* continue when debugging */
			ERRMSG1 (invalid SSerror: %g, sserror)

		dferror = Nlevels[RANDOM] - betprod;
		for (factor = 1; factor < Nfactors; factor++)
			if (member (factor, source) && !member (factor, Between))
				dferror *= Nlevels[factor] - 1;
		if (dferror <= 0)
			ERRMSG1 (invalid dferror: %d: dferror, dferror)

		mseffect = sseffect / dfeffect;
		mserror = sserror / dferror;
		if (fzero (mserror))
			{
			F = MAXF;
			p = 0.0;
			WARNING (zero MSerror implies infinite F ratio)
			}
		else
			p = pof (F = mseffect/mserror, dfeffect, dferror);

		psourcename (source, nterms);
		printf ("%16.4f %6d %14.4f %9.3f %6.3f ",
			sseffect, dfeffect, mseffect, F, p);
		if (Debug)
			{
			if (source != 0)
				{
				pre = 1.0 / (dferror / (F * dfeffect) + 1.0); /* Gary McClelland */
				if (F > 1.0)
					omega2 = (sseffect - dfeffect * mserror) /
								(sstotal + mserror);
				else
					omega2 = 0.0;
				printf ("%6.3f %6.3f ", pre, omega2);
				}
			}
		if (p <= 0.05) putchar ('*');
		if (p <= 0.01) putchar ('*');
		if (p <= 0.001) putchar ('*');
		putchar ('\n');

		perrorname (error);
		printf ("%16.4f %6d %14.4f\n\n", sserror, dferror, mserror);
		}
	if (Debug)
	printf ("Total	%16.4f %6d\n", sstotal, withprod);
	}

/*FUNCTION sumdesign:	summarize the design information */
sumdesign (ndata)
int 	ndata;
	{
	int 	factor;
	char	*lformat = "%-8.8s: ";
	char	*sformat = "%10.10s ";
	char	*dformat = "%10d ";

	printf (lformat, "FACTOR");
	for (factor = 0; factor <= Nfactors; factor++)
		printf (sformat, Factorname[factor]);
	putchar ('\n');
	
	printf (lformat, "LEVELS");
	for (factor = 0; factor < Nfactors; factor++)
		printf (dformat, Nlevels[factor]);
	printf (dformat, ndata);
	putchar ('\n');
	
	printf (lformat, "TYPE");
	printf (sformat, "RANDOM");
	for (factor = 1; factor < Nfactors; factor++)
		if (member (factor, Between))
			printf (sformat, "BETWEEN");
		else
			printf (sformat, "WITHIN");
	printf (sformat, "DATA");
	putchar ('\n');
	
	if (NAcount)
		{
		printf (lformat, "MISSING");
		for (factor = 0; factor < Nfactors; factor++)
			printf (sformat, "");
		printf (dformat, NAcount);
		putchar ('\n');
		}
	}

/*FUNCTION pcellheader:	print the cellmeans source header information */
pcellheader (source, stats)
int 	source;    /* bit array holding all members of the source */
Boole	stats;     /* print header for stats? */
	{
	int 	factor;

	if (! member (RANDOM, source))
		{
		if (stats)
			{
			printf ("SOURCE: ");
			if (source == 0)
				printf ("grand mean");
			else
				for (factor = 1; factor < Nfactors; factor++)
					if (member (factor, source))
						printf ("%s ", Factorname[factor]);
			putchar ('\n');
			}

		for (factor = 1; factor < Nfactors; factor++)
			printf ("%-7.7s ", Factorname[factor]);
		if (stats)
			printf ("   N       MEAN         SD         SE");
		putchar ('\n');
		}
	}

/*FUNCTION effectname: cellmeans utility to print effect names */
effectname (source, level)
int 	source;
int 	level[];
	{
	int 	factor;
	for (factor = 1; factor < Nfactors; factor++)
		if (member (factor, source))
			printf ("%-7.7s ", Levelname[factor][level[factor]]);
		else
			printf ("%-7.7s ", "");
	}

/*FUNCTION pcellstats:	cellmeans utility to print summary stats */
pcellstats (count, sum, sumsq, minval, maxval)
double	sum, sumsq, minval, maxval;
	{
	if (count)
		{
		printf ("%4d %10.4f ", count, sum/count);
		if (count > 1) /* ok to compute sd */
			{
			double sd = sqrt ((sumsq-sum*sum/count)/(count-1.0));
			double se = sd / sqrt ((double) count);
			printf ("%10.4f %10.4f", sd, se);
			}
		}
	else
		{
		printf ("   Empty cells are not allowed!");
		Errorflag = TRUE;
		}
	putchar ('\n');
	}

/*FUNCTION pcellplot: cellmeans utility to print error plot */
pcellplot (count, sum, sumsq, minval, maxval, grandmin, grandmax)
int 	count;
double	sum, sumsq;
double	minval, maxval;
double	grandmin, grandmax; /* for sclaing the plot */
	{
	double	mean, sd = 0.0;
	char	*errplot ();
	char	*plot = NULL;
	if (count)
		{
		mean = sum/count;
		if (count > 1)
			sd = sqrt ((sumsq-sum*sum/count)/(count-1.0));
		plot = errplot (Width, grandmin, grandmax, minval, maxval, count, mean, sd, 1);
		printf ("|%s|", plot);
		}
	putchar ('\n');
	}

/*FUNCTION psourcename:	print the name of a source factor */
psourcename (source, nterms)
int 	source;
int 	nterms;
	{
	int 	factor;

	if (source == 0)
		printf ("mean");
	else for (factor = 1; factor < Nfactors; factor++)
		if (member (factor, source))
			if (nterms == 1)
				{
				printf ("%-7.7s", Factorname[factor]);
				break;
				}
			else
				printf ("%c", Factorname[factor][0]);
	printf ("\t");
	}

/*FUNCTION perrorname:	print the name of the error term in WS/B format */
perrorname (error)
int 	error;
	{
	int 	factor;

	for (factor = 1; factor < Nfactors; factor++)
		if (member (factor, error) && ! member (factor, Between))
			printf ("%c", Factorname[factor][0]);
	printf ("%c/", Factorname[RANDOM][0]);
	for (factor = 1; factor < Nfactors; factor++)
		if (member (factor, error) && member (factor, Between))
			printf ("%c", Factorname[factor][0]);
	printf ("\t");
	}


printlevels (ioptr)
FILE	*ioptr;
	{
	int 	maxlev = 0;
	int 	factor, level;

	for (maxlev = factor = 0; factor < Nfactors; factor++)
		{
		if (Nlevels[factor] > maxlev)
			maxlev = Nlevels[maxlev];
		fprintf (ioptr, "%-7.7s%c",
			Factorname[factor],
			factor == Nfactors-1 ? '\n' : '\t');
		}
	for (level = 0; level < maxlev; level++)
		for (factor = 0; factor < Nfactors; factor++)
			{
			fprintf (ioptr, "%-7.7s%c",
				Nlevels[factor] > level ? Levelname[factor][level] : "",
				factor == Nfactors-1 ? '\n' : '\t');
			}
	}

printarray (array, n)
int 	*array;
int 	n;
	{
	int 	i;
	for (i = 0; i < n; i++)
		fprintf (stderr, "%3d ", array[i]);
	putc ('\n', stderr);
	}

printbracket (source, value)
int 	source;
double	value;
	{
	int 	factor;
	fprintf (stderr, "[%d=", source);
	for (factor = 0; factor < Nfactors; factor++)
		if (member (factor, source))
			putc (Factorname[factor][0], stderr);
	fprintf (stderr, "]\t	=	%g\n", value);
	}

usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXFACT, "maximum number of factors");
		const (MAXLEV, "maximum number of factor levels");
		const (MAXBET, "maximum number of between factor levels");
		const (MAXCHARS, "maximum number of characters in lines");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		iopt ('p', "",         "show error bar plots for N-way tests", Plot);
		iopt ('w', " width",   "width of error bar plots", Width);
		oper ("[names]", "factor names", "RANDOM A B ... DATA");
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
