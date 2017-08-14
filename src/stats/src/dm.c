
# line 2 "dm.y"
/*  Copyright 1980 Gary Perlman */

#include "stat.h"
PGM(dm,Data Manipulation,5.9,10/1/88)
#define	DM_VERSION      "5.9 10/1/88"

#ifdef sun /* need corrective version of atof */
double	myatof ();
#else
#define	myatof(s)   atof(s)
#endif

/*
dm is a data manipulator designed to manipulate files of columns of number and
strings.  The major components of this program are:
	(1) a parser, built by yacc, called yyparse.
	(2) a scanner, yylex, called by yyparse.
	(3) a parse tree node function, called by yyparse.
	(4) a function, eval, that evaluates the parse trees.
	(5) a main that calls I/O routines and the control loop.
The following section is a bunch of global declarations that will be put
literally into the program, y.tab.c by yacc.
*/

#define MAXEXPR      100             /* maximum number of expressions */
#define MAXSTRING    32              /* maximum length of input string */
#define MAXCOL       100             /* maximum number of input columns */
#define MAXCONST     100             /* maximum number of constants */
#define FLOATPTR     0               /* codes for parse tree node types */
#define OPERATOR     1
#define STRINGOP     2
#define STRINGPTR    3
#define PARSERROR    1               /* returned by yyparse on error */
/*
The following few numbers are reserved by dm to signal special conditions by
being returned by various routines.  They are hopefully numbers that no
expressions would ever evaluate to.
*/
#define	LARGE         9999999999999.0      /* a large number */
#define	SUPPRESS     -1125899906842624.    /* suppress output */
#define	STRINGFLAG   -8888888888888777.0   /* returned by eval */
#define	NIL          -998888677484837274.  /* cause nil output */
#define	EXITFLAG     -99999999999999.9     /* cause exit */

#define	S_NUMBER      "number"
#define	S_LEN         "len"
#define	S_LOG         "log"
#define	S_LOG10       "Log"
#define	S_EXP         "exp"
#define	S_SQRT        "sqrt"
#define	S_SIN         "sin"
#define	S_COS         "cos"
#define	S_TAN         "tan"
#define	S_ASIN        "asin"
#define	S_ACOS        "acos"
#define	S_ATAN        "atan"
#define	S_CEIL        "ceil"
#define	S_FLOOR       "floor"
#define	S_ROUND       "round"
#define	S_ABS         "abs"

char	Outpipe = 0;                       /* true if output is piped */
FILE	*Infile;                           /* data read from here */
char	Inputline[4096];                   /* INPUT read into here */
FILE	*Outfile;                          /* output from dm */
char	*Evalstr[MAXCOL+1];                /* ptrs to strings from eval */
char	Str[MAXCOL+1][MAXSTRING];          /* columns from each dataline */
char	*Expra;                            /* ptr to each expression */
typedef	union
	{
	int 	opr;        /* if operator or stringop */
	double	*num;       /* if FLOATPTR */
	char	*str;       /* if STRINGOP */
	} STUFF;
STUFF	Tmp1, Tmp2;  /* used in the parser to cast operators */
typedef struct enode        /* expression node in tree */
	{
	int 	etype;          /* type of node */
	STUFF	contents;
	struct	enode *lchild;
	struct	enode *rchild;
	} ENODE;
#define	ENULL ((ENODE *) NULL)
ENODE	*Expr[MAXEXPR+1];           /* ptr to each parse tree */
double	Input[MAXCOL+1];            /* input numbers */
#define	INLINE (Input[0])           /* input line number stored here */
double	Output[MAXEXPR+1];          /* output numbers */
#define	OUTLINE (Output[0])         /* output line number stored here */
double	Const[MAXCONST];            /* constants stored here */
int 	Nconst;                     /* number of constants */
double	Nil = NIL;                  /* flagged by NIL */
double	Suppress  = SUPPRESS;       /* flagged by KILL */
double	Stringflag = STRINGFLAG;    /* eval returns string */
double	Exitflag = EXITFLAG;        /* flagged by EXIT */
double	Randu;                      /* uniform rand num */
extern	double	Maxrand;            /* set by initrand */
int 	Seed;                       /* random seed sent to initrand */
Boole	Userand;                    /* true if Randu is used */
double	N;                          /* number of input cols */
double	Sum;                        /* sum of input cols */
int 	Should_output[MAXEXPR+1];   /* used with X */
int 	Exprno;                     /* expression number */
int 	Nexpr;                      /* number of expressions */

# line 116 "dm.y"
typedef union  { /* union of the data types the parser will deal with */
	int 	opr;
	char	*str;
	double	*num;
	ENODE	*ex;
	} YYSTYPE;
#ifdef __cplusplus
#  include <stdio.h>
#  include <yacc.h>
#endif	/* __cplusplus */ 
# define NUMBER 257
# define STRING 258
# define STRINDEX 259
# define IF 260
# define THEN 261
# define ELSE 262
# define NOR 263
# define NAND 264
# define EQ 265
# define NE 266
# define GE 267
# define GT 268
# define LE 269
# define LT 270
# define NOTIN 271
# define  C 67
# define UMINUS 272
# define  l 108
# define  L 76
# define  e 101
# define  a 97
# define  f 102
# define  r 114
# define  c 99
# define SQRT 273
# define SIN 274
# define COS 275
# define TAN 276
# define ATAN 277
# define ACOS 278
# define ASIN 279
# define NTYPE 280
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif

/* __YYSCLASS defines the scoping/storage class for global objects
 * that are NOT renamed by the -p option.  By default these names
 * are going to be 'static' so that multi-definition errors
 * will not occur with multiple parsers.
 * If you want (unsupported) access to internal names you need
 * to define this to be null so it implies 'extern' scope.
 * This should not be used in conjunction with -p.
 */
#ifndef __YYSCLASS
# define __YYSCLASS static
#endif
YYSTYPE yylval;
__YYSCLASS YYSTYPE yyval;
typedef int yytabelem;
# define YYERRCODE 256

# line 431 "dm.y"


