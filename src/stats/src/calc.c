extern char *malloc(), *realloc();

# line 2 "calc.y"
/*  Copyright 1981 Gary Perlman */

#ifdef sun /* need corrective version of atof */
double	myatof ();
#else
#define	myatof(s)   atof(s)
#endif

#ifdef __STDC__
#include "stdlib.h"
#else
extern	double	atof ();
extern	char	*malloc ();
#endif

#ifndef lint
static	char	sccsid[] = "@(#) calc.y 5.3 (|stat) 10/1/88";
#endif

#define	CALC_VERSION                   "5.3 10/1/88"
/* PGM(calc, Algebraic Modeling Calculator) */

#ifdef __MSDOS__
#define	MARK_EOF "Ctrl-Z"
#elif macintosh
#define	MARK_EOF "Apple-."
#else
#define	MARK_EOF "^D"
#endif

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <signal.h>
#ifdef	macintosh
#include <IOCtl.h>
#define isatty(x) (!ioctl(x, FIOINTERACTIVE, NULL))
#endif

#define	FZERO         (10e-10)
#define	fzero(x)      (fabs (x) < FZERO)
#define	isvarchar(c)  (isalnum (c) || (c) == '_')

#ifndef iscntrl
#define iscntrl(a) ((a)>0&&(a)<26)
#endif
#define OPERATOR     1
#define PARSERROR    1
#define	MAXVAR    1000 
#define	UNDEFINED   -99999999999.987654321
int 	Nvar;
char	*Varname[MAXVAR];
char	*Eptr;
int 	Printequation = 1;
char	*Prompt = "CALC: ";
int 	Interactive = 0;
char	Format[100] = "%.10g";    /* default print format for numbers */

typedef	union
	{
	int 	opr;        /* if OPERATOR */
	double	*num;       /* if NUMBER */
	int 	var;        /* if VARIABLE */
	} STUFF;
STUFF	Tmp1, Tmp2;  /* used in the parser to cast operators */
typedef struct enode        /* expression node in tree */
	{
	int 	etype;          /* type of node */
	STUFF	contents;
	struct	enode *left;
	struct	enode *right;
	} ENODE;
#define	ENULL ((ENODE *) NULL)
ENODE	*Expression, *Variable[MAXVAR];

double	eval (), Answer;
double	*Constant;
char	*getline ();
FILE	*Outfile = stdout;

# line 83 "calc.y"
typedef union 
	{
	int 	opr;   /* an operator */
	int 	var;   /* an index into the variable table */
	double	*num;  /* a pointer to a numerical constant */
	ENODE	*ex;   /* an expression in the parse tree */
	} YYSTYPE;
# define NUMBER 257
# define VARIABLE 258
# define IF 259
# define THEN 260
# define ELSE 261
# define EQ 262
# define NE 263
# define GE 264
# define LE 265
# define UMINUS 266
# define ABS 267
# define EXP 268
# define LOG 269
# define SQRT 270
# define COS 271
# define TAN 272
# define SIN 273
# define ACOS 274
# define ASIN 275
# define ATAN 276
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 287 "calc.y"


