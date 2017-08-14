/*  Copyright 1982 Gary Perlman */

/*LINTLIBRARY*/
static char sccsfid[] = "@(#) tmpfile.c 5.0 (|stat) 11/10/85";

#ifdef	__MSDOS__
#include "io.h"              /* for access function */
#	define	DIRCHAR   '\\'
#else
#	define	DIRCHAR   '/'
#endif

int
mytmpfile (pgm, filename)
char	*pgm;         /* name of the program */
char	*filename;    /* must be large enough to hold file name */
	{
	char	*basename;  /* will be the basename of pgm */
	int 	tmpnum = 0; /* temporary file number if the file exists */

	/* scan to end of string */
	for (basename = pgm; *basename; basename++)
		continue;
	/* scan backward to in front of directory character */
	while (basename > pgm && *(basename-1) != DIRCHAR)
		basename--;

#if	defined __MSDOS__ || defined macintosh
	strcpy (filename, basename);
	/* pad the tmpfile name with zeros */
	while (strlen (filename) < 8)
		strcat (filename, "0");
	strcat (filename, ".tmp");
	/* add the unique extension */
	while ((access (filename, 0) == 0) && tmpnum < 100)
		{
		tmpnum++;
		filename[6] = tmpnum/10 + '0';
		filename[7] = tmpnum%10 + '0';
		}
	if (tmpnum == 100)
		return (1);
#else
	(void) sprintf (filename, "/tmp/%s%d", basename, getpid ());
	(void) umask (0077);
#endif
	return (0);
	}

#ifdef TMPFILE
/*
	$Compile: cc -DTMPFILE -o %F %f
*/
main (argc, argv)
char	**argv;
	{
	char	newfile[100];
	while (--argc)
		{
		if (tmpfile (*++argv, newfile))
			putchar ('\007');
		puts (newfile);
		}
	}
#endif
