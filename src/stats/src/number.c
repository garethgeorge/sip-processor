/*  Copyright 1981 Gary Perlman */
/*BUG allows . as integer because . not followed by trailing zeros */

/*
	number: report if a string is a UNIX formatted number

	notes:
		a number in UNIX is one that can be converted from
		a string to an integer or real with no loss of information
			due to bad format
		all numbers can be surrounded by whitespace
		an integer has an optional minus sign, followed by digits
			as a special case, a decimal point and trailing zeros are allowed
		a real number has an optional minus sign followed by digits
		if a string has a decimal point, followed by zeros, it is real, not int

	value:
		3 if string is a scientific notation number
		2 if string is a real number (as seen by atof)
		1 if string is an integer [-] 0-9+ [.0*]
		0 for non-numbers

	compilation flags:
		-DNUMBER	includes test main program
		$Compile: cc -DNUMBER -O -o %F %f
	
	deficiencies:
		does not check to see if significant digits will be ignored
	
	author:
		Gary Perlman

	date:
		Wed May 22 13:30:40 EDT 1985
		Sun Sep  1 14:53:51 EDT 1985 (modified test module)
		Mon Jan 20 12:57:50 EST 1986 (allowed .0* in INT's)
		Tue Jun 10 15:53:54 EDT 1986 (return 3 for exp notation)
		
*/

#include <ctype.h>
#ifdef macintosh
#include <stdlib.h>
#endif

#ifndef lint
static char sccsfid[] = "@(#) number.c 5.5 (|stat) 7/26/87";
#endif

#define	IS_NOT      0            /* not a number */
#define	IS_INT      1            /* an integer */
#define	IS_REAL     2            /* a real number */
#define	IS_EXP      3            /* exponential notation */

typedef	int 	Boole;
#define	TRUE	1
#define	FALSE	0

#define	EOS         '\0'         /* end of string */

/*LINTLIBRARY*/

int
isinteger (string)
char	*string;
	{
	return (number (string) == IS_INT);
	}

int
number (string)
char	*string;                 /* the string to be tested */
	{
	int 	answer = IS_INT;     /* start by assuming it is an integer */
	Boole	before = FALSE;      /* anything before the decimal? */
	Boole	after = FALSE;       /* anything after the decimal? */
	char	*ptr;

	while (isspace (*string))    /* skip over blank space */
		string++;
	if (*string == EOS)          /* empty string not allowed */
		return (IS_NOT);
	if (*string == '+' || *string == '-') /* old atoi didn't allow '+' */
		{
		string++;
		if (!isdigit (*string) && *string != '.')
			return (IS_NOT);
		}
	if (isdigit (*string))       /* note that there was a digit before . */
		{
		before = TRUE;
		while (isdigit (*string))
			string++;
		}
	if (*string == '.')          /* found a decimal point, parse for real */
		{
		string++;
		ptr = string; /* going to check for trailing zeros for int */
		while (*ptr == '0')
			ptr++;
		while (isspace (*ptr))
			ptr++;
		if (*ptr == EOS) /* number ended with .0*, so integer */
			return (IS_INT);
		answer = IS_REAL;
		if (isdigit (*string))   /* note that there was a digit after . */
			{
			after = TRUE;
			while (isdigit (*string))
				string++;
			}
		}
	if (!before && !after)       /* must be digit somewhere */
		return (IS_NOT);
	if (*string == 'E' || *string == 'e') /* exponent */
		{
		answer = IS_EXP;
		string++;
		if (*string == '+' || *string == '-') /* optional sign */
			string++;
		if (!isdigit (*string))  /* missing exponent */
			return (IS_NOT);
		while (isdigit (*string))
			string++;
		}
	while (isspace (*string))    /* skip optional spaces */
		string++;
	/* should now have exhausted the input string */
	return (*string == EOS ? answer : IS_NOT);
	}


#ifdef NUMBER

#include <stdio.h>
/*
	exits with status = the number of args not numerical
	Shell Example:
		if number -i $*
		then
			echo processing $*
		else
			echo $0: arguments must be integers 
		fi
	Options:
		-i  arguments must be integer
		-n  arguments must be non-negative
*/
int 	NoNegative;   /* do the values have to be non-negative? */
int 	Integer;      /* do the values have to be integers? */

static
int
initial (argc, argv) char **argv;
	{
	extern	char	*optarg;
	extern	int 	optind;
	int 	errcnt = 0;
	int 	C;
	char	*optstring = "in";
	char	*usage = "[-in] string ...";

	while ((C = getopt (argc, argv, optstring)) != EOF)
		switch (C)
			{
			case 'i':
				Integer = 1;
				break;
			case 'n':
				NoNegative = 1;
				break;
			default:
				errcnt++;
				break;
			}
	if (errcnt)
		{
		fprintf (stderr, "Usage: %s %s\n", argv[0], usage);
		exit (1);
		}
	return (optind);
	}


main (argc, argv) char **argv;
	{
	int 	status = 0;
	int 	arg = initial (argc, argv);
	char	*string;

	while (arg < argc)
		{
		string = argv[arg++];
		if (NoNegative && *string == '-')
			status++;
		else switch (number (string))
			{
			case IS_NOT:
				status++;
				break;
			case IS_REAL:
				if (Integer)
					status++;
				break;
			case IS_INT:
				break;
			default: /* CAN'T HAPPEN */
				break;
			}
		}
	exit (status);
	}

#endif
