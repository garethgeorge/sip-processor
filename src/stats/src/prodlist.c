/*  Copyright 1986 Gary Perlman */

/*LINTLIBRARY*/

#include <math.h>

#ifdef __STDC__
#include "stdlib.h"
#else
extern	char	*malloc ();
#endif

#ifndef MAXFLOAT
#	ifdef HUGE
#		define	MAXFLOAT HUGE
#	else
#		define MAXFLOAT ((float)1.701411733192644299e+38)
#	endif
#endif

#ifdef	TRACE
#include <stdio.h>
#endif	TRACE

#include "prodlist.h" /* inclusion must follow others */
static void	primefactor ();

#ifndef	NULL
#define	NULL 0
#endif	NULL

#ifdef TRACE
#define	trace(X) printf X ; getchar ()
#else
#define trace(X)
#endif

static char sccsfid[] = "@(#) prodlist.c 5.1 (|stat) 12/22/86";

/*FUNCTION prod_new: create a new product list */
PLIST *
prod_new (n)
int 	n;
	{
	PLIST	*new = (PLIST *) malloc (sizeof (PLIST));
	trace (("Entering prod_new\n"));
	if (new)
		{
		new->n = n;
		new->power = (int  *) malloc (((unsigned) n + 1) * sizeof (int ));
		if (new->power == NULL)
			new = NULL;
		else
			prod_init (new);
		}
	trace (("Leaving prod_new, new = %d\n", (int) new));
	return (new);
	}

/*FUNCTION prod_rel: release a product list */
void
prod_rel (list)
PLIST	*list;
	{
	trace (("Entering prod_rel, list = %d\n", (int) list));
	if (list)
		{
		if (list->power)
			free ((char *) list->power);
		free ((char *) list);
		}
	trace (("Leaving prod_rel, list = %d\n", (int) list));
	}

/*FUNCTION prod_init: set all exponents in a list to 0 */
void
prod_init (list)
PLIST	*list;
	{
	register	int 	i;
	trace (("Entering prod_init, list len = %d\n", prod_n(list)));
	if (list == NULL)
		return;
	for (i = prod_n(list); i >= 2; i--)
		{
		prod_set (list, i, 0);
		}
	trace (("Leaving prod_init, list len = %d\n", prod_n(list)));
	}

/*FUNCTION prod_fact: add an expanded factorial to a list */
void
prod_fact (list, fact, sign)
register PLIST	*list;
register	int 	fact;    /* factorial */
int 	sign;                /* if < 0, then divide, else multiply */
	{
	trace (("Entering prod_fact, list = %d\n", (int) list));
	if (sign < 0)
		while (fact >= 2)
			{
			prod_div (list, fact);
			fact--;
			}
	else
		while (fact >= 2)
			{
			prod_mult (list, fact);
			fact--;
			}
	trace (("Leaving prod_fact, list = %d\n", (int) list));
	}

/*FUNCTION prod_compute: return the computed value of a product list */
double
prod_compute (list)
PLIST 	*list;
	{
	static	double	maxf = 0.0;       /* max float number */
	int 	p;                        /* a specific power exponent */
	register	int 	power;        /* loop through all the powers */
	double	logresult = 0.0;          /* = sum of logs of factors */
	double	log ();                   /* standard log function */
	double	logprime ();              /* returns logs of primes */
	
	trace (("Entering prod_compute, list = %d\n", (int) list));
	if (maxf == 0.0)                  /* set static over/underflow value */
		maxf = log (MAXFLOAT);
	
	/*ALGORITHM
		First we reduce the product list to a product of powers of primes.
		This is done to reduce the number of terms.
		Then we
			add the (product of the exponents times the logs of the primes)
		to obtain the
			log of the (product of the powers of the primes).
		We then compare the logsum to underflow and overflow limits,
			maxf (MAXFLOAT) is intentionally small for portability
		and then return the exponential of the logsum.
	*/
	primefactor (list);
	for (power = prod_n (list); power >= 2; power--)
		if (p = prod_get (list, power))
			logresult += logprime (power) * p;
	trace (("Leaving prod_compute, list = %d\n", (int) list));
	if (logresult <= -maxf)
		return (0.0);
	if (logresult >= maxf)
		return (MAXFLOAT); /* overflow */
	return (exp (logresult));
	}