yylex ()
	{
	extern	YYSTYPE yylval;
	char	tmpvarname[BUFSIZ];
	int 	i;
	while (isspace (*Eptr)) Eptr++;
	if (begins ("acos", Eptr)) {Eptr += 4; return (ACOS);}
	if (begins ("asin", Eptr)) {Eptr += 4; return (ASIN);}
	if (begins ("atan", Eptr)) {Eptr += 4; return (ATAN);}
	if (begins ("cos", Eptr)) {Eptr += 3; return (COS);}
	if (begins ("sin", Eptr)) {Eptr += 3; return (SIN);}
	if (begins ("tan", Eptr)) {Eptr += 3; return (TAN);}
	if (begins ("log", Eptr)) {Eptr += 3; return (LOG);}
	if (begins ("sqrt", Eptr)) {Eptr += 4; return (SQRT);}
	if (begins ("exp", Eptr)) {Eptr += 3; return (EXP);}
	if (begins ("abs", Eptr)) {Eptr += 3; return (ABS);}
	if (begins ("if", Eptr)) {Eptr += 2; return (IF);}
	if (begins ("then", Eptr)) {Eptr += 4; return (THEN);}
	if (begins ("else", Eptr)) {Eptr += 4; return (ELSE);}
	if (*Eptr == '$')
		{
		Eptr++;
		yylval.num = &Answer;
		return (NUMBER);
		}
	if (isdigit (*Eptr) || *Eptr == '.')
		{
		Constant = (double *) malloc (sizeof (double));
		if (Constant == NULL)
			errorexit ("Out of storage space");
		*Constant = myatof (Eptr);
		yylval.num = Constant;
		Eptr += skipnumber (Eptr, 1);
		return (NUMBER);
		}
	if (isvarchar (*Eptr))
		{
		for (i = 0; isvarchar (Eptr[i]); i++)
			tmpvarname[i] = Eptr[i];
		tmpvarname[i] = '\0';
		Eptr += i;
		i = 0;
		while (i < Nvar && strcmp (tmpvarname, Varname[i])) i++;
		if (i == Nvar)
			{
			Varname[i] = (char *) malloc ((unsigned) (strlen(tmpvarname)+1));
			if (Varname[i] == NULL)
				errorexit ("Out of storage space");
			(void) strcpy (Varname[i], tmpvarname);
			if (++Nvar == MAXVAR)
				errorexit ("Too many variables");
			}
		yylval.var = i;
		return (VARIABLE);
		}
	if (begins ("!=", Eptr)) { Eptr += 2; return (NE); }
	if (begins (">=", Eptr)) { Eptr += 2; return (GE); }
	if (begins ("<=", Eptr)) { Eptr += 2; return (LE); }
	if (begins ("==", Eptr)) { Eptr += 2; return (EQ); }
	if (begins ("**", Eptr)) { Eptr += 2; return ('^'); }
	if (begins ("&&", Eptr)) { Eptr += 2; return ('&'); }
	if (begins ("||", Eptr)) { Eptr += 2; return ('|'); }
	return ((int) *Eptr++);
	}

yyerror (msg)
char	*msg;
	{
	fprintf (Outfile, "%s:\n", msg);
	fprintf (Outfile, "Parsing error.  ");
	fprintf (Outfile, "This is left in input: [%s]\n", Eptr-1);
	}

ENODE *
node (datum, datatype, lson, rson)
STUFF	*datum;       /* pointer to a number or an operator */
int 	datatype;     /* NUMBER or VARIABLE or OPERATOR */
ENODE	*lson;        /* left part of tree */
ENODE	*rson;        /* right part of tree */
	{
	ENODE *newnode;
	newnode = (ENODE *) malloc (sizeof (ENODE));
	if (newnode == ENULL)
		errorexit ("Out of storage space");
	newnode->etype = datatype;
	if (datatype == OPERATOR)
		newnode->contents.opr = datum->opr;
	else if (datatype == VARIABLE)
		newnode->contents.var = datum->var;
	else /* (datatype == NUMBER) */
		newnode->contents.num = datum->num;
	newnode->left = lson;
	newnode->right = rson;
	return (newnode);
	}

main (argc, argv) int argc; char *argv[];
	{
	int 	i;
	signal (SIGINT, SIG_IGN);
	if (isatty (fileno (stdin)))
		{
		Interactive = 1;
		printf ("calc: version %s (Copyright 1981 Gary Perlman)\n", CALC_VERSION);
		printf ("Enter expressions after the prompt '%s'\n", Prompt);
		printf ("Quit with %s, get help with ?\n", MARK_EOF);
		}
	for (i = 1; i < argc; i++)
		process (argv[i]);
	process ("-");
	if (Interactive) /* wipe out prompt */
		printf ("\015           \015");
	exit (0);
	}

