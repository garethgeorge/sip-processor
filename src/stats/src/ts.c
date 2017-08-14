/*  Copyright 1981 Gary Perlman */

#include "stat.h"
PGM(ts,Time Series Analysis and Plots,5.2,01/20/87)

#ifdef __STDC__
double cor (float *x, float *y, int n);
double pof (double F, int df1, int df2);
#else
double cor ();
double pof ();
#endif

typedef struct
	{
	unsigned int	size;
	float	*data;
	} TS;

#ifdef __STDC__
TS *new_ts (int size);
TS *make_ts (TS *tss, int n);
void read_ts (TS *tss);
void print_ts (TS *tss);
void printstats (float *vector, int n);
void cor_ts (TS *tss);
void free_ts (TS *tss);
#else
TS *new_ts ();
TS *make_ts ();
void read_ts ();
void print_ts ();
void printstats ();
void cor_ts ();
void free_ts ();
#endif

#define	WIDTH 70           /* default plot width */

/* OPTIONS */
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */
double	Base = 1.0;        /* base value of the plot interval */
double	Interval = 1.0;    /* plot interval size for label */
int 	Axes = 1;          /* type of axes */
int 	Echo;              /* print out (possibly transformed) ts */
int 	Width = WIDTH;     /* width of plot */
int 	Length;            /* rescaled length of ts */
int 	Plot;              /* request a plot */
int 	Plotstyle;         /* specify plot style */
int 	Stats;             /* request summary statistics */
int 	Onumber;           /* number output */
int 	Autocor;           /* maximum lag for autocorrelations */

int 	NAcount = 0;       /* count of NA missing values */

#define MAXTSIZE   1000       /* max size for raw time series */
#define MAXCHARS     50       /* maximum number of chars in words */

#define	check_ts(ts) if (! (ts || ts->size) ) return;

TS *
new_ts (size)
int 	size;
	{
	TS	*tss;
	if ((tss = myalloc (TS, 1)) == NULL)
		return (NULL);
	tss->data = myalloc (float, size);
	if (tss->data == NULL)
		return (NULL);
	tss->size = 0;
	return (tss);
	}

TS *
make_ts (tss, n)
TS  	*tss;
int 	n;
	{
	TS  	*newts;
	int 	i;
	int 	tsn = tss->size;

	if (tsn == 0)
		return (tss);

	if (newts = new_ts (n))
		{
		newts->size = n;
		for (i = 0; i < n; i++)
			newts->data[i] = 0.0;
		for (i = 0; i < n*tsn; i++)
			newts->data[i/tsn] += tss->data[i/n];
		for (i = 0; i < n; i++)
			newts->data[i] /= tsn;
		}
	free_ts (tss);
	return (newts);
	}

void
read_ts (tss)
TS	*tss;
	{
	char	s[MAXCHARS];
	check_ts (tss);
	while (getword (s, stdin))
		{
		if (isna (s))
			{
			NAcount++;
			continue;
			}
		if (!number (s))
			ERRNUM (s,input value)
		if (tss->size >= MAXTSIZE)
			ERRSPACE (time series)
		tss->data[tss->size++] = atof (s);
		}
	}

void
print_ts (tss)
TS	*tss;
	{
	int 	i;
	check_ts (tss);
	for (i = 0; i < tss->size; i++)
		{
		if (Onumber)
			{
			if ((i >= 0) && ((i%Onumber) == 0))
				printf ("%8g	", i*Interval + Base);
			else
				printf ("%8s	", "");
			}
		printf ("%.5g\n", tss->data[i]);
		}
	}

