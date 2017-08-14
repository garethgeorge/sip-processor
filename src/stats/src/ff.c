/* Copyright 1985 Gary Perlman */

/*MANUAL.TH FF 1 "February 7, 1987" "Wang Institute" "UNIX User's Manual"
.\" $Compile: iroff -man.new %f
.SH NAME
ff \- fast text formatter
.SH USAGE
.B ff
[options] [-] [files]
.SH DESCRIPTION
.I ff
is a simple text formatter for flexible uniform formatting of
input files.
Program options are used to control formatting.
This is in contrast to text formatters like
.I nroff (1)
that require special format requests to be part of their input files.
Besides avoiding cryptic format requests in text,
.I ff
is considerably faster than traditional formatters like
.I nroff (1)
and even simple formatters like
.I fmt (1).
.PP
The most complicated concept with
.I ff
is that of a line break.
A line break causes an interruption in the filling
(evening out of the text lines).
Line breaks occur when special characters are seen at the beginnings
of lines, or when all lines are broken.
By default, any non-alphanumeric character will cause a break,
but this can be controlled with the
.B -B
option.
A blank line always causes a break.
*/


#include <stdio.h>
#include <ctype.h>
#include "string.h"

typedef	int 	Status;
#define	SUCCESS   ((Status) 0)
#define	FAILURE   ((Status) 1)
typedef	int 	Boole;
#define	TRUE      ((Boole) 1)
#define	FALSE     ((Boole) 0)

#define	TAB       '\t'
#define	EOL       '\n'
#define	FF        '\f'
#define	EOS       '\0'
#define	SP        ' '
#define	MAXLEN      512                /* max length of lines */
#define	MAXLINES    200                /* max # lines on pages */
#define MAXCHARS   4096                /* maximum number of chars in lines */

/* Alphabetical listing of this file's functions */
void	beginline ();  /*GF process text at the beginning of lines */
void	beginpage ();  /*GF handle pagination at page breaks */
Status	dobreak ();    /*GF handle broken lines, if appropriate */
void	dofill ();     /*GF do the text filling */
char	*dotab ();     /*GF return expanded tabs in line */
void	endpage ();    /*GF handle pagination at page ends */
char	*expand ();    /*GF expand strings in three part titles */
Status	ff ();         /*GF main formatting routine */
int 	initial ();    /*GF set options and check consistency */
char	*itoa ();      /*GF convert integer to ascii format, with padding */
void	dojustify ();  /*GF even (justify) the right margin of filled lines */
char	*preprocess ();/*GF handle blank trimming and titling */
void	println ();    /*GF print a line, watching for page boundaries */
void	prlims ();     /*GF print function limits */
void	propts ();     /*GF print program information summary */
void	repeat ();     /*GF repeatedly print a character */
Status	setint ();     /*GF check type & convert a string to an integer */
char	*threepart (); /*GF build three part titles */

/* GLOBALS */
char	*Pgm = "ff";         /*GV program name */
char	*Version = "87.6";   /*GV version number */
char	*Date = "02/08/87";  /*GV version date */
int 	Curpos;              /*GV current position on output line */
char	*Filename;           /*GV current input file name */
Boole	Filling;             /*GV is text being filled right now */
char	Justbuf[MAXLEN];     /*GV buffer for justified text */
int 	Justpos;             /*GV current position in justification buffer */
int 	Outline;             /*GV output line number */
int 	Pageline;            /*GV line number on current output page */
int 	Pagenum;             /*GV current page number in current file */
int 	Gpagenum;            /*GV global current page number */
Boole	Needfls;             /*GV must we call fls to get file stats */

/* Default values of options */
#define	MAXTAB     20                  /* max # of tab stops */
#define	FOOTSIZE    5                  /* default footer size */
#define	HEADSIZE    5                  /* default header size */
#define	NUMWIDTH    4                  /* default width of line numbers */
#define	PAGESIZE   66                  /* default length of page */
#define	WIDTH      72                  /* default width of page */
#define	FLSCHAR   '%'                  /* format char used by fls */
#define	PAGENUM   '#'                  /* expands to page number in titles */
#define	FILENAME  '*'                  /* expands to file name in title */
#define	HEADER	  "|File: *||Page: #|" /* default page header */