process (filename) char *filename;
	{
#ifdef macintosh
	int 	i;
#endif
	char	exprline[BUFSIZ];
	FILE	*ioptr;
	if (filename == NULL || !strcmp (filename, "-"))
		{
		ioptr = stdin;
		filename = NULL;
		}
	else if ((ioptr = fopen (filename, "r")) == NULL)
		{
		fprintf (stderr, "Can't open %s\n", filename);
		return;
		}
	if (filename)
		fprintf (Outfile, "Reading from %s\n", filename);
	for (;;)
		{
		if (ioptr == stdin && Interactive)
			fprintf (Outfile, Prompt);
		if (!getline (exprline, ioptr)) break;
#ifdef macintosh
		for (i=0; exprline[i] == Prompt[i] && exprline[i]; i++) ;
		for (; exprline[i] == ' ' || exprline[i] == '\t'; i++) ;
		strcpy(exprline, exprline+i);
#endif
		Eptr = exprline;
		while (isspace (*Eptr))
			Eptr++;
		if (*Eptr == '\0' || *Eptr == '?')
			{
			if (filename == NULL && Interactive)
				printmenu ();
			continue;
			}
		if (iscntrl (*Eptr))
			{
			control (Eptr);
			continue;
			}
		if (yyparse() == PARSERROR)
			continue;
		if (Printequation || ioptr != stdin)
			ptree (Outfile, Expression);
		if (fzero (Answer = eval (Expression)))
			Answer = 0.0;
		if (Printequation)
			printf (" =");
		if (Answer == UNDEFINED)
			fprintf (Outfile, "\tUNDEFINED\n");
		else
			{
			putc ('\t', Outfile);
			fprintf (Outfile, Format, Answer);
			putc ('\n', Outfile);
			}
		}
	if (ioptr != stdin)
		(void) fclose (ioptr);
	}

printmenu ()
	{
	puts ("Expressions are in standard C syntax (like algebra).");
	puts ("The following CTRL characters have special functions:");
	puts ("(You may have to precede the character with a ^V)");
	printf ("%s	end of input to CALC\n", MARK_EOF);
	puts ("^P	toggles the printing of equations");
	puts ("^Rfile	reads the expressions from the named file");
	puts ("^Wfile	writes all expressions to the named file");
	puts ("	If no file is supplied, then print to the screen");
	}

#define	CTRL_PRINT     16
#define	CTRL_READ      18
#define	CTRL_WRITE     23
#define	CTRL_VAR       22
#define	CTRL_FMT        6

control (key) char *key;
	{
	int 	var;
	FILE	*saveoutfile;
	switch (*key)
		{
		case CTRL_FMT:
			if (key[1] == '\0' || isspace (key[1]))
				fprintf (Outfile, "Current_Format = '%s'\n", Format);
			else
				strcpy (Format+1, key+1);
			return;
		case CTRL_PRINT:
			Printequation = !Printequation;
			return;
		case CTRL_READ:
			while (iscntrl (*key) || isspace (*key))
				key++;
			process (key);
			return;
		case CTRL_WRITE:
		case CTRL_VAR:
			while (*key && (iscntrl (*key) || isspace (*key)))
				key++;
			if (*key)
				{
				fprintf (Outfile, "Writing to %s\n", key);
				saveoutfile = Outfile;
				if ((Outfile = fopen (key, "a")) == NULL)
					{
					fprintf (stderr, "Can't open %s\n", key);
					Outfile = saveoutfile;
					}
				}
			for (var = 0; var < Nvar; var++)
				{
				fprintf (Outfile, "%-10s = ", Varname[var]);
				if (Outfile == stdout)
					{
					if (fzero (Answer = eval (Variable[var])))
						Answer = 0.0;
					if (Answer == UNDEFINED)
						fprintf (Outfile, " UNDEFINED = ");
					else
						{
						fprintf (Outfile, Format, Answer);
						fprintf (Outfile, " = ");
						}
					}
				ptree (Outfile, Variable[var]);
				fprintf (Outfile, "\n");
				}
			if (*key)
				{
				(void) fclose (Outfile);
				Outfile = saveoutfile;
				}
			return;
		default:
			fprintf (stderr, "Unknown control character.\n");
		}
	}

