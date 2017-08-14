/* Copyright 1986 Gary Perlman */

/*LINTLIBRARY*/
#include "stat.h"
FUN(ordstat,compute and print fivenums,5.1,02/04/86)

#ifdef __STDC__
double centile (int perc, float *v, int n);
#else
double centile ();
#endif

static
int
fltcmp (f1, f2)
float *f1, *f2;
	{
	if (*f1 < *f2)
		return (-1);
	if (*f1 == *f2)
		return (0);
	return (1);
	}

#define	ordlabel() printf ("%-9.9s %4s %4s %8s %8s %8s %8s %8s\n", \
	"", "N", "NA", "Min", "25%", "Median", "75%", "Max")
#define	ordprint() printf("%-9.9s %4d %4d %8.2f %8.2f %8.2f %8.2f %8.2f\n", \
	name, n, na, minval, q1, q2, q3, maxval)

double *
ordstat (vec, n, cond, name, na)
float	*vec;      /* data vector */
int 	n;         /* number of elements in vec */
int 	cond;      /* condition number */
char	*name;     /* name of the condition */
int 	na;        /* missing values */
	{
	static	double	fivenum[5];   /* computed values returned in here */
	double	minval;    /* min value in vec */
	double	q1;        /* first quartile (25th percentile) */
	double	q2;        /* 50th percentile */
	double	q3;        /* third quartile (25th percentile) */
	double	maxval;    /* max value in vec */
	float	*tvec;     /* temporary vector for sorting */
	int 	i;         /* loop index */
	 
	if (cond == 0)
		ordlabel ();
	if ((tvec = myalloc (float, n)) == NULL)
		ERRSPACE (data)
	for (i = 0; i < n; i++)
		tvec[i] = vec[i];
	qsort ((char *) tvec, n, sizeof (float), fltcmp);
	minval = tvec[0];
	maxval = tvec[n-1];
	q2 = centile (50, tvec, n);
	q1 = centile (25, tvec, n),
	q3 = centile (75, tvec, n);
	fivenum[0] = minval;
	fivenum[1] = q1;
	fivenum[2] = q2;
	fivenum[3] = q3;
	fivenum[4] = maxval;
	ordprint ();
	free ((char *) tvec);
	return (fivenum);
	}