/*MANUAL.SH OPTIONS
There are many, many options to allow control of
indentation, line width, line spacing, filling,
pagination with headers and footers,
line numbering, right justification,
and perhaps some other things.
They have extensive type and range checking
that produces diagnostic error messages,
so warnings of obviously wrong options will not be discussed here.
In general, options that imply the use of others
work the way they should; if the page size is set,
then pagination is automatically assumed.
Some combinations of options give impressive, even artistic, effects.
Making a small text file and playing with it is the easiest
way to learn how the options interact.
.de OP
.TP
.B -\\$1 \\$2
..
*/

	Boole	Breaklines = FALSE; /*MANUAL.OP b
Break all lines of text.
That is, don't even-out lines by filling.
By default, text lines are filled.
*/

	char	*Breakchars = NULL; /*MANUAL.OP B breakchars
Change the set of characters that cause line breaks at the start of lines to
.I breakchars.
By default, any characters but letters and numbers cause a break.
A good choice for break characters might be "*-+" and TABS
that might be used for lists.
*/

	Boole	Center = FALSE; /*MANUAL.OP c
Center all lines of text.
This option stops all filling of text.
*/

	Boole	Delspace = FALSE; /*MANUAL.OP d
Delete white space at the beginning and end of lines.
This option is useful to help un-format text to be re-formatted.
*/

	Boole	Delline = FALSE; /*MANUAL.OP D
Delete empty input lines.
An input line is empty if it has no characters,
not even invisible character like tabs or spaces.
This option can be combined with the option to remove white space
to delete visibly blank lines.
*/

	/*BUGFIX Rich Messenger found the bug that Footer should be "", not NULL */
	char	*Footer = ""; /*MANUAL.OP f footer
Set the page footer to the title-string
.I footer.
The default page footer is blank.
Titles can be any string,
but if the first character is not a letter or a digit,
but a punctuation character like /,
then that character separates the left,
center, and right fields of a title.
For example, the title
.ce
"/ff: fast formatter/#/1985/"
would have "ff: fast formatter" as a left justified field
and 1985 as a right justified field on each page.
The page number, within the input file, would be centered in the title.
There are two special characters, # and *,
that respectively are variables for the page number in a file
and the input file name.
The global page number, which is not reset for each file, is ##.
.ti +.5i
There is a whole language that allows you to insert information
about the file being printed.
You can see online help for this language by running the command:
.ce
	echo | ff -f %?
It is a complicated language, but you can avoid it by avoiding the
percent (%) character in titles.
*/

	int 	Footsize = FOOTSIZE; /*MANUAL.OP F footersize
Set the number of blank lines at the bottom of the page.
The footer, if any, is placed in the middle of the space,
which by default, is five lines.
If
.I footersize
is 0, no footer will be printed.
*/

	char	*Header = HEADER; /*MANUAL.OP h header
Set the page header.
See the description of three-part titles for the
.B -f footer
option.
The default page header is
.ce
"|File: *||Page: #|".
*/

	int 	Headsize = HEADSIZE; /*MANUAL.OP H headersize
See the description of the footer size.
*/

	int 	Indent = 0; /*MANUAL.OP i indent
Set the indentation of the text to
.I indent
spaces.
Note that this effectively reduces the usable width of the page.
*/

	int 	Tindent = 0; /*MANUAL.OP I tempindent
Set the temporary indent.
This causes filled text found immediately after a break to
be indented for one line.
It is useful for indenting the first lines of paragraphs.
If
.I tempindent
is negative,
the the temporary indent will be relative to the current
.I indent
value.
If the
.I tempindent
value is more negative than the
.I indent
value is positive,
.I ff
will automatically increase
.I indent.
*/

	Boole	Justify = FALSE; /*MANUAL.OP j
Justify the text.
That is, even the right margin by inserting spaces in the line.
Only filled text can be justified.
*/

	Boole	Numlines = FALSE; /*MANUAL.OP n
Number all output lines produced by the input text.
Lines from multiple line spacing or pagination will not
be numbered.
*/

	int 	Numwidth = NUMWIDTH; /*MANUAL.OP N numberwidth
Set the width of the line numbers.
The default is to take up 4 spaces.
Note that this effectively reduces the usable part of the page.
*/

	/*MANUAL.OP O
Print a summary of all the options and stop.
*/

	Boole	Paginate = FALSE; /*MANUAL.OP p
Paginate the output.
See the options for page header and footer control.
*/

	int 	Pagesize = PAGESIZE; /*MANUAL.OP P pagesize
Set the number of lines in a page to
.I pagesize.
By default, the standard 66 line page is assumed.
*/

	int 	Spacing = 1; /*MANUAL.OP s spacing
Set the line spacing.
By default, text is single spaced (\fIspacing\fR equals 1).
*/

	int 	Tab[MAXTAB];
	int 	Ntabs = 0; /*MANUAL.OP t tab
Set individual absolute and relative tab stops.
These values tell the formatter
where to put the text (from an unfilled line)
that follows a tab character.
Each tab stop is supplied with its own
.B -t
option; there is no way to bundle them.
.I tab
values can be integers without a plus sign.
These are \fIabsolute\fR tab settings;
the tabs go to that position.
The values must increase monotonically.
If a
.I tab
value is preceded by a plus sign,
then it is interpreted \fIrelative\fR to the previous tab setting.
For example, a tab setting of 40 followed by one of +20
will set the second tab stop to 60.
*/

	int 	Alltabs = 0; /*MANUAL.OP T tabs
Set tab stops to every
.I tabs
spaces.
It is useful to get equally spaced tabs.
This option cannot be used with the other tab setting option.
*/

	Boole	Uppercase = FALSE; /*MANUAL.OP u
Print All Input Text As Initial Upper-Case Titles,
Like This Sentence.
This option goes well with the one for centering lines.
*/

	int 	Width = WIDTH; /*MANUAL.OP w width
Set the page width.
By default, the page width is 72 characters.
Note that the usable line length is sometimes less
than the page width.
If line numbering or indentation is requested,
these subtract from the line length.
*/