/*FUNCTION primefactor: remove composite factors of a product list */
/*
	shift the representation of each power count
	to smaller numbers, so that, for example,
	the powers of 6 turn into powers of 2 and 3.
	
	Note that the list is reduced in reverse order, so that if we
	find that an index has factors, and one of them is composite,
	the that composite will be factored later.  We only need to check
	down to the minimum composite number: 4 because 3 and 2
	will not be factorable.
	
	After the procedure is done, the list is a product of powers of primes.
*/
static
void
primefactor (list)
register	PLIST	*list;
	{
	static	int 	divn = 0;            /* number of divisors set */
	static	int 	smalldiv[MAXN+1];    /* smallest divisor of numbers */
	static	int 	largediv[MAXN+1];    /* largediv[i] = i / smalldiv[i] */
	register	int 	i;
	int 	factor1, factor2;
	register	int 	p;
	void	dofactor ();
	
#ifdef	TRACE
	trace (("Entering primefactor, list = %d\n", (int) list));
	printlist ("shift  in: ", list);
#endif
	
	/* factor down to minimum composite number: 4 */
	if (prod_n (list) > divn)
		{
		trace (("Calling dofactor\n"));
		dofactor (smalldiv, largediv, divn+1, prod_n (list));
		trace (("Back from dofactor\n"));
		divn = prod_n (list);
		}
	for (i = prod_n (list); i >= 4; i--)       /* reverse order to retry */
		{
		if ((p = prod_get (list, i)) && (factor1 = smalldiv[i]) > 1)
			{
			factor2 = largediv[i];
			prod_pow (list, factor1, p);
			prod_pow (list, factor2, p);
			prod_set (list, i, 0);
			}
		}
#ifdef	TRACE
	printlist ("shift out: ", list);
	trace (("Leaving primefactor, list = %d\n", (int) list));
#endif
	}

/*FUNCTION printlist: print a product list */
#ifdef	TRACE
printlist (msg, list)
char	*msg;
PLIST	*list;
	{
	int 	i;
	int 	power;
	int 	count = 0;

	printf ("%s", msg);
	for (i = 2; i <= prod_n (list); i++)
		{
		count += abs (prod_get (list, i));
		if (prod_get (list, i))
			printf ("%d|%d ", i, prod_get (list, i));
		}
	printf ("(%d)\n", count);
	}
#endif	TRACE


#ifdef	STANDALONE

#include <stdio.h>
#define	N 1000
main ()
	{
	char	line[BUFSIZ];
	PLIST	*list = prod_new (N+1);
	int 	small[N+1];
	int 	big[N+1];
	int 	i;
	int 	n;

	prod_init (list);
	while (gets (line))
		{
		i = atoi (line+1);
		switch (*line)
			{
			default:
				puts ("[*/!?nr] N   or =");
				break;
			case '*': prod_mult (list, i);        break;
			case '/': prod_div (list, i);         break;
			case '!': prod_fact (list, i, 1);     break;
			case '?': prod_fact (list, i, -1);    break;
			case '=':
				printf ("	%f\n", prod_compute (list));
				break;
			case '0': prod_init (list);           break;
			case 'n': printf ("n = %d\n", n = i); break;
			case 'r':
				printf ("n = %d\n", n);
				printf ("r = %d\n", i);
				prod_init (list);
				prod_fact (list, n, 1);
				prod_fact (list, n-i, -1);
				printf ("permutations = %f\n", prod_compute (list));
				prod_fact (list, i, -1);
				printf ("combinations = %f\n", prod_compute (list));
				prod_pow (list, 2, -n);
				printf ("binomial     = %f\n", prod_compute (list));
				break;
			}
		}
	}

#endif	STANDALONE