double
eval (expression)
ENODE *expression;
	{
	double	tmp1, tmp2;

	if (expression == ENULL)
		return (UNDEFINED);
	if (expression->etype == NUMBER)
		return (*expression->contents.num);
	if (expression->etype == VARIABLE)
		{
		if (Variable[expression->contents.var])
			return (eval (Variable[expression->contents.var]));
		else
			return (UNDEFINED);
		}

	if ((tmp2 = eval (expression->right)) == UNDEFINED)
		return (UNDEFINED);

	switch (expression->contents.opr)
	{
	case '_': return (-tmp2);
	case '!': return (fzero (tmp2) ? 1.0 : 0.0);
	case LOG: if (tmp2 <= FZERO) return (UNDEFINED);
		else return (log (tmp2));
	case COS: return (cos (tmp2));
	case SIN: return (sin (tmp2));
	case TAN: return (tan (tmp2));
	case ACOS: return (acos (tmp2));
	case ASIN: return (asin (tmp2));
	case ATAN: return (atan (tmp2));
	case EXP: return (exp (tmp2));
	case ABS: return (fabs (tmp2));
	case SQRT: if (tmp2 < 0.0) return (UNDEFINED);
		else return (sqrt (tmp2));
	}

	if ((tmp1 = eval (expression->left)) == UNDEFINED)
		return (UNDEFINED);
	switch (expression->contents.opr)
	{
	case '+': return (tmp1 + tmp2);
	case '-': return (tmp1 - tmp2);
	case '*': return (tmp1 * tmp2);
	case '%': if ((int) tmp2 == 0) return (tmp1);
		else return ((double) (((int) tmp1) % ((int) tmp2)));
	case '/': if (fzero (tmp2)) return (UNDEFINED);
		else return (tmp1/tmp2);
	case '^': return (pow (tmp1, tmp2));
	case '&': return ((!fzero (tmp1) && !fzero (tmp2)) ? 1.0 : 0.0);
	case '|': return ((!fzero (tmp1) || !fzero (tmp2)) ? 1.0 : 0.0);
	case '>': return (tmp1 > tmp2 ? 1.0 : 0.0);
	case '<': return (tmp1 < tmp2 ? 1.0 : 0.0);
	case EQ : return (fzero (tmp1 - tmp2) ? 1.0 : 0.0);
	case NE : return (!fzero (tmp1 - tmp2) ? 1.0 : 0.0);
	case LE : return (tmp1 <= tmp2 ? 1.0 : 0.0);
	case GE : return (tmp1 >= tmp2 ? 1.0 : 0.0);
	case ':': return (0.0); /* dummy return for ? */
	case '?':
		if (expression->right->contents.opr == ':')
			return (!fzero (tmp1)
				? eval (expression->right->left)
				: eval (expression->right->right));
		else if (!fzero (tmp1)) return (eval (expression->right));
		else return (UNDEFINED);
	default: fprintf (stderr, "calc: Unknown operator '%c' = %d\n",
		expression->contents.opr, expression->contents.opr);
		return (UNDEFINED);
	}
	}

