/*  Copyright 1986 Gary Perlman */

/*LINTLIBRARY*/
#ifndef lint
static char sccsfid[] = "@(#) cistrcmp.c 5.0 (|stat) 6/11/86";
#endif

#include <ctype.h>

/* case insensitive string compare */
int
cistrcmp (s1, s2)
register	char	*s1;
register	char	*s2;
	{
	register	int 	c1;
	register	int 	c2;
	while (*s2)
		{
		if (isupper (c1 = *s1))
			c1 = tolower (c1);
		if (isupper (c2 = *s2))
			c2 = tolower (c2);
		if (c1 - c2)
			return (c1 - c2);
		s1++;
		s2++;
		}
	return (*s1);
	}