/*
Next is the scanner that will be repeatedly called by yyparse, yylex.
This simple program reads from a global char *Expra, set by main.  Variables
are handled by returning NUMBER or STRING, tokens defined in above grammar.
*/
yylex ()
	{
	extern	YYSTYPE yylval;
	char	*strsave ();
	int 	column;

	while (isspace (*Expra) || *Expra == '_')
		Expra++;
	if (isdigit (*Expra) || *Expra == '.')
		{
		if (Nconst == MAXCONST)
			ERRMSG1 (Maximum number of constants is %d, MAXCONST)
		Const[Nconst] = myatof (Expra);
		yylval.num = &Const[Nconst++];
		Expra += skipnumber (Expra, 1);
		return (NUMBER);
		}
	switch (*Expra)
		{
		case '"':
		case '\'':
			yylval.str = strsave ();
			return (STRING);
		case 'a':
			if (begins (S_ATAN, Expra))
				{Expra += 4; return (ATAN);}
			if (begins (S_ASIN, Expra))
				{Expra += 4; return (ASIN);}
			if (begins (S_ACOS, Expra))
				{Expra += 4; return (ACOS);}
			if (begins (S_ABS, Expra))
				{Expra += 3; return ('a');}
			if (begins ("and", Expra))
				{Expra += 3; return ('&');}
			break;
		case 'A':
			if (begins ("AND", Expra))
				{ Expra += 3; return ('&'); }
			break;
		case 'c':
			if (begins (S_COS, Expra))
				{Expra += 3; return (COS);}
			if (begins (S_CEIL, Expra))
				{Expra += 4; return ('c');}
			break;
		case 'f': if (begins (S_FLOOR, Expra))
				{Expra += 5; return ('f');}
			break;
		case 'r': if (begins (S_ROUND, Expra))
				{Expra += 5; return ('r');}
			break;
		case 'G':
			if (begins ("GE", Expra))
				{
				Expra += 2;
				return (GE);
				}
			if (begins ("GT", Expra))
				{
				Expra += 2;
				return (GT);
				}
			break;
		case 'e': if (begins ("else", Expra))
				{Expra += 4; return (ELSE);}
			else if (begins (S_EXP, Expra))
				{Expra += 3; return ('e');}
			break;
		case 'E':
			if (begins ("EQ", Expra))
				{ Expra += 2; return (EQ); }
			if (begins ("ELSE", Expra))
				{ Expra += 4; return (ELSE); }
			if (begins ("EXIT", Expra))
				Expra += 4;
			else Expra++;
			yylval.num = &Exitflag;
			return (NUMBER);
		case 'i':
			if (begins ("if", Expra))
				{Expra += 2; return (IF);}
			break;
		case 'I':
			if (begins ("IF", Expra))
				{ Expra += 2; return (IF); }
			if (begins ("INLINE", Expra))
				{
				Expra += 6;
				yylval.num = &INLINE;
				return (NUMBER);
				}
			if (begins ("INPUT", Expra))
				Expra += 5;
			else Expra++;
			yylval.str = Inputline;
			return (STRING);
		case 'K':
			if (begins ("KILL", Expra))
				Expra += 4;
			else Expra++;
			yylval.num = &Suppress;
			return (NUMBER);
		case 'l':
			if (begins (S_LOG, Expra))
				{Expra += 3; return ('l');}
			if (begins (S_LEN, Expra))
				{Expra += 3; return ('#');}
			break;
		case 'L':
			if (begins ("LE", Expra))
				{ Expra += 2; return (LE); }
			if (begins ("LT", Expra))
				{ Expra += 2; return (LT); }
			if (begins (S_LOG10, Expra))
				{Expra += 3; return ('L');}
			break;
		case 'n':
			if (begins ("not", Expra))
				{ Expra += 3; return ('!'); }
			if (begins (S_NUMBER, Expra))
				{ Expra += 6; return (NTYPE); }
			break;
		case 'N':
			if (begins ("NOT", Expra))
				{ Expra += 3; return ('!'); }
			if (begins ("NE", Expra) && !begins ("NEXT", Expra))
				{ Expra += 2; return (NE); }
			if (begins ("NIL", Expra))
				{
				Expra += 3;
				yylval.num = &Nil;
				}
			else if (begins ("NEXT", Expra))
				{
				Expra += 4;
				yylval.num = &Suppress;
				}
			else
				{
				Expra++;
				yylval.num = &N;
				}
			return (NUMBER);
		case 'o':
			if (begins ("or", Expra))
				{ Expra += 2; return ('|'); }
			break;
		case 'O':
			if (begins ("OR", Expra))
				{ Expra += 2; return ('|'); }
			if (begins ("OUTLINE", Expra))
				{
				Expra += 7;
				yylval.num = &OUTLINE;
				return (NUMBER);
				}
			break;
		case 'R':
			if (begins ("RAND", Expra))
				Expra += 4;
			else
				Expra++;
			Userand = TRUE;
			if (isdigit (*Expra))
				{
				Seed = atoi (Expra);
				while (isdigit (*Expra))
					Expra++;
				}
			yylval.num = &Randu;
			return (NUMBER);
		case 's':
			if (begins (S_SQRT, Expra))
				{
				Expra += 4;
				return (SQRT);
				}
			if (begins (S_SIN, Expra))
				{
				Expra += 3;
				return (SIN);
				}
			column = atoi (++Expra);
			Expra += skipnumber (Expra, 0);
			yylval.str = Str[column];
			return (STRING);
		case 'S':
			if (begins ("SKIP", Expra))
				{
				Expra += 4;
				yylval.num = &Suppress;
				}
			else
				{
				if (begins ("SUM", Expra))
					Expra += 3;
				else Expra++;
				yylval.num = &Sum;
				}
			return (NUMBER);
		case 't':
			if (begins (S_TAN, Expra))
				{Expra += 3; return (TAN);}
			if (begins ("then", Expra))
				{Expra += 4; return (THEN);}
			break;
		case 'T':
			if (begins ("THEN", Expra))
				{Expra += 4; return (THEN);}
			break;
		case 'x':
			if (Expra[1] == '[')
				{
				Expra++;
				return ('x');
				}
			column = atoi (++Expra);
			Expra += skipnumber (Expra, 0);
			yylval.num = &Input[column];
			return (NUMBER);
		case 'y':
			if (Expra[1] == '[')
				{
				Expra++;
				return ('y');
				}
			column = atoi (++Expra);
			Expra += skipnumber (Expra, 0);
			yylval.num = &Output[column];
			return (NUMBER);
		case '=': if (Expra[1] == '=') Expra += 2;
			else Expra++;
			return (EQ);
		case '<': if (Expra[1] == '=')
			{
			Expra += 2;
			return (LE);
			}
			Expra++;
			return (LT);
		case '>': if (Expra[1] == '=')
			{
			Expra += 2;
			return (GE);
			}
			Expra++;
			return (GT);
		case '!': switch (Expra[1])
				{
				case '=': Expra += 2; return (NE);
				case 'C': Expra += 2; return (NOTIN);
				case '&': Expra += 2; return (NAND);
				case '|': Expra += 2; return (NOR);
				default: Expra++; return ('!');
				}
		case '&': if (Expra[1] == '&') Expra++; break;
		case '|': if (Expra[1] == '|') Expra++; break;

		}
	return ((int) *Expra++);
	}

