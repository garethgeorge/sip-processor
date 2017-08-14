/*  Copyright 1982 Gary Perlman */

#include "stat.h"
PGM(dprime,Signal Detection Theory Analysis,5.3,08/22/90)


/*HISTORY
	08/29/90	GP	moved hit rate and false alarm rate over to line up
	08/29/90	GP	changed counts to longs for huges sample sizes
	08/22/90	GP	added checks for == 0.0, == 1.0 for _rate values
	08/22/90	GP	added criterion reporting
	08/21/90	GP	added -p option to print precision
	08/21/90	GP	added optional repetition count field for file input
	08/21/90	GP	added raw frequency count option for command line
	08/21/90	GP	found bug in yesno: always returned "yes"
*/

Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */
Boole	Doprecision;          /* print IR computation of Hits/(Hits+FA) */

#define	SQRT2PI               2.506628274631000502415765
#define	normaldensity(z)      (exp (-z*z * .5) / SQRT2PI)
double	critz ();

#define	MAXCOLS	      3
#define MAXCHARS BUFSIZ           /* maximum number of chars in lines */
char	*Column[MAXCOLS];
typedef	long	Count;
#define	atoc(s) atol(s)
long 	atol();
Count	Hit, Miss, False_alarm, Correct_rejection;
double	Hit_rate, False_alarm_rate, Precision;

fparselin (ioptr, array, maxcols)
FILE	*ioptr;
char	**array;
	{
	static	char	line[MAXCHARS];
	if (fgets (line, sizeof (line), ioptr))
		return (parselin (line, array, maxcols));
	else
		return (EOF);
	}

main (argc, argv) char **argv;
	{
	int 	ncols;
	int 	optind;
	int 	nargs;
	Boole 	presented, detected;   /* was signal presented? detected? */
	Count	count;                 /* repetition count of file input line */

	ARGV0;
	optind = initial (argc, argv);
	nargs = argc - optind;

	if (nargs == 0)
		{
		checkstdin ();
		while ((ncols = fparselin (stdin, Column, MAXCOLS)) != EOF)
			{
			if (ncols == 0)
				continue;
			count = 1;
			presented = yesno (Column[0]);
			detected  = yesno (Column[1]);
			if (ncols != 2) /* should allow optional count as third field */
				{
				if (ncols == 3 && isinteger (Column[2]))
					{
					count = atoc (Column[2]);
					if (count <= 0)
						ERRMSG1 (repetition count in input (%s) must be positive, Column[2])
					}
				else
					ERRMSG0 (each line must have 2 columns with optional repetition count)
				}
			if (presented)
				{
				if (detected)
					Hit += count;
				else
					Miss += count;
				}
			else /* not presented */
				{
				if (detected)
					False_alarm += count;
				else
					Correct_rejection += count;
				}
			}
		rawtable ();
		}
	else if (nargs == 2) /* hit rate and false alarm rate supplied */
		{
		if (Doprecision)
			{
			WARNING (precision can not be computed from hit-rate and false-alarm rate)
			Doprecision = FALSE;
			}
		if (!number (argv[optind]))
			ERRNUM (argv[optind],hit-rate)
		if (!number (argv[optind+1]))
			ERRNUM (argv[optind+1],false-alarm-rate)
		Hit_rate = atof (argv[optind]);
		False_alarm_rate = atof (argv[optind+1]);
		}
	else /* #hits #fa #miss #cr on command line */
		{
		if (!isinteger (argv[optind+0]) || (Hit = atoc (argv[optind+0])) < 0)
			ERRMSG1 (hits (%s) is not a valid count, argv[optind+0])
		if (!isinteger (argv[optind+1]) || (False_alarm = atoc (argv[optind+1])) < 0)
			ERRMSG1 (false-alarms (%s) is not a valid count, argv[optind+1])
		if (!isinteger (argv[optind+2]) || (Miss = atoc (argv[optind+2])) < 0)
			ERRMSG1 (misses (%s) is not a valid count, argv[optind+2])
		if (!isinteger (argv[optind+3]) || (Correct_rejection = atoc (argv[optind+3])) < 0)
			ERRMSG1 (correct-rejections (%s) is not a valid count, argv[optind+3])
		rawtable ();
		}
	computable ();
	exit (0);
	}

#define	dp_str(string) printf ("%10s", string)
#define	dp_real(value) printf ("%10.3f", value)
#define	dp_cnt(value)  printf ("%10ld", value)

