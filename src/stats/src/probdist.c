/*  Copyright 1986 Gary Perlman */

#include "stat.h"
PGM(probdist,Probability Distribution Functions,5.4,8/21/89)
/*
	5.4	added -q option for fast normal random number generation
*/

#ifdef TRACE
#define	trace(X) printf X; getchar ()
#else
#define trace(X)
#endif

#define	MAXFIELDS    50     /* max fields on input line */
#define	MAXCHARS BUFSIZ     /* maximum number of chars in lines */
#define	MAXBIN     1000     /* max number of binomial trials */

extern	double	poz (),     critz ();
extern	double	pof (),     critf ();
extern	double	pochisq (), critchi ();
extern	double	getratio ();
#define	BADPROB (-1.0)
extern	double	pobin ();
extern	int 	            critbin (), randbin ();
extern	double	apobin (),  acritbin ();

#define	crituni(p)          (1.0 - (p))
#define	probuni(val)        (1.0 - (val))
extern	double Maxrand;     /* defined in random.c */
#define	randuni()           ((double) rand () / Maxrand)
#define	pot(val,df)         pof ((val)*(val), 1, (df))
#define	critt(p,df)         sqrt (critf ((p), 1, (df)))

#define	U_NAME          "uniform"
#define	F_NAME          "F"
#define	T_NAME          "t"
#define	Z_NAME          "normal-z"
#define	B_NAME          "binomial"
#define	C_NAME          "chi-square"

#define	shortprint(val)     printf ("%.6f\n", val)
#define	f_print(df1,df2,val,prob) \
	printf ("%s %3d %3d %12.4f %10.6f\n", F_NAME, df1, df2, val, prob)
#define	t_print(df,val,prob) \
	printf ("%s %3d %12.4f %10.6f\n", T_NAME, df, val, prob)
#define	chi_print(df,val,prob) \
	printf ("%s %3d %12.4f %10.6f\n", C_NAME, df, val, prob)
#define	z_print(val,prob) \
	printf ("%s %12.4f %10.6f\n", Z_NAME, val, prob)
#define	u_print(val,prob) \
	printf ("%s %12.4f %10.6f\n", U_NAME, val, prob)
#define	b_print(N,p1,p2,val,prob) \
	printf ("%s %3d	%d/%d	%3d %10.6f\n", B_NAME, N, p1, p2, val, prob)

#define	okdist(string)      (strchr ("unztcxfb", *string) != NULL)
#define	okfunc(string)      (strchr ("rpcq", *string) != NULL)
#define	badval(dist,val)    fprintf (stderr, "%s: '%s' is not a legal %s distribution value\n", Argv0, val, dist)
#define	getsampsize(str,var) getposint (str, "random sample size", var)

Boole	Randinit = FALSE;     /* has the random number generator been set */
int 	Seed;                 /* used to set random number generator */
Boole	Verbose;              /* verbose output format */
Boole	Quick;                /* quick computational forms */
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */

/*FUNCTION getposint: convert string to positive integer with checking */
Status
getposint (string, desc, iptr)
char	*string;
char	*desc;
int 	*iptr;
	{
	if (!isinteger (string))
		{
		fprintf (stderr, "%s: '%s' (%s) is not an integer\n",
			Argv0, string, desc);
		return (FAILURE);
		}
	*iptr = atoi (string);
	if (*iptr <= 0)
		{
		fprintf (stderr, "%s: '%d' (%s) should have been a positive integer\n",
			Argv0, *iptr, desc);
		return (FAILURE);
		}
	return (SUCCESS);
	}