/*MANUAL.SH EXAMPLES
Some of these examples can make shell scripts or aliases.
.nf
.ta .5i
.sp
Centered Titles: title
	ff  -dcu  $*
Double Spaced Unfilled Paginated indented (for editing): draft
	ff  -s 2  -b  -p  -f "`date`"  -i 8  $*
Program Listing: cpr
	H="@        Dir: `pwd`@@File: *@"
	F="@        $NAME@`date`@Page #@"
	ff  -b  -N 8  -H 3  -h "$H"  -F 3  -f "$F"  -T 4  -w 79  -i 2  $*
Reformat Paragraphed Text: nr
	ff  -jd  -I 5  -i 10  -w 65  -B "TAB SP'*.@"  $*
.fi
*/
/*MANUAL.SH DIAGNOSTICS
Some options are incompatible with others.
For example, centered text cannot be right-justified.
.I ff
will not allow inconsistent combinations of options.
*/
/*MANUAL.SH "SEE ALSO"
fmt(1), nroff(1), scribe(1w)
*/
/*MANUAL.SH AUTHOR
Gary Perlman (with help from many students)
*/
/*MANUAL.SH STATUS
Pretty well tested.
*/

/* Macro Functions */
/*MACRO isendsent: is the character one that ends a sentence? */
#define	isendsent(c) ((c) == '.' || (c) == '?' || (c) == '!')

/*MACRO justchar: add char to a buffer that will later be flushed */
#define	justchar(c) (Justbuf[Justpos++] = (c))

/*MACRO fillchar: save to justify if necessary, else output */
#define fillchar(c) \
	{ \
	if (Justify == TRUE) justchar (c); \
	else putc (c, stdout); \
	}

/*FUNCTION main: loop through files in classic UNIX filter style */
main (argc, argv)
int 	argc;     /* argument count */
char	**argv;   /* argument vector */
	{
	Status 	ff ();      /* ff (file, ioptr) will filter files */
	int 	filter ();  /* runs ff on all filtered files */
	Status	status;     /* return status of filter () */
	int 	firstfile;  /* first file name index returned by initial */

	firstfile = initial (argc, argv);
	status = filter (argc, argv, firstfile, ff);
	exit (status);
	/*NOTREACHED*/
	}

/*FUNCTION prlims: print program limits */
void
prlims (ioptr)
FILE	*ioptr;
	{
	fprintf (ioptr, "%s: program limits:\n", Pgm);
	fprintf (ioptr, "%4d	maximum number of characters in input lines\n", MAXCHARS);
	fprintf (ioptr, "%4d	maximum output line length\n", MAXLEN);
	fprintf (ioptr, "%4d	maximum number of lines per page\n", MAXLINES);
	fprintf (ioptr, "%4d	maximum number of tab stops\n", MAXTAB);
	}

