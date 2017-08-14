/*  Copyright 1981 Gary Perlman */

#include "stat.h"
PGM(perm,Permute Line Order,5.6,8/22/93)

#define MAXCHARS BUFSIZ           /* maximum number of chars in lines */
unsigned	Maxlines = 1000;

int 	Seed;        /* random seed that might be set on command line */
Boole	Alphasort;   /* sort lines alphabetically */
Boole	Intsort;     /* sort lines as integers */
Boole	Expsort;     /* sort lines as real numbers */
Boole	Numsort;     /* sort as xxx.yyy numbers */
Boole	Sort;        /* true if any sorting is to be done */
Boole	Juxtaposed = TRUE;  /* allow adjacent input values to be juxtaposed */
Boole	Reverse;     /* multiplying factor to reverse the order of any sort */
Boole	Caseignore;  /* ignore case when making alpha comparisons */
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */

int 	jpossible   Cdecl ((char **array, int n, int * choice, int i));
int 	jgetunused  Cdecl ((char **array, int n, int i));
int 	jchoose     Cdecl ((char **array, int n, int *choice, int i));
Status	jstrpermut  Cdecl ((char **array, int n));
void	jprint      Cdecl ((char *name, char **array, int n));
void	strpermut   Cdecl ((char **array, int size));
int 	initial     Cdecl ((int argc, char **argv));
void	usinfo      Cdecl ((void));


#define sortwith(x) qsort ((char *) lptr, nlines, sizeof (*lptr), x)
#define	compare(name,fun,arg1,arg2) \
	int name (sp1, sp2) char **sp1, **sp2; { return (fun (*arg1, *arg2)); }

compare (alcmp,  strcmp, sp1, sp2)
compare (ralcmp, strcmp, sp2, sp1)

compare (cialcmp,  cistrcmp, sp1, sp2) /* case insensitive alpha compare */
compare (ciralcmp, cistrcmp, sp2, sp1) /* case insensitive reverse alpha */

compare (ncmp,  numcmp, sp1, sp2)
compare (rncmp, numcmp, sp2, sp1)

#define	diffint(a,b) (atoi(a) - atoi(b))
compare (intcmp,  diffint, sp1, sp2)
compare (rintcmp, diffint, sp2, sp1)

#define diffloat(a,b) ((atof (a) - atof (b)) < 0 ? -1 : 1)
compare (fltcmp,  diffloat, sp1, sp2)
compare (rfltcmp, diffloat, sp2, sp1)

main (argc, argv) char **argv;
	{
	char	**lptr;
	int 	nlines;
	int 	lineno;
#ifdef NEVER
	int 	fltcmp (), alcmp (), intcmp ();
	int 	rfltcmp (), ralcmp (), rintcmp ();
#endif

	ARGV0;
	initial (argc, argv);
	checkstdin ();
	nlines = readlines (&lptr, Maxlines, stdin);
	if (nlines == 0)
		exit (0);
	if (nlines > Maxlines)
		ERRMANY (lines, Maxlines)
	if (nlines < 0)
		ERRSPACE (lines)
	if (Sort)
		{
		if (Alphasort)
			{
			if (Caseignore)
				if (Reverse) sortwith (ciralcmp);
				else         sortwith (cialcmp);
			else
				if (Reverse) sortwith (ralcmp);
				else         sortwith (alcmp);
			}
		else if (Intsort)
			if (Reverse) sortwith (rintcmp);
			else         sortwith (intcmp);
		else if (Numsort)
			if (Reverse) sortwith (rncmp);
			else         sortwith (ncmp);
		else if (Expsort)
			if (Reverse) sortwith (rfltcmp);
			else         sortwith (fltcmp);
		}
    else if (Juxtaposed)
		strpermut (lptr, nlines);
	else if (nlines > 0 && nlines > 4)
		{
		if (jstrpermut (lptr, nlines) == FAILURE)
			ERRSPACE (non-juxtaposed permutation)
		}
	else
		ERRMSG0 (need at least 5 lines to get a non-juxtaposed permutation)
	for (lineno = 0; lineno < nlines; lineno++)
		puts (lptr[lineno]);
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

	while ((flag = getopt (argc, argv, "aceijl:nrs:LOV")) != EOF)
		switch (flag)
			{
			default:
				opterr++;
				break;
			/* put option cases here */
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			case 'c':
				Caseignore = TRUE;
				/* FALLTHROUGH */
			case 'a':
				Alphasort = TRUE;
				Numsort = Intsort = Expsort = FALSE;
				break;
			case 'e':
				Expsort = TRUE;
				Numsort = Alphasort = Intsort = FALSE;
				break;
			case 'i':
				Intsort = TRUE;
				Numsort = Alphasort = Expsort = FALSE;
				break;
			case 'j':
				Juxtaposed = FALSE;
				break;
			case 'l':
				if (setint (Argv0, 'l', optarg, &Maxlines, 1, MAXINT))
					opterr++;
				break;
			case 'n':
				Numsort = TRUE;
				Intsort = Alphasort = Expsort = FALSE;
				break;
			case 'r': Reverse = TRUE; break;
			case 's':
				if (setint (argv[0], flag, optarg, &Seed, 1, MAXINT))
					opterr++;
				break;
			}

	if (opterr) /* print usage message and bail out */
		USAGE ("[-a | -c | -e | -i | -n | -j] [-r] [-l lines] [-s seed]")

	Sort = Alphasort || Intsort || Numsort || Expsort;
	if (Reverse && !Sort)
		WARNING (reversal option has no effect without sorting option)
	if (Sort && Seed != 0)
		WARNING (setting random seed has no effect when sorting)
	if (Sort && !Juxtaposed)
		WARNING (not allowing juxtaposition ignored when sorting)

	usinfo ();

	ERROPT (optind)

	return (optind);
	}


