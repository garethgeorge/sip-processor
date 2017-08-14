/*  Copyright 1986 Gary Perlman */

/*LINTLIBRARY*/
#ifndef lint
static char sccsfid[] = "@(#) numcmp.c 5.0 (|stat) 6/11/86";
#endif

/*
	first compare integer component,
	then, if equal, use signed strcmp on decimal part
*/
#include <ctype.h>
#define	issign(c) ((c) == '-' || (c) == '+')
#define	skipint(s) \
	if (issign(*s)) s++; \
	while (isdigit (*s)) s++; \
	if (*s == '.') s++;
#define	skip0(s) while (*(s) == '0') (s)++;

int
numcmp (s1, s2)
register	char	*s1, *s2;
	{
	register	int 	diff;
	int 	sign = (*s1 == '-') ? -1 : 1;

	/* try to get out quickly for signed comparisons */
	if (*s1 == '-' && *s2 != '-')
		return (-1);
	if (*s1 != '-' && *s2 == '-')
		return (1);

	/* now they are both positive, or both negative */

	if (diff = atoi (s1) - atoi (s2))
		return (diff);
	skipint (s1);
	skipint (s2);
	while (isdigit (*s1) && isdigit (*s2)) /* must count leading zeros */
		{
		if (diff = (*s1 - *s2))
			return (sign * diff);
		s1++;
		s2++;
		}
	skip0 (s1);
	skip0 (s2);
	return (sign * (isdigit (*s1) * (*s1) - isdigit (*s2) * (*s2)));
	}