/*FUNCTION propts: print program options */
void
propts (ioptr)
FILE	*ioptr;
	{
	fprintf (ioptr, "%s: Fast Text Formatter Options:\n", Pgm);
	fprintf (ioptr, "  Online help: -L gives limits, -O gives options, -V gives version\n");
	fprintf (ioptr, "-b  	break all lines of text--do no filling\n");
	fprintf (ioptr, "-B s	line break characters\n");
	fprintf (ioptr, "-c  	center all text lines\n");
	fprintf (ioptr, "-d  	delete blank space around input lines\n");
	fprintf (ioptr, "-D  	delete blank input lines\n");
	fprintf (ioptr, "-f s	page footer three-part title\n");
	fprintf (ioptr, "-F i	page footer size (%d lines)\n", Footsize);
	fprintf (ioptr, "-h s	page header three-part title (%s)\n", Header);
	fprintf (ioptr, "-H i	page header size (%d lines)\n", Headsize);
	fprintf (ioptr, "-i i	text indentation (%d spaces)\n", Indent);
	fprintf (ioptr, "-I i	indent after line breaks (%d spaces)\n", Tindent);
	fprintf (ioptr, "-j  	justify the right margin of the text\n");
	fprintf (ioptr, "-n  	number output text lines\n");
	fprintf (ioptr, "-N i	width of line numbers (%d spaces)\n", Numwidth);
	fprintf (ioptr, "-p  	paginate output\n");
	fprintf (ioptr, "-P i	page size (%d lines)\n", Pagesize);
	fprintf (ioptr, "-s i	line spacing (%d line)\n", Spacing);
	fprintf (ioptr, "-t i	absolute or relative tab stop\n");
	fprintf (ioptr, "-T i	uniform tab stops\n");
	fprintf (ioptr, "-u  	show text with upper-case initial letters\n");
	fprintf (ioptr, "-w i	page line width (%d spaces)\n", Width);
	}