yyerror (msg)
char	*msg;
	{
	if (msg && *msg)
		fprintf (stderr, "\007dm: %s\n", msg);
	fprintf (stderr,
		"\007dm: Failure occurred with this left in input: (%s)\n", Expra-1);
#ifdef PTREE
	ptree (Expr[Exprno]);
	putchar ('\n');
#endif
	}

char *
strsave ()
	{
	char	buf[BUFSIZ], *bptr = buf;
	char	quotechar = *Expra++;
	while (*Expra && *Expra != quotechar)
		*bptr++ = *Expra++;
	if (*Expra == quotechar)
		Expra++;
	*bptr = '\0';
	return (strdup (buf));
	}

ENODE *
node (datum, dtype, lson, rson)
STUFF	*datum;                /* string, number, or operator */
int 	dtype;                 /* STRINGPTR, FLOATPTR, OPERATOR, STRINGOP */
ENODE	*lson;
ENODE	*rson;
	{
	ENODE	*newnode;
	newnode = myalloc (ENODE, 1);
	if (newnode == NULL)
		ERRSPACE(expressions)
	newnode->etype = dtype;
	switch (dtype)
		{
		case FLOATPTR: newnode->contents.num = datum->num; break;
		case STRINGPTR: newnode->contents.str = datum->str; break;
		case STRINGOP:
		case OPERATOR: newnode->contents.opr = datum->opr; break;
		default: fprintf (stderr, "\007dm/enode: unknown data type.\n");
		}
	newnode->lchild = lson;
	newnode->rchild = rson;
	return (newnode);
	}

main (argc, argv) int argc; char *argv[];
	{
	ARGV0;
	initial (argc, argv);
	loop ();
	exit (0);
	}

/*
	initial does the following:
		1) inits the random number generator
		2) reads in expressions from file, user, or argv[i].
		3) parses expressions.
		4) opens input and output files.
*/
initial (argc, argv) int argc; char **argv;
	{
	Boole	interactive = FALSE;    /* if true, input in interactive mode */
	Boole	input_by_hand = FALSE;  /* true if expressions input by hand */
	char	exprline[BUFSIZ];       /* expressions read into here */
	FILE	*exprfile;              /* expressions read from here */
	FILE	*getfile ();            /* gets a file open */
	argc--;
	if (argc)
		checkstdin ();
	if (argc == 0)
		{
		interactive = TRUE;
		printf ("dm: version %s (Copyright 1980 Gary Perlman)\n", DM_VERSION);
		exprfile = getfile ("Expression file? ", "r");
		if (exprfile == NULL)
			{
			input_by_hand = TRUE;
			exprfile = stdin;
			printf ("Enter ONE expression per line.\n");
			printf ("End with an empty line.\n");
			}
		}
	else if (argv[1][0] == 'E') /*Expra file flag */
		{
		if ((exprfile = fopen (&argv[1][1], "r")) == NULL)
			ERROPEN (&argv[1][1])
		}
	else
		exprfile = NULL;

	for (;;) /* PARSE expressions until done */
		{
	readexpr:
		if (exprfile == NULL) /*read Expras from argv[i] */
			{
			if (++Exprno > argc)
				break;
			Expra = argv[Exprno];
			}
		else /*read Expras from exprfile */
			{
			++Exprno;
			if (input_by_hand)
				printf ("expression[%d]: ", Exprno);
			if (getline (exprline, BUFSIZ, exprfile) <= 0)
				break;
			Expra = exprline;
			}

		while (isspace (*Expra))
			Expra++;
		if (*Expra == 'X')
			{
			Should_output[Exprno] = FALSE;
			Expra++;
			}
		else
			Should_output[Exprno] = TRUE;
		if (yyparse() == PARSERROR) /* call parser */
			{
			fprintf (stderr, "\007dm: error in parsing expr[%d].\n", Exprno--);
			if (input_by_hand)
				goto readexpr;
			else
				exit (1);
			}
#ifdef PTREE
		if (interactive)
			{
			printf ("e%d: ", Exprno);
			ptree (Expr[Exprno]);
			putchar ('\n');
			}
#endif
		}
		Nexpr = Exprno - 1;
		if (Nexpr == 0)
			{
			Exprno = 0;
			fprintf (stderr, "dm: \007No expressions were read in\n");
			if (input_by_hand && !feof (stdin))
				goto readexpr;
			else
				exit (1);
			}
		/* OPEN I/O files */
		if (interactive)
			{
			if ((Infile = getfile ("Input file? ", "r")) == NULL)
				Infile = stdin;
			if ((Outfile = getfile ("Output file or pipe? ", "w")) == NULL)
				Outfile = stdout;
			}
		else
			{
			Infile = stdin;
			Outfile = stdout;
			}
	if (Userand)
		initrand (Seed);
	}

#define printnum(ioptr, value)    fprintf (ioptr, "%g", value)

/*	loop runs the process on the input to produce the output */
loop ()
	{
	double	eval ();
	Boole	skip = FALSE;

	while (getinput () != EOF)
		{
		skip = FALSE;
		for (Exprno = 1; Exprno <= Nexpr; Exprno++)
			if ((Output[Exprno] = eval(Expr[Exprno])) == Suppress)
				{skip = TRUE; break;}
			else if (Output[Exprno] == Exitflag) exit (0);
		if (skip == TRUE) continue;
		OUTLINE += 1.0;
		for (Exprno = 1; Exprno <= Nexpr; Exprno++)
			if (Should_output[Exprno])
				{
				if (Output[Exprno] == Stringflag)
					fprintf (Outfile, "%s", Evalstr[Exprno]);
				else if (Output[Exprno] == Nil)
					continue;
				else
					printnum (Outfile, Output[Exprno]);
				if (Exprno < Nexpr)
					putc ('\t', Outfile);
				}
		putc ('\n', Outfile);
		/* fflush (Outfile);  why was this needed? */
		}
#if ! defined __MSDOS__ && ! defined macintosh /* no popen on MSDOS, macintosh*/
	if (Outpipe)
		(void) pclose (Outfile);
#endif
	}

