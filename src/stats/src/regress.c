/*  Copyright 1980 Gary Perlman */

#include "stat.h"
PGM(regress,Multiple Linear Regression,5.5,01/27/87)

#define MAXVAR          30       /* max number of variables */
#define MAXLEN          20       /* maximum length of a string */
#define MAXCHARS    BUFSIZ       /* maximum number of chars in lines */
#define REG              0       /* column number of regressor */
char	Varname[MAXVAR][MAXLEN]; /* names of variables */
int 	Ncases = 0;              /* number of points/lines read in */
int 	NAcount = 0;             /* number of NA missing values */
int 	Nvar;                    /* number of variables read in */
double	correl[MAXVAR][MAXVAR];  /* correlation matrix */
double	covar[MAXVAR][MAXVAR];   /* covariance matrix */
double	sum[MAXVAR];             /* sum of scores for each variable */

#define	FORMWIDTH   20           /* max space for format string */
#define	MAXWIDTH    20           /* max width of field */
#define	MINWIDTH     5           /* min width of field */
int 	Fieldwidth = 10;         /* width of field in format string */
#define	SFORMAT	    "%10.10s "   /* format of strings */
#define	LFORMAT	    "%-10.10s "  /* left justified strings */
#define	FFORMAT	    "%10.4f "    /* floating point format */
char	Sformat[FORMWIDTH] = SFORMAT;
char	Lformat[FORMWIDTH] = LFORMAT;
char	Fformat[FORMWIDTH] = FFORMAT;

#define	PRINTVEC(var,format,label)\
	{\
	printf (Lformat, label);\
	for (col = 0; col < Nvar; col++)\
		printf (format, var);\
	putchar ('\n');\
	}

/* OPTIONS */
int 	Doreg = 1;       /* should regression be done? */
int 	SumSquares;      /* print raw sums of squares */
int 	Covariance;      /* print covariance matrix */
int 	Partial;         /* partial correlation analysis */
int 	Equation;        /* save regression equation */
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */
Boole	Debug = FALSE;

initial (argc, argv) char **argv;
	{
	int 	row;
	extern	int 	optind;
	extern	char	*optarg;
	int 	C;
	int 	opterr = 0;
	ARGV0;
	while ((C = getopt (argc, argv, "ceprsDF:LOV")) != EOF)
		switch (C)
			{
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			case 'D': Debug = TRUE; break;
			case 'F':
				if (setint (Argv0, C, optarg, &Fieldwidth, MINWIDTH, MAXWIDTH))
					opterr++;
				else
					{
					sprintf (Lformat, "%%-%d.%ds ", Fieldwidth, Fieldwidth);
					sprintf (Sformat, "%%%d.%ds ", Fieldwidth, Fieldwidth);
					sprintf (Fformat, "%%%d.%df ", Fieldwidth,(Fieldwidth-2)/2);
					}
				break;
			case 'c': Covariance = 1; break;
			case 'e': Equation = 1; break;
			case 'p': Partial = 1; Doreg = 1; break;
			case 'r': Doreg = 0; break;
			case 's': SumSquares = 1; break;
			default: opterr++; break;
			}
	if (Equation)
		Doreg = 1;
	if (opterr)
		USAGE ("[-cprs] [variable names]")
	if (argc-optind > MAXVAR)
		ERRMANY (variable names, MAXVAR)
	usinfo ();
	for (row = optind; row < argc; row++)
		strncpy (Varname[row-optind], argv[row], MAXLEN);
	if (Doreg)
		if (Varname[REG][0] == '\0')
			strcpy (Varname[REG], "REG");
	for (row = (Doreg ? 1 : 0); row < MAXVAR; row++)
		if (Varname[row][0] == '\0')
			{
			Varname[row][0] = 'A' - (Doreg  ? 1 : 0) + row;
			Varname[row][1] = '\0';
			}
	checkstdin ();
	}

/*FUNCTION main */
main (argc, argv) int argc; char **argv;
	{
	initial (argc, argv);
	input ();
	compute ();
	if (Doreg)
		regress ();
	exit (0);
	}

