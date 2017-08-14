/*  Copyright 1985 Gary Perlman */

/* use splitter as string group labels */

#include "stat.h"
PGM(oneway,One-Way Between Groups t-Test/ANOVA,5.5,1/20/87)

main (argc, argv) char **argv;
	{
	ARGV0;
	initial (argc, argv);
	checkstdin ();
	readdata ();
	printstats ();
	doplot ();
	oneway ();
	exit (0);
	}

#define	MAXGROUP     20
#define MAXCHARS     50      /* maximum number of chars in words */
char	*Name[MAXGROUP];     /* names of the different groups */
int 	Ngroups = 0;         /* number of groups of data */
int 	Count[MAXGROUP];     /* number of cells in each group */
int 	NAcount[MAXGROUP];   /* count of NA values by group */
double	Sum[MAXGROUP];       /* sum of all values in the group */
double	Sumsq[MAXGROUP];     /* sums of squares */
double	Mins[MAXGROUP];      /* min value in the group */
double	Maxs[MAXGROUP];      /* max value in the group */
double	Mean[MAXGROUP];      /* mean of each group */
double	Standev[MAXGROUP];   /* standard deviation of each group */

/* OPTIONS */
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */
double	Splitter = -1.0;      /* used to indicate group differences */
Boole	Ttest = FALSE;        /* Ttest format for two group case */
Boole	Unweighted;           /* type of solution */
Boole	Plot = FALSE;         /* should plot be printed? */
int 	Plotwidth = 60;       /* width of error bar plot */

initial (argc, argv) char **argv;
	{
	extern	char	*optarg;
	extern	int 	optind;
	int 	errflg = 0;
	int 	C;
	int 	group;
	while ((C = getopt (argc, argv, "ptuw:s:LOV")) != EOF)
		switch (C)
			{
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			case 'p':
				Plot = TRUE;
				break;
			case 's':
				if (setreal (Argv0, 's', optarg, &Splitter))
					errflg++;
				break;
			case 'u': /* unweighted means solution */
				Unweighted = TRUE;
				break;
			case 'w':
				if (setint (Argv0, 'w', optarg, &Plotwidth, 10, 100))
					errflg++;
				Plot = TRUE;
				break;
			case 't': /* t-test format */
				Ttest = TRUE;
				break;
			default: errflg++; break;
			}
	if (errflg)
		USAGE ("[-ptu] [-w plotwidth] [-s splitter] [names]")
	usinfo ();
	for (group = 0; optind + group < argc; group++)
		Name[group] = argv[optind+group];
	}

char *
newname (group)
int 	group;
	{
	char	*name = myalloc (char, 10);
	(void) sprintf (name, "Group-%d", group);
	return (name);
	}

readdata ()
	{
	char	word[MAXCHARS];     /* string input data read in here */
	double	datum;              /* each data value */
	int 	grindex = 0;        /* group index == Ngroups - 1 */
	while (getword (word, stdin))
		{
		if (isna (word))
			{
			NAcount[grindex]++;
			continue;
			}
		if (!number (word))
			ERRNUM (word,input data)
		datum = atof (word);
		if (datum == Splitter) /* new group */
			{
			if (Count[grindex] != 0) /* until some data read for this group */
				grindex = Ngroups++;
			}
		else /* real datum */
			{
			if (grindex >= MAXGROUP)
				ERRMANY (groups,MAXGROUP)
			if (Count[grindex] == 0)
				{
				if (Ngroups == 0)
					Ngroups = 1;
				Mins[grindex] = Maxs[grindex] = datum;
				}
			else
				{
				if (datum < Mins[grindex])
					Mins[grindex] = datum;
				else if (datum > Maxs[grindex])
					Maxs[grindex] = datum;
				}
			Sum[grindex] += datum;
			Sumsq[grindex] += datum * datum;
			if (Name[grindex] == NULL)
				Name[grindex] = newname (grindex+1);
			Count[grindex]++;
			}
		}

	if (Count[grindex] == 0) /* ignore empty last group */
		Ngroups--;
	if (Ngroups > MAXGROUP)
		ERRMANY (groups,MAXGROUP)
	if (Ngroups == 0)
		ERRDATA
	}

