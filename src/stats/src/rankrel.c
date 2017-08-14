/* Copyright 1987 Gary Perlman */

#include "stat.h"

PGM(rankrel,Rank Order Analysis for Related Conditions,5.2,02/04/87)

#ifdef __STDC__
double cor (float *x, float *y, int n);
double critcor (int n, double p);
int ranksort (float *vec, float *svec, int *ivec, int n);
double	pof (double F, int df1, int df2);
#else
double cor ();         /* compute correlation */
double critcor ();     /* compute critical correlation value */
int ranksort ();       /* ranksort items */
double	pof ();        /* prob of F ratio */
#endif

/*
	matched groups median test
*/

/*
	rankrel centile getopt setreal number prodlist \
		primes chisq z setint ranksort ordstat
*/
#define	PFVEC(vec,count) \
	{ \
	int 	ind; \
	for(ind=0;ind<count;ind++) \
		printf("%g ", vec[ind]); \
	printf ("\n"); \
	}

/* critical N's over which to use approximations */
#define	NWILCOXON    16      /* according to Snedecor & Cochran */
#define	NSIGNTEST    25      /* according to Siegel */
#define	NSPEARMAN    10      /* according to Siegel */

#define	MAXCOND	     20       /* maximum number of conditions */
#define MAXCHARS BUFSIZ       /* maximum number of chars in lines */
#define	MAXDATA     100       /* max number of input data cases */
int 	Maxdata = MAXDATA;

char	*Condname[MAXCOND];   /* condition names */
float	*Condat[MAXCOND];     /* pointers into all data for each condition */
int 	Nconds;               /* number of conditions == ncols */
int 	Count;                /* number of data in each condition */
int 	NAcount[MAXCOND];     /* number of NA missing values */
int 	TNAcount;             /* number of missing cases */

/* OPTIONS */
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */
Boole	Rankmeans;            /* print average ranks for conditions? */
Boole	Yates = TRUE;         /* apply Yates' correction for continuity */
Boole	Docomps = TRUE;       /* do comparison tests? **/
#define	noteyates()	printf("\tNOTE: Yates' correction for continuity applied\n")

#define	statheader(header) printf ("\n%s:\n", header)

/*FUNCTION main: rankrel */
main (argc, argv) char **argv;
	{
	ARGV0;
	initial (argc, argv);
	checkstdin ();
	readdata ();
	summarize ();
	if (Nconds > 1)
		{
		if (Docomps)
			{
			if (Nconds == 2)
				{
				dosigntest (0, 1);
				dowilcoxon (0, 1);
				}
			dofriedman ();
			}
		dospearman ();
		}
	exit (SUCCESS);
	}

/*FUNCTION initial: set options and condition names */
initial (argc, argv) char **argv;
	{
	extern	char	*optarg;
	extern	int 	optind;
	int 	errflg = 0;
	int 	C;
	int 	cond;
	while ((C = getopt (argc, argv, "c:rsyLOV")) != EOF)
		switch (C)
			{
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			case 'c':
				if (setint (Argv0, 'c', optarg, &Maxdata, 1, MAXINT))
					errflg++;
				break;
			case 'r':
				Rankmeans = TRUE;
				break;
			case 's':
				Docomps = FALSE;
				break;
			case 'y':
				Yates = FALSE;
				break;
			default: errflg++; break;
			}
	if (errflg)
		USAGE ("[-ry] [-c maxcases] [names]")
	usinfo ();
	for (cond = 0; optind + cond < argc; cond++)
		{
		if (cond >= MAXCOND)
			ERRMANY (condition names,MAXCOND)
		Condname[cond] = argv[optind+cond];
		}
	}

/*FUNCTION usinfo: online help */
usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXCOND,  "maximum number of conditions");
		const (Maxdata,  "maximum number of input data points");
		const (MAXCHARS, "maximum number of characters in input lines");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		iopt ('c', "maxcases", "maximum number of input cases", Maxdata);
		lopt ('r', "print average ranks for conditions", Rankmeans);
		lopt ('s', "run comparative significance tests", Docomps);
		lopt ('y', "use Yates' correction for continuity", Yates);
		oper ("[names]", "condition names", "Cond-1 Cond-2 ...");
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (SUCCESS);
	}