int
getinput ()
	{
	int 	ncols;
	register int col;
	int 	randval;

	if (getline (Inputline, sizeof (Inputline), Infile) == EOF)
		return (EOF);
	if (Userand)
		{
		while ((randval = rand ()) < 0);
		Randu = randval/Maxrand;
		}
	Sum = 0.0;
	INLINE += 1.0;
	ncols = sstrings (Inputline, Str[1], MAXCOL, MAXSTRING);
	for (col = 1; col <= ncols; col++)
		if (number (Str[col]))
			Sum += (Input[col] = myatof(Str[col]));
		else
			Input[col] = 0.0;
	N = ncols;
	return (ncols);
	}


/*
eval is a recursive function that takes a parse tree of an expression,
and returns its value.  The major kludge in this program is how it handles
strings.  Since it wants to return a double, it cannot return a string, so
the use of strings is somewhat restricted.  When eval evals to a string, it
returns STRINGFLAG after setting a global char *Evalstr[Exprno] to the str
MAIN will look for this flag and switch its output to Evalstr[Exprno] rather
than Output[Exprno].
*/
double
eval (expression) ENODE *expression;
	{
	int 	diff;                 /*for string comparisons*/
	int 	sindex, character;    /*for STRINDEX function */
	char	*string_2b_indexed;   /*for STRINDEX function */
	double	tmp1, tmp2;
	int 	operator;
	if (expression == NULL) return (0.0);
	if (expression->etype == FLOATPTR)
		return (*expression->contents.num);
	if (expression->etype == STRINGPTR) 
		{
		Evalstr[Exprno] = expression->contents.str;
		return (Stringflag);
		}
	if (expression->etype == STRINGOP)
		{
		switch (expression->contents.opr)
			{
			case '=': /*string compare*/
				diff = strcmp (expression->lchild->contents.str,
					expression->rchild->contents.str);
				return (diff ? 0.0 : 1.0);
			case '>': /*string compare*/
				diff = strcmp (expression->lchild->contents.str,
					expression->rchild->contents.str);
				return (diff > 0 ? 1.0 : 0.0);
			case '<': /*string compare*/
				diff = strcmp (expression->lchild->contents.str,
					expression->rchild->contents.str);
				return (diff < 0 ? 1.0 : 0.0);
			case 'C': /*true is s1 is in s2 */
				diff = substr (expression->lchild->contents.str,
					expression->rchild->contents.str);
				return (diff ? 1.0 : 0.0);
			case '#':
				return ((double) strlen (expression->rchild->contents.str));
			case NTYPE:
				return ((double) number (expression->rchild->contents.str));
			case '[': /* string index */
				sindex = eval (expression->rchild);
				string_2b_indexed = expression->lchild->contents.str;
				character = string_2b_indexed[sindex-1];
				return (1.0 * character);
			}
		}
	operator = expression->contents.opr;
	if (operator == ':') return (0.0); /*dummy for conditional */
	tmp1 = eval (expression->lchild);
	tmp2 = eval (expression->rchild);
	switch (operator)
	{
	case 'x':
		sindex = (int) tmp2;
		if (sindex < 0 || sindex > N)
			ERRMSG1 (computed index for x (%d) is out of range, sindex)
		return (Input[sindex]);
	case 'y':
		sindex = (int) tmp2;
		if (sindex >= 0 && sindex <= Nexpr)
		return (Output[sindex]);
		ERRMSG1 (computed index for y (%d) is  out of range, sindex)
	case '_': return (-tmp2);
	case '!': return (fzero (tmp2) ? 1.0 : 0.0);
	case 'l': if (tmp2 <= 0.0)
		ERRMSG3 (log undefined for %g on line %.0f  expr[%d], tmp2, INLINE,Exprno)
		return (log (tmp2));
	case 'L': if (tmp2 <= 0.0)
		ERRMSG3(Log undefined for %g Input line %.0f  expr[%d], tmp2,INLINE,Exprno)
		return (log (tmp2) / LOGe10);
	case 'e': return (exp (tmp2));
	case SQRT:
		if (tmp2 < 0.0)
			ERRMSG3 (sqrt undefined for %g Input line %.0f  expr[%d],
				tmp2, INLINE, Exprno)
		return (sqrt (tmp2));
	case SIN: return (sin (tmp2));
	case COS: return (cos (tmp2));
	case TAN: return (tan (tmp2));
	case ATAN: return (atan (tmp2));
	case ACOS: return (acos (tmp2));
	case ASIN: return (asin (tmp2));
	case 'a': return (fabs (tmp2));
	case 'c': return (ceil (tmp2));
	case 'f': return (floor (tmp2));
	case 'r': return (floor (tmp2 + 0.5));
	case '+': return (tmp1 + tmp2);
	case '-': return (tmp1 - tmp2);
	case '*': return (tmp1 * tmp2);
	case '%': if (fzero (tmp2))
		ERRMSG2 (division by zero. input line %.0f  expr[%d], INLINE,Exprno)
		return ((double) (((int) tmp1) % ((int) tmp2)));
	case '/': if (fzero (tmp2))
		ERRMSG2 (division by zero. input line %.0f  expr[%d], INLINE,Exprno)
		return (tmp1/tmp2);
	case '^':
		if (tmp1 < 0.0 && (floor (tmp2) != tmp2))
			ERRMSG1 (power failure at line %.0f, INLINE)
		return (pow (tmp1, tmp2));
	case '>': return (tmp1 > tmp2 ? 1.0 : 0.0);
	case '<': return (tmp1 < tmp2 ? 1.0 : 0.0);
	case '=': return (fzero (tmp1 - tmp2) ? 1.0 : 0.0);
	case '&': return ((!fzero (tmp1) && !fzero (tmp2)) ? 1.0 : 0.0);
	case '|': return ((!fzero (tmp1) || !fzero (tmp2)) ? 1.0 : 0.0);
	case '?': if (!fzero (tmp1))
		return (eval (expression->rchild->lchild));
		return (eval (expression->rchild->rchild));
	default:
		ERRMSG3 (Unknown operator '%c' %d \\%3o, operator, operator, operator)
	}
	return (Exitflag);
	}

