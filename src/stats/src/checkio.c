/*  Copyright 1981 Gary Perlman */

/* LINTLIBRARY */
#include "stat.h"
FUN(checkio,check input,5.0,1985)

/*
	This file contains some basic io routines.
	  isatty:      true if the argument file descriptor is a tty
*/

#ifndef __MSDOS__ /* isatty */

#include <sgtty.h>
isatty (fd)
	{
	struct	sgttyb ttybuf;
	return (gtty (fd, &ttybuf) == 0);
	}

#else /* MSDOS version of isatty */
/*
ISATTY is an approximation to the UNIX isatty function.  ISATTY queries
the operating system through a function call to 44hex to determine if
file in question is directed toward console input or output.  If yes,
ISATTY returns a 1.  Else, ISATTY returns a 0.
	Fred Horan @ cornell
*/
#include "dos.h"

isatty(fd)
int 	fd;   /* file descriptor */
{
	union REGS iREG, oREG;
	struct SREGS	exmem;
	iREG.x.ax = 0x4400;             /* pass function 44hex in ah */
	iREG.x.bx = fd;                 /* pass file handle in bx */
	iREG.x.cx = iREG.x.dx = 0;      /* zero out remaining registers */
	segread(&exmem);                /* assume large model... */
	intdosx(&iREG, &oREG, &exmem);  /* make the operating system call */

/* x81 = check for ISDEV and console input; x82 ISDEV, console output */
	if ((oREG.x.dx & 0x81) == 0x81) return(1);
	return(0);
}
#endif