/*FUNCTION getratio: parse/return the ratio-of-two-integers, or decimal */
double
getratio (str, p1, p2)
char	*str;            /* input string is not affected */
int 	*p1, *p2;        /* receptacles for p1/p2 */
	{
	double	p = 0.0;     /* computed probability ratio */
	char	*sv = str;   /* save for error messages */
	*p1 = 0;             /* initial values */
	*p2 = 0;
	if (*str == '.' || *str == '0') /* decimal value, call atof() */
		{
		if (number (str))           /* string must be numerical */
			p = atof (str);
		else
			{
			fprintf (stderr, "%s: probability '%s' not a number\n", Argv0, sv);
			p = BADPROB;
			}
		}
	else
		{
		*p1 = atoi (str);
		while (isdigit (*str))
			str++;
		if (*str != '/')  /* slash delimiter */
			p = BADPROB;
		else
			{
			str++;
			*p2 = atoi (str);
			while (isdigit (*str))
				str++;
			if (*str) /* bad: extra stuff left over */
				p = BADPROB;
			}
		if (*p1 > 0 && *p2 > 0)
			p = (double) *p1 / (double) *p2;
		}
	/* range check */
	if (p == BADPROB)
		fprintf (stderr, "%s: ratio '%s' is malformed\n", Argv0, sv);
	else if (! (p > 0.0 && p < 1.0))
		{
		fprintf (stderr, "%s: probability '%g' from '%s' not in (0,1)\n", Argv0, p, sv);
		p = BADPROB;
		}
	return (p);
	}

Status
getprob (str, pptr)
char	*str;
double	*pptr;
	{
	int 	p1, p2;
	*pptr = getratio (str, &p1, &p2);
	return (*pptr == BADPROB);
	}

/*FUNCTION checkparams: check number of distribution parameters */
Status
checkparams (dist, nstrings, nparam, desc)
char	*dist;
char	*desc;
	{
	if (nstrings != nparam+1)
		{
		fprintf (stderr, "%s: %s distribution requires %s with value\n",
			Argv0, dist, desc);
		return (FAILURE);
		}
	return (SUCCESS);
	}

/*FUNCTION tell: tell correct usage for program, functions, distribution */
tellusage (commandline)
Boole	commandline;    /* full command line usage */
	{
	fprintf (stderr, "Usage: ");
	if (commandline)
		fprintf (stderr, "%s [-qv] [-s seed] ", Argv0);
	fprintf (stderr, "function distribution [parameters] value\n");
	}

tellfunctions (function)
char	*function;
	{
	fprintf (stderr, "%s: '%s' is not a legal function\n", Argv0, function);
	fprintf (stderr, "	Supported functions are: %s %s %s\n",
		"probability", "critical-value", "random-sample");
	}

telldistributions (dist)
char	*dist;
	{
	fprintf (stderr, "%s: '%s' is not a legal distribution\n", Argv0, dist);
	fprintf (stderr, "	Supported distributions are: %s %s %s %s %s %s\n",
		U_NAME, Z_NAME, B_NAME, C_NAME, T_NAME, F_NAME);
	}

/*FUNCTION initial: returns local version of optind, index to first operand */
int
initial (argc, argv) char **argv;
	{
	extern	char *optarg;    /* option value accessed through this by getopt */
	extern	int  optind;     /* will be index to first operand */
	int 	opterr = 0;      /* count of number of errors */
	int 	flag;            /* option flag characters read in here */

	while ((flag = getopt (argc, argv, "qs:vLOV")) != EOF)
		switch (flag)
			{
			default: opterr++; break;
			case 's': /* random seed */
				if (setint (argv[0], flag, optarg, &Seed, 1, MAXINT))
					opterr++;
				break;
			case 'q': Quick = TRUE; break;
			case 'v': Verbose = TRUE; break;
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			}

	if (opterr) /* print usage message and bail out */
		{
		tellusage (TRUE);
		exit (1);
		}

	usinfo ();

	return (optind);
	}

/*FUNCTION main */
main (argc, argv) int argc; char **argv;
	{
	Status	result = SUCCESS;
	int 	firstop;      /* index of first operand */
	char	line[BUFSIZ];
	char	*array[MAXFIELDS];
	int 	ncols;
	int 	linecount = 0;

	ARGV0;
	firstop = initial (argc, argv);
	if (firstop < argc)
		{
		if (result = probdist (argv+firstop, argc-firstop))
			tellusage (TRUE);
		}
	else
		{
		checkstdin ();
		while (gets (line))
			{
			linecount++;
			if (ncols = parselin (line, array, MAXFIELDS))
				{
				if (probdist (array, ncols))
					{
					fprintf (stderr, "\tError(s) found on input line %d\n",
						linecount);
					tellusage (FALSE);
					result = FAILURE;
					}
				}
			}
		}
	exit (result);
	}

