/*  Copyright 1980 Gary Perlman */

/*LINTLIBRARY*/
#ifndef STAT_H
#define STAT_H

#define	STAT_H_VERSION "@(#) stat.h 5.10 (|stat) 04/14/93"
#define	STAT_H_COPYRIGHT "@(#) Copyright 1980 Gary Perlman  All Rights Reserved"

#ifdef	bsd /* Berkeley compatibility macros */
#define	strchr(s,c)  index(s,c)
#define	strrchr(s,c) rindex(s,c)
#endif	/* BSD */

#include "stdio.h"
#include "ctype.h"
#include "math.h"
#include "string.h"

#include "const.h"

#include "stdlib.h"
#define	myalloc(type,size)  ((type *) malloc ((unsigned)(size)*sizeof(type)))
#define strdup(s)	strcpy((char *)malloc((unsigned)strlen(s)+1),s)

#ifndef atof /* may be a macro on some systems (e.g., Sun) */
extern	double	atof ();
#endif

/******************************* LIMITS *****************************/
#ifndef	MAXINT
# define MAXINT	(sizeof(int) == 4 ? 2147483647 : 32767)
#endif

#define	MIN_PLOT      5         /* plots at least 5 x 5 */
#ifdef __MSDOS__
#define	MAX_WIDTH   80          /* plot at most this wide */
#define	MAX_HEIGHT  60          /* plot at most this high */
#else
#define	MAX_WIDTH   100         /* plot at most this wide */
#define	MAX_HEIGHT  100         /* plot at most this high */
#endif

#define	FZERO	10e-10       /* smaller |values| are considered 0.0 */
#define	fzero(x) (fabs (x) < FZERO)
#define	MAXF    9999.0       /* infinite F ratio */
#define	MAXT      99.0       /* infinite t value */

/******************************* TYPES *****************************/
typedef	int       Status;      /* return/exit status of functions */
#define	SUCCESS   ((Status) 0)
#define	FAILURE   ((Status) 1)
typedef	int       Boole;       /* no Boolean type in C */
#define	TRUE      ((Boole) 1)
#define	FALSE     ((Boole) 0)
typedef	unsigned Posint;       /* mostly an abbreviation */

#ifdef	__STDC__
#define	Cdecl(x)   x    /* declare C function with prototype for params */
#else
#define	Cdecl(x)   ()   /* ignore parenthesized param list */
#endif

/******************************* SYSTEMS *****************************/
#ifdef lint /* define null strings for information */
#	define	PGM(name,purpose,ver,date) char *Argv0, *Version, *Purpose;
#	define	FUN(name,purpose,ver,date) extern char *Argv0;
#else
#	ifdef __STDC__ /* used on DOS and on ANSI C compilers */
# define PGM(name,purpose,ver,date) \
	char *Argv0 = #name;\
	static char CpRt[] = STAT_H_COPYRIGHT;\
	static char sccshid[] = STAT_H_VERSION;\
	static char sccspid[] = "@(#) " #name ".c " #ver " (|stat) " #date;\
	static char *Purpose = #purpose;\
	static char	*Version = "Program: " #name "  Version: " #ver "  Date: " #date;
# define FUN(name,purpose,ver,date) \
	extern char *Argv0;\
	static char sccsfid[] = "@(#) " #name ".c " #ver " (|stat) " #date;

#else /* non-ANSI UNIX */

# define PGM(name,purpose,ver,date) \
	char *Argv0 = "name";\
	static char CpRt[] = STAT_H_COPYRIGHT;\
	static char sccshid[] = STAT_H_VERSION;\
	static char sccspid[] = "@(#) name.c ver (|stat) date";\
	static char *Purpose = "purpose";\
	static char	*Version = "Program: name  Version: ver  Date: date";
# define FUN(name,purpose,ver,date) \
	extern char *Argv0;\
	static char sccsfid[] = "@(#) name.c ver (|stat) date";

#endif /* ANSI STDC or UNIX */
#endif /* lint */

/******************************* FUNCTIONS *****************************/
#ifdef __MSDOS__
#	define ARGV0 argv[0] = Argv0 /* old DOS does not set argv[0] properly */
#else
#	define ARGV0 Argv0 = argv[0] /* used for initialization */
#endif
#ifdef __STDC__
#	define print(x,f) fprintf (stderr, "%-16s = %f\n", #x,  x)
#else
#	define print(x,f) fprintf (stderr, "%-16s = %f\n", "x",  x)
#endif
#define	tprint(label,value) \
	printf ("\t%-35s %12.6f\n", label, value)
