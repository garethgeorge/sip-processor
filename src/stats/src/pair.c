/*  Copyright 1980 Gary Perlman */

#include "stat.h"
PGM(pair,Paired Data Analysis and Plots,5.2,01/20/87)

/*NEW
	put estimated X-Y values in axes so that
	a line can be drawn through by hand
*/

#ifdef __STDC__
double standev (double sum, double ss, int count);
#else
double standev ();
#endif

#define	prettyprint(title, x, y, d) \
	printf ("%-16s %16.4f %16.4f %16.4f\n", title, x, y, d)

#define	MAXPAIRS        2000     /* at most this many pairs */
#define MAXCHARS BUFSIZ           /* maximum number of chars in lines */

float	Xdata[MAXPAIRS];         /* data stored for plot in here */
float	Ydata[MAXPAIRS];         /* data stored for plot in here */
char	Plotchar;                /* plotting character */
Boole	Frameplot = TRUE;        /* should plot be framed? */
int 	Height = 19;             /* default height of plot */
int 	Width = 50;              /* default width of plot */ 
double	Top;                     /* top of plot or Max_y */
Boole	SetTop = FALSE;
double	Bottom;                  /* bottom of plot or Min_y */
Boole	SetBottom = FALSE;
double	Left;                    /* left of plot or Min_x */
Boole	SetLeft = FALSE;
double	Right;                   /* right of plot or Max_x */
Boole	SetRight = FALSE;
int 	Perline = 2;             /* number of points per line */
int 	Count = 0;               /* number of points counted in */
int 	NAcount = 0;             /* number of NA missing pairs */
double	Sum_x,   Sum_y,   Sum_d; /* sums of scores */
double	SS_x,    SS_y,    SS_d;  /* sums of squares */
double	Mean_x,  Mean_y,  Mean_d;/* means stored here */
double	SD_x,    SD_y,    SD_d;  /* standard deviations */
double	Sum_xy;                  /* cross product */
double	Min_x, Min_y, Min_d;     /* minimums */
double	Max_x, Max_y, Max_d;     /* maximums */
double	Intercept, Slope;        /* intercept and slope */
double	Correlation;             /* correlation coefficient */
char	*Name_x = "Column 1", *Name_y = "Column 2";
int 	Storedata = 1;           /* assume data will be stored */
int 	Wantstats = 0;           /* are statistics wanted? */
int 	Wantplot = 0;            /* is a plot wanted? */
Boole	InfoVersion;          /* print version information */
Boole	InfoLimits;           /* print program limits */
Boole	InfoOptions;          /* print usage information */


main (argc, argv) char **argv;
	{
	ARGV0;
	initial (argc, argv);
	readdata ();
	compstats ();
	if (Wantstats)
		printstats ();
	if (Wantplot)
		scatterplot (Xdata, Ydata, Count);
	exit (0);
	}


