/*  Copyright 1985 Gary Perlman */

#include "stat.h"
#include "prodlist.h"

#ifdef __STDC__
	static double compute (int A, int B, int C, int D, int N);
#else
	static double compute ();      /* utility to compute one probability */
#endif

/*LINTLIBRARY*/
FUN(fisher,Compute Fisher 2x2 Table Exact Test,5.4,01/07/87)

#ifndef	BADPROB
#define	BADPROB (-1.0)
#endif	BADPROB

#ifdef	DEBUG
extern	int 	Debug;
#endif	DEBUG
/*
	Compute probability of 2x2 contingency table:
		A	B
		C	D
	using Fisher's exact test:
		p = (A+B)!(A+C)!(B+D)!(C+D)!
			------------------------
				   N!A!B!C!D!
	This is the one-tailed probability of a single configuration.
	The Fisher exact test includes the obtained case, and all cases
	more deviant than the one obtained.

*/

#ifdef	FISHER2X2

main (argc, argv)
char	**argv;
	{
	double	fisher2x2 ();
	int 	A = atoi (argv[1]);
	int 	B = atoi (argv[2]);
	int 	C = atoi (argv[3]);
	int 	D = atoi (argv[4]);

	printf ("%3d  %3d | %3d\n", A, B, A+B);
	printf ("%3d  %3d | %3d\n", C, D, C+D);
	printf ("---------+----\n");
	printf ("%3d  %3d | %3d\n", A+C, B+D, A+B+C+D);
	printf ("One tailed probability = %g\n", fisher2x2 (A, B, C, D));
	}

#endif	FISHER2X2

/*FUNCTION fisher2x2:	compute cumulative probability of a 2x2 table */
/*
	return one-tailed probability of ABCD arrangement of counts,
	and for all more deviant arrangements.  Note that while Siegel
	(1956) states that the two-tailed probability is twice the one-tailed,
	Bradley (1968) has another solution.

	If N is too large, then the bogus probability -1 is returned.
*/
double
fisher2x2 (A, B, C, D)
int 	A, B,
		C, D;  /* values as arranged in the table */
	{
	int 	N =  A+B+C+D;
	int 	mincount;        /* minimum frequency in table */
	double	p = 0.0;         /* will be cumulative probability */
	int 	delta_A;         /* will be amount to add to A each cycle */

	if (N > MAXN || N <= 0)
		return (BADPROB);

	/* find smallest frequency */
	mincount = min (A, B);
	mincount = min (mincount, C);
	mincount = min (mincount, D);

	if (A == mincount || D == mincount)
		delta_A = (-1);
	else
		delta_A = (1);
	
#ifdef	DEBUG
	if (Debug)
		printf ("mincount = %d\n", mincount);
#endif	DEBUG

	/* sum every probability for min cell freq mincount..0 */
	while (mincount-- >= 0)
		{
#ifdef	DEBUG
		if (Debug)
			printf ("%g\n", compute (A, B, C, D, N));
#endif	DEBUG
		p += compute (A, B, C, D, N);

		/* adjust to new mincount, maintaining constant marginals */
		A += delta_A;
		D += delta_A;
		B -= delta_A;
		C -= delta_A;
		}

	return (p);
	}

/*FUNCTION compute:	compute probability of a particular configuration */
static
double
compute (A, B, C, D, N)
int 	A, B,
		C, D;  /* values as arranged in the table */
int 	N;     /* A+B+C+D */
	{
	static	PLIST	*list = NULL;
	static	int  	maxpow = 0;     /* max (N, maxpow) */

	maxpow = max (maxpow, N);
	if (list == NULL || maxpow > prod_n (list))
		{
		if (list)
			prod_rel (list);
		if ((list = prod_new (maxpow)) == NULL)
			return (BADPROB);
		}

	prod_n (list) = N;    /* kludge, but it may speed things up a lot */
	prod_init (list);                    /* initialize */

	prod_fact (list, A+B, 1);               /* (A+B)! */
	prod_fact (list, A+C, 1);               /* (A+C)! */
	prod_fact (list, B+D, 1);               /* (B+D)! */
	prod_fact (list, C+D, 1);               /* (C+D)! */

	prod_fact (list, N, -1);               /* N! */
	prod_fact (list, A, -1);               /* A! */
	prod_fact (list, B, -1);               /* B! */
	prod_fact (list, C, -1);               /* C! */
	prod_fact (list, D, -1);               /* D! */

	return (prod_compute (list));
	}

/*FUNCTION fishtail:	compute alternate value of A in two tailed test */
/*
	Given A, B, C, and D, there is a value for A that represents
	a configuration of cell frequencies roughly as deviant, but
	in the opposite direction.

	Bradley's formula for the new A value is:
		2 (A+B) (A+C) - NA
		------------------
				N
	which is simply twice the (expected cell frequency of A) - A
*/
int
fishtail (A, B, C, D)
int 	A, B,
		C, D;
	{
	int 	Aprime;
	int 	N = A+B+C+D;
	double 	expA = ((A+B) * (A+C)) / ((double) N);
	Aprime = 2.0 * expA - A;
	return (Aprime);
	}

fishtest (A, B, C, D)
int 	A, B, C, D;
	{
	double	fisher2x2 ();
	double	p1;             /* probability of main tail */
	double	p2;             /* probability of second tail */
	double	p;              /* p1 + p2 */
	int 	delta = A - fishtail (A, B, C, D);
	p1 = fisher2x2 (A, B, C, D);
	if (p1 >= 0.0) /* check for overflow */
		{
		tprint ("Fisher Exact One-Tailed Probability", p1);
#ifdef	DEBUG
		if (Debug)
			{
			printf ("delta = %d\n", delta);
			printf ("%d	%d	%d	%d\n", A-delta, B+delta, C+delta, D-delta);
			}
#endif	DEBUG
		p2 = fisher2x2 (A - delta, B + delta, C + delta, D - delta);
		if (p2 >= 0.0)
			{
			tprint ("Fisher Exact Other-Tail Probability", p2);
			p = p1 + p2;
			if (p > 1.0) /* tails overlap */
				p = 1.0;
			tprint ("Fisher Exact Two-Tailed Probability", p);
			}
		}
	}