/*FUNCTION input */
input ()
	{
	char	line[MAXCHARS];  /* each data line read in here */
	int 	ncols;           /* number of columns in an input line */
	char	*in[MAXVAR];     /* each input column of line in here */
	double	temp[MAXVAR];    /* each input column number in here */
	double	minx[MAXVAR];    /* minimum value of each variable */
	double	maxx[MAXVAR];    /* maximum value of each variable */
	register int row, col;

	while (gets (line))
		{
		if ((ncols = parselin (line, in, MAXVAR)) == 0)
			continue;
		if (ncols > MAXVAR)
			ERRMANY (input columns, MAXVAR)
		for (col = 0; col < ncols; col++)
			if (isna (in[col]))
				break;
		if (col < ncols)
			{
			NAcount++;
			continue;
			}
		if (Ncases == 0)
			{
			Nvar = ncols;
			for (row = 0; row < Nvar; row++)
				minx[row] = maxx[row] = atof (in[row]);
			}
		else if (ncols != Nvar)
			ERRRAGGED
		for (row = 0; row < Nvar; row++)
			{
			if (number (in[row]))
				temp[row] = atof (in[row]);
			else
				ERRNUM (in[row],input data)
			sum[row] += temp[row];
			if (temp[row] > maxx[row])
				maxx[row] = temp[row];
			if (temp[row] < minx[row])
				minx[row] = temp[row];
			for (col = 0; col <= row; col++)
				covar[row][col] += temp[row] * temp[col];
			}
		Ncases++;
		}
	if (Ncases <= 1)
		ERRDATA
	printf ("Analysis for %d cases of %d variables:\n", Ncases, Nvar);
	if (NAcount)
		printf ("Missing cases ignored: %d\n", NAcount);
	if (SumSquares)
		printmat (covar, "Raw SS Matrix");
	else
		PRINTVEC (Varname[col], Sformat, "Variable")
	PRINTVEC (minx[col], Fformat, "Min")
	PRINTVEC (maxx[col], Fformat, "Max")
	PRINTVEC (sum[col], Fformat, "Sum")
	for (row = 0; row < Nvar; row++)
		for (col = 0; col <= row; col++)
			covar[row][col] -= sum[row]*sum[col]/Ncases;
	PRINTVEC (sum[col]/Ncases, Fformat, "Mean")
	PRINTVEC (sqrt (covar[col][col]/(Ncases-1)), Fformat, "SD");
	if (Covariance)
		printmat (covar, "Covariance Matrix");
	}

/*FUNCTION compute: calculate correlation matrix */
compute ()
	{
	register int row, col;
	double	denom;
	for (row = 0; row < Nvar; row++)
		{
		for (col = 0; col < row; col++)
			if (!fzero (denom = covar[row][row] * covar[col][col]))
				correl[row][col] = covar[row][col]/sqrt (denom);
			else correl[row][col] = 0.0;
		correl[row][row] = 1.0;
		}
	printmat (correl, "Correlation Matrix");
	}

	double	invcor[MAXVAR][MAXVAR];  /* inverse correlation matrix */
	double	partial[MAXVAR][MAXVAR]; /* partial correlations */
