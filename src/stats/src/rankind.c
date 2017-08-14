/* Copyright 1987 Gary Perlman */

#include "stat.h"

PGM(rankind,Rank Order Analysis for Independent Conditions,5.3,02/04/87)

#ifdef __TURBOC__
int ranksort (float *vec, float *svec, int *ivec, int n);
#else
int ranksort ();
#endif

/*
	rankind getword centile getopt setreal number fisher2x2 prodlist \
		primes chisq z setint fiveplot numline ranksort ordstat chitest
*/

#define	MAXCOND	     20       /* maximum number of conditions */
#define MAXCHARS     50       /* maximum number of chars in words */
#ifdef	macintosh
#define	MAXDATA     750       /* max number of input data points */
#else
#define	MAXDATA    1000       /* max number of input data points */
#endif

/* critical N's over which to use approximations */
#define	NMANNWHITNEY      20  /* according to Siegel */
#define	N1KRUSKALWALLICE   5  /* for k = 3, any n > 5 --> not (small) */
#define	N2KRUSKALWALLICE  20  /* any n > 20 --> not (small) */
#define	NFISHER          100  /* largest total cell freq for exact test */
#define	SMALLCELL        5.0  /* anything smaller should be noted */

char	*Condname[MAXCOND];   /* condition names */
int 	Condno;               /* condition number */
int 	Nconds;               /* number of conditions == Condno + 1 */
int 	Count[MAXCOND];       /* number of data in each condition */
int 	NAcount[MAXCOND];     /* number of NA in each condition */
float	Mins[MAXCOND];
float	Quart1[MAXCOND];
float	Median[MAXCOND];      /* fivenums */
float	Quart3[MAXCOND];
float	Maxs[MAXCOND];
float	Alldata[MAXDATA];     /* all the data stored in here */
int 	Allcount;             /* total number of data points */
float	*Condat[MAXCOND];     /* pointers into Alldata for each condition */
float	Grandmin;             /* min of all the data */
float	Grandmedian;          /* median of all the data */
float	Grandmax;             /* max of all the data */

/* OPTIONS */
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */
double	Splitter = (-1.0);    /* value used to separate condition */
Boole	Rankmeans;            /* print average ranks? */
Boole	Plot;                 /* should we plot? */
Boole	Plotwidth = 60;       /* width of plot */
Boole	Yates = TRUE;         /* apply Yates' correction for continuity */

#define	statheader(header) printf ("\n%s:\n", header)
float	Rank[MAXDATA];
float	Sorted[MAXDATA];
double	Sumrank[MAXCOND];


main (argc, argv) char **argv;
	{
	int 	cond;
	 
	ARGV0;
	initial (argc, argv);
	checkstdin ();
	readdata ();
	summarize ();

	if (Plot)
		doplot ();
	if (Nconds > 1)
		{
		domedian ();
		for (cond = 0; cond < Allcount; cond++)
			Rank[cond] = Alldata[cond];
		if (ranksort (Rank, Sorted, NULL, Allcount))
			ERRMSG0 (could not rank data for rank order tests)
		averanks (Rank, Sumrank, Rankmeans);
		if (Nconds == 2) /* two conditions */
			domannwhitney (Sorted, Sumrank);
		dokruskalwallice (Sorted, Sumrank);
		}
	exit (SUCCESS);
	}


initial (argc, argv) char **argv;
	{
	extern	char	*optarg;
	extern	int 	optind;
	int 	errflg = 0;
	int 	C;
	int 	cond;
	while ((C = getopt (argc, argv, "prs:w:yLOV")) != EOF)
		switch (C)
			{
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			case 'p':
				Plot = TRUE;
				break;
			case 'r':
				Rankmeans = TRUE;
				break;
			case 's':
				if (setreal (Argv0, 's', optarg, &Splitter))
					errflg++;
				break;
			case 'w':
				if (setint (Argv0, 'P', optarg, &Plotwidth, 10, 100))
					errflg++;
				Plot = TRUE;
				break;
			case 'y': Yates = FALSE; break;
			default: errflg++; break;
			}
	if (errflg)
		USAGE ("[-pry] [-w plotwidth] [-s splitter] [names]")
	usinfo ();
	for (cond = 0; optind + cond < argc; cond++)
		{
		if (cond >= MAXCOND)
			ERRMANY (condition names,MAXCOND)
		Condname[cond] = argv[optind+cond];
		}
	}


usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXCOND,  "maximum number of conditions");
		const (MAXDATA,  "maximum number of input data points");
		const (MAXCHARS, "maximum number of characters in input words");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('p', "print fivenum plots", Plot);
		lopt ('r', "print average ranks for conditions", Rankmeans);
		ropt ('s', "splitter", "condition separator value", Splitter);
		iopt ('w', "width", "plot width", Plotwidth);
		lopt ('y', "use Yates' correction for continuity", Yates);
		oper ("[names]", "condition names", "Cond-1 Cond-2 ...");
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (SUCCESS);
	}


