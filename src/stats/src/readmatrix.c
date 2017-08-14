/*  Copyright 1985 Gary Perlman */

/*LINTLIBRARY*/
/*
	$Compile: fake "readmatrix readlines parselin -DREADMATRIX"
*/
#ifndef lint
static char sccsfid[] = "@(#) readmatrix.c 5.0 (|stat) 12/2/86";
#endif

#include <stdio.h>
#include <string.h>

#ifdef __STDC__
#include "stdlib.h"
#else
extern	char	*malloc ();
#endif

typedef	char	***Matrix;
typedef	int 	Counter;

#define	SUCCESS                0
#define	TOO_MANY_ROWS          (-1)
#define	NO_LINE_SPACE          (-2)
#define	NO_COL_SPACE           (-3)
#define	TOO_MANY_COLS          (-4)
#define	NO_MATRIX_SPACE        (-5)
#define	NO_MATRIX_ROW_SPACE    (-6)
#define	UNEVEN_MATRIX          (-7)

char *
errmatrix (errnum)
int 	errnum;
	{
	switch (errnum)
		{
		case TOO_MANY_ROWS:
			return ("too many rows");
		case NO_LINE_SPACE:
			return ("no space for input lines");
		case NO_COL_SPACE:
			return ("no space to count columns");
		case TOO_MANY_COLS:
			return ("too many columns");
		case NO_MATRIX_SPACE:
			return ("no space for matrix");
		case NO_MATRIX_ROW_SPACE:
			return ("no space for storing matrix row");
		case UNEVEN_MATRIX:
			return ("uneven number of columns in matrix");
		}
	return ("");
	}

readmatrix (matptr, nrowptr, ncolptr, maxrows, maxcols)
Matrix	*matptr;
Counter	*nrowptr;
Counter	*ncolptr;
Counter	maxrows;
Counter	maxcols;
	{
	Matrix	matrix;              /* matrix to be built */
	Counter	row;
	char	**line;              /* array of input lines */
	char	firstline[BUFSIZ];   /* first line to determine number of fields */
	char	**field;             /* fields on a line */

	/* read in the lines and check for errors */
	*nrowptr = readlines (&line, maxrows, stdin);
	if (*nrowptr > maxrows)
		return (TOO_MANY_ROWS);
	if (*nrowptr < 0)
		return (NO_LINE_SPACE);

	/* determine number of fields in first line */
	strcpy (firstline, line[0]);
	field = (char **) malloc (maxcols * sizeof (char *));
	if (field == NULL)
		return (NO_COL_SPACE);
	*ncolptr = parselin (firstline, field, maxcols);
	free (field);
	if (*ncolptr > maxcols)
		return (TOO_MANY_COLS);

	matrix = (Matrix) malloc (*nrowptr * sizeof (*matrix));
	if (matrix == NULL)
		return (NO_MATRIX_SPACE);
	*matptr = matrix;

	for (row = 0; row < *nrowptr; row++)
		{
		matrix[row] = (char **) malloc (*ncolptr * sizeof (**matrix));
		if (matrix[row] == NULL)
			return (NO_MATRIX_ROW_SPACE);
		if (*ncolptr != parselin (line[row], matrix[row], *ncolptr))
			return (UNEVEN_MATRIX);
		}
	return (SUCCESS);
	}


#ifdef	READMATRIX
main ()
	{
	Matrix	matrix;
	int 	nrows;
	int 	ncols;
	int 	row, col;
	int 	result;

	if (result = readmatrix (&matrix, &nrows, &ncols, 100, 100))
		printf ("readmatrix: %s\n", errmatrix (result));
	else
		{
		printf ("nrows = %d\n", nrows);
		printf ("ncols = %d\n", ncols);
		for (col = 0; col < ncols; col++)
			{
			for (row = 0; row < nrows; row++)
				printf ("'%s'	", matrix[row][col]);
			putchar ('\n');
			}
		}
	}
#endif	READMATRIX