ptree (ioptr, expression) ENODE *expression; FILE *ioptr;
	{
	if (expression == ENULL)
		return;
	if (expression->etype == VARIABLE)
		{
		fprintf (ioptr, "%s", Varname[expression->contents.var]);
		return;
		}
	else if (expression->etype == NUMBER)
		{
		if (*expression->contents.num == UNDEFINED)
			fprintf (ioptr, "UNDEFINED");
		else
			fprintf (ioptr, Format, *expression->contents.num);
		return;
		}
	switch	(expression->contents.opr)
		{
		case LOG: fprintf (ioptr, "log("); break;
		case ABS: fprintf (ioptr, "abs("); break;
		case EXP: fprintf (ioptr, "exp("); break;
		case SQRT: fprintf (ioptr, "sqrt("); break;
		case ATAN: fprintf (ioptr, "atan("); break;
		case ASIN: fprintf (ioptr, "asin("); break;
		case ACOS: fprintf (ioptr, "acos("); break;
		case TAN: fprintf (ioptr, "tan("); break;
		case SIN: fprintf (ioptr, "sin("); break;
		case COS: fprintf (ioptr, "cos("); break;
		case '_' : putc ('-', ioptr);
			ptree (ioptr, expression->right); return;
		case '?':
			fprintf (ioptr, "(if ");
			ptree (ioptr, expression->left);
			fprintf (ioptr, " then ");
			if (expression->right->contents.opr == ':')
				{
				ptree (ioptr, expression->right->left);
				fprintf (ioptr, " else ");
				ptree (ioptr, expression->right->right);
				}
			else ptree (ioptr, expression->right);
			putc (')', ioptr);
			return;
		default: putc ('(', ioptr);
			ptree (ioptr, expression->left);
			switch (expression->contents.opr)
				{
				case EQ: fprintf (ioptr, " == "); break;
				case NE: fprintf (ioptr, " != "); break;
				case GE: fprintf (ioptr, " >= "); break;
				case LE: fprintf (ioptr, " <= "); break;
				default: fprintf (ioptr, " %c ",expression->contents.opr);
				}
		}
	ptree (ioptr, expression->right);
	putc (')', ioptr);
	}

/* Suzanne Shouman fixed a bug here. Thanks */
begins (s1, s2) char *s1, *s2;
	{
	int 	alphlag = isvarchar (*s1);
	while (*s1)
		if (*s1++ != *s2++) return (0);
	return (alphlag ? !isvarchar(*s2) : 1);
	}

checkrecursion (varno, expr)
int 	varno;      /* look for recursion involving this variable */
ENODE	*expr;      /* look for recursion of varno in this expr */
	{
	if (expr == ENULL || expr->etype == NUMBER)
		return (0);

	if (expr->etype == VARIABLE)
		{
		if (expr->contents.var == varno)
			return (1);
		if (checkrecursion (varno, Variable[expr->contents.var]))
			return (1);
		}

	return
		(
		checkrecursion (varno, expr->left)
		||
		checkrecursion (varno, expr->right)
		);
	}

char *
getline (line, ioptr) char *line; FILE *ioptr;
	{
	register int C;
	register char *lptr = line;
	while ((C = getc (ioptr)) != '\n' && C != ';' && C != EOF)
		*lptr++ = C;
	if (C == EOF) return (NULL);
	while (C != '\n' && C != EOF) C = getc (ioptr);
	*lptr = '\0';
	return (line);
	}

errorexit (string) char *string;
	{
	fprintf (stderr, "calc: %s\n", string);
	control ("\027calc.save");
	fprintf (stderr, "Current state saved in calc.save\n");
	exit (1);
	}
int yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
-1, 58,
	262, 0,
	263, 0,
	264, 0,
	265, 0,
	60, 0,
	62, 0,
	-2, 12,
-1, 59,
	262, 0,
	263, 0,
	264, 0,
	265, 0,
	60, 0,
	62, 0,
	-2, 13,
-1, 60,
	262, 0,
	263, 0,
	264, 0,
	265, 0,
	60, 0,
	62, 0,
	-2, 14,
-1, 61,
	262, 0,
	263, 0,
	264, 0,
	265, 0,
	60, 0,
	62, 0,
	-2, 15,
-1, 62,
	262, 0,
	263, 0,
	264, 0,
	265, 0,
	60, 0,
	62, 0,
	-2, 16,
-1, 63,
	262, 0,
	263, 0,
	264, 0,
	265, 0,
	60, 0,
	62, 0,
	-2, 17,
	};
