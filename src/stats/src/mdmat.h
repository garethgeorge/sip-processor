/* Copyright 1985 Gary Perlman */
/*
	mdmat: multi-dimensional matrix utilities (see mdmat.c)
	This header file contains definitions and declarations
	used to communicate with the utilities.
	This version unfortunately communicates with global data.
	The constants R_DATA and I_DATA are incompatible because
	each is used to activate code specific to those applications.
	The chisq program uses integer data, while the anova program
	uses real data.  The code in mdmat will evolve into something
	more portable.
*/

#ifndef MDMAT_H
#define	MDMAT_H

#ifndef MAXFACT
#define MAXFACT      6            /* the maximum number of factors */
#endif
#ifndef	MAXLEV
#define	MAXLEV      500           /* maximum number of levels of a factor */
#endif	MAXLEV
extern	Posint	Maxlev;

#ifdef	R_DATA
#define	ZERO	0.0
#define	DATUM	float
#define	FORMAT	"%g"
#define	CONV	atof
#endif	R_DATA

#ifdef	I_DATA
#define	ZERO	0
#define	DATUM	unsigned short
#define	FORMAT	"%d"
#define	CONV	atoi
#endif	I_DATA

extern	Posint	Nfactors;         /* total number of factors */
extern	char	**Factname;       /* names of factors */
extern	Posint	*Nlevels;         /* number of levels of each factor */
extern	char	***Levelname;     /* level names */
extern	DATUM	*Datax;           /* will hold all the data */
extern	short	*Nreplics;        /* number of replications in each cell */

#define	member(factor,source)     (((1 << (factor)) & (source)) != 0)
#define	join(a,b)                 ((a) | (b))
#define	subset(a,b)               (((a)&(b)) == (a))
#define	enter(factor,source)      ((source) |= (1 << (factor)))

Posint	mdread ();                /* read in multidimensional matrix */
Boole	mdnext ();                /* assign next value in md array */
Posint	mdaddr ();                /* compute address in md array */
Posint	printsource ();           /* print the factor names of a source */

#endif	MDMAT_H