initial (argc, argv) char **argv;
	{
	extern	int 	optind;
	extern	char 	*optarg;
	int 	C;
	int 	opterr = 0;

	while ((C = getopt (argc, argv, "b:c:fh:l:pr:st:w:x:y:LOV")) != EOF)
		switch (C)
			{
			case 'O': InfoOptions = TRUE; break;
			case 'V': InfoVersion = TRUE; break;
			case 'L': InfoLimits = TRUE; break;
			case 'b':
				SetBottom = TRUE;
				Wantplot = TRUE;
				if (setreal (Argv0, 'b', optarg, &Bottom))
					opterr++;
				break;
			case 'c':
				Wantplot = TRUE;
				Plotchar = *optarg;
				break;
			case 'f':
				Frameplot = FALSE;
				Wantplot = TRUE;
				break;
			case 'h':
				if (setint (Argv0, 'h', optarg, &Height, MIN_PLOT, MAX_HEIGHT))
					opterr++;
				Wantplot = 1;
				break;
			case 'l':
				SetLeft = TRUE;
				Wantplot = TRUE;
				if (setreal (Argv0, 'l', optarg, &Left))
					opterr++;
				break;
			case 'p':
				Wantplot = TRUE;
				break;
			case 'r':
				SetRight = TRUE;
				if (setreal (Argv0, 'r', optarg, &Right))
					opterr++;
				break;
			case 's':
				Wantstats = 1;
				break;
			case 't':
				SetTop = TRUE;
				Wantplot = TRUE;
				if (setreal (Argv0, 't', optarg, &Top))
					opterr++;
				break;
			case 'w':
				if (setint (Argv0, 'w', optarg, &Width, MIN_PLOT, MAX_WIDTH))
					opterr++;
				Wantplot = 1;
				break;
			case 'x':
				Name_x = optarg;
				break;
			case 'y':
				Name_y = optarg;
				break;
			default: opterr++;
			}
	if (opterr)
		USAGE ("[-fps] [-h height] [-w width] [-c char] [-x xname] [-y yname]\n\t[-t top] [-b bottom] [-l left] [-r right]")
	ERROPT (optind)
	if (!Wantplot)
		{
		Storedata = 0;
		Wantstats = 1;
		}
	usinfo ();
	checkstdin ();
	}


readdata ()
	{
	char	line[MAXCHARS];
	char	*array[2];
	int 	fieldcount;
	double	x, y, d;
	int 	lineno = 0;

	while (fgets (line, sizeof (line), stdin))
		{
		lineno++;
		fieldcount = parselin (line, array, 2);
		if (fieldcount == 0)
			continue;
		if (isna (array[0]) || (fieldcount > 1 && isna (array[1])))
			{
			NAcount++;
			continue;
			}
		if (Count == 0)
			{
			switch (Perline = fieldcount)
				{
				case 1: Perline = 1;
					Min_x = 1.0;
					Min_y = atof (array[0]);
					break;
				case 2: Perline = 2;
					Min_x = atof (array[0]);
					Min_y = atof (array[1]);
					break;
				default:
					ERRMANY (columns, 2)
				}
			Max_x = Min_x;
			Max_y = Min_y;
			Min_d = Max_d = Min_x - Min_y;
			}
		else if (fieldcount != Perline)
			ERRMSG1 (Must have 1 or 2 numbers per line (see line %d), lineno)
		if (!number (array[0]) || ((Perline == 2) && !number (array[1])))
			ERRMSG1 (Non-numerical input at line %d, lineno)
		if (Perline == 2)
			{
			x	= atof (array[0]);
			y	= atof (array[1]);
			}
		else
			{
			x	= (double) (Count+1);
			y	= atof (array[0]);
			}

		d	= x - y;
		Sum_d	+= d;
		SS_d	+= d*d;
		Sum_x	+= x;
		SS_x	+= x*x;
		Sum_xy	+= x*y;
		Sum_y	+= y;
		SS_y	+= y*y;

		if (x > Max_x)
			Max_x = x;
		else if (x < Min_x)
			Min_x = x;

		if (y > Max_y)
			Max_y = y;
		else if (y < Min_y)
			Min_y = y;

		if (d > Max_d)
			Max_d = d;
		else if (d < Min_d)
			Min_d = d;

		if (Storedata)
			{
			if (Count < MAXPAIRS)
				{
				Xdata[Count] = x;
				Ydata[Count] = y;
				}
			else
				{
				WARNING (too much data for a plot)
				Storedata = 0;
				Wantstats = 1;
				}
			}
		Count++;
		}
	}


compstats ()
	{
	if (Count <= 1)
		ERRDATA
	Mean_x = Sum_x / Count;
	Mean_y = Sum_y / Count;
	Mean_d = Sum_d / Count;
	SD_x = standev (Sum_x, SS_x, Count);
	SD_y = standev (Sum_y, SS_y, Count);
	SD_d = standev (Sum_d, SS_d, Count);
	if (SD_x > FZERO)
		{
		Slope = (Sum_xy - Sum_x*Sum_y/Count) / (SS_x - Sum_x*Mean_x);
		Intercept = Mean_y - Slope * Mean_x;
		if (!fzero (SD_y))
			Correlation = Slope * SD_x / SD_y;
		else
			Correlation = 1.0;
		}
	else
		Intercept = Slope = Correlation = 0.0;
	}