#ifdef __STDC__
#define	tabprint(label,format,value) \
	printf ("%s	= %" #format "\n", #label, (value))
#else
#define	tabprint(label,format,value) \
	printf ("%s	= %format\n", "label", (value))
#endif

void
printstats (vector, n)
float	*vector;
int 	n;
	{
	double	sum = 0.0;
	double	ss  = 0.0;
	double	minx = vector[0];
	double	maxx = vector[0];
	int 	count = n;
	while (count-- > 0)
		{
		sum += vector[0];
		ss  += vector[0] * vector[0];
		if (vector[0] > maxx)
			maxx = vector[0];
		else if (vector[0] < minx)
			minx = vector[0];
		vector++;
		}
	tabprint (n,d,n);
	tabprint (NA,d,NAcount);
	tabprint (sum,g,sum);
	tabprint (ss,g,ss);
	tabprint (min,g,minx);
	tabprint (max,g,maxx);
	tabprint (range,g,maxx-minx);
	tabprint (midpt,g,(maxx+minx)/2.0);
	if (n)
		{
		tabprint (mean,g,sum/n);
		if (n > 1)
			tabprint (sd,g,n <= 1 ? 0.0 : sqrt ((ss-sum*sum/n)/(n-1)));
		}
	}

void
cor_ts (tss)
TS	*tss;
	{
	int 	lag;
	double	r, r2;
	double	Fr;
	double	p;
	int 	df;

	if (tss->size <= 1)
		{
		WARNING (No autocorrelation analysis for so few data)
		return;
		}
	printf ("%8s %6s %6s %5s %12s %5s %6s\n",
		"Lag", "r", "r^2", "n'", "F", "df", "p");
	for (lag = 0; lag <= Autocor; lag++)
	    {
	    df = tss->size - lag - 2;
		if (lag == 0)
			r = cor (NULL, tss->data, tss->size);
		else
			r = cor (tss->data, tss->data+lag, tss->size-lag);
	    r2 = r*r;
	    if (r2 == 1.0)
			{
			Fr = 0.0;
			p = 0.0;
			}
	    else
			{
			Fr = (r2 * df) / (1.0 - r2);
			p = pof (Fr, 1, df);
			}
	    printf ("%8d %6.3f %6.3f %5d %12.3f %5d %6.3f\n",
		         lag,    r,   r2, tss->size-lag, Fr, df, p);
	    }
	}

initial (argc, argv) char **argv;
	{
	extern	char	*optarg;
	extern	int 	optind;
	int 	errflg = 0;
	int 	C;
#define	OPTSTRING "ei:l:b:pP:sn:ac:w:LOV"
	while ((C = getopt (argc, argv, OPTSTRING)) != EOF)
		switch (C)
			{
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			case 'a': Plot = 1; Axes = !Axes; break;
			case 'b':
				if (setreal (Argv0, C, optarg, &Base))
					errflg++;
				break;
			case 'c':
				if (setint (Argv0, C, optarg, &Autocor, 1, 1000))
					errflg++;
				break;
			case 'e': Echo = 1; break;
			case 'i':
				if (setreal (Argv0, C, optarg, &Interval))
					errflg++;
				if (Interval <= FZERO)
					ERRVAL (g,Interval,interval size)
				Plot = 1;
				break;
			case 'l':
				if (setint (Argv0, C, optarg, &Length, 1, 1000))
					errflg++;
				break;
			case 'n':
				if (setint (Argv0, C, optarg, &Onumber, 1, 1000))
					errflg++;
				break;
			case 'P':
				if (setint (Argv0, C, optarg, &Plotstyle, 0, 2))
					errflg++;
				/* FALLTHROUGH */
			case 'p': Plot = 1; break;
			case 's': Stats = 1; break;
			case 'w':
				if (setint (Argv0, C, optarg, &Width, 1, 1000))
					errflg++;
				Plot = 1;
				break;
			default: errflg++; break;
			}
	if (errflg)
		USAGE ("[-aesp] [-b base] [-c autocor] [-i interval] [-l length]\n\t[-n number] [-P style] [-w width]")
	if (!Plot && !Autocor && !Echo) Stats = 1;
	ERROPT (optind)
	usinfo ();
	}

main (argc, argv) char**argv;
	{
	TS	*tss = new_ts (MAXTSIZE);
	ARGV0;
	initial (argc, argv);
	checkstdin ();
	read_ts (tss);
	if (Length)
		tss = make_ts (tss, Length);
	if (Echo)
		print_ts (tss);
	if (Stats)
		printstats (tss->data, tss->size);
	if (Autocor)
		cor_ts (tss);
	if (Plot)
		barplot (tss->data, tss->size, Plotstyle, Axes, Onumber, Width, Base, Interval);
	free_ts (tss);
	exit (0);
	}

void
free_ts (tss)
TS	*tss;
	{
	if (tss)
		{
		if (tss->size)
			free ((char *) tss->data);
		free ((char *) tss);
		}
	}

usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXTSIZE, "maximum size of input time series");
		const (MAXCHARS, "maximum number of characters in input fields");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		lopt ('a', "draw axes around plots", Axes);
		ropt ('b', "base", "base number of time series", Base);
		iopt ('c', "lag", "autocorrelations of lags 1:lag", Autocor);
		lopt ('e', "echo time series", Echo);
		ropt ('i', "width", "set plot interval width", Interval);
		iopt ('l', "length", "set series length", Length);
		iopt ('n', "number", "number output", Onumber);
		lopt ('p', "request plot", Plot);
		iopt ('P', "type", "specify plot style", Plotstyle);
		lopt ('s', "print summary statistics", Stats);
		iopt ('w', "width", "plot width", Width);
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