#ifdef PTREE
ptree (tree) ENODE *tree;
	{
	if (tree == NULL) return;
	if (tree->etype == FLOATPTR)
		if (*tree->contents.num < -LARGE)
			{
			double	*nptr = tree->contents.num;
			if (nptr == &Suppress)
				printf ("KILL");
			else if (nptr == &Exitflag)
				printf ("EXIT");
			else if (nptr == &Nil)
				printf ("NIL");
			else printf ("CONTROL");
			}
		else /* regular number */
			{
			double	*dptr = tree->contents.num;
			if (dptr > Input && dptr <= &Input[MAXCOL])
				printf ("x%d", dptr - Input);
			else if (dptr == &INLINE)
				printf ("INLINE");
			else if (dptr > Output && dptr <= &Output[MAXCOL])
				printf ("y%d", dptr - Output);
			else if (dptr == &OUTLINE)
				printf ("OUTLINE");
			else if (dptr == &N)
				printf ("N");
			else if (dptr == &Sum)
				printf ("SUM");
			else if (dptr == &Randu)
				printf ("RAND");
			else
				printnum (stdout, *dptr);
			}
	else if (tree->etype == STRINGPTR)
		{
		char	*sptr = tree->contents.str;
		if (sptr == Inputline)
			printf ("INPUT");
		else if (sptr >= Str[0] && sptr < Str[MAXCOL])
			printf ("s%d", (sptr - Str[0])/MAXSTRING);
		else printf ("'%s'", sptr);
		}
	else if (tree->etype == OPERATOR || tree->etype == STRINGOP)
		{
		int 	op = tree->contents.opr;
		printf ("(");
		ptree (tree->lchild);
		switch (op)
			{
			case NTYPE: printf ("%s ", S_NUMBER); break;
			case '#': printf ("%s ", S_LEN); break;
			case 'l': printf ("%s ", S_LOG); break;
			case 'L': printf ("%s ", S_LOG10); break;
			case 'e': printf ("%s ", S_EXP); break;
			case SQRT: printf ("%s ", S_SQRT); break;
			case SIN: printf ("%s ", S_SIN); break;
			case COS: printf ("%s ", S_COS); break;
			case TAN: printf ("%s ", S_TAN); break;
			case ATAN: printf ("%s ", S_ATAN); break;
			case ACOS: printf ("%s ", S_ACOS); break;
			case ASIN: printf ("%s ", S_ASIN); break;
			case 'c': printf ("%s ", S_CEIL); break;
			case 'f': printf ("%s ", S_FLOOR); break;
			case 'r': printf ("%s ", S_ROUND); break;
			case 'a': printf ("%s ", S_ABS); break;
			default:
				printf (" %c ", op);
			}
		ptree (tree->rchild);
		printf (")");
		}
	else printf ("(bad node type %d)", tree->etype);
	}
#endif

FILE *
getfile (prompt, mode) char *prompt, *mode;
	{
#if ! defined __MSDOS__ && ! defined macintosh /* no popen on MSDOS, macintosh*/
	FILE	*popen ();
#endif
	FILE	*fopen (), *ioptr;
	char	filename[BUFSIZ];
	char	*ptr = filename;
  newfile:
	printf ("%s", prompt);
	if (getline (filename, MAXSTRING, stdin) <= 0)
		return (NULL);
	while (isspace (*ptr))
		ptr++;
	if (*ptr == '\0')
		return (NULL);
	if (*mode == 'w')
		if (*ptr == '|')
			Outpipe = 1;
		else if (!canwrite (filename))
			goto newfile;
	if (Outpipe)
		{
#if ! defined __MSDOS__ && ! defined macintosh /* no popen on MSDOS, macintosh*/
		if ((ioptr = popen (ptr+1, "w")) == NULL)
#endif
			{
			fprintf (stderr, "Cannot create pipe.\n");
			Outpipe = 0;
			goto newfile;
			}
		}
	else if ((ioptr = fopen (filename, mode)) == NULL)
		{
		printf ("Cannot open '%s'.\n", filename);
		goto newfile;
		}
	return (ioptr);
	}

int
getline (string, maxlen, ioptr)
char	*string;
int 	maxlen;
FILE	*ioptr;
	{
	register	int 	inchar;
	register	int 	len = 0;
	while ((inchar = getc (ioptr)) != EOF)
		{
		if (inchar == '\n')
			break;
		else if (inchar == '\\')
			if ((inchar = getc (ioptr)) == EOF)
				inchar = '\\';
		string[len] = inchar;
		if (++len == maxlen)
			break;
		}
	string[len] = '\0';
	if (len == 0 && feof (ioptr))
		return (EOF);
	return (len);
	}

begins (s1, s2) char *s1, *s2;
	{
	while (*s1)
		if (*s1++ != *s2++)
			return (0);
	return (1);
	/* return (isalpha (*s2) ? 0 : 1);  can't be used because of s1 C s2 */
	}

substr (s1, s2) char *s1, *s2;
	{
	while (*s2)
		if (begins (s1, s2))
			return (1);
		else
			s2++;
	return (0);
	}
__YYSCLASS yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
-1, 86,
	265, 0,
	266, 0,
	267, 0,
	268, 0,
	269, 0,
	270, 0,
	-2, 15,
-1, 87,
	265, 0,
	266, 0,
	267, 0,
	268, 0,
	269, 0,
	270, 0,
	-2, 16,
-1, 88,
	265, 0,
	266, 0,
	267, 0,
	268, 0,
	269, 0,
	270, 0,
	-2, 19,
-1, 89,
	265, 0,
	266, 0,
	267, 0,
	268, 0,
	269, 0,
	270, 0,
	-2, 20,
-1, 90,
	265, 0,
	266, 0,
	267, 0,
	268, 0,
	269, 0,
	270, 0,
	-2, 23,
-1, 91,
	265, 0,
	266, 0,
	267, 0,
	268, 0,
	269, 0,
	270, 0,
	-2, 24,
	};