printstats ()
	{
	double	t_x, t_y, t_d;                  /* t test values */
	double	p_x, p_y, p_d, pof ();          /* probability levels */
	char	tmpstr[20];                     /* for internal formatting */
	double	t_r, p_r;                       /* correlation, t val & prob */
	double	fcount = (double) Count;        /* double Count */
	 
	printf ("Analysis for %d points:\n", Count);
	if (NAcount)
		printf ("Missing points ignored: %d\n", NAcount);
	printf ("%16s %16.16s %16.16s %16s\n", "", Name_x, Name_y, "Difference");
	prettyprint ("Minimums",     Min_x,     Min_y,     Min_d);
	prettyprint ("Maximums",     Max_x,     Max_y,     Max_d);
	prettyprint ("Sums",         Sum_x,     Sum_y,     Sum_d);
	prettyprint ("SumSquares",   SS_x,      SS_y,      SS_d);
	prettyprint ("Means",        Mean_x,    Mean_y,    Mean_d);
	prettyprint ("SDs",          SD_x,      SD_y,      SD_d);
	if (SD_x > FZERO)
		{
		t_x = Mean_x*sqrt(fcount)/SD_x;
		p_x = pof (t_x*t_x, 1, Count-1);
		}
	else
		{
		t_x = MAXT;
		p_x = 0.0;
		}
	if (SD_y > FZERO)
		{
		t_y = Mean_y*sqrt(fcount)/SD_y;
		p_y = pof (t_y*t_y, 1, Count-1);
		}
	else
		{
		t_y = MAXT;
		p_y = 0.0;
		}
	if (SD_d > FZERO)
		{
		t_d = Mean_d*sqrt(fcount)/SD_d;
		p_d = pof (t_d*t_d, 1, Count-1);
		}
	else
		{
		t_d = MAXT;
		p_d = 0.0;
		}
	sprintf (tmpstr, "t(%d)", Count-1);
	prettyprint (tmpstr, t_x, t_y, t_d);
	prettyprint ("p", p_x, p_y, p_d);
	if (Count > 2 && Correlation != 1.0 && Correlation != -1.0)
		{
		t_r = Correlation / sqrt ((1.0 - Correlation*Correlation) / (fcount - 2.0));
		p_r = pof (t_r*t_r, 1, Count-2);
		}
	else
		{
		t_r = MAXT;
		p_r = 0.0;
		}
	putchar ('\n');
	sprintf (tmpstr, "t(%d)", Count-2);
	printf ("%16s %16s %16s %16s\n", "Correlation", "r-squared", tmpstr, "p");
	printf ("%16.4f %16.4f %16.4f %16.4f\n",
		Correlation, Correlation*Correlation, t_r, p_r);
	printf ("%16s %16s\n", "Intercept", "Slope");
	printf ("%16.4f %16.4f\n", Intercept, Slope);
	}

double
standev (sum, ss, count) double sum, ss;
	{
	if (count <= 1)
		return (0.0);
	return (sqrt ((ss-sum*sum/count)/(count-1)));
	}


#define	EPSILON (.000000001)
#define scale(x,min,max,width) ((int) (width*((x)-(min))/((max)-(min)+EPSILON)))
#define	MAXCOUNT    127     /* max plot frequency */