char *
newname (cond)
int 	cond;
	{
	char	*name = myalloc (char, 10);
	(void) sprintf (name, "Cond-%d", cond);
	return (name);
	}


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

readdata ()
	{
	char	word[MAXCHARS];     /* string input data read in here */
	double	datum;              /* each data value */
	int 	cond;               /* loop index for conditions */
	
	Condat[0] = &Alldata[0];
	while (getword (word, stdin))
		{
		if (isna (word))
			{
			NAcount[Condno]++;
			continue;
			}
		if (!number (word))
			ERRNUM (word,input data)
		datum = atof (word);
		if (datum == Splitter) /* new condition */
			{
			if (Count[Condno] != 0) /* ignore superfluous leading splitter(s) */
				{
				Condno++;
				Condat[Condno] = &Alldata[Allcount];
				}
			}
		else /* real datum */
			{
			if (Allcount >= MAXDATA)
				ERRMANY (data points,MAXDATA)
			if (Condno >= MAXCOND)
				ERRMANY (conditions,MAXCOND)
			Alldata[Allcount] = datum;
			Count[Condno]++;
			Allcount++;
			}
		}
	if (Count[Condno] == 0)
		Condno--;
	Nconds = Condno + 1;
	for (cond = 0; cond < Nconds; cond++)
		if (Condname[cond] == NULL || *Condname[cond] == '\0')
			Condname[cond] = newname (cond+1);
	}


doplot ()
	{
	char	*fiveplot ();
	int 	cond;
	
	putchar ('\n');
	for (cond = 0; cond < Nconds; cond++)
		{
		printf ("%-9.9s |%s|\n", Condname[cond],
			fiveplot (Mins[cond], Quart1[cond], Median[cond],
				Quart3[cond], Maxs[cond], Plotwidth, Grandmin, Grandmax));
		}
	printf ("%-9.9s  ", "");
	numline (Grandmin, Grandmax, Plotwidth);
	}


domedian ()
	{
	int 	above[MAXCOND];      /* count of values above Grandmedian */
	int 	below[MAXCOND];      /* count of values below Grandmedian */
	int 	*matrix[2];          /* matrix of above/below */
	char	*rowname[2];         /* names of above/below rows */
	int 	tabove = 0;          /* total above Grandmedian */
	int 	tbelow = 0;          /* total below Grandmedian */
	int 	n;                   /* total scores not tied */
	int 	i;
	int 	cond;
	float	condval;             /* a value in a condition */
	
	statheader ("Median-Test");
	for (cond = 0; cond < Nconds; cond++)
		{
		above[cond] = 0;
		below[cond] = 0;
		for (i = 0; i < Count[cond]; i++)
			{
			condval = Condat[cond][i];
			if (condval < Grandmedian)
				below[cond]++;
			else if (condval > Grandmedian)
				above[cond]++;
			}
		tabove += above[cond];
		tbelow += below[cond];
		}
	n = tabove + tbelow;
	/* 2 x 2 design, use Fisher exact test if N <= NFISHER */
	if (n == 0)
		{
		printf ("\tHmmm, all these numbers seem to equal the median\n");
		return;
		}
	if (Nconds == 2 && n <= NFISHER)
		fishtest (above[0], above[1], below[0], below[1]);
	matrix[0] = above;
	matrix[1] = below;
	rowname[0] = "above";
	rowname[1] = "below";
	chitest (matrix, rowname, Condname, 2, Nconds, Yates);
	}


domannwhitney (sorted, sumrank)
float	*sorted;    /* sorted data */
double	*sumrank;   /* sum of all ranks in conditions */
	{
	int 	n1 = Count[0];
	int 	n2 = Count[1];
	int 	bign = max (n1, n2);
	int 	n1n2 = n1*n2;
	double	R1 = sumrank[0];
	double	U;                  /* U statistic for Ux, x is larger group */
	double	mean = n1n2*0.5;    /* expected mean for U */
	double	sqrt ();
	double	sd;                 /* expected sd (will be corrected with T) */
	double	T, tiecorrect ();   /* correction factor for ties */
	double	poz ();
	double	z;                  /* Z statistic for U */

	statheader ("Mann-Whitney U");

	U = n1n2 + n1*(n1+1)*0.5 - R1;
	if (U < mean)
		U = n1n2 - U;
		
	tprint ("U", U);
	tprint ("U'", n1n2 - U);
	
	/* normal approximation */
	T = sqrt (tiecorrect (sorted, Allcount));
	if (fzero (T))
		{
		printf ("\tHmmm, all these numbers seem tied\n");
		return;
		}
	sd = sqrt ( T * n1n2 * (n1+n2+1) / 12.0 );
	z = (U - mean) / sd;
	tprint ("z(U) (corrected for ties)", z);
	tprint ("One tailed p(z(U))", 1.0 - poz (z));
	
	if (bign <= NMANNWHITNEY)
		checktable ("U", "n", bign);
	}