#define chiprint(chi,dfval,prob) \
	printf ("\tchisq   %12.6f     df %3d      p %9.6f\n", chi, dfval, prob)

#define	bellmsg()    putc ('\007', stderr); /*PORTABILITY*/
#ifdef macintosh
#include <IOCtl.h>
#define isatty(x) (!ioctl(x, FIOINTERACTIVE, NULL))
#endif
#define	checkstdin() \
	(isatty (fileno (stdin)) && fprintf (stderr,"\007%s: Reading input from terminal:\n", Argv0))

#ifndef	max
#	define	min(a,b) ((a) < (b) ? (a) : (b))
#	define	max(a,b) ((a) > (b) ? (a) : (b))
#endif	max

/******************************* STDC MESSAGES *****************************/
#ifdef __STDC__
#define WARNING(msg) { bellmsg(); fprintf (stderr, "%s: %s.\n", Argv0, #msg); }
#define	ERRMSG3(msg, arg1, arg2, arg3)\
	{\
	bellmsg();\
	fprintf (stderr, "%s: " #msg ".\n", Argv0, arg1, arg2, arg3);\
	exit (FAILURE);\
	}
/******************************* OLDC MESSAGES *****************************/
#else
#define WARNING(msg) { bellmsg(); fprintf (stderr, "%s: msg.\n", Argv0); }
#define	ERRMSG3(msg, arg1, arg2, arg3)\
	{\
	bellmsg();\
	fprintf (stderr, "%s: msg.\n", Argv0, arg1, arg2, arg3);\
	exit (FAILURE);\
	}
#endif

/******************************* ERROR MESSAGES *****************************/
#define USAGE(synopsis) {\
	bellmsg();\
	fprintf (stderr, "Usage: %s %s\n", Argv0, synopsis);\
	exit (FAILURE);\
	}
#define ERRMSG2(msg, arg1, arg2) ERRMSG3 (msg, arg1, arg2, 0)
#define ERRMSG1(msg, arg1)       ERRMSG3 (msg, arg1, 0,    0)
#define ERRMSG0(msg)             ERRMSG3 (msg, 0,    0,    0)
#define ERRDATA              ERRMSG0 (Not enough (or no) input data)
#define ERRMANY(stuff,n)     ERRMSG1 (Too many stuff; at most %d allowed, n)
#define ERROPEN(file)        ERRMSG1 (Cannot open '%s', file)
#define ERRSPACE(whatever)   ERRMSG0 (No storage space left for whatever)
#define ERRNUM(str,type)     ERRMSG1 ('%s' (type) is not a number, str)
#define ERRPROB(value)       ERRMSG1 ('%g' is not a probability, value)
#define ERROPT(arg) {if (arg < argc) ERRMSG1(%d operand(s) ignored on command line, argc-arg)}
#define ERRVAL(fmt,var,str)  ERRMSG1 (%fmt is an illegal value for the str, var)
#define ERRRAGGED            ERRMSG0 (Ragged input file)
#define isinteger(str)       (number (str) == 1)
#define INTEGER(str)         isinteger(str)
#define	isna(str)            (str[0] == 'N' && str[1] == 'A' && str[2] == '\0')

/******************************* OPTIONS/LIMITS *****************************/
#define	pver(ver) printf ("%s\n", ver)
#define	plim(pgm) printf ("%s: program limits:\n", pgm)
#define	ppgm(pgm, purpose) printf ("%s: %s\n", pgm, purpose);
#ifdef __STDC__
#	define	popt(flag,arg,desc,fmt,val)\
		printf ("-%c %-10.10s %-40s " #fmt "\n", flag, arg, desc, val)
#else
#	define	popt(flag,arg,desc,fmt,val)\
		printf ("-%c %-10.10s %-40s fmt\n", flag, arg, desc, val)
#endif
#define	oper(op,desc,dflt)\
	printf ("   %-10.10s %-40s %s\n", op, desc, dflt)
#define	sopt(flag,arg,desc,val) popt(flag,arg,desc,%s,(val ? val : ""))
#define	copt(flag,arg,desc,val) popt(flag,arg,desc,%c,(val ? val : '_'))
#define	lopt(flag,desc,val)     popt(flag,"",desc,%s,(val ? "TRUE" : "FALSE"))
#define	iopt(flag,arg,desc,val) popt(flag,arg,desc,%d,val)
#define	ropt(flag,arg,desc,val) popt(flag,arg,desc,%g,val)
#define	const(val,desc)         printf ("%10d    %s\n", val, desc)

#endif STAT_H
