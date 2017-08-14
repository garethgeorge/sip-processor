/* index and strchr are identical functions with different names */
#include "stdio.h"

/* this is included because index is not on some UNIX systems */
char *
index (s, c)
register	char	*s;
register	int 	c;
	{
	while (*s)
		if (c == *s) return (s);
		else s++;
	return (NULL);
	}

/* this is included because strchr is not on some UNIX systems */
char *
strchr (s, c)
register	char	*s;
register	int 	c;
	{
	while (*s)
		if (c == *s) return (s);
		else s++;
	return (NULL);
	}
