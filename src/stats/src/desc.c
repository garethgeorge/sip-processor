/*  Copyright 1979 Gary Perlman */

#include "stat.h"
PGM(desc,Descriptive Statistics and Histograms,5.2,01/20/87)

#ifdef __STDC__
	double centile (int perc, float *v, int n);
	int bindex (float xval);
	int fltcmp (float *f1, float *f2);
#	define	vprint(label,format,var) printf (#label " = %" #format "\n", var)
#else
	double	centile ();                 /* percentile function */
	int bindex ();
	int fltcmp ();
#	define	vprint(label,format,var) printf ("label = %format\n", var)
#endif

/*
	This program analyses single distributions of data.
	It was first written by Gary Perlman at UCSD August 1979.
*/

#ifdef SMALL_MEM
#define	MAXBINS      250       /* maximum number of bins for tables */
#define	MAXPOINTS   2500       /* maximum number of input points if storing */
#elif macintosh
#define	MAXBINS      500       /* maximum number of bins for tables */
#define	MAXPOINTS   5000       /* maximum number of input points if storing */
#else
#define	MAXBINS     1000       /* maximum number of bins for tables */
#define	MAXPOINTS  10000       /* maximum number of input points if storing */
#endif
#define MAXCHARS      50       /* maximum number of chars in words */

Boole	Stats;                /* print statistics */
Boole	Table;                /* print a table of some sort */
Boole	Histogram;            /* print histogram */
Boole	Frequencies;          /* print frequency table */
Boole	Proportions;          /* print proportions table */
Boole	Cumulative;           /* make table cumulative */
Boole	Storedata;            /* store the data */
Boole	Onepass;              /* can run with one pass */
Boole	Setmaximum;           /* maximum value has been set */
Boole	Setminimum;           /* minimum value has been set */
Boole	Setintwidth;          /* interval width has been set */
Boole	Variable;             /* print stats in variable format */
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */

float	Datax[MAXPOINTS];     /* data stored in here */
int 	freq[MAXBINS];        /* frequency counts stored in here */
int 	N;                    /* number of points read in */
double	Sum;                  /* sum of all points read in */
double	s2;                   /* sum of squares of all points read in */
double	s3;                   /* sum of cubes of all points read in */
double	s4;                   /* sum of x^4 of all points read in */
double	Gmean;                /* geometric mean */
double	Hmean;                /* harmonic mean */
Boole	Allgtzero = TRUE;     /* all points greater than zero? */
double	F_null;               /* null value for t test */
double	Minx;                 /* min value of x */
double	Maxx;                 /* max value of x */
int 	NAcount = 0;          /* number of NA missing values */
int 	Undermin;             /* number of points less than minimum */
int 	Overmax;              /* number of points more than maximum */
double	Intwidth;             /* width of interval of frequency count bins */
double	Minimum;              /* minimum allowable value of x */
double	Maximum;              /* maximum allowable value of x */

main (argc, argv) int argc; char *argv[];
	{
	ARGV0;
	initial (argc, argv);
	checkstdin ();
	input ();
	if (Stats)
		printstats ();
	if (Table)
		printtable ();
	exit (0);
	}