# define YYNPROD 36
# define YYLAST 456
int yyact[]={

     7,    25,     5,    23,    32,    37,     1,     3,    22,    20,
     0,    21,     6,    24,     0,    23,     0,     0,     0,     0,
    22,    20,    23,    21,    71,    24,    29,    22,    31,    34,
    23,    32,    24,     0,     0,    22,    20,     0,    21,     0,
    24,    23,    32,     0,     0,    68,    22,    20,     0,    21,
     0,    24,     0,    29,     0,    31,    34,     0,     0,     0,
    25,     0,     0,     0,    29,     0,    31,    34,    23,    32,
     0,     0,    25,    22,    20,     0,    21,     0,    24,    25,
     0,     0,     0,     0,     0,     0,     0,    25,     0,     0,
    33,    29,     0,    31,    34,    23,    32,     0,    25,     0,
    22,    20,     0,    21,     0,    24,    23,    32,     0,     0,
     0,    22,    20,     0,    21,     0,    24,    33,    29,     0,
    31,     0,     0,     0,     0,    25,     0,     0,    33,    29,
     0,    31,     0,    23,    32,     0,     0,     0,    22,    20,
     0,    21,     0,    24,     0,     0,     0,     0,     0,     0,
     0,     0,    25,     0,    23,    33,    29,     0,    31,    22,
    20,     0,    21,    25,    24,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    29,     0,    31,
     0,     0,    33,     0,     0,     0,     0,     0,     0,     0,
    25,     0,     0,    33,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    25,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    19,     4,     8,    35,    26,    27,
    30,    28,     0,     0,    17,    16,    15,    18,    12,    14,
    13,     9,    10,    11,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    70,    35,    26,    27,    30,    28,     0,
     0,     0,     0,     0,     0,    35,    26,    27,    30,    28,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    35,    26,    27,    30,    28,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    35,
    26,    27,    30,    28,     0,     0,     0,     0,     0,     0,
     0,    26,    27,    30,    28,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    26,    27,
    30,    28,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    26,
    27,    30,    28,     2,     0,     0,     0,    36,     0,    38,
    39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
    49,    50,    51,     0,    52,    53,    54,    55,    56,    57,
    58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
     0,    69,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    72,    73 };
int yypact[]={

   -33, -1000,    31,   -33,   -56,   -33,   -33,   -33,   -33,   -33,
   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33, -1000,
   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,
   -33,   -33,   -33,   -33,   -33,   -33,     4,   -33,    31, -1000,
   117,    -7, -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
 -1000, -1000,   -15,   -15,   -93,   -93,   -93,   -93,   -22,   -22,
   -22,   -22,   -22,   -22,   117,    96,   -34,    69, -1000,    31,
   -33,   -33,    58,    69 };
int yypgo[]={

     0,   383,     6 };
int yyr1[]={

     0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1 };
int yyr2[]={

     0,     3,     7,     7,     5,     7,     7,     7,     7,     7,
     7,     5,     7,     7,     7,     7,     7,     7,     7,     7,
     5,    11,     9,     7,     5,     5,     5,     5,     5,     5,
     5,     5,     5,     5,     3,     3 };
int yychk[]={

 -1000,    -2,    -1,    40,   258,    35,    45,    33,   259,   274,
   275,   276,   271,   273,   272,   269,   268,   267,   270,   257,
    43,    45,    42,    37,    47,    94,   262,   263,   265,    60,
   264,    62,    38,   124,    63,   261,    -1,    61,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,
   260,    58,    -1,    -1 };
