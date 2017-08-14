/* Copyright 1986 Gary Perlman */

/*LINTLIBRARY*/
#include "stat.h"
FUN(chitest,compute and print chi-square test,5.0,12/26/86)

#define	MAXLEV      40     /* at most a 40 x 40 matrix */
#define	SMALLFREQ    5.0   /* any smaller frequency may cause problems */

#define	SFORMAT "%6.6s "    /* format for string labels */
#define	LFORMAT "%-6.6s "   /* format for left justified labels */
#define	CFORMAT "%6d "      /* format for cell counts */
#define	EFORMAT "%6.2f "    /* format for expected frequencies */
#define	PFORMAT "%6.1f "    /* format for percentages */
static	char	*Sformat = SFORMAT;
static	char	*Lformat = LFORMAT;
static	char	*Cformat = CFORMAT;
static	char	*Eformat = EFORMAT;
static	char	*Pformat = PFORMAT;

#define	chiexpect(colfreq,rowfreq,n) (((colfreq) * (rowfreq)) / (double) (n))

static
double
chicell (freq, expect, yates)
int 	freq;     /* individual cell frequency */
double	expect;   /* expected value for cell */
Boole	yates;    /* apply Yates' correction for continuity? */
	{
	double	diff = freq - expect;
	if (yates)
		{
		diff = fabs (diff) - 0.5;
		if (diff < 0.0) /* over-correction */
			diff = 0.0;
		}
	if (fzero (expect))
		return (0.0);
	else
		return (diff * diff / expect);
	}


chitest (matrix, rname, cname, nrows, ncols, yates)
int 	**matrix;   /* vector of rows, each of which is a vector of freqs */
char	**rname;    /* row names */
char	**cname;    /* column names */
int 	nrows;      /* number of rows */
int 	ncols;      /* number of columns */
Boole	yates;      /* should Yates' correction for continuity be used? */
	{
	int 	row, col;
	int 	rtotal[MAXLEV];    /* row totals */
	int 	ctotal[MAXLEV];    /* column totals */
	int 	n = 0;             /* total frequency count */
	int 	df;                /* degrees of freedom */
	double	expect;            /* expected cell value */
	double	chival;            /* compute chi-square value */
	double	p;                 /* probability of chival */
	double	pochisq ();
	int 	smallcount = 0;    /* count of cells with expected values < SMALL */
	
	if (nrows > MAXLEV || ncols > MAXLEV)
		return;
	
	df = (nrows - 1) * (ncols - 1);
	if (df > 1)  /* only use yates' correction when df == 1 */
		yates = FALSE;
	else if (df <= 0)
		return;
	
	for (row = 0; row < nrows; row++)
		rtotal[row] = 0;
	for (col = 0; col < ncols; col++)
		ctotal[col] = 0;
	n = 0;
	for (row = 0; row < nrows; row++)
		{
		for (col = 0; col < ncols; col++)
			{
			rtotal[row] += matrix[row][col];
			ctotal[col] += matrix[row][col];
			n += matrix[row][col];
			}
		}
	
	chival = 0.0;
	for (row = 0; row < nrows; row++)
		for (col = 0; col < ncols; col++)
			{
			expect = chiexpect (ctotal[col], rtotal[row], n);
			if (expect < SMALLFREQ)
				smallcount++;
			chival += chicell (matrix[row][col], expect, yates);
			}
	
	/* print contingency table */
	if (cname != NULL)
		{
		putchar ('\t');
		if (rname != NULL)
			printf (Lformat, "");
		for (col = 0; col < ncols; col++)
			printf (Sformat, cname[col]);
		putchar ('\n');
		}
	for (row = 0; row < nrows; row++)
		{
		putchar ('\t');
		if (rname != NULL)
			printf (Lformat, rname[row]);
		for (col = 0; col < ncols; col++)
			printf (Cformat, matrix[row][col]);
		printf (Cformat, rtotal[row]);
		putchar ('\n');
		}
	putchar ('\t');
	if (rname != NULL)
		printf (Lformat, "");
	for (col = 0; col < ncols; col++)
		printf (Cformat, ctotal[col]);
	printf (Cformat, n);
	putchar ('\n');
	
	if (yates)
		printf ("\tNOTE: Yates' correction for continuity applied\n");
	if (smallcount)
		printf ("\tWARNING: %d of %d cells had expected frequencies less than %g\n",
			smallcount, ncols*nrows, SMALLFREQ);
	p = pochisq (chival, df);
	chiprint (chival, df, p);
	}


#ifdef	CHITEST

int 	row1[] = { 1, 3, 5, 7, 9 };
int 	row2[] = { 20, 21, 22, 23, 29 };
char	*colnames[] = { "c1", "c2", "c3", "c4", "c5" };
char	*rownames[] = { "r1", "r2" };
int 	*matrix[] = { row1, row2 };
main ()
	{
	chitest (matrix, rownames, colnames, 2, 5, TRUE);
	}
#endif	CHITEST
