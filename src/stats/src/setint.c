/*  Copyright 1984 Gary Perlman */

#include <stdio.h>
#if defined macintosh || defined __STDC__
#include <stdlib.h>
#endif

/*LINTLIBRARY*/
#ifndef lint
static char sccsfid[] = "@(#) setint.c 5.0 (|stat) 1/5/86";
#endif

#define	Status	int
#define	SUCCESS 0
#define	FAILURE	1

extern	int 	number ();

/*FUNCTION setint: check type, convert string, and set integer option */
Status
setint (pgm, flag, value, var, minval, maxval)
char	*pgm;
int 	flag;    /* the single character option name */
char	*value;  /* the candidate value, in string format */
int 	*var;    /* ptr to variable to stuff in answer */
int 	minval;  /* minimum allowed value */
int 	maxval;  /* maximum allowed value */
	{
	int 	tmpvar;
	if (number (value) == 1) /* number returns 1 for integers, 2 for reals */
		{
		tmpvar = atoi (value);
		if (tmpvar >= minval && tmpvar <= maxval)
			{
			*var = tmpvar;
			return (SUCCESS);
			}
		fprintf (stderr, "%s: -%c option value must be between %d and %d\n",
			pgm, flag, minval, maxval);
		return (FAILURE);
		}
	fprintf (stderr, "%s: -%c option requires an integer value\n", pgm, flag);
	return (FAILURE);
	}