int yydef[]={

     0,    -2,     1,     0,    34,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    35,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     4,    11,
    20,     0,    24,    25,    26,    27,    28,    29,    30,    31,
    32,    33,     5,     6,     7,     8,     9,    10,    -2,    -2,
    -2,    -2,    -2,    -2,    18,    19,     0,    23,     2,     3,
     0,     0,    22,    21 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"NUMBER",	257,
	"VARIABLE",	258,
	"#",	35,
	"=",	61,
	"?",	63,
	"IF",	259,
	"THEN",	260,
	":",	58,
	"ELSE",	261,
	"|",	124,
	"&",	38,
	"!",	33,
	"EQ",	262,
	"NE",	263,
	"GE",	264,
	"LE",	265,
	"<",	60,
	">",	62,
	"+",	43,
	"-",	45,
	"*",	42,
	"/",	47,
	"%",	37,
	"^",	94,
	"UMINUS",	266,
	"ABS",	267,
	"EXP",	268,
	"LOG",	269,
	"SQRT",	270,
	"COS",	271,
	"TAN",	272,
	"SIN",	273,
	"ACOS",	274,
	"ASIN",	275,
	"ATAN",	276,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"start : expr",
	"expr : '(' expr ')'",
	"expr : VARIABLE '=' expr",
	"expr : '#' expr",
	"expr : expr '+' expr",
	"expr : expr '-' expr",
	"expr : expr '*' expr",
	"expr : expr '%' expr",
	"expr : expr '/' expr",
	"expr : expr '^' expr",
	"expr : '-' expr",
	"expr : expr EQ expr",
	"expr : expr NE expr",
	"expr : expr LE expr",
	"expr : expr '<' expr",
	"expr : expr GE expr",
	"expr : expr '>' expr",
	"expr : expr '&' expr",
	"expr : expr '|' expr",
	"expr : '!' expr",
	"expr : expr '?' expr ':' expr",
	"expr : IF expr THEN expr",
	"expr : expr ELSE expr",
	"expr : ACOS expr",
	"expr : ASIN expr",
	"expr : ATAN expr",
	"expr : COS expr",
	"expr : SIN expr",
	"expr : TAN expr",
	"expr : LOG expr",
	"expr : EXP expr",
	"expr : ABS expr",
	"expr : SQRT expr",
	"expr : VARIABLE",
	"expr : NUMBER",
};
#endif /* YYDEBUG */
#line 1 "/usr/lib/yaccpar"
/*	@(#)yaccpar 1.10 89/04/04 SMI; from S5R3 1.10	*/

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	{ free(yys); free(yyv); return(0); }
#define YYABORT		{ free(yys); free(yyv); return(1); }
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
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
#define YYFLAG		(-1000)

/*
** static variables used by the parser
*/
static YYSTYPE *yyv;			/* value stack */
static int *yys;			/* state stack */

static YYSTYPE *yypv;			/* top of value stack */
static int *yyps;			/* top of state stack */

static int yystate;			/* current state */
static int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */

int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */


/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */
	unsigned yymaxdepth = YYMAXDEPTH;

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yyv = (YYSTYPE*)malloc(yymaxdepth*sizeof(YYSTYPE));
	yys = (int*)malloc(yymaxdepth*sizeof(int));
	if (!yyv || !yys)
	{
		yyerror( "out of memory" );
		return(1);
	}
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

			(void)printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				(void)printf( "end-of-file\n" );
			else if ( yychar < 0 )
				(void)printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				(void)printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
			/*
			** reallocate and recover.  Note that pointers
			** have to be reset, or bad things will happen
			*/
			int yyps_index = (yy_ps - yys);
			int yypv_index = (yy_pv - yyv);
			int yypvt_index = (yypvt - yyv);
			yymaxdepth += YYMAXDEPTH;
			yyv = (YYSTYPE*)realloc((char*)yyv,
				yymaxdepth * sizeof(YYSTYPE));
			yys = (int*)realloc((char*)yys,
				yymaxdepth * sizeof(int));
			if (!yyv || !yys)
			{
				yyerror( "yacc stack overflow" );
				return(1);
			}
			yy_ps = yys + yyps_index;
			yy_pv = yyv + yypv_index;
			yypvt = yyv + yypvt_index;
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

			(void)printf( "Received token " );
			if ( yychar == 0 )
				(void)printf( "end-of-file\n" );
			else if ( yychar < 0 )
				(void)printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				(void)printf( "%s\n", yytoks[yy_i].t_name );
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

				(void)printf( "Received token " );
				if ( yychar == 0 )
					(void)printf( "end-of-file\n" );
				else if ( yychar < 0 )
					(void)printf( "-none-\n" );
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
					(void)printf( "%s\n", yytoks[yy_i].t_name );
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
				yyerror( "syntax error" );
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
						(void)printf( _POP_, *yy_ps,
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

					(void)printf( "Error recovery discards " );
					if ( yychar == 0 )
						(void)printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						(void)printf( "token -none-\n" );
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
						(void)printf( "token %s\n",
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
			(void)printf( "Reduce by (%d) \"%s\"\n",
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
# line 109 "calc.y"
 { Expression = yypvt[-0].ex;} break;
case 2:
# line 111 "calc.y"
 { yyval.ex = yypvt[-1].ex; } break;
case 3:
# line 112 "calc.y"

			{
			if (checkrecursion (yypvt[-2].var, yypvt[-0].ex))
				{
				fprintf (stderr, "calc: Can't have recursive definitions\n");
				Variable[yypvt[-2].var] = NULL;
				}
			else Variable[yypvt[-2].var] = yypvt[-0].ex;
			yyval.ex = yypvt[-0].ex;
		} break;
case 4:
# line 122 "calc.y"

		{
		Constant = (double *) malloc (sizeof (double));
		if (Constant == NULL)
			errorexit ("Out of storage space");
		*Constant = eval (yypvt[-0].ex);
		Tmp1.num = Constant;
		yyval.ex = node (&Tmp1, NUMBER, ENULL, ENULL);
		} break;
case 5:
# line 131 "calc.y"

		{
		Tmp1.opr = '+';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 6:
# line 136 "calc.y"

		{
		Tmp1.opr = '-';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 7:
# line 141 "calc.y"

		{
		Tmp1.opr = '*';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 8:
# line 146 "calc.y"

		{
		Tmp1.opr = '%';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 9:
# line 151 "calc.y"

		{
		Tmp1.opr = '/';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 10:
# line 156 "calc.y"

		{
		Tmp1.opr = '^';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 11:
# line 161 "calc.y"

		{
		Tmp1.opr = '_';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 12:
# line 166 "calc.y"

		{
		Tmp1.opr = EQ;
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 13:
# line 171 "calc.y"

		{
		Tmp1.opr = NE;
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 14:
# line 176 "calc.y"

		{
		Tmp1.opr = LE;
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 15:
# line 181 "calc.y"

		{
		Tmp1.opr = '<';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 16:
# line 186 "calc.y"

		{
		Tmp1.opr = GE;
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 17:
# line 191 "calc.y"

		{
		Tmp1.opr = '>';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 18:
# line 196 "calc.y"

		{
		Tmp1.opr = '&';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 19:
# line 201 "calc.y"

		{
		Tmp1.opr = '|';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 20:
# line 206 "calc.y"

		{
		Tmp1.opr = '!';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 21:
# line 211 "calc.y"

		{
		Tmp1.opr = '?';
		Tmp2.opr = ':';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-4].ex, node (&Tmp2, OPERATOR, yypvt[-2].ex, yypvt[-0].ex));
		} break;
case 22:
# line 217 "calc.y"

		{
		Tmp1.opr = '?';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 23:
# line 222 "calc.y"

		{
		Tmp1.opr = ':';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 24:
# line 227 "calc.y"

		{
		Tmp1.opr = ACOS;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 25:
# line 232 "calc.y"

		{
		Tmp1.opr = ASIN;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 26:
# line 237 "calc.y"

		{
		Tmp1.opr = ATAN;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 27:
# line 242 "calc.y"

		{
		Tmp1.opr = COS;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 28:
# line 247 "calc.y"

		{
		Tmp1.opr = SIN;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 29:
# line 252 "calc.y"

		{
		Tmp1.opr = TAN;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 30:
# line 257 "calc.y"

		{
		Tmp1.opr = LOG;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 31:
# line 262 "calc.y"

		{
		Tmp1.opr = EXP;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 32:
# line 267 "calc.y"

		{
		Tmp1.opr = ABS;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 33:
# line 272 "calc.y"

		{
		Tmp1.opr = SQRT;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 34:
# line 277 "calc.y"

		{
		Tmp1.var = yypvt[-0].var;
		yyval.ex = node (&Tmp1, VARIABLE, ENULL, ENULL);
		} break;
case 35:
# line 282 "calc.y"

		{
		Tmp1.num = yypvt[-0].num;
		yyval.ex = node (&Tmp1, NUMBER, ENULL, ENULL);
		} break;
	}
	goto yystack;		/* reset registers in driver code */
}
