/*  Copyright 1982 Gary Perlman */

#include "stat.h"
PGM(series,Generate an Additive Series of Numbers,5.4,10/24/86)

#define	startstr  argv[1]     /* argv start value */
#define	endingstr argv[2]     /* argv ending value */
#define	stepstr   argv[3]     /* argv stepsize value */

char	*Format = "%g\n";     /* print format for numbers in series */
double	Start;                /* series start value */
double	Ending;               /* series end value (note: end is reserved word */
#define	RANGE (Ending-Start)  /* just used to make expression clearer */
double	Step = 1.0;           /* default stepsize - might be negated */

/*FUNCTION main: series */
main (argc, argv) char **argv;
	{
	long	nitems;           /* number of items to generate */
	long 	item;             /* item number in series */
	double	value;            /* current value in series */
	int 	nargs = argc - 1; /* number of arguments on command line */

	ARGV0;                    /* set up name of program */

	/* first check for problems with arguments */
	switch (nargs)
		{
		case 3: /* series start ending stepsize */
			if (!number (stepstr))
				ERRNUM (stepstr,stepsize)        /* always EXITs */
			/* FALLTHROUGH */
		case 2: /* series start ending */
			if (!number (startstr))
				ERRNUM (startstr,series start)   /* always EXITs */
			if (!number (endingstr))
				ERRNUM (endingstr,series end)    /* always EXITs */
			break;
		default:
			USAGE ("start end [stepsize]")       /* always EXITs */
		}

	/* set series values */
	Start   = atof (startstr);
	Ending  = atof (endingstr);
	if (nargs == 3) /* stepsize supplied on command line */
		{
		Step     = fabs (atof (stepstr));
		if (!fzero (RANGE) && fzero (Step)) /* any step okay if RANGE == 0 */
			ERRMSG0 (stepsize must be non-zero) /* always EXITs */
		}

	if (Start > Ending) /* stepsize must be negative */
		Step = (-Step);

	if (fzero (RANGE)) /* Start == Ending, so just print one number */
		nitems = 1;
	else
		nitems = RANGE / Step + 1.0 + FZERO;     /* RANGE < 0 iff Step < 0 */

	/* finally, we print the series, multiplying to avoid rounding */
	for (item = 0; item < nitems; item++)
		{
		value = Start + Step * item;
		printf (Format, fzero (value) ? 0.0 : value);
		}

	exit (SUCCESS);
	}