rawtable ()
	{
	dp_str ("");     
		dp_str ("signal"); dp_str ("noise");
	putchar ('\n');
	dp_str ("yes");
		dp_cnt (Hit);  dp_cnt (False_alarm);       dp_cnt (Hit+False_alarm);
	putchar ('\n');
	dp_str ("no");
		dp_cnt (Miss); dp_cnt (Correct_rejection); dp_cnt (Miss+Correct_rejection);
	putchar ('\n');
	dp_str ("");
		dp_cnt (Hit+Miss); dp_cnt (False_alarm+Correct_rejection);
		dp_cnt (Hit+Miss+False_alarm+Correct_rejection);
	putchar ('\n');
	putchar ('\n');
	Hit_rate = Hit ? Hit / (double) (Hit + Miss) : 0.0;
	Precision = Hit ? (Hit / (double) (Hit + False_alarm)) : 0.0;
	False_alarm_rate = False_alarm
		? False_alarm / (double) (False_alarm + Correct_rejection)
		: 0.0;
		
	}

computable ()
	{
	double	zhr, zfar;        /* critical Z's for hit-rate & false-alarm-rate */
	double	dprime, beta;     /* computed statistics */
	double	criterion;        /* point above middle of noise distribution */
	if (Hit_rate <= 0.0 || Hit_rate >= 1.0)
		ERRMSG1 (The hit-rate (%g) must be greater than 0.0 and less than 1.0, Hit_rate)
	if (False_alarm_rate <= 0.0 || False_alarm_rate >= 1.0)
		ERRMSG1 (The false-alarm-rate (%g) must be greater than 0.0 and less than 1.0, False_alarm_rate)
	zhr    = critz (Hit_rate);
	zfar   = critz (False_alarm_rate);
	dprime = zhr - zfar;
	criterion = -zfar;
	beta   = normaldensity (zhr) / normaldensity (zfar);
	dp_str ("");
	dp_str ("hr");
	dp_str ("far");
	dp_str ("dprime");
	dp_str ("beta");
	dp_str ("criterion");
	if (Doprecision)
		dp_str ("precision");
	putchar ('\n');
	dp_str ("");
	dp_real (Hit_rate);
	dp_real (False_alarm_rate);
	dp_real (dprime);
	dp_real (beta);
	dp_real (criterion);
	if (Doprecision)
		dp_real (Precision);
	putchar ('\n');
	}

yesno (string) char *string;
	{
	int 	val;
	if (isinteger (string))
		{
		val = atoi (string);
		if (val == 1 || val == 0)
			return (val);
		}
	else /* check for upper/lower case strings */
		{
		if (!strcmp (string, "yes"))      return (1);
		if (!strcmp (string, "YES"))      return (1);
		if (!strcmp (string, "no"))       return (0);
		if (!strcmp (string, "NO"))       return (0);
		if (!strcmp (string, "signal"))   return (1);
		if (!strcmp (string, "SIGNAL"))   return (1);
		if (!strcmp (string, "noise"))    return (0);
		if (!strcmp (string, "NOISE"))    return (0);
		}
	ERRMSG1 (illegal value (%s) in input, string)
	/*NOTREACHED*/
	}

/*FUNCTION initial: returns local version of optind, index to first operand */
int
initial (argc, argv) char **argv;
	{
	extern	char *optarg;    /* option value accessed through this by getopt */
	extern	int  optind;     /* will be index to first operand */
	int 	opterr = 0;      /* count of number of errors */
	int 	flag;            /* option flag characters read in here */
	int 	nargs;           /* number of non-option arguments on cmd line */
	char	*optstring =     /* getopt string to be filled in */
		"LOVp";
	char	*usage =         /* part of usage summary to match optstring */
"[-p] [hit-rate false-alarm-rate] [hits false-alarms misses correct-rejections]";
	while ((flag = getopt (argc, argv, optstring)) != EOF)
		switch (flag)
			{
			default:
				opterr++;
				break;
			/* put option cases here */
			case 'p': Doprecision = TRUE; break;
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

	nargs = argc - optind;
	if (nargs != 0 && nargs != 2 && nargs != 4)
		USAGE (usage)

	return (optind);
	}

usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXCHARS, "maximum number of characters in lines");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('p',         "request computation of precision", Doprecision);
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