initial (argc, argv) int argc; char **argv;
	{
	extern	int 	optind;
	extern	char 	*optarg;
	int 	C;
	int 	opterr = 0;
	if (argc == 1)
		{
		Storedata = TRUE;
		Stats = TRUE;
		return;
		}
	while ((C = getopt (argc, argv, "cfF:t:hi:m:M:posvOVL")) != EOF)
		switch (C)
		{
		case 'O': InfoOptions = TRUE; break;
		case 'V': InfoVersion = TRUE; break;
		case 'L': InfoLimits = TRUE; break;
		case 'c': Cumulative = TRUE; break;
		case 'f': Frequencies = Table = TRUE; break;
		case 't':
		case 'F':
			if (setreal (Argv0, C, optarg, &F_null))
				opterr++;
			Stats = TRUE;
			break;
		case 'h': Histogram = Frequencies = Table = TRUE; break;
		case 'i':
			if (setreal (Argv0, C, optarg, &Intwidth))
				opterr++;
			if (Intwidth < FZERO)
				{
				fprintf (stderr, "%s: interval width (%g) must be > 0\n",
					Argv0, Intwidth);
				opterr++;
				}
			Setintwidth = Table = TRUE;
			break;
		case 'm':
			if (setreal (Argv0, C, optarg, &Minimum))
				opterr++;
			Setminimum = TRUE;
			break;
		case 'M':
			if (setreal (Argv0, C, optarg, &Maximum))
				opterr++;
			Setmaximum = TRUE;
			break;
		case 'p': Proportions = Table =  TRUE; break;
		case 'o': Storedata = TRUE;
		case 's': Stats = TRUE; break;
		case 'v': Variable = Stats = TRUE; break;
		default: opterr++;
		}
	if (opterr)
		USAGE ("[-cfhopsv] [-i interval] [-m min] [-M max]  [-F|-t Ho]")
	usinfo ();
	ERROPT (optind)
	if (Table)
		{
		if (Setintwidth && Setminimum)
			Onepass = TRUE;
		else
			Storedata = TRUE;
		if (!Frequencies && !Proportions)
			Histogram = TRUE;
		}
	else if (Setminimum || Setmaximum)
		Stats = TRUE;
	}

input ()
	{
	double	x;                     /* each datum read in here */
	double	x2;                    /* square of x */
	char	stringx[MAXCHARS];     /* string version of x read in here */
	while (getword (stringx, stdin))
		{
		if (isna (stringx))
			{
			NAcount++;
			continue;
			}
		if (!number (stringx))
			ERRNUM (stringx,input value)
		x = atof (stringx);
		if (Setminimum && x < Minimum)
			{
			Undermin++;
			continue;
			}
		if (Setmaximum && x > Maximum)
			{
			Overmax++;
			continue;
			}
		if (N == 0)
			{
			Maxx = x;
			Minx = x;
			}
		if (Storedata)
			if (N == MAXPOINTS)
				{
				WARNING (too much data for storing)
				Storedata = FALSE;
				}
			else
				Datax[N] = x;
		if (Onepass)
			freq[bindex((float) x)]++;
		x2 = x*x;
		Sum += x;
		s2  += x2;
		s3  += x2*x;
		s4  += x2*x2;
		if (Allgtzero && x > FZERO)
			{
			Gmean += log (x);
			Hmean += 1.0 / x;
			}
		else
			Allgtzero = FALSE;
		if (x > Maxx)
			Maxx = x;
		if (x < Minx)
			Minx = x;
		N++;
		}
	if (N <= 1)
		ERRDATA
	}

int
fltcmp (f1, f2)
float *f1, *f2;
	{
	if (*f1 < *f2)
		return (-1);
	if (*f1 == *f2)
		return (0);
	return (1);
	}

