%{
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
%}

/*                    G R A M M A R                       */
/*
First is the set of precedences and associativities, followed by the grammar
Actions are associated with each part of the grammar matched.  These amount
to creating an enode representing the expression parsed, created by node.
One notable feature about the grammar is that it uses the same symbols for
comparing both strings and numbers, the difference being made explicit in
the actions for the string comparison functions.
*/
%union { /* union of the data types the parser will deal with */
	int 	opr;
	char	*str;
	double	*num;
	ENODE	*ex;
	}
%type <ex> expr string stringexpr
%token <num> NUMBER                          /* returned by yylex for numbers */
%token <str> STRING                          /* returned by yylex for strings */
%token <opr> STRINDEX                        /* string index operator flag */
%left <opr> '?' ':' IF THEN ELSE             /* if then else */
%left <opr> '|' NOR                          /* logical or */
%left <opr> '&' NAND                         /* logical and */
%nonassoc <opr> '!'                          /* logical not */
%nonassoc <opr> EQ NE GE GT LE LT NOTIN 'C'  /* comparators */
%left  <opr> '+' '-'                         /* plus and minus */
%left  <opr> '*' '/' '%'                     /* mult, div, mod */
%right <opr> '^'                             /* exponentiation */
%nonassoc <opr> UMINUS                       /* unary minus */
%nonassoc <opr> '#' 'l' 'L' 'e' 'a' 'f' 'r' 'c'  SQRT /* unary functions */
%nonassoc <opr> SIN COS TAN ATAN ACOS ASIN   /* trig functions */
%nonassoc <opr> NTYPE
%%
/*                     P R O D U C T I O N S                         */
start:
	stringexpr
		{
		Expr[Exprno] = $1;
		};
stringexpr :
	expr
		{
		$$ = $1;
		}|
	string
		{
		$$ = $1;
		};
expr :
	'x' '[' expr ']'
		{
		Tmp1.opr = 'x';
		$$ = node (&Tmp1, OPERATOR, ENULL, $3);
		}|
	'y' '[' expr ']'
		{
		Tmp1.opr = 'y';
		$$ = node (&Tmp1, OPERATOR, ENULL, $3);
		}|
	'('  expr ')'
		{
		$$ = $2;
		}|
	expr '+' expr
		{
		Tmp1.opr = '+';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr '-' expr
		{
		Tmp1.opr = '-';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr '*' expr
		{
		Tmp1.opr = '*';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr '%' expr
		{
		Tmp1.opr = '%';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr '/' expr
		{
		Tmp1.opr = '/';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr '^' expr
		{
		Tmp1.opr = '^';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	string EQ string
		{
		Tmp1.opr = '=';
		$$ = node (&Tmp1, STRINGOP, $1, $3);
		}|
	string NE string
		{
		Tmp1.opr = '!';
		Tmp2.opr = '=';
		$$ = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, STRINGOP, $1, $3));
		}|
	expr EQ expr
		{
		Tmp1.opr = '=';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr NE expr
		{
		Tmp1.opr = '!';
		Tmp2.opr = '=';
		$$ = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, OPERATOR, $1, $3));
		}|
	string GT string
		{
		Tmp1.opr = '>';
		$$ = node (&Tmp1, STRINGOP, $1, $3);
		}|
	string GE string
		{
		Tmp1.opr = '!';
		Tmp2.opr = '<';
		$$ = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, STRINGOP, $1, $3));
		}|
	expr GT expr
		{
		Tmp1.opr = '>';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr GE expr
		{
		Tmp1.opr = '!';
		Tmp2.opr = '<';
		$$ = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, OPERATOR, $1, $3));
		}|
	string LT string
		{
		Tmp1.opr = '<';
		$$ = node (&Tmp1, STRINGOP, $1, $3);
		}|
	string LE string
		{
		Tmp1.opr = '!';
		Tmp2.opr = '>';
		$$ = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, STRINGOP, $1, $3));
		}|
	expr LT expr
		{
		Tmp1.opr = '<';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr LE expr
		{
		Tmp1.opr = '!';
		Tmp2.opr = '>';
		$$ = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, OPERATOR, $1, $3));
		}|
	expr '&' expr
		{
		Tmp1.opr = '&';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr NAND expr
		{
		Tmp1.opr = '!';
		Tmp2.opr = '&';
		$$ = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, OPERATOR, $1, $3));
		}|
	expr '|' expr
		{
		Tmp1.opr = '|';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr NOR expr
		{
		Tmp1.opr = '!';
		Tmp2.opr = '|';
		$$ = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, OPERATOR, $1, $3));
		}|
	'-' expr %prec UMINUS
		{
		Tmp1.opr = '_';
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	'!' expr
		{
		Tmp1.opr = '!';
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	'l' expr
		{
		Tmp1.opr = 'l';
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	'L' expr
		{
		Tmp1.opr = 'L';
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	SQRT expr
		{
		Tmp1.opr = SQRT;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	SIN expr
		{
		Tmp1.opr = SIN;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	COS expr
		{
		Tmp1.opr = COS;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	TAN expr
		{
		Tmp1.opr = TAN;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	ATAN expr
		{
		Tmp1.opr = ATAN;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	ACOS expr
		{
		Tmp1.opr = ACOS;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	ASIN expr
		{
		Tmp1.opr = ASIN;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	'e' expr
		{
		Tmp1.opr = 'e';
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	'a' expr
		{
		Tmp1.opr = 'a';
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	'f' expr
		{
		Tmp1.opr = 'f';
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	'r' expr
		{
		Tmp1.opr = 'r';
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	'c' expr
		{
		Tmp1.opr = 'c';
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	NTYPE string
		{
		Tmp1.opr = NTYPE;
		$$ = node (&Tmp1, STRINGOP, ENULL, $2);
		}|
	'#' string
		{
		Tmp1.opr = '#';
		$$ = node (&Tmp1, STRINGOP, ENULL, $2);
		}|
	string '[' expr ']'
		{
		Tmp1.opr = '[';
		$$ = node (&Tmp1, STRINGOP, $1, $3);
		}|
	string 'C' string
		{
		Tmp1.opr = 'C';
		$$ = node (&Tmp1, STRINGOP, $1, $3);
		}|
	string NOTIN string
		{
		Tmp1.opr = '!';
		Tmp2.opr = 'C';
		$$ = node (&Tmp1, OPERATOR, ENULL,
			node (&Tmp2, STRINGOP, $1, $3));
		}|
	expr '?' stringexpr ':' stringexpr
		{
		Tmp1.opr = '?';
		Tmp2.opr = ':';
		$$ = node (&Tmp1, OPERATOR, $1,
			node (&Tmp2, OPERATOR,  $3, $5));
		}|
	IF expr THEN stringexpr ELSE stringexpr 
		{
		Tmp1.opr = '?';
		Tmp2.opr = ':';
		$$ = node (&Tmp1, OPERATOR, $2,
			node (&Tmp2, OPERATOR, $4, $6));
		}|
	NUMBER
		{
		Tmp1.num = $1;
		$$ = node (&Tmp1, FLOATPTR, ENULL, ENULL);
		};
string :
	'('  string ')'
		{
		$$ = $2;
		}|
	STRING
		{
		Tmp1.str = $1;
		$$ = node (&Tmp1, STRINGPTR, ENULL, ENULL);
		};
%%

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
