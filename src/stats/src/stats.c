/*  Copyright 1985 Gary Perlman */

#include "stat.h"
PGM(stats,Simple Summary Statistics,5.2,01/20/87)

#ifdef	min
#	undef	min
#	undef	max
#endif

#define	MAXSTORE 1000       /* maximum store for computing median */
#define	MAXCHARS   50       /* maximum length of input word */

/* Options names for requesting statistics */
Boole	Verbose = FALSE;
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */
char	*Stat[] = 
	{
	"n", "min", "max", "sum", "ss", "mean", "var", "sd", "skew", "kurt", "se", "NA", NULL
	};

/* these defines must have values that correspond to the array above */
#define	PRINT_N     0
#define	PRINT_MIN   1
#define	PRINT_MAX   2
#define	PRINT_SUM   3
#define	PRINT_SS    4
#define	PRINT_MEAN  5
#define	PRINT_VAR   6
#define	PRINT_SD    7
#define	PRINT_SKEW  8
#define	PRINT_KURT  9
#define	PRINT_SE   10
#define	PRINT_NA   11

/*
	returns the index number of string in (array ending in null pointer)
	if string is not in array,
		then array[ sindex (string, array) ] == NULL
	assumes string and array are not NULL
*/
int
sindex (string, array)
register	char	*string;
register	char	**array;
	{
	register	int 	i;

	for (i = 0; array[i] != NULL; i++)
		if (!strcmp (string, array[i]))
			break;

	return (i);
	}