#define	var(count,sum,ss) (count<=1) ? 0.0 : ((ss-sum*sum/count)/(count-1))
printstats ()
	{
	int 	group;
	int 	tcount = 0;        /* total count */
	int 	tnacount = 0;      /* count of all NA's */
	double	tsum = 0.0;        /* total sum */
	double	tss = 0.0;         /* total sum of squares */
	double	tmin = Mins[0];    /* overall min */
	double	tmax = Maxs[0];    /* overall max */
	char	*namefmt = "%-9.9s ";
	char	*numfmt = "%10.3f ";
	int 	numwidth = 10;
	int 	countwidth = 5;

	for (group = 0; group < Ngroups; group++)
		tnacount += NAcount[group];
	if (Ngroups)
		{
		printf (namefmt, "Name");
		printf ("%*s ", countwidth, "N");
		if (tnacount)
			printf ("%*s ", countwidth, "NA");
		printf ("%*s ",    numwidth, "Mean");
		printf ("%*s ",    numwidth, "SD");
		printf ("%*s ",    numwidth, "Min");
		printf ("%*s ",    numwidth, "Max");
		putchar ('\n');
		}
	for (group = 0; group < Ngroups; group++)
		{
		printf (namefmt, Name[group]);
		printf ("%*d ", countwidth, Count[group]);
		if (tnacount)
			printf ("%*d ", countwidth, NAcount[group]);
		printf (numfmt, Mean[group] = Sum[group]/Count[group]);
		Standev[group] = sqrt (var (Count[group], Sum[group], Sumsq[group]));
		printf (numfmt, Standev[group]);
		printf (numfmt, Mins[group]);
		printf (numfmt, Maxs[group]);
		putchar ('\n');
		}
	if (Ngroups > 1) /* print totals over groups */
		{
		for (group = 0; group < Ngroups; group++)
			{
			tcount += Count[group];
			tsum += Sum[group];
			tss += Sumsq[group];
			if (Mins[group] < tmin)
				tmin = Mins[group];
			if (Maxs[group] > tmax)
				tmax = Maxs[group];
			}
		printf (namefmt, "Total");
		printf ("%*d ", countwidth, tcount);
		if (tnacount)
			printf ("%*d ", countwidth, tnacount);
		printf (numfmt, tsum/tcount);
		printf (numfmt, sqrt (var (tcount, tsum, tss)));
		printf (numfmt, tmin);
		printf (numfmt, tmax);
		putchar ('\n');
		}
	}

/*
	compute one-way anova based on Guilford & Fruchter (5th Ed) p. 239
	unweighted solution based on Keppel p. 351
*/
oneway ()
	{
	double	Wsum;          /* weighted solution grand sum */
	double	Wsumsq;        /* weighted solution grand sum squares (SStotal) */
	int 	Wcount;        /* weighted solution N total */
	double	Ucount;        /* unweighted sample size */
	double	SSn;           /* will be SUM(i) Sum[i]*Sum[i]/Count[i] */
	int 	dfbetween;     /* degrees of freedom numerator */
	int 	dfwithin;      /* degrees of freedom denominator */
	double	WSSbetween;    /* weighted solution sum of squares between groups */
	double	USSbetween;    /* unweighted ss between groups */
	double	MeanGsq;       /* sum of squares of group means */
	double	MeanSum;       /* sum of all means */
	double	SSwithin;      /* sum of squares within groups */
	int 	group;         /* loop index */

	if (Ngroups < 2)
		ERRMSG0 (You need at least two groups of data for a comparison)

	Wsum = Wsumsq = 0.0;
	Ucount = 0.0;
	MeanSum = 0.0;
	MeanGsq = 0.0;
	Wcount = 0;
	for (group = 0; group < Ngroups; group++)
		{
		Wsum += Sum[group];
		Wsumsq += Sumsq[group];
		Wcount += Count[group];
		Ucount += 1.0 / Count[group];
		MeanGsq += Mean[group]*Mean[group];
		MeanSum += Mean[group];
		}
	Ucount = Ngroups / Ucount;
	MeanSum = MeanSum * MeanSum / Ngroups;

	if (Wcount <= Ngroups) 
		ERRMSG0 (You need more than one datum per group for comparison)

	for (SSn = 0.0, group = 0; group < Ngroups; group++)
		SSn += (Sum[group] * Sum[group]) / Count[group];

	WSSbetween = SSn - Wsum*Wsum/Wcount;
	USSbetween = Ucount * (MeanGsq - MeanSum);
	SSwithin = Wsumsq - SSn;
	dfbetween = Ngroups - 1;
	dfwithin = Wcount - Ngroups;

	if (Unweighted)
		ftable ("Unweighted", USSbetween, dfbetween, SSwithin, dfwithin);
	else
		ftable ("Weighted", WSSbetween, dfbetween, SSwithin, dfwithin);
	}