unsigned char	Plot[MAX_HEIGHT][MAX_WIDTH];
scatterplot (x, y, n)
float	*x, *y;
	{
	int 	i;
	int 	col, row;
	int 	height2 = Height/2;
	int 	ix, iy;
	double	xval, yval;
	float 	minx = SetLeft ? Left : Min_x;
	float	maxx = SetRight ? Right : Max_x;
	float	miny = SetBottom ? Bottom : Min_y;
	float	maxy = SetTop ? Top : Max_y;

	for (row = 0; row < Height; row++)
		for (col = 0; col < Width; col++)
			Plot[row][col] = 0;

	for (i = 0; i < n; i++)
		{
		xval = x[i];
		yval = y[i];
		if (yval < miny || yval > maxy || xval < minx || xval > maxx)
			{
			fprintf (stderr, "%s: point %d (%g,%g) out of range\n",
				Argv0, i+1, xval, yval);
#ifdef	DEBUG
			printf ("maxy    %20.10f\n", maxy);
			printf ("yval    %20.10f\n", yval);
			printf ("miny    %20.10f\n", miny);
			printf ("minx    %20.10f\n", minx);
			printf ("xval    %20.10f\n", xval);
			printf ("maxx    %20.10f\n", maxx);
#endif	DEBUG
			}
		else
			{
			iy = scale (yval, miny, maxy, Height);
			ix = scale (xval, minx, maxx, Width);
			if (Plot[iy][ix] < MAXCOUNT)
				Plot[iy][ix]++;
			}
		}

	if (Frameplot)
		{
		putchar ('|');
		for (col = 0; col < Width; col++)
			putchar ('-');
		putchar ('|');
		printf ("%g", maxy);
		putchar ('\n');
		}
	for (row = Height-1; row >= 0; row--)
		{
		if (Frameplot)
			putchar ('|');
		for (col = 0; col < Width; col++)
			if (Plot[row][col])
				{
				if (Plotchar)
					putchar (Plotchar);
				else if (Plot[row][col] >= 10)
					putchar ('*');
				else
					putchar (Plot[row][col]+'0');
				}
			else putchar (' ');
		if (Frameplot)
			putchar ('|');
		if (row == height2)
			printf ("%s", Name_y);
		putchar ('\n');
		}
	if (Frameplot)
		{
		putchar ('|');
		for (col = 0; col < Width; col++)
			putchar ('-');
		putchar ('|');
		printf ("%g", miny);
		putchar ('\n');
		numline (minx, maxx, Width+2); /* width + frame sides */
		}
#define	CORRWIDTH   8     /* length of " r = %x.yf for Correlation */
	for (i = (Width - strlen (Name_x) - CORRWIDTH) / 2; i > 0; i--)
		putchar (' ');
	printf ("%s  r=%6.3f\n", Name_x, Correlation);
	}


usinfo ()
	{
	if (InfoVersion)
		pver (Version);
	if (InfoLimits)
		{
		plim (Argv0);
		const (MAXPAIRS, "maximum number of pairs for plots");
		const (MAX_WIDTH, "maximum width of plot");
		const (MAX_HEIGHT, "maximum height of plot");
		const (MIN_PLOT, "minimum plot height or width");
		const (MAXCHARS, "maximum number of characters in lines");
		}
	if (InfoOptions)
		{
		ppgm (Argv0, Purpose);
		ropt ('b', "bottom",   "minimum y axis plot value", Bottom);
		copt ('c', "char",     "plotting character", Plotchar);
		lopt ('f', "frame plot", Frameplot);
		iopt ('h', "height",   "height of plot", Height);
		ropt ('l', "left",     "minimum x axis plot value", Left);
		lopt ('p', "print plot", Wantplot);
		ropt ('r', "right",    "maximum x axis plot value", Right);
		lopt ('s', "print statistics", Wantstats);
		ropt ('t', "top",      "maximum y axis plot value", Top);
		iopt ('w', "width", "width of plot", Width);
		sopt ('x', "name",     "x axis name", Name_x);
		sopt ('y', "name",     "y axis name", Name_y);
		}
	if (InfoVersion || InfoLimits || InfoOptions)
		exit (0);
	}