/*FUNCTION newname: make a new default name for a condition */
char *
newname (cond)
int 	cond;
	{
	char	*name = myalloc (char, 10);
	(void) sprintf (name, "Cond-%d", cond);
	return (name);
	}

/*FUNCTION checktable: tell analyst to look for value in a table */
checktable (stat, parname, parval)
char	*stat;
char	*parname;
int 	parval;
	{
	printf ("\tCheck a table for %s", stat);
	if (parname && *parname)
		printf (" with %s = %d", parname, parval);
	putchar ('\n');
	}

/*FUNCTION: readdata: read in data into global arrays */
readdata ()
	{
	char	line[MAXCHARS];
	char	*sarray[MAXCOND];
	int 	ncols;
	int 	cond;
	Boole	dropcase;
	
	while (gets (line))
		{
		ncols = parselin (line, sarray, MAXCOND);
		if (ncols == 0)
			continue;
		dropcase = FALSE;
		for (cond = 0; cond < ncols; cond++)
			if (isna (sarray[cond]))
				{
				NAcount[cond]++;
				dropcase = TRUE;
				}
		if (dropcase == TRUE)
			{
			TNAcount++;
			continue;
			}
		if (Nconds == 0) /* initialize */
			{
			Nconds = ncols;
			if (Nconds > MAXCOND)
				ERRMANY (conditions,MAXCOND)
			for (cond = 0; cond < Nconds; cond++)
				{
				Condat[cond] = myalloc (float, Maxdata);
				if (Condat[cond] == NULL)
					ERRSPACE (data)
				}
			}
		if (Count == Maxdata)
			ERRMANY (cases,Maxdata)
		if (ncols != Nconds)
			ERRRAGGED
		for (cond = 0; cond < Nconds; cond++)
			if (number (sarray[cond]))
				Condat[cond][Count] = atof (sarray[cond]);
			else
				ERRNUM (sarray[cond],input value)
		Count++;
		}
	if (Count <= 1)
		ERRDATA
	for (cond = 0; cond < Nconds; cond++)
		if (Condname[cond] == NULL || *Condname[cond] == '\0')
			Condname[cond] = newname (cond+1);
	}

/*FUNCTION summarize: print summary statistics */
summarize ()
	{
	int 	cond;
	int 	i;
	int 	allcount = 0;
	float	*alldata = myalloc (float, Nconds * Count);
	int 	tna = 0;
	
	if (TNAcount)
		printf ("Cases with missing data: %d ignored\n", TNAcount);
	for (cond = 0; cond < Nconds; cond++)
		tna += NAcount[cond];
	for (cond = 0; cond < Nconds; cond++)
		{
		if (alldata)
			for (i = 0; i < Count; i++)
				alldata[allcount++] = Condat[cond][i];
		ordstat (Condat[cond], Count, cond, Condname[cond], NAcount[cond]);
		}
	if (alldata != NULL)
		{
		if (Nconds > 1)
			ordstat (alldata, Nconds*Count, Nconds, "Total", tna);
		free ((char *) alldata);
		}
	else
		WARNING (not enough space to compute grand median)
	}

/*FUNCTION dosigntest: compute sign test for two conditions */
/*
	ties are dropped and N is reduced
	if N > NSIGNTEST, then use normal approximation: apobin
	else use exact binomial probability
*/
dosigntest (cond1, cond2)
int 	cond1, cond2;
	{
	float	*data1 = Condat[cond1];
	float	*data2 = Condat[cond2];
	int 	N = 0;                   /* number of cases in test */
	int 	r = 0;                   /* number of cases first > second */
	int 	i;
	double	p;
	double	apobin (), pobin ();
	
	statheader ("Binomial Sign Test");
	
	for (i = 0; i < Count; i++)
		{
		if (data1[i] == data2[i])
			continue;
		if (data1[i] > data2[i])
			r++;
		N++;
		}
	printf ("\tNumber of cases %s is above %s: %3d\n",
		Condname[cond1], Condname[cond2], r);
	printf ("\tNumber of cases %s is below %s: %3d\n",
		Condname[cond1], Condname[cond2], N-r);
	if (r < N * 0.5)
		r = N - r;
	if (N > NSIGNTEST)
		{
		p = apobin (N, 1, 2, r);
		tprint ("One-tail probability approximation", p);
		noteyates ();
		}
	else
		{
		p = pobin (N, 1, 2, r);
		tprint ("One-tail probability (exact)", p);
		}
	}