ftable (solution, ssbetween, dfbetween, sswithin, dfwithin)
char	*solution;       /* name of the solution analysis type */
double	ssbetween;       /* ss between groups */
int 	dfbetween;       /* degrees of freedom numerator */
double	sswithin;        /* ss within groups (error) */
int 	dfwithin;        /* degrees of freedom numerator */
	{
	double	msbetween;   /* mean square effect */
	double	mswithin;    /* mean square errror */
	double	F;           /* F ratio */
	double	p, pof ();   /* probability of F */

	if (dfbetween <= 0 || dfwithin <= 0)
		{
		WARNING (invalid degrees of freedom)
		return;
		}

	if (sswithin < FZERO)
		{
		WARNING (zero error term implies infinite F or t statistic);
		return;
		}

	msbetween = ssbetween / dfbetween;
	mswithin = sswithin / dfwithin;

	if (fzero (mswithin))
		{
		F = 99999.999;
		p = 0.0;
		}
	else
		{
		F = msbetween / mswithin;
		p = pof (F, dfbetween, dfwithin);
		}

	putchar ('\n');
	printf ("%s Means Analysis:\n", solution);
	if (Ngroups == 2 && Ttest == TRUE)
		printf ("t(%d) = %.3f   p = %.3f\n", dfwithin, sqrt (F), p);
	else
		{
		printf ("%-8s %10s %5s %10s %8s %5s\n",
			"Source", "SS", "df", "MS", "F", "p");
		printf ("%-8s ", "Between");
		printf ("%10.3f ", ssbetween);
		printf ("%5d ", dfbetween);
		printf ("%10.3f ", msbetween);
		printf ("%8.3f ", F);
		printf ("%5.3f ", p);
		if (p < .001) putchar ('*');
		if (p < .01) putchar ('*');
		if (p < .05) putchar ('*');
		putchar ('\n');
		printf ("%-8s ", "Within");
		printf ("%10.3f ", sswithin);
		printf ("%5d ", dfwithin);
		printf ("%10.3f ", mswithin);
		putchar ('\n');
		}
	}

doplot ()
	{
	char	*linegraph ();
	double	minval = Mins[0];
	double	maxval = Maxs[0];
	int 	group;
	
	if (Plot == FALSE)
		return;
	
	for (group = 1; group < Ngroups; group++)
		{
		if (Mins[group] < minval)
			minval = Mins[group];
		if (Maxs[group] > maxval)
			maxval = Maxs[group];
		}
	
	putchar ('\n');
	for (group = 0; group < Ngroups; group++)
		printf ("%-9.9s |%s|\n", Name[group],
			linegraph (group, minval, maxval));
	printf ("%-9.9s  ", "");
	numline (minval, maxval, Plotwidth);
	}

#define	EPSILON (.000000001)
#define scale(x,minx,maxx) ((int) (Plotwidth*((x)-(minx))/((maxx)-(minx)+EPSILON)))
#define	addch(line,i,c) (line[(i)] = (c))

#define	PLOT_MINCHAR      '<'
#define	PLOT_MAXCHAR      '>'
#define	PLOT_MEANCHAR     '#'
#define	PLOT_MINSE        '('
#define	PLOT_MAXSE        ')'
#define	PLOT_STANDEV      '='
#define	PLOT_TWOSTANDEV   '-'

char *
linegraph (group, minval, maxval)
int 	group;
double	minval;     /* global minimum */
double	maxval;     /* global maximum */
	{
	static	char	line[BUFSIZ];
	int 	i;
	int 	low, high;          /* range values in graph buffer */
	int 	minlow, maxhigh;    /* scaled versions of group extrema */
	double	sqrt ();
	double	se = Standev[group] / sqrt ((double) Count[group]);

	for (i = 0; i < Plotwidth; i++)
		line[i] = ' ';
	line[Plotwidth] = '\0';

	minlow = scale (Mins[group], minval, maxval);
	maxhigh = scale (Maxs[group], minval, maxval);

	/* two standard deviations above and below the mean */
	low = scale (Mean[group]-2.0*Standev[group], minval, maxval);
	high = scale (Mean[group]+2.0*Standev[group], minval, maxval);
	if (low < minlow)
		low = minlow;
	if (high > maxhigh)
		high = maxhigh;
	for (i = low; i <= high; i++)
		addch (line, i, PLOT_TWOSTANDEV);

	/* one standard deviation above and below the mean */
	low = scale (Mean[group]-Standev[group], minval, maxval);
	high = scale (Mean[group]+Standev[group], minval, maxval);
	if (low < minlow)
		low = minlow;
	if (high > maxhigh)
		high = maxhigh;
	for (i = low; i <= high; i++)
		addch (line, i, PLOT_STANDEV);

	/* one standard error around the mean */
	low = scale (Mean[group] - se, minval, maxval);
	high = scale (Mean[group] + se, minval, maxval);
	if (low < minlow)
		low = minlow;
	if (high > maxhigh)
		high = maxhigh;
	addch (line, low, PLOT_MINSE);
	addch (line, high, PLOT_MAXSE);
	
	/* write in minimum and maximum, then mean */
	addch (line, minlow, PLOT_MINCHAR);
	addch (line, maxhigh, PLOT_MAXCHAR);
	i = scale (Mean[group], minval, maxval);
	addch (line, i, PLOT_MEANCHAR);
	return (line);
	}

usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXGROUP, "maximum number of groups");
		const (MAXCHARS, "maximum number of characters in input words");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('p', "print error bar plots", Plot);
		ropt ('s', "splitter", "group membership separator value", Splitter);
		lopt ('t', "print significance test in t-test format", Ttest);
		lopt ('u', "use unweighted means analysis", Unweighted);
		iopt ('w', "plotwidth", "plot width", Plotwidth);
		oper ("[names]", "group names", "Group-1 Group-2 ...");
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
