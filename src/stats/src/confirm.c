/*  Copyright 1981 Gary Perlman */

/* LINTLIBRARY */
#include "stat.h"
FUN(confirm,get confirmation to a prompt,5.2,12/22/86)

/* get a yes/no 1/0 answer from the user */
confirm (msg, arg)
char	*msg;   /* prompt in printf format */
char	*arg;   /* optional string argument to format string */
	{
	char	line[10];
	FILE	*dttyin = stdin, *dttyout = stderr;
  getconfirm:
	fprintf (dttyout, msg, arg ? arg : "");
	fprintf (dttyout, " (y/n) ");
	fflush (dttyout);
	if (fgets (line, 10, dttyin) == NULL)
		return (0);
	switch (*line)
		{
		case 'Y': case 'y':
			return (1);
		case 'N': case 'n':
			return (0);
		default:
			fprintf (dttyout, "type y for yes, n for no\n");
		}
	goto getconfirm;
	}

/* returns true if it is okay to write on a file that might exist */
canwrite (filename) char *filename;
	{
	if (access (filename, 0)) /* no such file */
		return (1);
	return (confirm ("Overwrite %s?", filename));
	}
