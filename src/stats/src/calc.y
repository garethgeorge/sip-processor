%{
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
%}

%union
	{
	int 	opr;   /* an operator */
	int 	var;   /* an index into the variable table */
	double	*num;  /* a pointer to a numerical constant */
	ENODE	*ex;   /* an expression in the parse tree */
	}

%type <ex> expr
%token <num> NUMBER
%token	<var> VARIABLE
%nonassoc <opr> '#'
%right <opr> '='
%left <opr> '?' IF THEN
%left <opr> ':' ELSE
%left <opr> '|'
%left <opr> '&'
%nonassoc <opr> '!'
%nonassoc <opr> EQ NE GE LE '<' '>'
%left <opr> '+' '-'
%left <opr> '*' '/' '%'
%right <opr> '^'
%nonassoc <opr> UMINUS ABS EXP LOG SQRT COS TAN SIN ACOS ASIN ATAN
%%

start:
	expr = { Expression = $1;};
expr :
	'('  expr ')' = { $$ = $2; }|
	VARIABLE '=' expr =
			{
			if (checkrecursion ($1, $3))
				{
				fprintf (stderr, "calc: Can't have recursive definitions\n");
				Variable[$1] = NULL;
				}
			else Variable[$1] = $3;
			$$ = $3;
		}|
	'#' expr =
		{
		Constant = (double *) malloc (sizeof (double));
		if (Constant == NULL)
			errorexit ("Out of storage space");
		*Constant = eval ($2);
		Tmp1.num = Constant;
		$$ = node (&Tmp1, NUMBER, ENULL, ENULL);
		}|
	expr '+' expr =
		{
		Tmp1.opr = '+';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr '-' expr =
		{
		Tmp1.opr = '-';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr '*' expr =
		{
		Tmp1.opr = '*';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr '%' expr =
		{
		Tmp1.opr = '%';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr '/' expr =
		{
		Tmp1.opr = '/';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr '^' expr =
		{
		Tmp1.opr = '^';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	'-' expr %prec UMINUS =
		{
		Tmp1.opr = '_';
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	expr EQ expr =
		{
		Tmp1.opr = EQ;
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr NE expr =
		{
		Tmp1.opr = NE;
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr LE expr =
		{
		Tmp1.opr = LE;
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr '<' expr =
		{
		Tmp1.opr = '<';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr GE expr =
		{
		Tmp1.opr = GE;
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr '>' expr =
		{
		Tmp1.opr = '>';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr '&' expr =
		{
		Tmp1.opr = '&';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	expr '|' expr =
		{
		Tmp1.opr = '|';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	'!' expr      =
		{
		Tmp1.opr = '!';
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	expr '?' expr ':' expr =
		{
		Tmp1.opr = '?';
		Tmp2.opr = ':';
		$$ = node (&Tmp1, OPERATOR, $1, node (&Tmp2, OPERATOR, $3, $5));
		}|
	IF expr THEN expr =
		{
		Tmp1.opr = '?';
		$$ = node (&Tmp1, OPERATOR, $2, $4);
		}|
	expr ELSE expr =
		{
		Tmp1.opr = ':';
		$$ = node (&Tmp1, OPERATOR, $1, $3);
		}|
	ACOS expr =
		{
		Tmp1.opr = ACOS;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	ASIN expr =
		{
		Tmp1.opr = ASIN;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	ATAN expr =
		{
		Tmp1.opr = ATAN;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	COS expr =
		{
		Tmp1.opr = COS;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	SIN expr =
		{
		Tmp1.opr = SIN;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	TAN expr =
		{
		Tmp1.opr = TAN;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	LOG expr =
		{
		Tmp1.opr = LOG;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	EXP expr =
		{
		Tmp1.opr = EXP;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	ABS expr =
		{
		Tmp1.opr = ABS;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	SQRT expr =
		{
		Tmp1.opr = SQRT;
		$$ = node (&Tmp1, OPERATOR, ENULL, $2);
		}|
	VARIABLE =
		{
		Tmp1.var = $1;
		$$ = node (&Tmp1, VARIABLE, ENULL, ENULL);
		}|
	NUMBER =
		{
		Tmp1.num = $1;
		$$ = node (&Tmp1, NUMBER, ENULL, ENULL);
		};
%%

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
