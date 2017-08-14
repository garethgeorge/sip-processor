/*  Copyright 1986 Gary Perlman */

/*LINTLIBRARY*/

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "prodlist.h"

#ifdef macintosh
#include <stdlib.h>
#endif

static char sccsfid[] = "@(#) binomial.c 5.0 (|stat) 12/22/86";

#define	BADPROB    (-1.0)       /* an illegal probability return value */

#ifdef	TRACE
#define	printbin(msg, N, p1, p2, r, prob) \
	printf ("%-10.10s %3d %d/%d  %3d %8.4f   %12.10f\n", \
		msg, N, p1, p2, r, prob, prob);
#define	trace(X) printf X ; getchar ()
#else
#define	printbin(msg, N, p1, p2, r, prob)
#define trace(X)
#endif	TRACE

#ifndef	max
#	define	min(a,b) ((a) < (b) ? (a) : (b))
#	define	max(a,b) ((a) > (b) ? (a) : (b))
#endif	max

/*FUNCTION binomial: compute individual term of binomial probability */
double
binomial (N, p1, p2, r)
int 	N;         /* distribution size */
int 	p1, p2;    /* theta = p1/p2 */
int 	r;         /* number of successes */
	{
	static	PLIST	*list = NULL;
	static	int  	maxpow = 0;     /* max (N, p2) (note: p2 > p1) */

	trace (("Entering binomial\n"));
	printbin ("\t", N, p1, p2, r, -1.0);
	maxpow = max (maxpow, N);
	maxpow = max (maxpow, p2);
	if (list == NULL || maxpow > prod_n (list))
		{
		if (list)
			prod_rel (list);
		if ((list = prod_new (maxpow)) == NULL)
			return (BADPROB);
		}

	/* multiply by N! / r!(N-r)! */
	prod_init (list);
	prod_fact (list, N, 1);
	prod_fact (list, r, -1);
	prod_fact (list, N-r, -1);
	
	/* multiply by p1/p2 ** r */
	prod_pow (list, p1, r);
	prod_pow (list, p2, -r);

	/* multiply by  (1 - p1/p2) ** (N-r) */
	/* this equals ((p2-p1)/p2) ** (N-r) */
	prod_pow (list, p2-p1, N-r);
	prod_pow (list, p2, -(N-r));

	trace (("Leaving binomial\n"));
	return (prod_compute (list));
	}

/*FUNCTION pobin: compute cumulative term of binomial probability */
double
pobin (N, p1, p2, r)
int 	N;         /* distribution size */
int 	p1, p2;    /* p = p1/p2 */
int 	r;         /* number of successes */
	{
	double	cum = 0.0;    /* cumulative probability */
	double	prob;         /* individual term */
	int 	i;
	int 	maxprob;

	trace (("Entering pobin\n"));
	printbin ("\t", N, p1, p2, r, -1.0);
	maxprob = (N * p1) / p2 + 1;
	/* there is a way to exit early when p == 0.0
	   and there is no way that p will increase.  We can make use of the
	   maximum expected value of p:
			maxprob (N, p1/p2) == N * p1/p2;
		this could be adjusted so that the check was against some EPSILON
	*/
	for (i = r; i <= N; i++)
		{
		prob = binomial (N, p1, p2, i);
		if (prob == BADPROB)
			return (BADPROB);
		printbin ("  binomial", N, p1, p2, i, prob);
		if (prob == 0.0 && i > maxprob) /* prob can only go down from here */
			break;
		cum += prob;
		}
	trace (("Leaving pobin\n"));
	return (cum);
	}

/*FUNCTION apobin: approximate cumulative term of binomial probability */
double
apobin (N, p1, p2, r)
int 	N;         /* distribution size */
int 	p1, p2;    /* p = p1/p2 */
int 	r;         /* number of successes */
	{
	double	poz ();                         /* probability of normal z */
	double	p = (double) p1 / (double) p2;  /* probability of success */
	double	mean = N * p;                   /* expected binomial value */
	double	sd = sqrt (mean * (1.0 - p));   /* sd of binomial values */
	double	diff = r - mean;                /* obtained deviation */
	double	z;                              /* z value for diff */
	double	prob;                           /* computed probability */
	
	/* correction for continuity */
	if (diff < 0.0)
		{
		diff += 0.5;
		if (diff > 0.0) /* over-correction */
			diff = 0.0;
		}
	else if (diff > 0.0)
		{
		diff -= 0.5;
		if (diff < 0.0) /* over-correction */
			diff = 0.0;
		}
	
	z = diff / sd;
	prob = 1.0 - poz (z);
	
	return (prob);
	}

/*FUNCTION critbin: compute critical r for binomial probability */
int
critbin (N, p1, p2, critp)
int 	N;         /* distribution size */
int 	p1, p2;    /* p = p1/p2 */
double	*critp;    /* critcal probability (actual p returned in here) */
/*
	Note: for some N/p, it is not possible to get an obtained r
	that is less probable than the value desired.
*/
	{
	double	cum = 0.0;     /* cumulative probability */
	double	prob;          /* individual probability term */
	int 	r;             /* number of successes */
	
	for (r = N; r >= 0; r--)
		{
		prob = binomial (N, p1, p2, r);
		if (cum + prob > *critp)
			{
			*critp = cum;
			return (r+1);
			}
		cum += prob;
		}
	return (N+1); /* this is a flag, not a possible value of r */
	}

/*FUNCTION acritbin: approximate critical r for binomial probability */
int
acritbin (N, p1, p2, critp)
int 	N;         /* distribution size */
int 	p1, p2;    /* p = p1/p2 */
double	critp;     /* critical probability */
/*
	Note: for some N/p, it is not possible to get an obtained r
	that is less probable than the value desired.
*/
	{
	double	p = (double) p1 / (double) p2;   /* probability of success */
	double	mean = N * p;
	double	sd = sqrt (mean * (1.0 - p));
	double	critz ();
	double	z = critz (1.0 - critp);
	int 	r = (int) (z * sd + mean + 0.5);
	return (r);
	}


#ifdef	BINOMIAL

main (argc, argv) char **argv;
	{
	int 	N = atoi (argv[1]);
	int 	p1;
	int 	p2;
	double	p;
	int 	r = atoi (argv[3]);
	int 	i;
	double	prob;
	double	cum;

	if (argc != 4)
		{
		fprintf (stderr, "Usage: %s N p1/p2 r\n", argv[0]);
		exit (1);
		}
	if ((p = getratio (argv[2], & p1, &p2)) == BADPROB)
		{
		fprintf (stderr, "%s: invalid ratio: %s\n", argv[0], argv[2]);
		exit (1);
		}
	if (r > N)
		{
		fprintf (stderr, "%s: r (%d) must be less than N (%d)\n",
			argv[0], r, N);
		exit (1);
		}
#define	pp(str,val) printf ("%-20.20s %7.4f\n", str, val)
#define	pi(str,val) printf ("%-20.20s %7d\n", str, val)
	pp ("exact point prob", binomial (N, p1, p2, r));
	pp ("exact cum prob", pobin (N, p1, p2, r));
	pp ("approx cum prob", apobin (N, p1, p2, r));
	pi ("exact crit .05", critbin (N, p1, p2, .05));
	pi ("approx crit .05", acritbin (N, p1, p2, .05));
	pi ("exact crit .01", critbin (N, p1, p2, .01));
	pi ("approx crit .01", acritbin (N, p1, p2, .01));
	exit (0);
	}

#endif	BINOMIAL