/*FUNCTION dowilcoxon: compute Wilcoxon test for two conditions */
/*
	T = min (sumposranks, sumnegranks)
	if N > NWILCOXON, use normal approximation (seems good even when N = 8)
		mean = N (N+1) / 4
		se = sqrt (N (N+1) (2N+1) / 24 )
		z = (T - U) / se      (one tailed)
*/
dowilcoxon (cond1, cond2)
int 	cond1, cond2;
	{
	float	*data1 = Condat[cond1];
	float	*data2 = Condat[cond2];
	double	diff;                      /* data1 - data2 */
	float	*rank;                     /* absolute differences of conditions */
	Boole	*neg;                      /* is difference negative? */
	int 	i;                         /* loop varioable for cases */
	double	sumnegranks = 0.0;         /* sum of all negatively signed ranks */
	double	sumposranks = 0.0;         /* sum of all positively signed ranks */
	double	T;                         /* Wilcoxin statistic */
	int 	N = 0;                     /* number of non-zero differences */
	double	mean;
	double	se;
	double	z, poz ();
	
	rank = myalloc (float, Count);
	neg = myalloc (Boole, Count);
	if (rank == NULL || neg == NULL)
		ERRSPACE(Wilcoxon Test)
	
	statheader ("Wilcoxon Matched-Pairs Signed-Ranks Test");
	printf ("    Comparison of %s and %s\n", Condname[cond1], Condname[cond2]);
	for (i = 0; i < Count; i++)
		{
		diff = data1[i] - data2[i];
		if (!fzero (diff))
			{
			neg[N] = (diff < 0.0);
			rank[N] = fabs (diff);
			N++;
			}
		}
	if (N == 0)
		{
		printf ("\tHmmm, all these pairs seem to be the same.\n");
		return;
		}
	if (ranksort (rank, NULL, NULL, N))
		ERRMSG0 (could not rank data for Wilcoxon test)
	for (i = 0; i < N; i++)
		if (neg[i])
			sumnegranks += rank[i];
		else
			sumposranks += rank[i];
	T = min (sumnegranks, sumposranks);
	tprint ("T (smaller ranksum of like signs)", T);
	tprint ("N (number of signed differences)", (double) N);
	mean = N * (N+1) * 0.25;
	se = sqrt (N * (N+1) * (2.0 * N + 1.0) / 24.0);
	z = fabs (T - mean);
	if (Yates)
		{
		z -= 0.5;
		if (z < 0.0)
			z = 0.0;
		}
	z /= se;
	tprint ("z", z);
	tprint ("One-tail probability approximation", 1.0 - poz (z));
	if (Yates)
		noteyates ();
	if (N <= NWILCOXON)
		checktable ("T", "N", N);
	free ((char *) neg);
	free ((char *) rank);
	}

