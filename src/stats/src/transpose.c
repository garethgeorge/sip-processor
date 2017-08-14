/*  Copyright 1982 Gary Perlman */

#include "stat.h"
PGM(transpose,Transpose Rows and Columns of Matrix Format File,5.1,4/15/89)
/*HISTORY
	4/15/89	changed printf call to avoid printing NULL pointers
	3/4/85	initial version
*/

#ifdef	macintosh
#define MAXCOLS  75
#define MAXLINES 75
#else
#define MAXCOLS  100
#define MAXLINES 100
#endif
#define MAXCHARS BUFSIZ           /* maximum number of chars in lines */
char	*A[MAXLINES][MAXCOLS];

/* OPTIONS */
char	Format[10] = "%s\t";  /* format of fields */
int 	Formwidth = 0;        /* width of format fields */
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */

initial (argc, argv) char **argv;
	{
	extern	char	*optarg;
	extern	int 	optind;
	int 	errflg = 0;
	int 	C;
	ARGV0;
	while ((C = getopt (argc, argv, "f:LOV")) != EOF)
		switch (C)
			{
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			case 'f':
				if (setint (Argv0, C, optarg, &Formwidth, -100, 100))
					errflg++;
				Format[0] = '%';
				(void) strcpy (Format+1, optarg);
				strcat (Format, "s ");
				break;
			default: errflg++; break;
			}
	if (errflg)
		USAGE ("[-f format]")
	usinfo ();
	ERROPT (optind);
	}

main (argc, argv) char **argv;
	{
	int 	c, l;
	int 	ncols = 0;
	int 	lines = 0;
	int 	maxcols = 0;
	char	*array[MAXCOLS];
	char	line[MAXCHARS];

	initial (argc, argv);
	checkstdin ();
	while (gets (line))
		{
		ncols = parselin (line, array, MAXCOLS);
		if (ncols == 0)
			continue;
		if (lines == MAXLINES)
			ERRMANY (lines, MAXLINES)
		if (ncols > maxcols)
			if (ncols > MAXCOLS)
				ERRMANY (columns, MAXCOLS)
			else
				maxcols = ncols;
		for (c = 0; c < ncols; c++)
			A[lines][c] = strdup (array[c]);
		lines++;
		}
	for (c = 0; c < maxcols; c++)
		{
		for (l = 0; l < lines; l++)
			printf (Format, A[l][c] ? A[l][c] : "");
		putchar ('\n');
		}
	exit (SUCCESS);
	}

usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXCOLS, "maximum number of input columns");
		const (MAXLINES, "maximum number of input lines");
		const (MAXCHARS, "maximum number of characters in lines");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		iopt ('f', "width", "width of fields", Formwidth);
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (SUCCESS);
	}
