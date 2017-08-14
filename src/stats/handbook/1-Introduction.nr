.CH 1 "Introduction"
.de AA
.br
The purpose, environment, and philosophy
of the |STAT programs are introduced.
.br
.rm AA
..
.if n .AA
.SH "Capabilities and Requirements"
.P
|STAT is a small statistical package I have developed on the UNIX
operating system
(Ritchie & Thompson, 1974)
at the University of California San Diego and at the Wang Institute of
Graduate Studies.
Over twenty programs allow the manipulation and
analysis of data and are complemented by this documentation
and manual entries for each program.
The package has been distributed to hundreds of UNIX sites
and the portability of the package, written in C
(Kernighan & Ritchie, 1979),
was demonstrated when it was ported from UNIX to MSDOS
at Cornell University
on an IBM PC using the
Lattice C compiler.
This handbook is designed to be a tutorial introduction
and reference for the most popular parts of release 5.3 of |STAT
(January, 1987) and updates through February, 1987.
Full reference information on the programs is found in the online
manual entries and in the online options help available with most of the
programs.
.P "Dataset Sizes
|STAT programs have mostly been run on small datasets,
the kind obtained in controlled psychological experiments,
not the large sets obtained in surveys or physical experiments.
The programs' performances on datasets with more than about 10,000
points is not known, and the programs should not be used for them.
.P "System Requirements
The programs run on almost any version of UNIX.
They are compatible with UNIX systems dating back to Version 6 UNIX
(circa 1975).
On MSDOS, the programs run on versions 2.X through 3.X.
MSDOS versions earlier than 2.0 may not support the pipes often used
with |STAT programs,
and MSDOS version 4.0 formats are not compatible.
Space requirements for MSDOS are about 1 megabyte of disk space,
and at least 96 kilobytes of main memory.
Hard disk storage is preferred, but not mandatory.
.P "Availability and Distribution
.\" VOLATILE ORDERING INSTRUCTIONS
.de LI
.ta 4n
.if t \(bu	\\$1
.if n *	\\$1
..
.nf
Please take care to follow all the instructions below.
.LI "Please indicate the items that you would like to order.
.LI "Orders must be prepaid.  Purchase orders are not acceptable.
.LI "Make your check/(postal)money order payable to G. Perlman.
.LI "Checks must be in US funds drawn on a US bank.
.LI "Please include a delivery address label to speed service.
	International orders: please indicate your country name.
.nf
.ta .25i +1i
.sp
.\" VOLATILE PRICE INFORMATION
.B "UNIX C Source Version of |STAT: $20/30
   C Language Source Code & Online Manual Entries
      1/2 inch 9 track mag tape, 1600 bpi tar format ($20)
      1/4 inch cartridge tape, tar format ($30)
.B "DOS Executable Version of |STAT: $15
   Preformatted Manuals & Executables (without Source Code) ($15)
      2S/2D (360K) DOS 5.25" floppy diskettes
      HD (1.2M) DOS 5.25" or 3.5" floppies (by special request)
.B "DOS Turbo C Source Code Version for |STAT: $10
   Turbo C Language Source Code, Project Files, Online Manual
      HD (1.2M) DOS 5.25"inch or 3.5" floppy diskette
.B "Handbook (highly recommended for new users): $10
   Examples, Ref. Materials, CALC & DM Manuals, Manual Entries
      Typeset Manual (over 100 8.5 x 11 inch pages)
.I "Prices include cost of media and airmail delivery worldwide.
.fi
.SH "Design Philosophy"
.P
|STAT programs promote a particular style of data analysis.
The package is interactive and programmable.
Data analysis is typically not a single action
but an iterative process in which a goal of understanding
some data is approached.
Many tools are used to provide several analyses of data,
and based on the feedback provided by one analysis,
new analyses are suggested.
.P
The design philosophy of |STAT is easy to summarize.
|STAT consists of several separate programs that can be used
apart or together.
The programs are called and combined at the command level,
and common analyses can be saved in files using UNIX shell scripts
or MSDOS batch files.
.P
Understanding the design philosophy behind |STAT programs
makes it easier to use them.
|STAT programs are designed to be tools,
used with each other, and with standard UNIX and MSDOS tools.
This is possible because the programs make few assumptions
about file formats used by other programs.
Most of the programs read their inputs from the standard input
(what is typed at the keyboard, unless redirected from a file),
and all write to the standard output
(what appears on the screen, unless saved to a file or sent to another program).
The data formats are readable by people,
with fields (columns) on lines separated by white space
(blank spaces or tabs).
Data are line-oriented,
so they can be operated on by many programs.
An example of a filter program on UNIX and MSDOS that can be used with
the |STAT programs is the
.T sort
utility,
which puts lines in numerical or alphabetical order.
The following command sorts the lines in the file
.T input
and saves the result in the file
.T sorted .
.(
sort  <  input  >  sorted
.)
The
.T <
symbol causes
.T sort
to read from
.T input
and the
.T >
causes
.T sort
to write to the file
.T sorted .
Because
.T sort
exists on UNIX and MSDOS,
it is not necessary to duplicate its function in |STAT,
which does not duplicate existing tools.
(In all following examples,
.T "this font will be used
to show text (e.g., commands and program names) that
would be seen by people using the programs.
.P
User efficiency is supported over program efficiency.
That does not mean the programs are slow,
but ease-of-use is not sacrificed to save computer time.
Input formats are simple and readable by people.
There is extensive checking to protect against invalid analyses.
Output formats of analysis programs are designed to be easy to understand.
Data manipulation programs are designed to produce uncluttered output
that is ready for input to other programs.
.P
On UNIX and MSDOS, a
.I filter
is a program that reads from the standard input,
also called
.T stdin
(the keyboard, unless redirected from a file)
and writes to the standard output,
also called
.T stdout
(the screen, unless redirected to a file).
Most |STAT programs are filters.
They are small programs that can be used alone, or with other programs.
|STAT users typically keep their data in a
.I "master data file" .
With data manipulation programs,
extractions from the master data file are transformed
into a format suitable for input to an analysis program.
The original data do not change,
but copies are made for transformations and analysis.
Thus, an analysis consists of an extraction of data,
optional transformations, and some analysis.
Pictorially, this can be shown as:
.(
data | extract | transform | format | analysis | results
.)
where a copy a subset of the data has been extracted,
transformed,
reformatted,
and analyzed by chaining several programs.
Data manipulation functions,
sometimes built into analysis programs in other packages,
are distinct programs in |STAT.
The use of pipelines,
signaled with the pipe symbol,
.T | ,
is the reason for the name |STAT.
.SH "Table of |STAT Programs"
.P
|STAT programs are divided into two categories.
There are programs for data manipulation:
data generation, transformation, formatting, extraction, and validation.
And there are programs for data analysis:
summary statistics, inferential statistics, and data plots.
The data manipulation programs can be used for tasks outside of statistics.
.sp
.de PG
.P "\\$1"
.nf
..
.de Pg
	\f(CW\\$1	\fR\\$2
..
.ta .25i +1i +1i
.nf
.PG "Data Manipulation Programs
.	Pg abut          "join data files beside each other
.	Pg colex         "column extraction/formatting
.	Pg dm            "conditional data extraction/transformation
.	Pg dsort         "multiple key data sorting filter
.	Pg linex         "line extraction
.	Pg maketrix      "create matrix format file from free-format input
.	Pg perm          "permute line order randomly, numerically, alphabetically
.	Pg probdist      "probability distribution functions
.	Pg ranksort      "convert data to ranks
.	Pg repeat        "repeat strings or lines in files
.	Pg reverse       "reverse lines, columns, or characters
.	Pg series        "generate an additive series of numbers
.	Pg transpose     "transpose matrix format input
.	Pg validata      "verify data file consistency
.PG "Data Analysis Programs
.	Pg anova         "multi-factor analysis of variance
.	Pg calc          "interactive algebraic modeling calculator
.	Pg contab        "contingency tables and chi-square
.	Pg desc          "descriptions, histograms, frequency tables
.	Pg dprime        "signal detection d' and beta calculations
.	Pg features      "display features of items
.	Pg oneway        "one-way anova/t-test with error-bar plots
.	Pg pair          "paired data statistics, regression, scatterplots
.	Pg rankind       "rank order analysis for independent conditions
.	Pg rankrel       "rank order analysis for related conditions
.	Pg regress       "multiple linear regression and correlation
.	Pg stats         "simple summary statistics
.	Pg ts            "time series analysis and plots
.fi
.SH "Table of UNIX and MSDOS Utilities"
.P
The UNIX and MSDOS environments are similar,
at least as far as |STAT is concerned,
but many command names differ.
The following table shows the pairing of UNIX names
with their MSDOS equivalents.
.sp
.\"========================================
.nf
.ta .25i +1.25i +1.25i
.if n .ta 2 +15n +15n
.ft B
	UNIX	MSDOS	Purpose
.de UM
	\f(CW\\$1	\\$2	\fR\\$3
..
.UM cat            type       "print files to stdout
.UM cd,pwd         cd         "change/print working directory
.UM cp             copy       "copy files
.UM diff           comp       "compare and list file differences
.UM echo           echo       "print text to standard output
.UM grep           find       "search for pattern in files
.UM ls             dir        "list files in directory
.UM mkdir          mkdir      "create a new directory
.UM more           more       "paginate text on screen
.UM mv             rename     "move/rename files
.UM print          print      "print files on printer
.UM rm             del,erase  "remove/delete files
.UM rmdir          rmdir      "remove an empty directory
.UM sort           sort       "sort lines in files

.UM shell-script   batch-file "programming language
.UM $1,$2          %1,%2      "variables
.UM /dev/tty       con        "terminal keyboard/screen
.UM /dev/null      nul        "empty file, infinite sink
.fi
.\"========================================
.TC
.if t .AA
