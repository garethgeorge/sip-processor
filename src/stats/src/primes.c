/*  Copyright 1986 Gary Perlman */

/*LINTLIBRARY*/

#define	NPRIMES    11        /* number of primes used to factor */
#define	MAXPRIME   97        /* maximum prime whose log we have preset */
#define	MAXTABLE 1000        /* maximum number whose log we can store */
#include "primes.h"          /* defines of logs of primes up to MAXPRIME */

static char sccsfid[] = "@(#) primes.c 5.0 (|stat) 12/22/86";

static	int 	Primelist[] =
		{
		           2,    3,    5,    7,   11,   13,   17,
		          19,   23,   29,   31,   37,   41,   43,
				  47,   53,   59,   61,   67,   71,   73,
				  79,   83,   89,   97
		};

/*
	natural logarithms of primes
	zeros will never be used
	later values will be filled in with values from log() function
*/
static	double Logtable[MAXTABLE+1] =
	{
	/*  0- 9 */   0.0, 0.0,   LN_2, LN_3,  0.0, LN_5, 0.0, LN_7,  0.0, 0.0,
	/* 10-19 */   0.0, LN_11, 0.0,  LN_13, 0.0,  0.0, 0.0, LN_17, 0.0, LN_19,
	/* 20-29 */   0.0, 0.0,   0.0,  LN_23, 0.0,  0.0, 0.0, 0.0,   0.0, LN_29,
	/* 30-39 */   0.0, LN_31, 0.0,  0.0,   0.0,  0.0, 0.0, LN_37, 0.0, 0.0,
	/* 40-49 */   0.0, LN_41, 0.0,  LN_43, 0.0,  0.0, 0.0, LN_47, 0.0, 0.0,
	/* 50-59 */   0.0, 0.0,   0.0,  LN_53, 0.0,  0.0, 0.0, 0.0,   0.0, LN_59,
	/* 60-69 */   0.0, LN_61, 0.0,  0.0,   0.0,  0.0, 0.0, LN_67, 0.0, 0.0,
	/* 70-79 */   0.0, LN_71, 0.0,  LN_73, 0.0,  0.0, 0.0, 0.0,   0.0, LN_79,
	/* 80-89 */   0.0, 0.0,   0.0,  LN_83, 0.0,  0.0, 0.0, 0.0,   0.0, LN_89,
	/* 90-99 */   0.0, 0.0,   0.0,  0.0,   0.0,  0.0, 0.0, LN_97, 0.0, 0.0
	};

/*FUNCTION logprime: return precomputed log of prime or call log function */
double
logprime (p)
int 	p;
	{
	double	log ();
	/* assert (p is prime) */
	if (p >= 2 && p <= MAXPRIME)
		return (Logtable[p]);
	if (p <= MAXTABLE)
		{
		if (Logtable[p] == 0.0)
			Logtable[p] = log ((double) p);
		return (Logtable[p]);
		}
	return (log((double)p));
	}

/*FUNCTION dofactor: set arrays so for all i, small[i] * large[i] = i */
void
dofactor (small, large, i, n)
int 	*small;     /* array of smaller divisors of numbers */
int 	*large;     /* array of larger factors of numbers */
register
int 	i;          /* start with this value */
int 	n;          /* end with this value */
	{
	register	int  	p;
	for (; i <= n; i++)
		{
		small[i] = 1;
		large[i] = i;
		for (p = 0; p < NPRIMES && Primelist[p] < i; p++)
			{
			if (i % Primelist[p] == 0)
				{
				small[i] = Primelist[p];
				large[i] = i / Primelist[p];
				break;
				}
			}
		}
#ifdef	GENDIV
		gendiv ("small", small, n);
		gendiv ("large", small, n);
#endif	GENDIV
	}


#ifdef	CHECK_ROUNDING_ERRORS
#ifdef __STDC__
#	define	pnum(x) printf ("%-20.20s %25.20lf\n", #x, x)
#else
#	define	pnum(x) printf ("%-20.20s %25.20lf\n", "x", x)
#endif

main ()
	{
	extern	double	exp (), log ();
	int 	i;
	double	d;
	for (i = 2; i <= MAXPRIME; i++)
		if (Logtable[i] != 0.0)
			{
			d = i;
			putchar ('\n');
			printf ("%d\n", i);
			pnum(Logtable[i]);
			pnum(log(d));
			pnum(exp(Logtable[i]));
			pnum(exp(log(d)));
			}
	}
#endif	CHECK_ROUNDING_ERRORS

#ifdef	GENDIV

gendiv (name, array, n)
char	*name;
int 	*array;
	{
	int 	i;
	printf ("static	%s = {", name);
	for (i = 0; i < n; i++)
		{
		if (i % 10 == 0)
			printf ("\n\t");
		printf ("%3d, ", array[i]);
		}
	printf ("%3d\n\t};\n", array[n]);
	}

#endif	GENDIV
