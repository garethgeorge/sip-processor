/*MANUAL.TH FPACK 1 "October 14, 1986" "Wang Institute" "UNIX User's Manual"
*/

/*MANUAL.SH NAME
fpack \- pack and unpack ascii files with simple archiving scheme
*/

/*MANUAL.SH USAGE
.B fpack
[-v]
[files]
*/

/*MANUAL.SH DESCRIPTION
.I fpack
is a simple plain-text-file archiving scheme to either
reduce the number of files or to package them together.
It is designed to be portable to systems between which
files may be transferred, such as between UNIX and MSDOS.
It can save space on systems that use disk blocks for files that occupy
a small part of a block.
One of the program's requirements is that it does not alter the format
of its input, so files like documents or human readable data files
are not converted to a special format.
This allows unpacking by hand in an emergency
(e.g., the recipient of an archive does not have
.I fpack
to unpack).
.PP
Files are delimited by a special string at the start of a line:
.br
	fpack:!@#$%^&*():	<filename>
.br
*/

/*MANUAL.SH OPTIONS
.de OP
.TP
.B -\\$1 \\$2
..
.OP f
Forceful action.
.I fpack
will overwrite existing files it is unpacking
and continue when it can't open files.
.OP v
Verbose output.
.I fpack
will name the files it packs or unpacks.
*/

/*MANUAL.SH NOTES
.PP
Text outside file delimiters in an archive will be ignored.
So, files packed and sent through mailers
that add header lines and trailing signatures
will be unpacked safely.
.PP
If a file does not end with a newline character,
one will be silently added.
.PP
If a file to be unpacked exists,
then it will not be overwritten.
Instead, the packed contents for the file(s) being unpacked will be ignored.
*/

/*MANUAL.SH EXAMPLES
.nf
Pack up some C source files.
	fpack *.c > archive
Unpack all files.
	fpack < archive
.fi
*/


#include "stat.h"
PGM(fpack,Pack Files Info Plain Text Archive,1.3,11/02/87)

Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */
Boole	Verbose = FALSE;
Boole	Force = FALSE;
#define	blabber(fun,file) Verbose && fprintf (stderr, "fpack: %s '%s'\n", fun, file)

#define	MAGIC	"fpack:!@#$%^&*(): " /* default file delimiter */
#define MAXCHARS BUFSIZ              /* maximum number of chars in lines */

/*FUNCTION main */
main (argc, argv)
int 	argc;     /* argument count */
char	**argv;   /* argument vector */
	{
	Status	result = SUCCESS;
	Status	fpack ();
	int 	optind;

	ARGV0;

	optind = initial (argc, argv);

	if (optind == argc) /* no files, unpack stdin */
		{
		if (isatty (fileno (stdin)))
			{
			fprintf (stderr, "%s: can't unpack input from keyboard\n", Argv0);
			exit (1);
			}
		result = funpack ();
		}
	else
		{
		listfiles (argc, argv, optind);
		result = filter (argc, argv, optind, fpack);
		if (result == SUCCESS)
			printf ("%s\n", MAGIC); /* end of files */
		}

	exit (result);
	}

/*FUNCTION fpack:	pack files for later extraction by funpack */
Status
fpack (file, ioptr)
char	*file;
FILE	*ioptr;
	{
	char	line[MAXCHARS];
	char	*ptr;
	
	blabber ("packing", file);
	printf ("%s%s\n", MAGIC, file); /* this line fixed by Dave Tolle */
	while (fgets (line, sizeof (line), ioptr))
		{
		for (ptr = line; *ptr && *ptr != '\n'; ptr++)
			putc (*ptr, stdout);
		putc ('\n', stdout);
		}
	return (SUCCESS);
	}

listfiles (argc, argv, optind)
char	**argv;
	{
	char	*file;
	printf ("Listing of files in this pack:\n");
	while (optind < argc)
		{
		file = argv[optind++];
		printf ("  %s\n", file);
		}
	}

/*FUNCTION funpack:	unpack and create files packed by fpack */
Status
funpack ()
	{
	FILE	*ioptr = NULL;
	char	line[MAXCHARS];
	int 	maglen = strlen (MAGIC);
	char	*ptr;

	while (gets (line))
		{
		if (!strncmp (MAGIC, line, maglen))
			{
			if (ioptr)
				{
				fclose (ioptr);
				ioptr = NULL;
				}
			ptr = line + maglen;
			if (*ptr == '\0') /* done */
				ioptr = NULL;
			else if (!Force && access (ptr, 4) == 0) /* readable file exists */
				{
				fprintf (stderr, "fpack: '%s' exists (not unpacked)\n", ptr);
				ioptr = NULL;
				}
			else if ((ioptr = fopen (ptr, "w")) == NULL)
				{
				fprintf (stderr, "fpack: Can't create '%s'\n", ptr);
				if (!Force)
					return (FAILURE);
				}
			if (ioptr != NULL)
				blabber ("unpacking", ptr);
			}
		else if (ioptr != NULL)
			{
			fputs (line, ioptr);
			putc ('\n', ioptr);
			}
		}
	return (SUCCESS);
	}


/*FUNCTION initial: returns local version of optind, index to first operand */
int
initial (argc, argv) char **argv;
	{
	extern	char *optarg;    /* option value accessed through this by getopt */
	extern	int  optind;     /* will be index to first operand */
	int 	opterr = 0;      /* count of number of errors */
	int 	flag;            /* option flag characters read in here */
	char	*optstring =     /* getopt string to be filled in */
		"fvLOV";
	char	*usage =         /* part of usage summary to match optstring */
		"[-fv] [files]";

	while ((flag = getopt (argc, argv, optstring)) != EOF)
		switch (flag)
			{
			default:
				opterr++;
				break;
			/* put option cases here */
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			case 'v':
				Verbose = TRUE;
				break;
			case 'f':
				Force = TRUE;
				break;
			}

	if (opterr) /* print usage message and bail out */
		{
		fprintf (stderr, "Usage: %s %s\n", argv[0], usage);
		exit (1);
		}
	
	usinfo ();

	return (optind);
	}

usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXCHARS, "maximum number of characters in lines");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('v', "verbose information about operations", Verbose);
		lopt ('f', "act forcefully on all operations", Force);
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