printstats ()
	{
	double	pof ();                     /* probability of F ratio */
	double	M	= Sum/N;                /* mean */
	double	M2	= M*M;                  /* square of mean */
	double	var	= (s2 - M*Sum)/(N-1);   /* variance */
	double	sd	= sqrt (var);           /* standard deviation */
	double	sk;                         /* skew */
	double	kt;                         /* kurtosis */
	double	q1, q3;                     /* first and third quartiles */
	double	median;                     /* 50th percentile */
	char	*line	=
	"------------------------------------------------------------";
	double	tval, fval, prob;
	if (var < FZERO)
		ERRMSG2 (All these %d numbers equal %.4g, N, M)
	sk = (s3 - 3.0*M*s2 + 3.0*M2*Sum - M2*Sum)/(N*var*sd);
	kt = (s4-4.*M*s3+6.*M2*s2-4.*M2*M*Sum+N*M2*M2)/(N*var*var);
	if (Storedata)
		{
		qsort ((char *) Datax, N, sizeof (float), fltcmp);
		median = centile (50, Datax, N);
		q1 = centile (25, Datax, N),
		q3 = centile (75, Datax, N);
		}
	/*	PRINT FREQUENCY COUNTS */
	if (!Variable)
		puts (line);
	if (Variable)
		{
		vprint (undermin,d,Undermin);
		vprint (count,d,N);
		vprint (overmax,d,Overmax);
		vprint (missing,d,NAcount);
		vprint (sum,g,Sum);
		vprint (sumsq,g,s2);
		}
	else
		{
		printf ("%12s%12s%12s%12s%12s\n",
			"Under Range", "In Range", "Over Range", "Missing", "Sum");
		printf ("%12d%12d%12d%12d%12.3f\n", Undermin, N, Overmax, NAcount, Sum);
		puts (line);
		}
	/*	PRINT CENTRAL TENDENCY */
	if (Variable)
		{
		vprint (mean,g,M);
		if (Storedata)
			vprint (median,g,median);
		vprint (midpoint,g,(Maxx+Minx)/2.0);
		if (Allgtzero == TRUE)
			{
			vprint (geomean,g,exp (Gmean/N));
			vprint (harmean,g,N/Hmean);
			}
		}
	else
		{
		printf ("%12s%12s%12s%12s%12s\n",
			"Mean", "Median", "Midpoint", "Geometric", "Harmonic");
		printf ("%12.3f", M);
		if (Storedata)
			printf ("%12.3f", median);
		else
			printf ("%12s", "");
		printf ("%12.3f", (Maxx+Minx)/2.0);
		if (Allgtzero == TRUE)
			printf("%12.3f%12.3f\n", exp (Gmean/N), N/Hmean);
		else
			putchar ('\n');
		puts (line);
		}
	/*	PRINT VARIABILITY */
	if (Variable)
		{
		vprint (sd,g,sd);
		if (Storedata)
			vprint (quartdev,g,(q3-q1)/2.0);
		vprint (range,g,Maxx-Minx);
		vprint (semean,g,sqrt (var/N));
		}
	else
		{
		printf ("%12s%12s%12s%12s\n", "SD", "Quart Dev", "Range", "SE mean");
		printf("%12.3f", sd);
		if (Storedata)
			printf ("%12.3f", (q3-q1)/2.0);
		else
			printf ("%12s", "");
		printf ("%12.3f", Maxx-Minx);
		printf ("%12.3f\n", sqrt(var/N));
		puts (line);
		}
	/*	PRINT FIVENUMS */
	if (Variable)
		{
		vprint (min,g,Minx);
		if (Storedata)
			{
			vprint (q1,g,q1);
			vprint (q2,g,median);
			vprint (q3,g,q3);
			}
		vprint (max,g,Maxx);
		}
	else
		{
		printf ("%12s", "Minimum");
		if (Storedata)
			printf ("%12s%12s%12s", "Quartile 1", "Quartile 2", "Quartile 3");
		printf ("%12s\n", "Maximum");
		printf ("%12.3f", Minx);
		if (Storedata)
			printf ("%12.3f%12.3f%12.3f", q1, median, q3);
		printf ("%12.3f\n", Maxx);
		puts (line);
		}

	if (Variable)
		{
		vprint (skew,g,sk);
		vprint (kurt,g,kt);
		}
	else
		{
		printf ("%12s%12s%12s%12s\n", "Skew", "SD Skew", "Kurtosis", "SD Kurt");
		printf ("%12.3f%12.3f%12.3f%12.3f\n",
			sk, sqrt (6.0/N), kt, sqrt (24.0/N));
		puts (line);
		}

	tval = (M - F_null)/(sqrt (var/N));
	fval = tval*tval;
	prob = pof (fval, 1, N-1);
	if (Variable)
		{
		vprint (nullmean,g,F_null);
		vprint (t,g,tval);
		vprint (probt,g,prob);
		vprint (F,g,fval);
		vprint (probF,g,prob);
		}
	else
		{
		printf ("%12s%12s%12s%12s%12s\n",
			"Null Mean", "t", "prob (t)", "F", "prob (F)");
		printf ("%12.3f%12.3f%12.3f%12.3f%12.3f\n", 
			F_null, tval, prob, fval, prob);
		puts (line);
		}
	}