/*FUNCTION initial: set options */
int
initial (argc, argv)
int 	argc;
char	**argv;
	{
	extern	char	*optarg;   /* string value to option set by getopt */
	extern	int 	optind;    /* will be index of first command operand */
	int 	errcount = 0;      /* count of number of errors */
	int 	flag;              /* options flag names read in here */
	char	*optstring =       /* Boolean flags and integer options with : */
		"LOVbcdDjnpuB:f:F:h:H:i:I:N:P:s:t:T:w:";
	Boole	optinfo = FALSE;   /* print option information */
	Boole	liminfo = FALSE;   /* print limits information */
	Boole	verinfo = FALSE;   /* print version information */

	argv[0] = Pgm;
	while ((flag = getopt (argc, argv, optstring)) != EOF)
		switch (flag)
		{
		default:
			errcount++;
			break;

		case 'b':
			Breaklines = TRUE;
			break;

		case 'B':
			Breakchars = optarg;
			break;

		case 'c':
			Center = TRUE;
			Breaklines = TRUE;
			break;

		case 'd':
			Delspace = TRUE;
			break;

		case 'D':
			Delline = TRUE;
			break;

		case 'f':
			Footer = optarg;
			while (*optarg)
				if (*optarg++ == FLSCHAR)
					Needfls = TRUE;
			Paginate = TRUE;
			break;

		case 'F':
			if (setint (Pgm, flag, optarg, &Footsize, 0, MAXLINES) == FAILURE)
				errcount++;
			Paginate = TRUE;
			break;

		case 'h':
			Header = optarg;
			while (*optarg)
				if (*optarg++ == FLSCHAR)
					Needfls = TRUE;
			Paginate = TRUE;
			break;

		case 'H':
			/*BUGFIX Headsize must be greater than zero because it is used
			to determine where page breaks occur */
			if (setint (Pgm, flag, optarg, &Headsize, 1, MAXLINES) == FAILURE)
				errcount++;
			Paginate = TRUE;
			break;

		case 'i':
			if (setint (Pgm, flag, optarg, &Indent, 0, MAXLEN) == FAILURE)
				errcount++;
			break;

		case 'I':
			if (setint (Pgm, flag, optarg, &Tindent, -MAXLEN, MAXLEN) == FAILURE)
				errcount++;
			break;

		case 'j':
			Justify = TRUE;
			break;

		case 'L':
			liminfo = TRUE;
			break;

		case 'N':
			if (setint (Pgm, flag, optarg, &Numwidth, 1, MAXLEN) == FAILURE)
				errcount++;
			/* FALLTHROUGH */

		case 'n':
			Numlines = TRUE;
			break;

		case 'O':
			optinfo = TRUE;
			break;

		case 'P':
			if (setint (Pgm, flag, optarg, &Pagesize, 1, MAXLINES) == FAILURE)
				errcount++;
			/* FALLTHROUGH */

		case 'p':
			Paginate = TRUE;
			break;

		case 's':
			if (setint (Pgm, flag, optarg, &Spacing, 1, MAXLINES) == FAILURE)
				errcount++;
			break;

		case 't':
			if (Ntabs >= MAXTAB)
				{
				fprintf (stderr, "%s: at most %d -%c options allowed\n",
					Pgm, MAXTAB, flag);
				errcount++;
				}
			else if (setint (Pgm, flag, optarg, &Tab[Ntabs], 0, MAXLEN) == FAILURE)
				errcount++;
			else if (Ntabs > 0)
				{
				if (*optarg == '+')
					Tab[Ntabs] += Tab[Ntabs-1];
				else if (Tab[Ntabs] <= Tab[Ntabs-1])
					{
					fprintf (stderr, "%s: -%c values must increase\n",
						Pgm, flag);
					errcount++;
					}
				}
			if (Tab[Ntabs] >= MAXLEN)
				{
				fprintf (stderr, "%s: -%c values must be < %d\n",
					Pgm, flag, MAXLEN);
				errcount++;
				}
			Ntabs++; /*BUG should not increment on error */
			break;

		case 'T':
			if (setint (Pgm, flag, optarg, &Alltabs, 1, MAXLEN) == FAILURE)
				errcount++;
			break;

		case 'u':
			Uppercase = TRUE;
			break;
		
		case 'V':
			verinfo = TRUE;
			break;

		case 'w':
			if (setint (Pgm, flag, optarg, &Width, 1, MAXLEN) == FAILURE)
				errcount++;
			break;
		}

	/* check for requests for online help */
	if (verinfo || liminfo || optinfo)
		{
		if (verinfo)
			fprintf (stdout, "Program: %s  Version: %s  Date: %s\n",
				Pgm, Version, Date);
		if (liminfo)
			prlims (stdout);
		if (optinfo)
			propts (stdout);
		exit (0);
		}

	/* Now check validity of option settings */
	if (Tindent < 0 && Indent < (-Tindent))
		Indent = (-Tindent);
	if (Ntabs > 0 && Alltabs > 0)
		{
		fprintf (stderr, "%s: can't set individual and all tabs\n", Pgm);
		errcount++;
		}
	if (Center == TRUE && Justify == TRUE)
		{
		fprintf (stderr, "%s: centering and justifying incompatible\n", Pgm);
		errcount++;
		}
	else if (Breaklines == TRUE && Justify == TRUE)
		{
		fprintf (stderr, "%s: breaking and justifying incompatible\n", Pgm);
		errcount++;
		}
	if (Ntabs > 0 && Center == TRUE)
		{
		fprintf (stderr,"%s: centering and setting tabs incompatible\n", Pgm);
		errcount++;
		}
	if ((Ntabs > 0 || Alltabs > 0) && (Justify == TRUE))
		{
		fprintf (stderr, "%s: tabstops and justifying incompatible\n", Pgm);
		errcount++;
		}

	/* Print an error message and exit or return index to first file name */
	if (errcount > 0)
		{
		fprintf (stderr, "Usage: %s [options] [-] [files]\n", Pgm);
		fprintf (stderr, "\t(Use the -O option to get an option summary)\n");
		exit (FAILURE);
		}
	return (optind);
	}

/*FUNCTION repeat: repeat a character some number of times */
void
repeat (c, n)
int 	c;     /* character to print */
int 	n;     /* number of times to print c */
	{
	while (n-- > 0)
		putc (c, stdout);
	}

/*FUNCTION dotab: expand tabs to spaces to tab stops, returns static buffer */
/*ALGORITHM
	if all tabs are set to the same value (Alltabs > 0)
		then advance on a tab to the next tab stop
	else if there are individual tabs set (Ntabs > tabno)
		then advance (possibly retreat!) to next set tab
	else just treat the tab like a space character
*/
char *
dotab (line)
register	char	*line;
	{
	static	char outline[MAXLEN];   /* new line will be built here */
	register char *lptr;            /* pointer to current position in outline */
	register char *nextptr;         /* position of next tab stop */
	int 	tabno = 0;              /* how many tabs have been processed */

	for (lptr = outline; *line != EOS && *line != EOL; line++)
		{
		if (*line == TAB)
			{
			if (Alltabs > 0)
				nextptr = lptr + Alltabs - ((lptr - outline) % Alltabs);
			else if (Ntabs > tabno) /* move to next set tab */
				nextptr = outline + Tab[tabno++];
			else
				nextptr = lptr + 1;
			if (lptr < nextptr)
				do	{
					*lptr++ = SP;
				} while (lptr < nextptr);
			else lptr = nextptr;
			}
		else
			*lptr++ = *line;

		/* check for line overflow */
		if (lptr >= (outline + MAXLEN))
			return (NULL);
		}

	/* end the expanded tab string and return */
	*lptr = EOS;
	return (outline);
	}

