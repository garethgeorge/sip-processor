/*
	Function:    filter "Filter Command Line Files In Classic UNIX Style"
	Created:     Sat Aug 10 21:57:12 EDT 1985
	By:          Gary Perlman (Wang Institute, Tyngsboro, MA 01879 USA)
	Copyright    1985 Gary Perlman
	Compilation: nothing unusual
	Tester:      $Compile: cc -DCAT -o cat %f
	Preconditions:
		The index of the first file operand has been determined.
	Postconditions:
		All files have been opened, processed, and closed.
	Returns:
		The return status (non-zero is bad) depends on the accessibility
		of files, the ability to open them, and the return statuses of
		the called function.
	Exceptions:
		If any file cannot be accessed, then none will be processed.
		During processing, if something goes wrong (a file that could
		be accessed cannot be opened, or the file processor returns a
		non-zero status), processing continues.
	Notes:
		"-" is the conventional name for the standard input.
			It can only be read once.
		Fputs and putc are used to print error messages to avoid
			loading fat fprintf just because filter used it.
*/


#include <stdio.h>

#ifdef macintosh
#include <IOCtl.h>
#define isatty(x) (!ioctl(x, FIOINTERACTIVE, NULL))
#endif

#ifndef lint
static	char	sccsid[] = "@(#) filter.c 1.3 (|STAT) 04/01/90";
#endif

#ifdef CAT

int
cat (file, ioptr)
char	*file;
register	FILE	*ioptr;
	{
	register	int 	c;
	while ((c = getc (ioptr)) != EOF)
		putchar (c);
	return (0);
	}

main (argc, argv) char **argv;
	{
	int 	cat ();

	argv[0] = "cat";
	if (filter (argc, argv, 1, cat))
		{
		fprintf (stderr, "Usage: %s [files] [-]\n", argv[0]);
		exit (1);
		}
	exit (0);
	}

#endif


/* LINTLIBRARY */
static
void
errmsg (pgm, file, dflt)
char	*pgm;       /* name of the program running */
char	*file;      /* file operand to be mentioned (if any) */
char	*dflt;      /* default message */
	{
	fputs (pgm, stderr);
	putc (':', stderr);
	putc (' ', stderr);
	fputs (dflt, stderr);
	if (file)
		{
		putc (' ', stderr);
		putc ('\'', stderr);
		fputs (file, stderr);
		putc ('\'', stderr);
		}
	putc ('\n', stderr);
	}

/*FUNCTION filter: process file with filter function */
#define	isstdin(file) (file[0] == '-' && file[1] == '\0')

int
filter (argc, argv, curarg, process)
int 	argc;          /* real number of command line args */
char	**argv;        /* command line argument pointer */
int 	curarg;        /* first argv to filter */
int 	(*process) (); /* status process (char *name, FILE *ioptr) */
	{
	int 	status = 0;         /* return status of this function */
	int 	arg;                /* loop index variable */
	char	*file;              /* name of the current file */
	char	*pgm = argv[0];     /* name of the program */
	FILE	*ioptr;             /* file pointer for opening */
	int 	countstdin = 0;     /* number of times stdin is processed */
	extern	int errno;          /* system error number */

	if (curarg == argc)
		{
		if (isatty (fileno (stdin)))
			fprintf (stderr,"\007%s: Reading input from keyboard:\n", argv[0]);
		status += ((*process) ("-", stdin));
		}
	else
		{
		/* first check to make sure all files can be opened to read */
		for (arg = curarg; arg < argc; arg++)
			{
			file = argv[arg];
			if (isstdin (file))
				{
				countstdin++;
				continue;
				}
			else if (access (file, 4))
				{
				errmsg (pgm, file, "Can't access file");
				status++;
				}
			}
		if (countstdin > 1)
			{
			errmsg (pgm, NULL, "Can only read standard input once");
			status++;
			}
		if (status == 0)
			for (arg = curarg; arg < argc; arg++)
				{
				file = argv[arg];
				if (isstdin (file))
					{
					if (isatty (fileno (stdin)))
						fprintf (stderr,"\007%s: Reading input from keyboard:\n", argv[0]);
					status += ((*process) (file, stdin) != 0);
					}
				else
					{
					if (ioptr = fopen (file, "r"))
						{
						status += ((*process) (file, ioptr) != 0);
						(void) fclose (ioptr);
						}
					else
						{
						errmsg (pgm, file, "Can't open file");
						status++;
						}
					}
				}
		}
	return (status);
	}

/*NOTES
	Some modifications might be useful but unpopular:
		If there is piped input (!isatty (fileno (stdin))),
		and the standard input is not read,
		then some information may be ignored,
		so a warning should be printed.
		Unfortunately, this would break things like vi filters.

		**Mon Oct 13 15:47:22 EDT 1986
		If there is not piped input, 
		and the standard input is being read from the keyboard,
		then prompt the user for input with something like:
			pgm: reading input from terminal
		This would avoid the problem of people forgetting to supply
		an input redirection.
*/