void
strpermut (array, size)
char	*array[];
int 	size;
	{
	int 	i, irand;
	char	*temp;
	extern	double Maxrand;
	int 	randval;
 
	initrand (Seed);
	for (i = 0; i < size; i++)
		{
		while ((randval = rand ()) < 0);
		irand = (int) (randval / Maxrand * (size-i)) + i;
		temp = array[i];
		array[i] = array[irand];
		array[irand] = temp;
		}
	}

/* macro to determine if two items are adjacent in a wrap-around array */
#define	adj(index,lastchoice,n) \
	((index==0 && lastchoice==n-1) || \
	(index==n-1 && lastchoice==0) || \
	index==lastchoice-1 || index==lastchoice+1)

/*FUN jchoose: choose an item from array that is not adjecent to previous */
static
int
jchoose (array, n, choice, i)
char	**array;
int 	n;
int 	*choice;
int 	i;
	{
	int 	index;
	extern	double	Maxrand;
	for (;;)
		{
		index = rand();
		index = (int) ((index / Maxrand) * n);
		if (i == 0)
			return (index);
		if (array[index] != NULL && !adj (index, choice[i-1], n))
			return (index);
		}
	}

/*FUN jgetunused: gets the next unused element >= i */
static
int
jgetunused (array, n, i)
char	**array;
int 	n;
int 	i;
	{
	while (i < n)
		if (array[i] != NULL)
			return (i);
		else
			i++;
	/* should not get here! */
	return (i);
	}

/*FUN jpossible: returns whether it is possible to complete non-adjacent perm */
/*	There are some conditions in the permutation that cannot be allowed.
	If there are 2 or 3 elements to fill, and they are all adjacent,
	then there is no way to choose subsequent items that will not be adjacent,
	so we must backrack one step and hope for better luck */
static
int
jpossible (array, n, choice, i)
char	**array;    /* the items left to choose */
int 	n;          /* the number of items to choose */
int 	*choice;    /* the previous choices */
int 	i;          /* the item we are choosing */
	{
	int 	nleft = n - i;
	int 	index[3];
	if (nleft >= 4 || nleft == 1)
		return (1);
	index[0] = jgetunused (array, n, 0);
	if (nleft == 2) /* if the other one left is adjacent, then impossible */
		{
		index[1] = jgetunused (array, n, index[0]+1);
		return (!adj (index[0], index[1], n));
		}
	else if (nleft == 3) /* if all three are adjacent, then impossible */
		{
		index[1] = jgetunused (array, n, index[0]+1);
		index[2] = jgetunused (array, n, index[1]+1);
		/* figuring adjacency is trick because of wrap-around */
		if (adj (index[0], index[1], n) && adj (index[1], index[2], n))
			return (0); /* all in a row */
		if (adj (index[0], index[1], n) && adj (index[2], index[0], n))
			return (0); /* two in a row at start and one in last position */
		if (adj (index[1], index[2], n) && adj (index[2], index[0], n))
			return (0); /* two in a row at end, and one in first position */
		return (1);
		}
	return (1);
	}

static
void
jprint (name, array, n)
char	*name;
char	**array;
int 	n;
	{
	int 	i;
	printf ("%-6s: ", name);
	for (i = 0; i < n; i++)
		printf ("%-6s ", array[i]);
	printf ("\n");
	}

Status
jstrpermut (array, n)
char **array;
int 	n;
	{
	int 	*choice;
	int 	i;
	int 	index;
	char	**temp;
	extern	double Maxrand;
	if (n <=4)
		return (FAILURE); /* no non-adjacent permutations */
	if ((temp = (char **) malloc ((unsigned) n * sizeof (*array))) == NULL)
		return (FAILURE); /* no space */
	if ((choice = (int *) malloc ((unsigned) n * sizeof (*choice))) == NULL)
		return (FAILURE); /* no space */
	for (i = 0; i < n; i++)
		temp[i] = array[i];
	/* choose a new array[i], making sure that no adjacent one is allowed */
	initrand (Seed);
	for (i = 0; i < n; i++)
		{
		/*	as elements of temp[i] are assigned back to array[i],
			we set temp[i] to NULL to indicate it is used. */
		index = jchoose (temp, n, choice, i);
		/* printf ("array[%d] = temp[%d] = %s\n", i, index, temp[index]); */
		array[i] = temp[index];
		temp[index] = NULL;
		choice[i] = index;
		/* if this choice would put us in an impossible state, backtrack */
		/* jprint ("temp", temp, n); */
		/* jprint ("array", array, n); */
		if (!jpossible (temp, n, choice, i+1))
			{
			temp[index] = array[i];  /* put back the choice */
			i--;                     /* and try again */
			}
		}
	free ((char *) temp);
	free ((char *) choice);
	return (SUCCESS);
	}


void
usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXCHARS, "maximum number of characters in lines");
		const (Maxlines, "maximum number of input lines");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('a', "order lines alphabetically", Alphasort);
		lopt ('c', "case insensitive alphabetic ordering", Caseignore);
		lopt ('e', "order lines as xxx.yyyEzzz numbers", Expsort);
		lopt ('i', "order lines as integers", Intsort);
		lopt ('j', "adjacent values can be juxtaposed", Juxtaposed);
		iopt ('l', "lines", "maximum number of input lines", Maxlines);
		lopt ('n', "order lines as xxx.yyy numbers", Numsort);
		lopt ('r', "reverse sorting order of lines", Reverse);
		iopt ('s', "seed", "integer random number generator seed", Seed);
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