# define YYNPROD 55
# define YYLAST 494
__YYSCLASS yytabelem yyact[]={

     9,    53,    25,    32,    41,   118,     2,     7,    31,    29,
    77,    30,     8,    33,    32,    34,    56,    55,   112,    31,
   109,     1,    32,    41,    33,    52,     0,    31,    29,    45,
    30,     0,    33,     0,     0,     0,    32,    41,     0,     0,
     0,    31,    29,    11,    30,     0,    33,   109,    45,     0,
     0,     0,    96,     0,     0,     0,     0,     0,     0,   115,
    34,     0,    45,     0,    20,     0,    23,     0,    19,    21,
     0,    34,     0,    53,     0,    10,     0,     0,   114,    34,
     0,    22,     0,     0,     0,     0,     0,     5,     6,     0,
    43,     0,   113,    34,    32,    41,     0,    52,     0,    31,
    29,     0,    30,     0,    33,     0,    32,    41,     0,    43,
   108,    31,    29,     0,    30,     0,    33,     0,   116,   117,
    45,    32,    41,    43,     0,   119,    31,    29,     0,    30,
     0,    33,    45,    32,    41,     0,     0,     0,    31,    29,
    32,    30,     0,    33,     0,    31,    29,    45,    30,    32,
    33,    34,     0,     0,    31,    29,     0,    30,     0,    33,
     0,     0,     0,    34,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    34,     0,
     0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
    34,     0,     0,    43,     0,     0,     0,    34,     0,    46,
    47,    49,    48,    51,    50,    54,    34,     0,    43,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    27,    28,     0,    26,    28,    44,
    42,    35,    36,    38,    37,    40,    39,     0,     0,     0,
    12,    13,    14,    15,    16,    17,    18,    24,    44,    42,
    35,    36,    38,    37,    40,    39,     0,     0,     0,     0,
     0,     0,    44,    42,    35,    36,    38,    37,    40,    39,
     0,    46,    47,    49,    48,    51,    50,    54,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   111,     0,
    44,    42,    35,    36,    38,    37,    40,    39,     0,     0,
     0,     0,    44,    42,    35,    36,    38,    37,    40,    39,
     0,     0,     0,     0,     0,     0,     0,    44,    42,    35,
    36,    38,    37,    40,    39,     0,     0,     0,     0,     0,
    42,    35,    36,    38,    37,    40,    39,     0,    35,    36,
    38,    37,    40,    39,    60,     4,     0,     0,     0,     0,
     0,     0,    58,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    76,
    78,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     4,    97,    98,    99,   100,   101,   102,     3,   104,   105,
     0,     0,     0,     0,     0,    57,    59,    61,    62,    63,
    64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
    74,    75,   110,     0,    79,     0,     0,    80,    81,    82,
    83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
    93,    94,    95,     0,     0,     0,     0,     0,     0,     0,
   103,     0,     0,   106,   107,     0,     4,     4,     0,     0,
     0,     0,     0,     4 };
__YYSCLASS yytabelem yypact[]={

   -33, -3000, -3000,    84,   -66,   -74,   -75,   -33,   -33,   -33,
   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,
   -33,   -33,   -33,   -33,   -30,   -30,   -33, -3000, -3000,   -33,
   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,
   -33,   -33,   -33,   -33,   -33,   -33,   -30,   -30,   -30,   -30,
   -30,   -30,   -33,   -30,   -30,   -33,   -33,    69,     6, -3000,
   -66,   103, -3000, -3000, -3000, -3000, -3000, -3000, -3000, -3000,
 -3000, -3000, -3000, -3000, -3000, -3000, -3000,   -30, -3000,    57,
   -23,   -23,   -79,   -79,   -79,   -79,   112,   112,   112,   112,
   112,   112,   103,   103,    96,    96,   -40, -3000, -3000, -3000,
 -3000, -3000, -3000,    -1, -3000, -3000,   -15,   -34, -3000, -3000,
   -21,   -33,   -33, -3000, -3000, -3000,  -257, -3000,   -33, -3000 };
__YYSCLASS yytabelem yypgo[]={

     0,   427,   374,     6,    21 };
__YYSCLASS yytabelem yyr1[]={

     0,     4,     3,     3,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     2,     2 };
__YYSCLASS yytabelem yyr2[]={

     0,     3,     3,     3,     9,     9,     7,     7,     7,     7,
     7,     7,     7,     7,     7,     7,     7,     7,     7,     7,
     7,     7,     7,     7,     7,     7,     7,     7,     7,     5,
     5,     5,     5,     5,     5,     5,     5,     5,     5,     5,
     5,     5,     5,     5,     5,     5,     5,     9,     7,     7,
    11,    13,     3,     7,     3 };
__YYSCLASS yytabelem yychk[]={

 -3000,    -4,    -3,    -1,    -2,   120,   121,    40,    45,    33,
   108,    76,   273,   274,   275,   276,   277,   278,   279,   101,
    97,   102,   114,    99,   280,    35,   260,   257,   258,    43,
    45,    42,    37,    47,    94,   265,   266,   268,   267,   270,
   269,    38,   264,   124,   263,    63,   265,   266,   268,   267,
   270,   269,    91,    67,   271,    91,    91,    -1,    -2,    -1,
    -2,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -2,    40,    -2,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -3,    -2,    -2,    -2,
    -2,    -2,    -2,    -1,    -2,    -2,    -1,    -1,    41,    41,
    -2,   261,    58,    93,    93,    93,    -3,    -3,   262,    -3 };