/*FUNCTION usinfo: provide on-line help */
usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXCHARS, "maximum number of characters in lines");
		const (MAXBIN,   "maximum number of binomial trials");
		const (1,        "minimum sample size for random number generation");
		const (0,        "minimum probability");
		const (1,        "maximum probability");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('q',         "use quick computational forms", Quick);
		iopt ('s', "seed", "integer random number generator seed", Seed);
		lopt ('v',         "verbose output format", Verbose);
		oper ("[function",    "prob, critval, or rand", "");
		oper ("distrib'n",    "f, t, u, x, z, or b distribution", "");
		oper ("[params]",     "parameters for distribution", "");
		oper ("value]",       "stat, prob, or count to match function", "");
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (SUCCESS);
	}

/*FUNCTION probdist: process probdist requests in string array */
probdist (string, nstrings)
char	**string;    /* array of parameters: "prob" "f" "1" "2" "12.56" */
int 	nstrings;    /* number of parameters */
	{
	char	*function;        /* rand, prob, or crit */
	char	*dist;            /* u, z/n, f, chisq/x, bin */
	
	if (nstrings < 3)
		return (FAILURE);
	function = string[0];
	dist = string[1];
	string += 2;
	nstrings -= 2;
	
	if (isupper (*function))
		*function = tolower (*function);
	if (isupper (*dist))
		*dist = tolower (*dist);
	if (!okfunc (function) || !okdist (dist))
		{
		if (!okfunc (function))
			tellfunctions (function);
		if (!okdist (dist))
			telldistributions (dist);
		return (FAILURE);
		}
	
	if (*function == 'r' && Randinit == FALSE)
		{
		initrand (Seed);
		Randinit = TRUE;
		}
	
	switch (*dist)
		{
		case 'b': return (pd_binomial (function, string, nstrings));
		case 't': return (pd_t (function, string, nstrings));
		case 'u': return (pd_uni (function, string, nstrings));
		case 'n': 
		case 'z': return (pd_z (function, string, nstrings));
		case 'c': 
		case 'x': return (pd_chisq (function, string, nstrings));
		case 'f': return (pd_f (function, string, nstrings));
		}
	return (0);
	}

/*FUNCTION: uniform distribution */
Status
pd_uni (function, string, nstrings)
char	*function;
char	**string;    /* distrib parameters followed by value for function */
int 	nstrings;
	{
	char	*svalue = string[nstrings-1];
	double	p;                    /* probability of value in distribution */
	double	stat;                 /* value in distribution */
	int 	nrand;                /* random sample size */
	char	*dist = U_NAME;       /* distribution name */
	
	if (checkparams (dist, nstrings, 0, "no parameters"))
		return (FAILURE);
	
	switch (*function)
		{
		case 'r': /* random number generation */
			if (getsampsize (svalue, &nrand))
				return (FAILURE);
			while (nrand--)
				{
				p = randuni ();
				stat = crituni (p);
				Verbose ? u_print (stat, p) : shortprint (stat);
				}
			 break;
		case 'p': /* probability */
			if (!number (svalue) || (stat = atof (svalue)) < 0.0 || stat > 1.0)
				{
				badval (dist, svalue);
				return (FAILURE);
				}
			p = probuni (stat);
			Verbose ? u_print (stat, p) : shortprint (p);
			break;
		case 'q':
		case 'c': /* critical value or quantile */
			if (getprob (svalue, &p))
				return (FAILURE);
			stat = crituni (p);
			Verbose ? u_print (stat, p) : shortprint (stat);
			break;
		}
	return (SUCCESS);
	}

/*FUNCTION: normal z distribution */
Status
pd_z (function, string, nstrings)
char	*function;
char	**string;    /* distrib parameters followed by value for function */
int 	nstrings;
	{
	char	*svalue = string[nstrings-1];
	double	p;                    /* probability of value in distribution */
	double	stat;                 /* value in distribution */
	int 	nrand;                /* random sample size */
	char	*dist = Z_NAME;       /* distribution name */
	int 	i;
	
	if (checkparams (dist, nstrings, 0, "no parameters"))
		return (FAILURE);
	
	switch (*function)
		{
		case 'r': /* random number generation */
			if (getsampsize (svalue, &nrand))
				return (FAILURE);
			while (nrand--)
				{
				if (Quick) /* add 12 randuni and subtract 6 */
					{
					stat = (-6.0);
					for (i = 0; i < 12; i++)
						stat += randuni ();
					if (Verbose)
						p = poz (stat);
					}
				else
					{
					p = randuni ();
					stat = critz (p);
					}
				Verbose ? z_print (stat, p) : shortprint (stat);
				}
			 break;
		case 'p': /* probability */
			if (!number (svalue))
				{
				badval (dist, svalue);
				return (FAILURE);
				}
			stat = atof (svalue);
			p = poz (stat);
			Verbose ? z_print (stat, p) : shortprint (p);
			break;
		case 'q':
		case 'c': /* critical value or quantile */
			if (getprob (svalue, &p))
				return (FAILURE);
			stat = critz (p);
			Verbose ? z_print (stat, p) : shortprint (stat);
			break;
		}
	return (SUCCESS);
	}

