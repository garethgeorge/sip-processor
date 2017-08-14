/*  Copyright 1981 Gary Perlman */

#include "stat.h"
PGM(abut,Join Corresponding Lines of Files,5.2,5/31/88)

/*	abut reads from its argument files, one line at a time
	per cycle, and prints all those lines on one line to
	the standard output.
*/

#define MAXFILES     16
#define MAXCHARS BUFSIZ
char	Format[100] = "%s\t";  /* width of printed field */
int 	Formwidth = 0;         /* width of format field */
Boole	Numlines = FALSE;      /* true if lines are to be numbered */
Boole	Cycle = FALSE;         /* true if abut should cycle through files */
                               /* until all have been done once */
Boole	InfoVersion;           /* print version information */
Boole	InfoLimits;            /* print program limits */
Boole	InfoOptions;           /* print usage information */

/*FUNCTION main */
main (argc, argv) char **argv;
	{
	char	*ptr;
	int 	nfiles = 0;
	int 	filenum;
	int 	linenumber = 0;
	Boole	done;                /* true if output is to stop */
	Boole	doneonce[MAXFILES];  /* true if file has been exhausted >= once */
	FILE	*ioptr[MAXFILES];
	char	inputline[MAXCHARS];    /* input lines read in here */
	char	outputline[MAXCHARS];   /* output line built in here */
	char	tmpline[MAXCHARS];      /* input fmt'd here to copy to outputline */

	ARGV0;

	/* open all files in advance */
	for (filenum = initial (argc, argv); filenum < argc; filenum++)
		{
		if (!strcmp (argv[filenum], "-"))
			ioptr[nfiles++] = stdin;
		else if ((ioptr[nfiles++] = fopen (argv[filenum], "r")) == NULL)
			ERROPEN (argv[filenum])
		}

	for (filenum = 0; filenum < nfiles; filenum++)
		doneonce[filenum] = FALSE;

	done = FALSE;
	while (done == FALSE)
		{
		*outputline = '\0';
		if (Numlines == TRUE)
			(void) sprintf (outputline, "%-4d ", ++linenumber);
		for (filenum = 0; filenum < nfiles; filenum++)
			{
			if (ioptr[filenum] != NULL)
				{
				if (fgets (inputline, sizeof (inputline), ioptr[filenum]) == NULL)
					{
					doneonce[filenum] = TRUE;
					*inputline = '\0';
					if (Cycle == TRUE) /* rewind input file */
						{
						if (ioptr[filenum] != stdin)
							{
							rewind (ioptr[filenum]);
							if (fgets (inputline, sizeof (inputline), ioptr[filenum]) == NULL)
								*inputline = '\0';
							}
						}
					else
						ioptr[filenum] = NULL;
					}
				}
			else
				inputline[0] = '\0';

			/* trim trailing space in inputline */
			for (ptr = inputline; *ptr; ptr++);
			while (ptr > inputline && isspace (*(ptr-1)))
				ptr--;
			*ptr = '\0';

			(void) sprintf (tmpline, Format, inputline);
			(void) strcat (outputline, tmpline);
			}

		/* see if we have done all the files at least once */
		done = TRUE;
		for (filenum = 0; filenum < nfiles; filenum++)
			if (doneonce[filenum] == FALSE)
				done = FALSE;

		/* we got something this time around, so print outputline */
		if (done == FALSE)
			printf ("%s\n", outputline);
		}

	exit (0);
	}

/*FUNCTION initial */
int
initial (argc, argv) char **argv;
	{
	int 	C;
	int 	errflg = 0;
	extern	int optind;
	extern	char *optarg;

	while ((C = getopt (argc, argv, "cnf:LOV")) != EOF)
		switch (C)
			{
			default: errflg++; break;
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			case 'n': Numlines = TRUE; break;
			case 'c': Cycle = TRUE; break;
			case 'f':
				if (setint (Argv0, C, optarg, &Formwidth, -100, 100))
					errflg++;
				*Format = '%';
				strcpy (Format+1, optarg);
				(void) strcat (Format, "s ");
				break;
			}

	usinfo ();

	if (argc - optind > MAXFILES)
		ERRMANY (files, MAXFILES)

	if (optind == argc || errflg)
		USAGE ("[-cn] [-f format] file1 file2 ...")

	return (optind);
	}

/*FUNCTION usinfo */
usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXFILES, "maximum number of files");
		const (MAXCHARS, "maximum number of characters in lines");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('c', "cycle through input files", Cycle);
		iopt ('f', "width", "width of format string", Formwidth);
		lopt ('n', "number output lines", Numlines);
		oper ("files", "input files", "");
		oper ("-", "insert standard input", "");
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