int
process (argc, argv, optind) char **argv; int optind;
	{
	int 	status = 0;    /* success status */
	int 	n = 0;         /* the number of points read in */
	int 	NA = 0;        /* number of missing NA values */
	double 	x;             /* the value read in */
	double 	x2;            /* the value squared, used for speed up */
	char	word[MAXCHARS];/* data as strings read in here */
	double	sum = 0.0;     /* sum of scores */
	double	ss = 0.0;      /* sum of squared scores */
	double	s3 = 0.0;      /* sum of cubed scores */
	double	s4 = 0.0;      /* sum of forth powered scores */
	double	mean = 0.0;    /* mean of the data */
	double	m2;            /* square of the mean, used for speed up */
	double	var = 0.0;     /* variance of the data */
	double	sd = 0.0;      /* standard deviation of the data */
	double	se = 0.0;      /* standard error of the mean */
	double	skew = 0.0;    /* third moment around the mean */
	double	kurt = 0.0;    /* kurtosis: fourth moment around the mean */
	double	min  = 0.0;    /* minimum value of the input */
	double	max  = 0.0;    /* maximum value of the input */
	int 	i;             /* utility loop variable */

	checkstdin ();

	while (getword (word, stdin))
		{
		if (isna (word))
			NA++;
		else if (number (word))
			{
			x = atof (word);
			if (n == 0)
				min  = max  = x;
			else if (x < min)
				min  = x;
			else if (x > max)
				max  = x;
			x2   = x * x;
			sum += x;
			ss  += x2;
			s3  += x * x2;
			s4  += x2 * x2;
			n++;
			}
		/* else non-numerical is simply ignored */
		}

	/* post loop computations */
	if (n > 0) /* if n == 0, then all statistics are 0 */
		{
		mean = sum / n;
		m2 = mean * mean;
		if (n > 1) /* if n == 1, then all variability measures are 0 */
			{
			var = (ss - mean*sum) / (n-1);
			sd = sqrt (var);
			se = sd / sqrt ((double) n);
			if (sd > .000000001) /* this is done to avoid division by zero */
				{
				skew = (s3 - 3.0*mean*ss + 3.0*m2*sum - m2*sum) / (n*var*sd);
				kurt = (s4-4.*mean*s3+6.*m2*ss-4.*m2*mean*sum+n*m2*m2)
					 /                  (n*var*var);
				}
			}
		}

#ifdef __STDC__
#define	longprint(var,fmt)  printf (#var "\t=\t%" #fmt "\n", var)
#define	shortprint(var,fmt) if (Verbose) longprint(var,fmt); \
							else printf ("%" #fmt, var);
#else
#define	longprint(var,fmt)  printf ("var\t=\t%fmt\n", var)
#define	shortprint(var,fmt) if (Verbose) longprint(var,fmt); \
							else printf ("%fmt", var);
#endif
	/* now output results */
	if (optind == argc) /* no options, print everything */
		{
		longprint (n,d);
		longprint (NA,d);
		longprint (min,g);
		longprint (max,g);
		longprint (sum,g);
		longprint (ss,g);
		longprint (mean,g);
		longprint (var,g);
		longprint (sd,g);
		longprint (se,g);
		longprint (skew,g);
		longprint (kurt,g);
		}
	else
		{
		for (i = optind; i < argc; i++) /* print statistics in order of requests */
			{
			switch ( sindex (argv[i], Stat) )
				{
				case PRINT_N:
					shortprint (n,d);
					break;
				case PRINT_NA:
					shortprint (NA,d);
					break;
				case PRINT_MIN:
					shortprint (min,g);
					break;
				case PRINT_MAX:
					shortprint (max,g);
					break;
				case PRINT_SUM:
					shortprint (sum,g);
					break;
				case PRINT_SS:
					shortprint (ss,g);
					break;
				case PRINT_MEAN:
					shortprint (mean,g);
					break;
				case PRINT_VAR:
					shortprint (var,g);
					break;
				case PRINT_SD:
					shortprint (sd,g);
					break;
				case PRINT_SE:
					shortprint (se,g);
					break;
				case PRINT_SKEW:
					shortprint (skew,g);
					break;
				case PRINT_KURT:
					shortprint (kurt,g);
					break;
				default:
					/* this cannot happen because of earlier check */
					status++;
					break;
				}
			if (Verbose == FALSE)
				{
				if (i == argc - 1)
					putc ('\n', stdout);
				else
					putc ('\t', stdout);
				}
			}
		}
	return (status);
	}

main (argc, argv)
int 	argc;     /* argument count */
char	**argv;   /* argument vector */
	{
	int 	optind;

	ARGV0;
	optind = initial (argc, argv);

	exit (process (argc, argv, optind));
	}

/*FUNCTION initial: returns local version of optind, index to first operand */
int
initial (argc, argv) char **argv;
	{
	extern	char *optarg;    /* option value accessed through this by getopt */
	extern	int  optind;     /* will be index to first operand */
	int 	opterr = 0;      /* count of number of errors */
	int 	flag;            /* option flag characters read in here */
	int 	i;               /* loop variable of command line operands */
	char	*optstring =     /* getopt string to be filled in */
		"vLOV";
	char	*usage =         /* part of usage summary to match optstring */
		"[-v] [requests]";

	while ((flag = getopt (argc, argv, optstring)) != EOF)
		switch (flag)
			{
			default:
				opterr++;
				break;
			/* put option cases here */
			case 'v':
				Verbose = TRUE;
				break;
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			}

	if (opterr) /* print usage message and bail out */
		{
		fprintf (stderr, "Usage: %s %s\n", argv[0], usage);
		exit (1);
		}
	
	usinfo ();

	for (i = optind; i < argc; i++)
		if (Stat[ sindex (argv[i], Stat) ] == NULL)
			{
			fprintf (stderr, "%s: unknown statistic '%s'\n", argv[0], argv[i]);
			opterr++;
			}
	
	if (opterr)
		{
		printopts (stderr);
		exit (1);
		}

	return (optind);
	}

printopts (ioptr)
FILE	*ioptr;
	{
	int 	i;
	fprintf (ioptr, "%s: allowed statistics are:", Argv0);
	for (i = 0; Stat[i]; i++)
		fprintf (ioptr, " %s", Stat[i]);
	putc ('\n', ioptr);
	}

usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXCHARS, "maximum number of characters in input words");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('v', "verbose output (label requested stats)", Verbose);
		printopts (stdout);
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
