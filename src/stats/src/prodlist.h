/* Copyright 1986 Gary Perlman */

#ifndef	PRODLIST_H
#define	PRODLIST_H

typedef struct
	{
	int 	*power;     /* power[i] is the exponent of i */
	int  	n;          /* number of terms in power list */
	} PLIST;

#define	MAXN    1000         /* maximum exponent in a product list */

#define	prod_get(list,i)       ((list->power)[i])
#define	prod_set(list,i,p)     ((list->power)[i] = (p))
#define	prod_n(list)           (list->n)
#define	prod_pow(list,i,p)     prod_set (list, (i), prod_get (list, (i)) + (p))
#define	prod_mult(list,i)      prod_pow (list, i, 1)
#define	prod_div(list,i)       prod_pow (list, i, (-1))

#ifdef __STDC__
PLIST	*prod_new ();               /* returns a pointer to a product list */
void	prod_rel(PLIST *list);      /* release space for a product list */
void	prod_init (PLIST *list);    /* set all product list exponents to zero */
void	prod_fact (PLIST *list, int fact, int sign);
                                    /* insert an expanded factorial to list */
double	prod_compute (PLIST *list); /* compute the value of a product list */
#else
PLIST	*prod_new ();
void	prod_rel();
void	prod_init ();
void	prod_fact ();
double	prod_compute ();
#endif

/* binomial.c */
#ifdef __STDC__
double binomial (int N, int p1, int p2, int r);
double pobin (int N, int p1, int p2, int r);
int critbin (int N, int p1, int p2, double *critp);
#else
double binomial ();
double pobin ();
int critbin ();
#endif

/* primes.c */
#ifdef __STDC__
double logprime (int p);
void dofactor (int *small, int *large, int i, int n);
#else
double logprime ();
void dofactor ();
#endif

#endif	PRODLIST_H