/*FUNCTION dojustify: even the right margin for all lines */
/*ALGORITHM	directly output any indenting spaces (this is not expanded)
			trim trailing spaces (don't want to pad at end)
			count spaces inside line (where extra spaces will be inserted)
			distribute the extra spaces needed among the spaces there
	Note: if we are not filling (e.g., last line of output before a break)
		then we do not justify the right margin.
*/
void
dojustify ()
	{
	register char *line;       /* will point to first non-space char on line */
	int 	width;             /* width of line to pad - #'s and indent */
	register char *lptr;       /* zips through the line */
	int     pad;               /* will need to pad with this many spaces */
	int     spaces = 0;        /* will be number of embedded spaces in line */
	register int i;            /* used inside inner loop */
	int 	n = 0;             /* number of spaces before space insert */

	if (Justpos == 0)          /* nothing to justify, so bail out */
		return;

	/* strip spaces from end of line and end with EOS */
	for (lptr = Justbuf+Justpos; lptr>Justbuf && isspace(lptr[-1]); lptr--)
		continue;
	*lptr = EOS;

	width = Width - (Numlines == TRUE ? Numwidth : 0);
	pad = width - (lptr - Justbuf);

	for (line = Justbuf; *line == SP; line++)
		putc (*line, stdout);

	if (Filling == TRUE && pad > 0) /* might not fill last line of output */
		for (lptr = line; *lptr != EOS; lptr++)
			if (*lptr == SP)
				spaces++;

	if (spaces > 0) /* we have places to insert spaces */
		{
		for (lptr = line; *lptr != EOS; lptr++)
			{
			if (*lptr == SP)
				for (i = 0; i < pad; i++)
					if (++n == spaces)
						{
						putc (SP, stdout);
						n = 0;
						}
			putc (*lptr, stdout);
			}
		}
	else /* just output the line */
		for (lptr = line; *lptr != EOS; lptr++)
			putc (*lptr, stdout);

	Justpos = 0; /* reset buffer position for next line */
	}

/*FUNCTION println: print lines while watching for page boundaries */
void
println (count)
int 	count;    /* how many lines to print (== Spacing) */
	{
	Curpos = 0;
	while (count-- > 0)
		{
		putc (EOL, stdout);
		Pageline++;
		if (Paginate == TRUE && ((Pagesize - Pageline) == Footsize))
			{
			endpage (TRUE);
			return;
			}
		}
	}

/*FUNCTION beginline: do any needed justification, paginate if requested, */
/*ALGORITHM
	handle line numbering
	temp and regular indents based on prev. filling
*/
void
beginline (filling)
Boole	filling;     /* are we filling now? */
	{
	int 	count;
	Boole	newfill = (Filling == FALSE && filling == TRUE);

	Filling = filling;
	if (Justify == TRUE)
		dojustify ();
	Outline++;
	Curpos = 0;

	if (Paginate == TRUE && Pageline == 0)
		beginpage ();
	else if (Outline > 1)
		println (Spacing);

	if (Numlines == TRUE)
		{
		char	*ptr = itoa (Outline, Numwidth - 1);
		Curpos += Numwidth;
		while (*ptr)
			putc (*ptr++, stdout);
		putc (SP, stdout);
		}

	count = Indent;
	if (newfill == TRUE)
		count += Tindent;
	Curpos += count;

	if (Justify == TRUE)
		while (count-- > 0)
			justchar (SP);
	else
		repeat (SP, count);
	}

/*FUNCTION itoa: integer to ascii conversion */
char *
itoa (n, pad)
register int n;    /* the integer to be printed as a string */
int 	pad;       /* amount of space to pad number to */
	{
	static char numbuf[MAXLEN]; /* answer built in here */
	register char *nptr;        /* will be pointer to beginning of number */
	Boole	negflg = FALSE;     /* is the number a negative value? */

	/* static numbuf is initialized to 0's, so numbuf[MAXLEN-1] == EOS */
	if (n == 0)
		{
		nptr = &numbuf[MAXLEN-1];
		*--nptr = '0';
		}
	else
		{
		if (n < 0)
			{
			n = (-n);
			negflg = TRUE;
			}
		for (nptr = &numbuf[MAXLEN-1]; n != 0; n /= 10)
			*--nptr = (n % 10) + '0';
		if (negflg == TRUE)
			*--nptr = '-';
		}

	while (pad > numbuf+MAXLEN-1-nptr)
		*--nptr = SP;

	return (nptr);
	}