printtable ()
	{
	register int point;            /* looping variable */
	register int i;                /* looping variable */
	int 	maxindex;              /* maximum index for Maxx */
	double	midpoint;              /* midpoint of each interval */
	int 	cumf	= 0;           /* cumulative frequency */
	double	fcumf	= 0.0;         /* floating cumulative frequency */
	if (!Setminimum)
		Minimum = floor (Minx);
	if (!Setmaximum)
		Maximum = Maxx;
	if (!Setintwidth)
		{
		Intwidth = (Maxx-Minimum)/sqrt(2.0*N);
		if (fabs (Intwidth) > 1.0)
			Intwidth = floor (Intwidth);
		}
	if (!Onepass)
		for (point=0; point<N; point++)
			freq[ bindex ( Datax[point] ) ]++;
	midpoint = Minimum - Intwidth/2.0;
	maxindex = bindex ((float) Maximum);
	printf ("%12s", "Midpt");
	if (Frequencies)
		{
		printf ("%8s", "Freq");
		if (Cumulative)
			printf ("%8s", "Cum");
		}
	if (Proportions)
		{
		printf ("%8s", "Prop");
		if (Cumulative)
			printf ("%8s", "Cum");
		}
	putchar ('\n');
	for (i = 0; i <= maxindex; i++)
		{
		printf ("%12.3f", midpoint += Intwidth);
		if (Frequencies)
			{
			printf ("%8d", freq[i]);
			if (Cumulative)
				printf ("%8d", cumf += freq[i]);
			}
		if (Proportions)
			{
			printf ("%8.3f", freq[i]*1.0/N);
			if (Cumulative)
				{
				fcumf += freq[i];
				printf ("%8.3f", fcumf/N);
				}
			}
		if (Histogram)
			{
			putchar (' ');
			for (point = 1; point <= freq[i]; point++)
				putchar ('*');
			}
		putchar ('\n');
		}
	}

#ifdef __STDC__
int
bindex (float xval)
#else
int
bindex (xval)
float	xval;
#endif
	{
	int 	answer;
	float	findex;
	if (xval == Minimum)
		return (0);
	findex = (xval - Minimum)/Intwidth;
	if (floor (findex) == findex)
		answer = findex - 1.0;
	else
		answer = findex;
	if (answer >= MAXBINS)
		{
		ERRMSG1 (bin[%d] is out of range, answer)
		}
	return (answer);
	}

usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXPOINTS, "maximum number of data points");
		const (MAXBINS,   "maximum number of frequency bins");
		const (MAXCHARS,  "maximum number of characters in input numbers");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('c', "cumulative frequencies or proportions", Cumulative);
		lopt ('f',         "request table of frequencies", Frequencies);
		ropt ('F', "Ho",   "F-test against mean Ho",  F_null);
		lopt ('h',         "request a histogram", Histogram);
		ropt ('i', "width","interval width for tables & histograms", Intwidth);
		ropt ('m', "min",  "minimum allowable value", Minimum);
		ropt ('M', "max",  "maximum allowable value", Maximum);
		lopt ('o',         "request order statistics", Storedata);
		lopt ('p',         "request table of proportions", Proportions);
		lopt ('s',         "request summary statistics", Stats);
		ropt ('t', "Ho",   "t-test against mean Ho",  F_null);
		lopt ('v',         "print statistics in name=value format", Variable);
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
