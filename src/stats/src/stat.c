/*  Copyright 1980 Gary Perlman */

#include "stat.h"
PGM(name,purpose,version,date)

main (argc, argv) char **argv;
	{
	int 	optind;
	ARGV0;
	optind = initial (argc, argv);
	checkstdin ();
	if (optind == argc) /* no operands */
		{
		}
	else while (optind < argc)
		{
		}
	exit (0);
	}

/* OPTIONS */

/* returns local version of optind, index to first operand */
int
initial (argc, argv) char **argv;
	{
	extern	char *optarg;    /* option value accessed through this by getopt */
	extern	int  optind;     /* will be index to first operand */
	int 	opterr = 0;      /* count of number of errors */
	int 	flag;            /* option flag characters read in here */
	char	*optstring =     /* getopt string */
		"";
	char	*usage =         /* variable part of usage summary */
		"";

	while ((flag = getopt (argc, argv, optstring)) != EOF)
		switch (flag) /* put option cases here */
			{
			default: opterr++; break;
			}

	if (opterr)
		{
		fprintf (stderr, "Usage: %s %s\n", argv[0], usage);
		exit (1);
		}
	return (optind);
	}