/*FUNCTION regress */
regress ()
	{
	register int row, col, var;
	double	pof ();                  /* probability of F */
	double	invert ();       	     /* matrix inversion. returns det */
	double	beta[MAXVAR];            /* standardized regression weights */
	double	b[MAXVAR];               /* regression coefficients slopes */
	double	a;                       /* intercept in regression equation */
	double	F;                       /* F ratio */
	double	Rsq;                     /* squared correlation coefficient */
	double	SEestsq;                 /* squared SE estimate */
	int 	npred = Nvar - 1;        /* number of predictors */
	int 	dferror = Ncases - Nvar;/* degrees of freedom for error */
	char	Fline[20];               /* holds the F ratio line */

	if (npred < 1)
		ERRMSG0 (No predictor variables for regression)
	if (dferror < 1)
		ERRMSG0 (Not enough degrees of freedom for regression)

	for (row = 0; row < Nvar; row++)
		for (col = 0; col < row; col++)
			correl[col][row] = correl[row][col]; /* make symmetric */
	for (row = 0; row < Nvar; row++)
		for (col = 0; col < Nvar; col++)
			{
			invcor[row][col] = correl[row][col]; /* copy */
			if (row != 0 && col != 0)
				partial[row-1][col-1] = correl[row][col];
			}

	if (fzero (invert (partial, Nvar-1)))
		{
		WARNING (These predictors seem to be non-orthogonal)
		ERRMSG0 (Singular partial correlation matrix)
		}

	if (fzero (invert (invcor, Nvar)))
		Rsq = 1.0;
	else if fzero (invcor[REG][REG])
		Rsq = 0.0;
	else
		Rsq = 1.0 - 1.0/invcor[REG][REG];

	for (var = 1; var < Nvar; var++)
		{
		beta[var] = b[var] = 0.0;
		for (col = 1; col < Nvar; col++)
			beta[var] += partial[var-1][col-1] * correl[REG][col];
		}

	a = sum[REG];
	for (var = 1; var < Nvar; var++)
		if (!fzero (covar[var][var]))
			{
			b[var] = beta[var] * sqrt (covar[REG][REG]/covar[var][var]);
			a -= b[var] * sum[var];
			}
	a /= Ncases;
	putchar ('\n');

	printf ("Regression Equation for %s:\n", Varname[REG]);
	printf ("%s  =  ", Varname[REG]);
	for (var = 1; var < Nvar; var++)
		printf ("%.4g %s  +  ", b[var], Varname[var]);
	printf ("%g\n", a);

	if (Equation)
		{
		FILE	*ioptr = fopen ("regress.eqn", "w");
		if (ioptr)
			{
			fprintf (ioptr, "s1\n");
			for (var = 1; var < Nvar; var++)
				fprintf (ioptr, "(x%d * %.20g) + ", var+1, b[var]);
			fprintf (ioptr, "%.20g\n", a);
			(void) fclose (ioptr);
			}
		else
			WARNING (could not open regression output file)
		}

	if (fzero (Rsq - 1.0))
		F = MAXF;
	else
		F = Rsq * dferror / (npred * (1.0 - Rsq));

	SEestsq = covar[REG][REG]*(1.0-Rsq)/dferror;

	putchar ('\n');
	printf ("Significance test for prediction of %s\n", Varname[REG]);
	printf (Sformat, "Mult-R");
	printf (Sformat, "R-Squared");
	printf (Sformat, "SEest");
	(void) sprintf (Fline, "F(%d,%d)", npred, dferror);
	printf (Sformat, Fline);
	printf (Sformat, "prob (F)");
	putchar ('\n');
	printf (Fformat, sqrt (Rsq));
	printf (Fformat, Rsq);
	printf (Fformat, sqrt (SEestsq));
	printf (Fformat, F);
	printf (Fformat, pof (F, npred, dferror));
	putchar ('\n');

	if (Partial)
		{
		putchar ('\n');
		printf ("Significance test(s) for predictor(s) of %s\n", Varname[REG]);
		printf (Lformat, "Predictor");
		printf (Sformat, "beta");
		printf (Sformat, "b");
		printf (Sformat, "Rsq");
		printf (Sformat, "se");
		(void) sprintf (Fline, "t(%d)", dferror);
		printf (Sformat, Fline);
	/*
		(void) sprintf (Fline, "F(1,%d)", dferror);
		printf (Sformat, Fline);
	*/
		printf (Sformat, "p");
		putchar ('\n');
		for (var = 1; var < Nvar; var++)
			{
			double	rsq = 0.0;
			double	SEsq;
			double	Fvar = 0.0;
			double	p;
			double	SSres;

			if (!fzero (partial[var-1][var-1]))
				rsq = 1.0 - 1.0/partial[var-1][var-1];
			else
				rsq = 0.0;
			SSres = covar[var][var] * (1.0 - rsq);
			if (SSres > FZERO && SEestsq > FZERO)
				{
				SEsq = SEestsq/SSres;
				Fvar = b[var]*b[var] / SEsq;
				p = pof (Fvar, 1, dferror);
				}
			else
				{
				SEsq = 0.0;
				Fvar = MAXF;
				p = 0.0;
				}
			printf (Lformat, Varname[var]);
			printf (Fformat, beta[var]);
			printf (Fformat, b[var]);
			printf (Fformat, rsq);
			printf (Fformat, sqrt (SEsq));
			printf (Fformat, sqrt (Fvar));
		/* printf (Fformat, Fvar); */
			printf (Fformat, p);
			putchar ('\n');
			}
		}
	}

/*FUNCTION invert: returns determinant of input matrix. modifies matrix. */
double
invert (matrix, size)
double matrix[MAXVAR][MAXVAR];
int size;
	{
	int	j,k,l;
	double	pivot;
	double	temp;
	double	det = 1.0;

	for (j = 0; j < size; j++)
		{
		pivot = matrix[j][j];
		det *= pivot;
		matrix[j][j] = 1.0;
		for (k = 0; k < size; k++)
			if (!fzero (pivot))
				matrix[j][k] /= pivot;
			else
				return (0.0);
		for (k = 0; k < size; k++)
			if (k != j)
				{
				temp = matrix[k][j];
				matrix[k][j] = 0.0;
				for (l = 0; l < size; l++)
					matrix[k][l] -= matrix[j][l] * temp;
				}
		}
	return (det);
	}

/*FUNCTION usinfo */
usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXVAR, "maximum number of variables");
		const (MAXLEN, "maximum length of variable names");
		const (MAXCHARS, "maximum number of characters in lines");
		const (MAXWIDTH, "maximum width of output fields");
		const (MINWIDTH, "minimum width of output fields");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('c', "print covariance matrix", Covariance);
		lopt ('e', "save regression equation in regress.eqn", Equation);
		iopt ('F', "width", "width of output fields", Fieldwidth);
		lopt ('p', "partial correlation analysis", Partial);
		lopt ('r', "print regression analysis", Doreg);
		lopt ('s', "print sums of squares", SumSquares);
		oper ("[names]", "variable names", "[REG] A B ...");
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}

printmat (mat, label)
double	mat[MAXVAR][MAXVAR];
char	*label;
	{
	int 	row, col;
	printf ("\n%s:\n", label);
	for (row = 0; row < Nvar; row++)
		{
		printf (Lformat, Varname[row]);
		for (col = 0; col <= row; col++)
			printf (Fformat, mat[row][col]);
		putchar ('\n');
		}
	PRINTVEC (Varname[col], Sformat, "Variable")
	}