__YYSCLASS yytabelem yydef[]={

     0,    -2,     1,     2,     3,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    52,    54,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    29,
     0,    30,    31,    32,    33,    34,    35,    36,    37,    38,
    39,    40,    41,    42,    43,    44,    45,     0,    46,     0,
     7,     8,     9,    10,    11,    12,    -2,    -2,    -2,    -2,
    -2,    -2,    25,    26,    27,    28,     0,    13,    14,    17,
    18,    21,    22,     0,    48,    49,     0,     0,     6,    53,
     0,     0,     0,    47,     4,     5,     0,    50,     0,    51 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

__YYSCLASS yytoktype yytoks[] =
{
	"NUMBER",	257,
	"STRING",	258,
	"STRINDEX",	259,
	"?",	63,
	":",	58,
	"IF",	260,
	"THEN",	261,
	"ELSE",	262,
	"|",	124,
	"NOR",	263,
	"&",	38,
	"NAND",	264,
	"!",	33,
	"EQ",	265,
	"NE",	266,
	"GE",	267,
	"GT",	268,
	"LE",	269,
	"LT",	270,
	"NOTIN",	271,
	"C",	67,
	"+",	43,
	"-",	45,
	"*",	42,
	"/",	47,
	"%",	37,
	"^",	94,
	"UMINUS",	272,
	"#",	35,
	"l",	108,
	"L",	76,
	"e",	101,
	"a",	97,
	"f",	102,
	"r",	114,
	"c",	99,
	"SQRT",	273,
	"SIN",	274,
	"COS",	275,
	"TAN",	276,
	"ATAN",	277,
	"ACOS",	278,
	"ASIN",	279,
	"NTYPE",	280,
	"-unknown-",	-1	/* ends search */
};

__YYSCLASS char * yyreds[] =
{
	"-no such reduction-",
	"start : stringexpr",
	"stringexpr : expr",
	"stringexpr : string",
	"expr : 'x' '[' expr ']'",
	"expr : 'y' '[' expr ']'",
	"expr : '(' expr ')'",
	"expr : expr '+' expr",
	"expr : expr '-' expr",
	"expr : expr '*' expr",
	"expr : expr '%' expr",
	"expr : expr '/' expr",
	"expr : expr '^' expr",
	"expr : string EQ string",
	"expr : string NE string",
	"expr : expr EQ expr",
	"expr : expr NE expr",
	"expr : string GT string",
	"expr : string GE string",
	"expr : expr GT expr",
	"expr : expr GE expr",
	"expr : string LT string",
	"expr : string LE string",
	"expr : expr LT expr",
	"expr : expr LE expr",
	"expr : expr '&' expr",
	"expr : expr NAND expr",
	"expr : expr '|' expr",
	"expr : expr NOR expr",
	"expr : '-' expr",
	"expr : '!' expr",
	"expr : 'l' expr",
	"expr : 'L' expr",
	"expr : SQRT expr",
	"expr : SIN expr",
	"expr : COS expr",
	"expr : TAN expr",
	"expr : ATAN expr",
	"expr : ACOS expr",
	"expr : ASIN expr",
	"expr : 'e' expr",
	"expr : 'a' expr",
	"expr : 'f' expr",
	"expr : 'r' expr",
	"expr : 'c' expr",
	"expr : NTYPE string",
	"expr : '#' string",
	"expr : string '[' expr ']'",
	"expr : string 'C' string",
	"expr : string NOTIN string",
	"expr : expr '?' stringexpr ':' stringexpr",
	"expr : IF expr THEN stringexpr ELSE stringexpr",
	"expr : NUMBER",
	"string : '(' string ')'",
	"string : STRING",
};
#endif /* YYDEBUG */
#define YYFLAG  (-3000)
/* @(#) $Revision: 1.1.1.1 $ */    

/*
** Skeleton parser driver for yacc output
*/

#if defined(NLS) && !defined(NL_SETN)
#include <msgbuf.h>
#endif

#ifndef nl_msg
#define nl_msg(i,s) (s)
#endif

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab

#ifndef __RUNTIME_YYMAXDEPTH
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#else
#define YYACCEPT	{free_stacks(); return(0);}
#define YYABORT		{free_stacks(); return(1);}
#endif

#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( (nl_msg(30001,"syntax error - cannot backup")) );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
/* define for YYFLAG now generated by yacc program. */
/*#define YYFLAG		(FLAGVAL)*/

/*
** global variables used by the parser
*/
# ifndef __RUNTIME_YYMAXDEPTH
__YYSCLASS YYSTYPE yyv[ YYMAXDEPTH ];	/* value stack */
__YYSCLASS int yys[ YYMAXDEPTH ];		/* state stack */
# else
__YYSCLASS YYSTYPE *yyv;			/* pointer to malloc'ed value stack */
__YYSCLASS int *yys;			/* pointer to malloc'ed stack stack */

#if defined(__STDC__) || defined (__cplusplus)
#include <stdlib.h>
#else
	extern char *malloc();
	extern char *realloc();
	extern void free();
#endif /* __STDC__ or __cplusplus */


static int allocate_stacks(); 
static void free_stacks();
# ifndef YYINCREMENT
# define YYINCREMENT (YYMAXDEPTH/2) + 10
# endif
# endif	/* __RUNTIME_YYMAXDEPTH */
long  yymaxdepth = YYMAXDEPTH;

__YYSCLASS YYSTYPE *yypv;			/* top of value stack */
__YYSCLASS int *yyps;			/* top of state stack */

__YYSCLASS int yystate;			/* current state */
__YYSCLASS int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
__YYSCLASS int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

	/*
	** Initialize externals - yyparse may be called more than once
	*/
# ifdef __RUNTIME_YYMAXDEPTH
	if (allocate_stacks()) YYABORT;
# endif
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
# ifndef __RUNTIME_YYMAXDEPTH
			yyerror( (nl_msg(30002,"yacc stack overflow")) );
			YYABORT;
# else
			/* save old stack bases to recalculate pointers */
			YYSTYPE * yyv_old = yyv;
			int * yys_old = yys;
			yymaxdepth += YYINCREMENT;
			yys = (int *) realloc(yys, yymaxdepth * sizeof(int));
			yyv = (YYSTYPE *) realloc(yyv, yymaxdepth * sizeof(YYSTYPE));
			if (yys==0 || yyv==0) {
			    yyerror( (nl_msg(30002,"yacc stack overflow")) );
			    YYABORT;
			    }
			/* Reset pointers into stack */
			yy_ps = (yy_ps - yys_old) + yys;
			yyps = (yyps - yys_old) + yys;
			yy_pv = (yy_pv - yyv_old) + yyv;
			yypv = (yypv - yyv_old) + yyv;
# endif

		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( (nl_msg(30003,"syntax error")) );
				yynerrs++;
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 1:
# line 142 "dm.y"
{
		Expr[Exprno] = yypvt[-0].ex;
		} break;
case 2:
# line 147 "dm.y"
{
		yyval.ex = yypvt[-0].ex;
		} break;
case 3:
# line 151 "dm.y"
{
		yyval.ex = yypvt[-0].ex;
		} break;
case 4:
# line 156 "dm.y"
{
		Tmp1.opr = 'x';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-1].ex);
		} break;
case 5:
# line 161 "dm.y"
{
		Tmp1.opr = 'y';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-1].ex);
		} break;
case 6:
# line 166 "dm.y"
{
		yyval.ex = yypvt[-1].ex;
		} break;
