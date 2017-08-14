/*  Copyright 1982 Gary Perlman */

/*LINTLIBRARY*/
#include <stdio.h>
#include <ctype.h>
static char sccsfid[] = "@(#) parselin.c 5.1 (|stat) 2/22/85";

/* parselin reads from line into array, an array of maxstrings pointers.
   It returns the number of strings read in, or maxstrings + 1 if some
   information is discarded.  The input string is clobbered for space.
*/

parselin (line, array, maxstrings)
register	char	*line;
char	**array;
	{
	int 	nstrings = 0;           /* number of strings read in */
	int 	qchar;                  /* quote character */
	while (isspace (*line)) line++;
	while (*line)
		{
		qchar = (*line == '"' || *line == '\'') ? *line++ : '\0';
		array[nstrings] = line;
		while (*line && (qchar ? (*line != qchar) : (!isspace (*line)))) line++;
		if (*line) *line++ = '\0';
		while (isspace (*line)) line++;
		if (++nstrings == maxstrings)
			return (maxstrings + (*line ? 1 : 0));
		}
	return (nstrings);
	}

#ifdef PARSELINE
#define	MAX 100
main ()
	{
	int 	ncols;
	int 	col;
	char	*array[MAX];
	char	line[BUFSIZ];
	while (fgets (line, BUFSIZ, stdin))
		{
		ncols = parselin (line, array, MAX);
		printf ("%3d ", ncols);
		for (col = 0; col < ncols; col++)
			{
			printquote (array[col]);
			putchar (' ');
			}
		putchar ('\n');
		}
	exit (0);
	}
printquote (s)
char	*s;
	{
	char	*ptr;
	int 	squote = 0;   /* is there a single quote in string? */
	int 	dquote = 0;   /* is there a double quote in string? */
	int 	space  = 0;   /* is there a space in string? */
	for (ptr = s; *ptr; ptr++)
		{
		if (*ptr == '\'') squote++;
		else if (*ptr == '"') dquote++;
		else if (*ptr == ' ') space++;
		}
	if (space == 0 && *s != '\'' && *s != '"')
		printf ("%s", s);
	else if (dquote && !squote)
		printf ("'%s'", s);
	else if (squote && !dquote)
		printf ("\"%s\"", s);
	else /* can't quote this properly! */
		printf ("%s");
	}
#endif
