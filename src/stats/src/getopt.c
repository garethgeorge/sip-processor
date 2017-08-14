/*
	I got this off net.sources from Henry Spencer.
	It is a public domain getopt(3) like in System V.
	I have made the following modifications:

	A test main program was added, ifdeffed by GETOPT.
	This main program is a public domain implementation
	of the getopt(1) program like in System V.  The getopt
	program can be used to standardize shell option handling.
		e.g.  cc -DGETOPT getopt.c -o getopt
*/
#include <stdio.h>
#include <string.h>

#ifndef lint
static	char	sccsfid[] = "@(#) getopt.c 5.1 (UTZoo) 1985";
#endif

/*HISTORY
	1985	original version
	6/26/90	added static strchr to avoid system difference problems
	8/15/92	changed name of strchr to mystrchr to avoid name class problems
*/

/* this is included because strchr is not on some UNIX systems */
static
char *
mystrchr (s, c)
register	char	*s;
register	int 	c;
	{
	if (*s)
		while (*s)
			if (c == *s)
				return (s);
			else
				s++;
	return (NULL);
	}

#define	ARGCH    (int)':'
#define BADCH	 (int)'?'
#define EMSG	 ""
#define	ENDARGS  "--"

/*
 * get option letter from argument vector
 */
int	opterr = 1,		/* useless, never set or used */
	optind = 1,		/* index into parent argv vector */
	optopt;			/* character checked for validity */
char	*optarg;		/* argument associated with option */

#define tell(s)	fputs(*nargv,stderr);fputs(s,stderr); \
		fputc(optopt,stderr);fputc('\n',stderr);return(BADCH);


int
getopt(nargc,nargv,ostr)
int	nargc;
char	**nargv,
	*ostr;
{
	static char	*place = EMSG;	/* option letter processing */
	register char	*oli;		/* option letter list index */

	if(!*place) {			/* update scanning pointer */
		if(optind >= nargc || *(place = nargv[optind]) != '-' || !*++place) return(EOF);
		if (*place == '-') {	/* found "--" */
			++optind;
			return(EOF);
		}
	}				/* option letter okay? */
	if ((optopt = (int)*place++) == ARGCH || !(oli = mystrchr(ostr,optopt))) {
		if(!*place) ++optind;
		tell(": illegal option -- ");
	}
	if (*++oli != ARGCH) {		/* don't need argument */
		optarg = NULL;
		if (!*place) ++optind;
	}
	else {				/* need an argument */
		if (*place) optarg = place;	/* no white space */
		else if (nargc <= ++optind) {	/* no arg */
			place = EMSG;
			tell(": option requires an argument -- ");
		}
	 	else optarg = nargv[optind];	/* white space */
		place = EMSG;
		++optind;
	}
	return(optopt);			/* dump back option letter */
}


#ifdef GETOPT

#ifndef lint
static	char	sccspid[] = "@(#) getopt.c 5.1 (WangInst) 6/15/85";
#endif

main (argc, argv) char **argv;
	{
	char	*optstring = argv[1];
	char	*argv0 = argv[0];
	extern	int 	optind;
	extern	char	*optarg;
	int 	opterr = 0;
	int 	C;
	char	*opi;
	if (argc == 1)
		{
		fprintf (stderr, "Usage: %s optstring args\n", argv0);
		exit (1);
		}
	argv++;
	argc--;
	argv[0] = argv0;
	while ((C = getopt (argc, argv, optstring)) != EOF)
		{
		if (C == BADCH) opterr++;
		printf ("-%c ", C);
		opi = mystrchr (optstring, C);
		if (opi && opi[1] == ARGCH)
			if (optarg)
				printf ("\"%s\" ", optarg);
			else opterr++;
		}
	printf ("%s", ENDARGS);
	while (optind < argc)
		printf (" \"%s\"", argv[optind++]);
	putchar ('\n');
	exit (opterr);
	}

#endif
