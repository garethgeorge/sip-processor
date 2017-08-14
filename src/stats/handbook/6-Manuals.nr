.de MP\"mini para
.if t .sp 6p
.if n .sp
.if \\n(.$ \fB\\$1\fR
..
.CH 6 "Manual Entries
.TC
This chapter contains the alphabetically ordered manual entries
for the programs.
The format follows that used on UNIX systems,
and to be honest,
it takes some getting used to.
One possible source of confusion for users
is the format of examples in the entries.
The examples are chosen to work on UNIX
using my preferred command shell,
.T ksh ,
so some translation is needed for UNIX
.T csh
users,
and for MSDOS users.
See Chapter 3 on conventions used in the entries.
Besides the manual entries,
there is online help with most programs
with the
.T -O
option.
Information about limits,
previously part of the entries,
is now only available with the
.T -L
option.
.MP "Learning About the Programs.
After learning how to use a few programs,
it would be a good idea to skim the manual entries
to see all the programs and their options.
Besides the data manipulation and analysis programs,
there are manual entries for special programs included in the
|STAT distribution.
.T cat
is provided for MSDOS versions that do not have the corresponding
UNIX program.
The MSDOS
.T type
utility does not handle multiple files nor wildcards;
.T cat
does both.
.T ff
is a versatile text formatting filter that allows
control of text filling to any width, right justification,
line spacing, pagination, line numbering, tab expansion,
and so on.
.T fpack
creates a plain text archive of a series of files.
.T fpack
can save space by reducing space wasted by many small files,
and it can save time in file transfers by sending several
files in one package.
.MP "Reading Manual Entries Online.
The
.T manstat
program lets you read the manual entries online,
assuming that they have been installed.
To read the entry on a program, say
.T desc ,
you just type:
.ce
.T "manstat desc
