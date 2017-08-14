/*  Copyright 1984 Gary Perlman */

#include "stat.h"
FUN(setreal,set a real option,5.0,1/5/86)

/*FUNCTION setreal: check type, convert string, and set real option */
setreal (pgm, flag, value, var)
char	*pgm;
int 	flag;    /* the single character option name */
char	*value;  /* the candidate value, in string format */
double	*var;    /* ptr to variable to stuff in answer */
	{
	if (number (value)) /* number returns 1 for integers, 2 for reals */
		{
		*var = atof (value);
		return (0);
		}
	fprintf (stderr, "%s: -%c option requires a real value\n", pgm, flag);
	return (2);
	}