/*FUNCTION: Fisher F distribution */
Status
pd_f (function, string, nstrings)
char	*function;
char	**string;    /* distrib parameters followed by value for function */
int 	nstrings;
	{
	char	*svalue = string[nstrings-1];
	int 	df1;                  /* degrees of freedom numerator */
	int 	df2;                  /* degrees of freedom denominator */
	double	p;                    /* probability of value in distribution */
	double	stat;                 /* value in distribution */
	int 	nrand;                /* random sample size */
	char	*dist = F_NAME;       /* distribution name */
	
	if (checkparams (dist, nstrings, 2, "two degrees of freedom"))
		return (FAILURE);
	if (getposint (string[0], "degrees of freedom numerator", &df1))
		return (FAILURE);
	if (getposint (string[1], "degrees of freedom denominator", &df2))
		return (FAILURE);
	
	switch (*function)
		{
		case 'r': /* random number generation */
			if (getsampsize (svalue, &nrand))
				return (FAILURE);
			while (nrand--)
				{
				p = randuni ();
				stat = critf (p, df1, df2);
				Verbose ? f_print (df1, df2, stat, p) : shortprint (stat);
				}
			 break;
		case 'p': /* probability */
			if (!number (svalue) || (stat = atof (svalue)) < 0.0)
				{
				badval (dist, svalue);
				return (FAILURE);
				}
			p = pof (stat, df1, df2);
			Verbose ? f_print (df1, df2, stat, p) : shortprint (p);
			break;
		case 'q':
		case 'c': /* critical value or quantile */
			if (getprob (svalue, &p))
				return (FAILURE);
			stat = critf (p, df1, df2);
			Verbose ? f_print (df1, df2, stat, p) : shortprint (stat);
			break;
		}
	return (SUCCESS);
	}

/*FUNCTION: chi-sqaure distribution */
Status
pd_chisq (function, string, nstrings)
char	*function;
char	**string;    /* distrib parameters followed by value for function */
int 	nstrings;
	{
	char	*svalue = string[nstrings-1];
	int 	df;                   /* degrees of freedom */
	double	p;                    /* probability of value in distribution */
	double	stat;                 /* value in distribution */
	int 	nrand;                /* random sample size */
	char	*dist = C_NAME;       /* distribution name */
	
	if (checkparams (dist, nstrings, 1, "degrees of freedom"))
		return (FAILURE);
	if (getposint (string[0], "degrees of freedom", &df))
		return (FAILURE);
	
	switch (*function)
		{
		case 'r': /* random number generation */
			if (getsampsize (svalue, &nrand))
				return (FAILURE);
			while (nrand--)
				{
				p = randuni ();
				stat = critchi (p, df);
				Verbose ? chi_print (df, stat, p) : shortprint (stat);
				}
			 break;
		case 'p': /* probability */
			if (!number (svalue) || (stat = atof (svalue)) < 0.0)
				{
				badval (dist, svalue);
				return (FAILURE);
				}
			p = pochisq (stat, df);
			Verbose ? chi_print (df, stat, p) : shortprint (p);
			break;
		case 'q':
		case 'c':
			if (getprob (svalue, &p))
				return (FAILURE);
			stat = critchi (p, df);
			Verbose ? chi_print (df, stat, p) : shortprint (stat);
			break;
		}
	return (SUCCESS);
	}