/*FUNCTION expand: insert file/page for characters in field of 3part title */
char *
oldexpand (title, gpage, page, file)
register	char	*title;   /* the title to be expanded */
int 	gpage;                /* global page number */
int 	page;                 /* the current page number in current file */
char	*file;                /* the name of the current file */
	{
	static 	char answer[MAXLEN];  /* title expanded into this buffer */
	register 	char *aptr;       /* pointer to answer buf */
	register	char *ptr;        /* generic string handling pointer */
	
	for (aptr = answer; title != NULL && *title != EOS; title++)
		switch (*title)
		{
		default: 
			*aptr++ = *title;
			break;
	
		case PAGENUM:
			if (title[1] == PAGENUM) /* double pagenum gets global page */
				{
				ptr = itoa (gpage, 0);
				title++;
				}
			else
				ptr = itoa (page, 0);
			while (*ptr != EOS)
				*aptr++ = *ptr++;
			break;
	
		case FILENAME:
			for (ptr = file; *ptr != EOS; ptr++)
				*aptr++ = *ptr;
			break;
		}
	*aptr = EOS;
	return (answer);
	}

/*FUNCTION expand: insert file/page for characters in field of 3part title */
char *
expand (title, gpage, page, file)
char	*title;               /* the title to be expanded */
int 	gpage;                /* the global page number */
int 	page;                 /* the current page number in file */
char	*file;                /* the name of the current file */
	{
	static 	char answer[MAXLEN];  /* title expanded into this buffer */
	char	numbuf[MAXLEN];       /* titles with page numbers expanded here */
	
	strcpy (numbuf, oldexpand (title, gpage, page, file));
	if (Needfls)
		fls (file, numbuf, answer);
	else
		strcpy (answer, numbuf);
	return (answer);
	}

/*FUNCTION threepart: 3-part title with left/center/right justified fields */
/*  any punctuation character can be the title delimiter: see nroff(1) */
char *
threepart (title, gpage, page, file, width)
char	*title;   /* the three part title */
int 	gpage;    /* the global current page number */
int 	page;     /* the current page number in this file */
char	*file;    /* the current file name */
int 	width;    /* the current page width */
	{
	static char answer[MAXLEN];   /* answer stuffed in here */
	register char *aptr;          /* pointer to answer buffer */
	int 	delim;                /* title delimiter character */

	title = expand (title, gpage, page, file);
	if (!ispunct (*title))
		return (title);
	for (aptr = answer; aptr < answer + width; aptr++)
		*aptr = SP;
	aptr = answer;
	delim = *title++;
	while (*title != EOS && *title != delim)
		*aptr++ = *title++;
	if (*title++ != EOS) /* now do center */
		{
		char	*lptr = title;
		while (*lptr != EOS && *lptr != delim)
			lptr++;
		aptr = answer + (width - (lptr - title)) / 2;
		if (aptr >= answer)
			while (*title != EOS && *title != delim)
				*aptr++ = *title++;
		else
			while (*title != EOS && *title != delim)
				title++;
		if (*title++ != EOS) /* now do left part */
			{
			char	*eptr = title;
			while (*eptr != EOS && *eptr != delim)
				eptr++;
			eptr--;
			aptr = answer + width - 1;
			while (eptr >= title)
				*aptr-- = *eptr--;
			}
		}
	answer[width] = EOS;
	return (answer);
	}

/*FUNCTION beginpage: handle page header */
void
beginpage ()
	{
	Pagenum++;
	Gpagenum++;
	if (Paginate == TRUE && Headsize > 0)
		{
		int 	space1 = Headsize / 2;
		int 	space2 = Headsize - space1;
		char	*optr = threepart (Header, Gpagenum, Pagenum, Filename, Width);

		repeat (EOL, space1);
		while (*optr != EOS)
			putc (*optr++, stdout);
		repeat (EOL, space2);
		Pageline = Headsize;
		}
	}