dokruskalwallice (sorted, sumrank)
float	*sorted;    /* sorted data */
double	*sumrank;   /* sum of all ranks in conditions */
	{
	int 	cond;
	double	H;                      /* computed statistic */
	double	T, tiecorrect ();       /* correction factor for ties */
	double	pochisq ();
	Boole	smallsample = TRUE;     /* send the analyst to the tables */
	
	statheader ("Kruskal-Wallis");
	
#ifdef	NEVER
	/* This operation from BMD '79, page 612, did not work */
	if (Nconds == 2) /* report Mann-Whitney U */
		{
		double	U;
		U = sumrank[0] - Count[0] * (Count[0]+1.0) / 2.0;
		tprint ("U (not corrected for ties)", U);
		}
#endif	NEVER
	H = 0.0;
	for (cond = 0; cond < Nconds; cond++)
		H += sumrank[cond] * sumrank[cond] / Count[cond];
	H *= 12.0 / (Allcount * (Allcount + 1.0));
	H -= 3.0 * (Allcount + 1);
	tprint ("H (not corrected for ties)", H);
	T = tiecorrect (sorted, Allcount);
	if (fzero (T))
		{
		printf ("\tHmmm, all these numbers seem tied\n");
		return;
		}
	tprint ("Tie correction factor", T);
	H /= T;
	tprint ("H (corrected for ties)", H);
	if (Nconds > 3)
		smallsample = FALSE;
	else if (Nconds == 3)
		{
		for (cond = 0; cond < Nconds; cond++)
			if (Count[cond] > N1KRUSKALWALLICE)
				smallsample = FALSE;
		}
	else
		for (cond = 0; cond < Nconds; cond++)
			if (Count[cond] > N2KRUSKALWALLICE)
				smallsample = FALSE;
		
	chiprint (H, Nconds-1, pochisq (H, Nconds - 1));
	if (smallsample)
		checktable ("Kruskal-Wallis H", NULL, 0);
	}


printvec (v, n)
float	*v;
int 	n;
	{
	int 	i;
	for (i = 0; i < n; i++)
		printf ("%g ", v[i]);
	putchar ('\n');
	}

/*FUNCTION averanks: report average sum ranks */
averanks (rank, sumrank, display)
float	*rank;        /* input ranks */
double	*sumrank;     /* output sums of ranks */
Boole	display;
	{
	int 	cond;
	int 	i, j;

	i = 0;
	for (cond = 0; cond < Nconds; cond++)
		{
		sumrank[cond] = 0.0;
		for (j = 0; j < Count[cond]; j++)
			sumrank[cond] += rank[i++];
		}

	if (display)
		{
		statheader ("Average Ranks");
		for (cond = 0; cond < Nconds; cond++)
			{
			printf ("\t\t%-12s %4d %10.2f\n",
				Condname[cond], Count[cond], sumrank[cond]/Count[cond]);
			}
		}
	}

/*FUNCTION tiecorrect: compute tie correction dividing factor for U and H */
double
tiecorrect (sorted, n)
float	*sorted;                 /* vector of sorted values */
int 	n;                       /* number of values in sorted vector */
	{
	double	T = 0.0;             /* correction factor for ties */
	int 	nties;               /* number of ties at a particular rank */
	int 	i;
	
	if (n <= 1)
		return (1.0);

	for (i = 0; i < n-1; i++)
		{
		if (sorted[i] == sorted[i+1])
			{
			nties = 1;
			while (i < n-1 && sorted[i] == sorted[i+1])
				{
				nties++;
				i++;
				}
			T += (double) nties * (double) nties * (double) nties - nties;
			}
		}
	T /= (double) n * (double) n * (double) n - n;
	T = 1.0 - T;
	return (T);
	}


summarize ()
	{
	int 	cond;
	double	*ordstat ();       /* returns fivenums in vector */
	double	*fivenum;          /* returned by ordstat */
	int 	tnacount = 0;      /* total of NA counts */
	for (cond = 0; cond < Nconds; cond++)
		tnacount += NAcount[cond];
	for (cond = 0; cond < Nconds; cond++)
		{
		fivenum = ordstat (Condat[cond], Count[cond], cond, Condname[cond], NAcount[cond]);
		Mins[cond]   = fivenum[0];
		Quart1[cond] = fivenum[1];
		Median[cond] = fivenum[2];
		Quart3[cond] = fivenum[3];
		Maxs[cond]   = fivenum[4];
		}
	if (Nconds > 1)
		fivenum = ordstat (Alldata, Allcount, Nconds, "Total", tnacount);
	Grandmin = fivenum[0];
	Grandmedian = fivenum[2];
	Grandmax = fivenum[4];
	}