/*FUNCTION dofriedman: multi-condition test */
dofriedman ()
	{
	double	R = 0.0;            /* sum of squared sum of ranks in conds */
	double	chi_r;              /* function of R and formula */
	double	pochisq ();
	int 	i, cond;
	float	rank[MAXCOND];      /* ranks of each case computed here */
	double	sumrank[MAXCOND];   /* sum of ranks in a condition */
	/* the following two vectors save allocation time for ranksort */
	float	sorted[MAXCOND];    /* sorted condition values */
	int 	order[MAXCOND];     /* sorted order index numbers */
	Boole	needtable = FALSE;  /* should we recommend looking at a table? */
	
	for (cond = 0; cond < Nconds; cond++)
		sumrank[cond] = 0.0;
	for (i = 0; i < Count; i++)
		{
		for (cond = 0; cond < Nconds; cond++)
			rank[cond] = Condat[cond][i];
		if (ranksort (rank, sorted, order, Nconds))
			ERRMSG0 (could not rank data for Friedman test)
		for (cond = 0; cond < Nconds; cond++)
			sumrank[cond] += rank[cond];
		}
	if (Rankmeans)
		{
		statheader ("Average Ranks");
		for (cond = 0; cond < Nconds; cond++)
			printf ("\t\t%-12s %10.1f %10.2f\n",
				Condname[cond], sumrank[cond], sumrank[cond]/Count);
		}
	for (cond = 0; cond < Nconds; cond++)
		R += sumrank[cond] * sumrank[cond];
	chi_r = 12.0 / (Count*Nconds*(Nconds+1)) * R - (3 * Count * (Nconds+1));
	statheader ("Friedman Chi-Square Test for Ranks");
	tprint ("Chi-square of ranks", chi_r);
	chiprint (chi_r, Nconds-1, pochisq (chi_r, Nconds - 1));
	if (Nconds == 2) /* really, this is a two-tailed wilcoxon */
		needtable = TRUE;
	if (Nconds == 3 && Count <= 9)
		needtable = TRUE;
	if (Nconds == 4 && Count <= 4)
		needtable = TRUE;
	if (needtable)
		checktable ("Friedman", "N", Count);
	}

/*FUNCTION dospearman: compute & print rank-order correlation matrix */
/*
	WARNING: this procedure clobbers the original global data
*/	
dospearman ()
	{
	/* the following two vectors save allocation time for ranksort */
	float	*sorted;    /* sorted condition values */
	int 	*order;     /* sorted order index numbers */
	double	r;
	int 	cond;
	int 	cond2;
	
	order = myalloc (int, Count);
	sorted = myalloc (float, Count);
	if (order == NULL || sorted == NULL)
		ERRSPACE(Spearman Rho)
	for (cond = 0; cond < Nconds; cond++)
		if (ranksort (Condat[cond], sorted, order, Count))
			ERRMSG0 (could not rank data for Spearman Rho)
	
	statheader ("Spearman Rank Correlation (rho) [corrected for ties]");
	pcritrho (Count);
	if (Count < NSPEARMAN)
		checktable ("Spearman rho", "N", Count);
	if (Nconds == 2)
		{
		r = cor (Condat[0], Condat[1], Count);
		tprint ("rho", r);
		return;
		}
	
#define	LFORMAT    "%-8.8s "
#define	SFORMAT    "%8.8s "
#define	PFORMAT    "%8.3f "
	printf (LFORMAT, "");
	for (cond = 0; cond < Nconds; cond++)
		printf (SFORMAT, Condname[cond]);
	putchar ('\n');
	for (cond = 0; cond < Nconds; cond++)
		{
		printf (SFORMAT, Condname[cond]);
		for (cond2 = 0; cond2 < Nconds; cond2++)
			{
			if (cond == cond2)
				printf (SFORMAT, "");
			else
				{
				r = cor (Condat[cond], Condat[cond2], Count);
				printf (PFORMAT, r);
				}
			}
		putchar ('\n');
		}
	printf (LFORMAT, "");
	for (cond = 0; cond < Nconds; cond++)
		printf (SFORMAT, Condname[cond]);
	putchar ('\n');
	
	free ((char *) sorted);
	free ((char *) order);
	}

/*FUNCTION critcor: compute critical value for correlation */
/*
	t = r * sqrt (df / (1-r*r));
	F = r*r * df / (1-r*r)
	r = sqrt (F / (df + F))
*/
double
critcor (n, p)
int 	n;   /* number of points */
double	p;   /* significance level */
	{
	int 	df = n - 2;
	double	critf ();         /* critical F ratio */
	double	sqrt ();
	double	r;
	double	f;
	
	if (p <= 0.0 || p >= 1.0 || df < 1)
		return (0.0);
	f = critf (p, 1, df);
	r = sqrt (f / (df + f));
	return (r);
	}

pcritrho (n)
int 	n;
	{
	double	p05 = critcor (n, .05);
	double	p01 = critcor (n, .01);
	tprint ("Critical r (.05) t approximation", p05);
	tprint ("Critical r (.01) t approximation", p01);
	}
