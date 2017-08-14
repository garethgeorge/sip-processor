/*  Copyright 1986 Gary Perlman */

#include <ctype.h>

extern	double	atof ();

#ifndef lint
static char sccs2fid[] = "@(#) skipnumber.c 5.1 (|stat) 10/1/88";
#endif

/*LINTLIBRARY*/

int                 /* returns number of characters to skip */
skipnumber (string, isreal)
char	*string;    /* input string, probably in number format */
int 	isreal;     /* if true, then skip over real part too */
	{
	register	char	*ptr;

	ptr = string;
	while (isspace (*ptr))
		ptr++;
	if (*ptr == '-' || *ptr == '+')
		ptr++;
	while (isdigit (*ptr))
		ptr++;
	if (isreal)
		{
		if (*ptr == '.')
			ptr++;
		while (isdigit (*ptr))
			ptr++;
		if (*ptr == 'E' || *ptr == 'e')
			{
			ptr++;
			if (*ptr == '+' || *ptr == '-')
				ptr++;
			while (isdigit (*ptr))
				ptr++;
			}
		}
	return (ptr - string);
	}

#ifdef sun /* special version of atof to adapt to SUN Microsystems bug */
/*
	The SUN version of atof does not require an 'e' or 'E' for
	optional exponents, so it thinks that a string like "10-1"
	is 0.1.   The following copies the honest number into a
	buffer and insures that atof can't read too much as a number
*/
double
myatof (s)
char	*s;
	{
	char	buf[100];
	int 	i, toskip;
	toskip = skipnumber (s,1);
	for (i = 0; i < toskip; i++)
		buf[i] = s[i];
	buf[toskip] = '\0';
	return (atof (buf));
	}
#endif

#ifdef	SKIPNUMBER

/* $Compile: cc -DSKIPNUMBER -o skipnumber skipnumber.c */
/* $Test: skipnumber -1.2345e-678abc +123.456E+789+987 */

#include <stdio.h>

main (argc, argv) char **argv;
	{
	int 	arg;
	char	*s;

	for (arg = 1; arg < argc; arg++)
		{
		s = argv[arg];
		printf ("%20s %20s %20s\n",
			s, s + skipnumber (s, 1), s + skipnumber (s, 0));
		}
	}

#endif	SKIPNUMBER
