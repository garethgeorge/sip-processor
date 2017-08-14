/*  Copyright 1982 Gary Perlman */

#include "stat.h"
PGM(reverse,Reverse Lines/Fields/Characters,5.1,10/13/86)

#define	MAXFIELDS    100
#define	MAXCHARS  BUFSIZ
char	**Lines;
int 	Maxlines = 2000;
Boole	Rlines, Rchars, Rfields;
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */

/*FUNCTION revchar: reverse characters in a string */
char *
revchar (s)
register	char	*s;
	{
	static	char	buf[MAXCHARS];
	register	char	*ptr;
	register	char	*sptr;

	ptr = buf;
	sptr = s;
	while (*sptr)
		sptr++;
	while (sptr > s)
		*ptr++ = *--sptr;
	*ptr = '\0';
	return (buf);
	}

/*FUNCTION revfield: reverse fields in a string */
char *
revfield (line)
char	*line;
	{
	char	*array[MAXFIELDS];
	char	buf[MAXCHARS];
	int		ncols, i;
	char	*ptr = line;

	strcpy (buf, line);
	if ((ncols = parselin (buf, array, MAXFIELDS)) > MAXFIELDS)
		ERRMANY (fields, MAXFIELDS)
	for (i = ncols-1; i >= 0; i--)
		{
		strcpy (ptr, array[i]);
		while (*ptr)
			ptr++;
		if (i != 0)
			*ptr++ = '\t';
		}
	*ptr = '\0';
	return (line);
	}

/*FUNCTION initial: set program options */
initial (argc, argv)
char	**argv;
	{
	int 	errcnt = 0;
	extern	int optind;
	int 	C;

	while ((C = getopt (argc, argv, "cflLOV")) != EOF)
		switch (C)
			{
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			case 'f': Rfields = 1; break;
			case 'l': Rlines = 1; break;
			case 'c': Rchars = 1; break;
			default:
				errcnt++;
			}
	if (errcnt)
		USAGE ("[-cfl]")                   /* always exits */
	if (!Rchars && !Rfields)
		Rlines = TRUE;
	usinfo ();
	ERROPT (optind)                        /* may exit */
	checkstdin ();
	}

/*FUNCTION main: reverse */
main (argc, argv) char **argv;
	{
	char	line[MAXCHARS];
	int 	nlines = 0;

	ARGV0;
	initial (argc, argv);
	if (Rlines)
		{
		nlines = readlines (&Lines, Maxlines, stdin);
		if (nlines == 0) /* nothing to reverse, so we are done */
			exit (SUCCESS);
		if (nlines > Maxlines)
			ERRMANY (input lines, Maxlines)       /* always exits */
		if (nlines < 0)
			ERRSPACE (input lines for reversal)   /* always exits */
		while (nlines > 0)
			println (Lines[--nlines]);
		}
	else /* no line reversal */
		while (gets (line))
			println (line);

	exit (SUCCESS);
	}

println (line)
char	*line;
	{
	if (Rchars)
		line = revchar (line);
	if (Rfields)
		line = revfield (line);
	puts (line);
	}

/*FUNCTION usinfo: print information about options, limits, version */
usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (Maxlines,  "maximum number of lines to reverse");
		const (MAXFIELDS, "maximum number of fields to reverse");
		const (MAXCHARS,  "maximum length of input lines");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('c', "reverse characters", Rchars);
		lopt ('f', "reverse fields",     Rfields);
		lopt ('l', "reverse lines",      Rlines);
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (SUCCESS);
	}
