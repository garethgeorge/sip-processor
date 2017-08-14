/*  Copyright 1984 Gary Perlman */

/* LINTLIBRARY */
#include "stat.h"
FUN(getword,read a string from a file,5.0,2/23/85)

/*
	getword (string, ioptr) is equivalent to fscanf (ioptr, "%s", string)
	but much faster.  It returns a NULL pointer on EOF or the end
	of the string when one is found.  The return result can be used
	to find the length of the obtained string.
*/

char *
getword (string, ioptr)
register	char	*string;
register	FILE	*ioptr;
	{
	register	int 	C;
	while ((C = getc (ioptr)) != EOF && isspace (C));
	if (C == EOF) return (NULL);
	do
		{
		*string++ = C;
	} while ((C = getc (ioptr)) != EOF && !isspace (C));
	*string = '\0';
	return (string); /* pointer to the end of string */
	}

#ifdef	STANDALONE

main ()
	{
	char	word[BUFSIZ];
	char	*endword;
	while (endword = getword (word, stdin))
		printf ("%4d	'%s'\n", endword-word, word);
	exit (0);
	}

#endif