case 7:
# line 170 "dm.y"
{
		Tmp1.opr = '+';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 8:
# line 175 "dm.y"
{
		Tmp1.opr = '-';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 9:
# line 180 "dm.y"
{
		Tmp1.opr = '*';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 10:
# line 185 "dm.y"
{
		Tmp1.opr = '%';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 11:
# line 190 "dm.y"
{
		Tmp1.opr = '/';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 12:
# line 195 "dm.y"
{
		Tmp1.opr = '^';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 13:
# line 200 "dm.y"
{
		Tmp1.opr = '=';
		yyval.ex = node (&Tmp1, STRINGOP, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 14:
# line 205 "dm.y"
{
		Tmp1.opr = '!';
		Tmp2.opr = '=';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, STRINGOP, yypvt[-2].ex, yypvt[-0].ex));
		} break;
case 15:
# line 212 "dm.y"
{
		Tmp1.opr = '=';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 16:
# line 217 "dm.y"
{
		Tmp1.opr = '!';
		Tmp2.opr = '=';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, OPERATOR, yypvt[-2].ex, yypvt[-0].ex));
		} break;
case 17:
# line 224 "dm.y"
{
		Tmp1.opr = '>';
		yyval.ex = node (&Tmp1, STRINGOP, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 18:
# line 229 "dm.y"
{
		Tmp1.opr = '!';
		Tmp2.opr = '<';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, STRINGOP, yypvt[-2].ex, yypvt[-0].ex));
		} break;
case 19:
# line 236 "dm.y"
{
		Tmp1.opr = '>';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 20:
# line 241 "dm.y"
{
		Tmp1.opr = '!';
		Tmp2.opr = '<';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, OPERATOR, yypvt[-2].ex, yypvt[-0].ex));
		} break;
case 21:
# line 248 "dm.y"
{
		Tmp1.opr = '<';
		yyval.ex = node (&Tmp1, STRINGOP, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 22:
# line 253 "dm.y"
{
		Tmp1.opr = '!';
		Tmp2.opr = '>';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, STRINGOP, yypvt[-2].ex, yypvt[-0].ex));
		} break;
case 23:
# line 260 "dm.y"
{
		Tmp1.opr = '<';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 24:
# line 265 "dm.y"
{
		Tmp1.opr = '!';
		Tmp2.opr = '>';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, OPERATOR, yypvt[-2].ex, yypvt[-0].ex));
		} break;
case 25:
# line 272 "dm.y"
{
		Tmp1.opr = '&';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 26:
# line 277 "dm.y"
{
		Tmp1.opr = '!';
		Tmp2.opr = '&';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, OPERATOR, yypvt[-2].ex, yypvt[-0].ex));
		} break;
case 27:
# line 284 "dm.y"
{
		Tmp1.opr = '|';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 28:
# line 289 "dm.y"
{
		Tmp1.opr = '!';
		Tmp2.opr = '|';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, OPERATOR, yypvt[-2].ex, yypvt[-0].ex));
		} break;
case 29:
# line 296 "dm.y"
{
		Tmp1.opr = '_';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 30:
# line 301 "dm.y"
{
		Tmp1.opr = '!';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 31:
# line 306 "dm.y"
{
		Tmp1.opr = 'l';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 32:
# line 311 "dm.y"
{
		Tmp1.opr = 'L';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 33:
# line 316 "dm.y"
{
		Tmp1.opr = SQRT;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 34:
# line 321 "dm.y"
{
		Tmp1.opr = SIN;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 35:
# line 326 "dm.y"
{
		Tmp1.opr = COS;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 36:
# line 331 "dm.y"
{
		Tmp1.opr = TAN;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 37:
# line 336 "dm.y"
{
		Tmp1.opr = ATAN;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 38:
# line 341 "dm.y"
{
		Tmp1.opr = ACOS;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 39:
# line 346 "dm.y"
{
		Tmp1.opr = ASIN;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 40:
# line 351 "dm.y"
{
		Tmp1.opr = 'e';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 41:
# line 356 "dm.y"
{
		Tmp1.opr = 'a';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 42:
# line 361 "dm.y"
{
		Tmp1.opr = 'f';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 43:
# line 366 "dm.y"
{
		Tmp1.opr = 'r';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 44:
# line 371 "dm.y"
{
		Tmp1.opr = 'c';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 45:
# line 376 "dm.y"
{
		Tmp1.opr = NTYPE;
		yyval.ex = node (&Tmp1, STRINGOP, ENULL, yypvt[-0].ex);
		} break;
case 46:
# line 381 "dm.y"
{
		Tmp1.opr = '#';
		yyval.ex = node (&Tmp1, STRINGOP, ENULL, yypvt[-0].ex);
		} break;
case 47:
# line 386 "dm.y"
{
		Tmp1.opr = '[';
		yyval.ex = node (&Tmp1, STRINGOP, yypvt[-3].ex, yypvt[-1].ex);
		} break;
case 48:
# line 391 "dm.y"
{
		Tmp1.opr = 'C';
		yyval.ex = node (&Tmp1, STRINGOP, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 49:
# line 396 "dm.y"
{
		Tmp1.opr = '!';
		Tmp2.opr = 'C';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, STRINGOP, yypvt[-2].ex, yypvt[-0].ex));
		} break;
case 50:
# line 403 "dm.y"
{
		Tmp1.opr = '?';
		Tmp2.opr = ':';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-4].ex,
			node (&Tmp2, OPERATOR,  yypvt[-2].ex, yypvt[-0].ex));
		} break;
case 51:
# line 410 "dm.y"
{
		Tmp1.opr = '?';
		Tmp2.opr = ':';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-4].ex,
			node (&Tmp2, OPERATOR, yypvt[-2].ex, yypvt[-0].ex));
		} break;
case 52:
# line 417 "dm.y"
{
		Tmp1.num = yypvt[-0].num;
		yyval.ex = node (&Tmp1, FLOATPTR, ENULL, ENULL);
		} break;
case 53:
# line 423 "dm.y"
{
		yyval.ex = yypvt[-1].ex;
		} break;
case 54:
# line 427 "dm.y"
{
		Tmp1.str = yypvt[-0].str;
		yyval.ex = node (&Tmp1, STRINGPTR, ENULL, ENULL);
		} break;
	}
	goto yystack;		/* reset registers in driver code */
}

# ifdef __RUNTIME_YYMAXDEPTH

static int allocate_stacks() {
	/* allocate the yys and yyv stacks */
	yys = (int *) malloc(yymaxdepth * sizeof(int));
	yyv = (YYSTYPE *) malloc(yymaxdepth * sizeof(YYSTYPE));

	if (yys==0 || yyv==0) {
	   yyerror( (nl_msg(30004,"unable to allocate space for yacc stacks")) );
	   return(1);
	   }
	else return(0);

}


static void free_stacks() {
	if (yys!=0) free((char *) yys);
	if (yyv!=0) free((char *) yyv);
}

# endif  /* defined(__RUNTIME_YYMAXDEPTH) */