/*FUNCTION endpage: break filling (output justified text) print footer */
/*ALGORITHM
	begin a new page if there is more
*/
void
endpage (more)
Boole	more;
	{
	if (Justify == TRUE)
		dojustify ();

	if (Paginate == TRUE)
		{
		int 	space1 = Footsize / 2;
		int 	space2 = Footsize - space1;
		char	*optr = threepart (Footer, Gpagenum, Pagenum, Filename, Width);

		repeat (EOL, Pagesize - Pageline - space2);
		while (*optr != EOS)
			putc (*optr++, stdout);
		repeat (EOL, space2);
		Pageline = 0;
		}
	else if (Curpos > 0 || Indent > 0 || Numlines == TRUE)
		putc (EOL, stdout);

	if (more == TRUE)
		beginpage ();
	}

/*FUNCTION preprocess: trim space, find blank lines, do titling */
char *
preprocess (line)
register	char	*line;
	{
	register	char	*lptr;
	if (Paginate == TRUE && *line == FF)
		{
		line++;
		endpage (TRUE);
		}
	if (Delspace == TRUE) /* delete blank space */
		{
		while (isspace (*line))
			line++;
		for (lptr = line; *lptr != EOS; lptr++)
			continue;
		while (lptr > line && isspace (*(lptr-1)))
			lptr--;
		*lptr = EOS;
		}
	if (Delline == TRUE && (*line == EOL || *line == EOS))
		return (NULL);
	if (Uppercase == TRUE)
		{
		Boole 	newword;    /* are we at the start of a new word? */
		for (lptr = line, newword = TRUE; *lptr != EOS; lptr++)
			{
			if (newword && islower (*lptr))
				*lptr = toupper (*lptr);
			newword = !isalnum (*lptr);
			}
		}
	return (line);
	}

/*FUNCTION dobreak: process broken line & return success status */
Status
dobreak (lptr)
register char *lptr;    /* line to print out */
	{
	/* break: do tabs, or centering, and then dump the line out */
	beginline (FALSE);
	if (Alltabs > 0 || Ntabs > 0) /* some tabs have been set */
		{
		if ((lptr = dotab (lptr)) == NULL)
			{
			fprintf (stderr, "%s: malformed tab in %s at line %d\n",
				Pgm, Filename, Outline);
			return (FAILURE);
			}
		}
	else if (Center == TRUE && *lptr != EOS && *lptr != EOL)
		repeat (SP, (Width - strlen (lptr)) / 2);
	while (*lptr != EOS && *lptr != EOL)
		putc (*lptr++, stdout);
	Curpos = Width; /* signal end of line */
	return (SUCCESS);
	}

/*FUNCTION dofill: do line filling */
void
dofill (lptr)
register	char	*lptr;
	{
	register char *eptr;       /* pointer to end of filled words */
	register int wordlen;      /* length of words */

	while (isspace (*lptr))
		lptr++;
	while (*lptr != EOS) /* fill text by picking up word by word */
		{
		eptr = lptr;
		while (*eptr != EOS && !isspace (*eptr))
			eptr++;
		wordlen = eptr - lptr;
		if ((Outline == 0) || (Curpos + wordlen) >= Width)
			beginline (TRUE);
		else if (Curpos < Width) /* space before word */
			{
			fillchar (SP);
			Curpos++;
			}
		for (Curpos += wordlen; lptr < eptr; lptr++)
			fillchar (*lptr);
		while (isspace (*lptr))
			lptr++;

		/* extra space at sentence ends (.?!) */
		if ((Curpos < Width) && isendsent (*(eptr-1)) && wordlen > 2)
			{
			fillchar (SP);
			Curpos++;
			}
		}
	}

/*FUNCTION ff: main formatting routine */
/*ALGORIITHM
	initializes for each new file, reads lines, trims space and blank lines
	switches between filling and non filling based on breakchars
*/
Status
ff (file, ioptr)
char	*file;    /* file name */
FILE	*ioptr;   /* opened input pointer */
	{
	char	line[MAXCHARS];    /* lines read in here */
	register char *lptr;       /* pointer used to go through line */
	Status	status = SUCCESS;

	Outline = Pagenum = Pageline = Curpos = 0;
	Filename = file;
	Filling = FALSE;

	while (fgets (lptr = line, sizeof (line), ioptr))
		{
		if ((lptr = preprocess (lptr)) == NULL)
			continue;

		if ((Breaklines == TRUE)
		|| (*lptr == EOL)
		|| (Breakchars ? strchr (Breakchars, *lptr) != NULL : !isalnum (*lptr)))
			{
			if (dobreak (lptr) == FAILURE)
				return (FAILURE);
			}

		else

			dofill (lptr);
		}

	Filling = FALSE;
	endpage (FALSE);
	return (status);
	}
