/*  Copyright 1985 Gary Perlman */

#include "stat.h"

/*LINTLIBRARY*/
FUN(ranksort,sort column array into ranks,1.2,04/30/88)
/* ranksort: shell sort a vector into ranks */

#ifdef __STDC__
int ranksort (float *vec, float *svec, int *ivec, int n);
#else
int ranksort ();
#endif

int                  /* returns status */
ranksort (vec, svec, ivec, n)
float	*vec;        /* input vector of values */
float	*svec;       /* sorted input vector, may be ignored, used for space */
int 	*ivec;       /* integer order vector */
int 	n;
	{
	int 	gap, i, j;
	float	temp;         /* temp var for swapping */
	int	 	itemp;        /* temp var for swapping */
	int 	dupcount;     /* number of duplicates */
	int 	sumranks;     /* sum of duplicate ranks */
	double	averank;      /* average rank of duplicates */
	int 	needivec = (ivec == NULL);
	int 	needsvec = (svec == NULL);

	if (needivec)
		ivec = myalloc (int, n);
	if (needsvec)
		svec = myalloc (float, n);
	if (ivec == NULL || svec == NULL)
		return (1);
	for (i = 0; i < n; i++)
		{
		svec[i] = vec[i];
		ivec[i] = i;
		}

	/* shellsort algorithm */
	for (gap = n/2; gap > 0; gap /= 2)
		for (i = gap; i < n; i++)
			for (j = i-gap; j >= 0 && svec[j] > svec[j+gap]; j -= gap)
				{
				temp        = svec[j];
				svec[j]     = svec[j+gap];
				svec[j+gap] = temp;
				itemp       = ivec[j];
				ivec[j]     = ivec[j+gap];
				ivec[j+gap] = itemp;
				}

	/*
		Now svec is the sorted input vec and ivec has the order
			svec[i] = vec[ivec[i]];
		To convert vec to ranks, march through svec, looking for
		duplicates, and assign the ranks back to vec.  Duplicate
		values get the average of their ranks.
	*/
	sumranks = 0;
	dupcount = 0;
	for (i = 0; i < n; i++)
		{
		sumranks += i;
		dupcount++;
		if ((i == n-1) || (svec[i] != svec[i+1]))
			{
			averank = (double) sumranks / (double) dupcount + 1.0;
			for (j = i - dupcount + 1; j <= i; j++)
				vec[ivec[j]] = (float) averank;
			sumranks = 0;
			dupcount = 0;
			}
		}
	if (needivec)
		free ((char *) ivec);
	if (needsvec)
		free ((char *) svec);
	return (0);
	}

#ifdef RANKSORT

PGM(ranksort,Rank Order Data in Columns,5.1,11/28/86)

#define	MAXDATA     100        /* max number of data points */
#define	MAXVAR      100        /* max number of variables */
#define MAXCHARS BUFSIZ        /* maximum number of chars in lines */

Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */
int 	Ndata;
int 	Maxdata = MAXDATA;
int 	Nvars;
Boole	Reverse = FALSE;      /* reverse rank ordering */

float	**readmatrix ();

main (argc, argv) char **argv;
	{
	float	**matrix;
	ARGV0;
	initial (argc, argv);
	checkstdin ();
	if (matrix = readmatrix ())
		{
		rankmatrix (matrix, Nvars, Ndata);
		if (Reverse)
			revmatrix (matrix, Nvars, Ndata);
		printmatrix (matrix, Nvars, Ndata);
		}
	else if (Ndata == 0)
		ERRDATA
	else
		ERRMSG0 (Can not read matrix)
	exit (0);
	}

/*FUNCTION initial: returns local version of optind, index to first operand */
int
initial (argc, argv) char **argv;
	{
	extern	char *optarg;    /* option value accessed through this by getopt */
	extern	int  optind;     /* will be index to first operand */
	int 	opterr = 0;      /* count of number of errors */
	int 	flag;            /* option flag characters read in here */

	while ((flag = getopt (argc, argv, "l:rLOV")) != EOF)
		switch (flag)
			{
			default:
				opterr++;
				break;
			/* put option cases here */
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			case 'l': 
				if (setint (Argv0, flag, optarg, &Maxdata, 0, MAXINT))
					opterr++;
				break;
			case 'r': Reverse = TRUE; break;
			}

	if (optind < argc-1) /* too many args */
		opterr++;

	if (opterr) /* print usage message and bail out */
		USAGE ("[-r] [-l lines]")
	
	ERROPT (optind)

	usinfo ();

	return (optind);
	}

float **
makematrix (nvars, ndata)
unsigned	nvars;
unsigned	ndata;
	{
	float	**matrix;
	int 	var;

	if (matrix = myalloc (float *, nvars))
		for (var = 0; var < nvars; var++)
			{
			matrix[var] = myalloc (float, ndata);
			if (matrix[var] == NULL)
				return (NULL);
			}
	return (matrix);
	}

float **
readmatrix ()
	{
	char	line[MAXCHARS];
	char	*array[MAXVAR];
	float	**matrix = NULL;
	int 	n;

	while (gets (line))
		{
		if (n = parselin (line, array, MAXVAR))
			{
			if (Nvars == 0)
				{
				if (n > MAXVAR)
					ERRMANY (columns, MAXVAR)
				Nvars = n;
				matrix = makematrix (Nvars, Maxdata);
				if (matrix == NULL)
					ERRSPACE (data)
				}
			else if (n != Nvars)
				ERRRAGGED
			if (Ndata == Maxdata)
				ERRMANY (rows of data, Maxdata)
			for (n = 0; n < Nvars; n++)
				if (!number (array[n]))
					ERRNUM(array[n],column value)
				else
					matrix[n][Ndata] = atof (array[n]);
			Ndata++;
			}
		}
	return (matrix);
	}

rankmatrix (matrix, nvars, ndata)
float	**matrix;
int 	nvars;
int 	ndata;
	{
	int 	var;
	int 	*ivec = myalloc (int, ndata);
	float	*svec = myalloc (float, ndata);

	if (ivec != NULL && svec != NULL)
		{
		for (var = 0; var < nvars; var++)
			ranksort (matrix[var], svec, ivec, ndata);
		free ((char *) ivec);
		free ((char *) svec);
		}
	}

printmatrix (matrix, nvars, ndata)
float	**matrix;
int 	nvars;
int 	ndata;
	{
	int 	i, var;

	for (i = 0; i < ndata; i++)
		for (var = 0; var < nvars; var++)
			printf ("%g%c", matrix[var][i], var == nvars - 1 ? '\n' : '\t');
	}

revmatrix (matrix, nvars, ndata)
float	**matrix;
int 	nvars;
int 	ndata;
	{
	int 	i, var;
	double	delta = ndata + 1.0;

	for (i = 0; i < ndata; i++)
		for (var = 0; var < nvars; var++)
			matrix[var][i] = delta - matrix[var][i];
	}

usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXVAR, "maximum number of variables");
		const (MAXDATA, "default maximum number of lines");
		const (MAXCHARS, "maximum number of characters in lines");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('r', "reverse rank ordering", Reverse);
		iopt ('l', "lines", "maximum number of input lines", Maxdata);
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}

#endif RANKSORT