/*FUNCTION: Student's t distribution */
Status
pd_t (function, string, nstrings)
char	*function;
char	**string;    /* distrib parameters followed by value for function */
int 	nstrings;
	{
	char	*svalue = string[nstrings-1];
	int 	df;                   /* degrees of freedom */
	double	p;                    /* probability of value in distribution */
	double	stat;                 /* value in distribution */
	int 	nrand;                /* random sample size */
	char	*dist = T_NAME;       /* distribution name */
	
	if (checkparams (dist, nstrings, 1, "degrees of freedom"))
		return (FAILURE);
	if (getposint (string[0], "degrees of freedom", &df))
		return (FAILURE);
	
	switch (*function)
		{
		case 'r': /* random number generation */
			if (getsampsize (svalue, &nrand))
				return (FAILURE);
			while (nrand--)
				{
				p = randuni ();
				stat = critt (p, df);
				Verbose ? t_print (df, stat, p) : shortprint (stat);
				}
			 break;
		case 'p': /* probability */
			if (!number (svalue))
				{
				badval (dist, svalue);
				return (FAILURE);
				}
			stat = atof (svalue);
			p = pot (stat, df);
			Verbose ? t_print (df, stat, p) : shortprint (p);
			break;
		case 'q':
		case 'c': /* critical value or quantile */
			if (getprob (svalue, &p))
				return (FAILURE);
			stat = critt (p, df);
			Verbose ? t_print (df, stat, p) : shortprint (stat);
			break;
		}
	return (SUCCESS);
	}

/*FUNCTION: binomial distribution */
Status
pd_binomial (function, string, nstrings)
char	*function;
char	**string;    /* distrib parameters followed by value for function */
int 	nstrings;
	{
	char	*svalue = string[nstrings-1];
	int 	N;                   /* number of Bernouli trials */
	int 	p1, p2;              /* probability ratio parameters */
	double	p = 0.0;             /* prob of success == p1/p2 */
	int 	r;                   /* number of successes */
	int 	nrand;               /* random sample size */
	char	*dist = B_NAME;      /* distribution name */

	trace (("Entering pd_binomial\n"));
	if (checkparams (dist, nstrings, 2, "N and probability ratio"))
		return (FAILURE);
	if (getposint (string[0], "number of trials", &N))
		return (FAILURE);
	if (N > MAXBIN)
		{
		fprintf (stderr, "%s: maximum binomial trials: %d\n", Argv0, MAXBIN);
		return (FAILURE);
		}
	if ((p = getratio (string[1], &p1, &p2)) == BADPROB)
		return (FAILURE);
	if (p1 == 0 || p2 == 0) /* maybe use approximate binomial */
		{
		fprintf (stderr, "%s: invalid ratio supplied (%s)\n", Argv0, string[1]);
		return (FAILURE);
		}
	if (p2 > MAXBIN)
		{
		fprintf (stderr, "%s: maximum ratio denominator: %d\n", Argv0, MAXBIN);
		return (FAILURE);
		}
	
	switch (*function)
		{
		case 'r': /* random number generation */
			if (getsampsize (svalue, &nrand))
				return (FAILURE);
			while (nrand--)
				{
				r = randbin (N, p1, p2);
				Verbose ? b_print (N, p1, p2, r, p) : printf ("%d\n", r);
				}
			 break;
		case 'p': /* probability */
			if (!number (svalue) || (r = atoi (svalue)) < 0 || r > N)
				{
				badval (dist, svalue);
				return (FAILURE);
				}
			trace (("Calling pobin\n"));
			p = pobin (N, p1, p2, r);
			trace (("Back from pobin\n"));
			Verbose ? b_print (N, p1, p2, r, p) : shortprint (p);
			break;
		case 'q':
		case 'c': /* critical value or quantile */
			if (getprob (svalue, &p))
				return (FAILURE);
			r = critbin (N, p1, p2, &p);
			Verbose ? b_print (N, p1, p2, r, p) : printf ("%d\n", r);
			break;
		}
	 return (SUCCESS);
	 }

/*FUNCTION randbin: return a random binomial number */
int
randbin (N, p1, p2)
register
int 	N;         /* distribution size */
int 	p1, p2;    /* p = p1/p2 */
	{
	double	p = (double) p1 / (double) p2;
	int 	r = 0;                /* number of successes */
	extern	double	Maxrand;
	
	while (N-- > 0)
		if (((double) rand ()) / Maxrand < p)
			r++;
	return (r);
	}
