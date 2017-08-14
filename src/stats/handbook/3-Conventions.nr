.CH 3 "Conventions"
.de AA
.br
Features common to all the |STAT programs are covered.
This information makes it easier to learn about new |STAT programs,
and serves as a reference for experienced users.
.br
.rm AA
..
.if n .AA
.SH "Command Line Interpreters"
.P
|STAT analyses consist of a series of commands,
each on a single line,
hence the name
.I "command line" .
Commands are typed by users into a command line interpreter,
itself a program that runs the commands typed in.
On MSDOS, there is no special name given to the command line interpreter.
On UNIX, the command line interpreters are called
.I shells ,
and there are several of them.
Users are expected to know the conventions of their command line interpreters.
Some of the examples in this handbook and in the manual entries
will not work because of differences in how command lines are formatted.
Minor modifications to the examples are sometimes needed.
.P
Some command line interpreters support
in-line editing, which is useful when running |STAT analyses
because data analysis is an iterative process in which minor
changes in analyses, and hence commands, are common.
.P "Special Characters
Command line interpreters have special characters
to perform special tasks.
On both MSDOS and UNIX,
there are special characters for file input, output, and pipe redirection:
.(
<	\fRredirect standard input from the following file\fP
>	\fRredirect standard output to the following file\fP
|	\fRredirect standard output to the following command\fP
.)
UNIX and MSDOS both have patterns (sometimes called ``wildcards'')
to match file names.
For example,
.T *.c
matches all files that end with a
.T c
suffix.
Also, the
.T ?
can be used in patterns to match any one character.
An important difference between UNIX and MSDOS command line interpreters
is that on UNIX,
the pattern matching is part of the shell,
and so is available to every program,
while on MSDOS, it is part of only some programs.
.P
It is sometimes necessary to quote the special meaning of special characters
so that they are not seen by the command line interpreter.
For example,
an expression for
.T dm
might contain the symbols
.T *
for multiplication or
.T <
for comparison.
Both these characters are special to UNIX shells,
while only
.T <
is special to MSDOS.
The blank space and tab characters are special on both UNIX and MSDOS,
and are used to separate command line arguments.
Special characters can be quoted by enclosing command line
arguments in double quotes.
For example,
.T dm
expressions may contain special characters, and strings may contain spaces.
.(
dm  "if x1 > 10 then 'Large number on line:' else SKIP"  INLINE
.)
.SH "Command Formats"
.P
|STAT programs are run on UNIX and MSDOS by typing the name of the program,
program options, and program operands (e.g., expressions or file names).
Program names, options, and operands, are separated or
.I delimited
by blank space.
On UNIX, program names are lower case,
while on the case-insensitive MSDOS, they are always upper case,
although users can type the names in lower case.
Program options and operands can be complex,
so it is sometimes useful to insert spaces into an option value
or an operand, either to modify the output or to make the
command line more readable.
This is done by quoting (with double quotes)
the parts that should be kept together.
.P "Simple Commands
A simple command consists of a program name, program options
delimited with minus signs, and program operands,
such as file or variable names.
Here are some examples:
.(
dm  x1+x2  x3/x4
calc  model
regress  -p  age  height  weight
desc  -h  -i 1  -m 0  -cfp
series  1  100  .5
probdist  random  normal  100
.)
.P "Pipelines of Commands
A pipeline of commands is a series of simple commands
joined by the pipe symbol,
.T | .
In a pipeline, the output from one simple command is the input
to the next command in the pipeline.
The following pipeline creates a series of numbers from 1 to 100,
transforms it by using the
.T dm
logarithm function,
and then makes a histogram of the result.
.(
series 1 100  |  dm logx1  |  desc -h
.)
The following pipeline abuts three files beside one another,
and passes the result to the
.T regress
program,
which prints their correlation matrix.
.(
abut age height weight  |  regress -r age height weight
.)
Note that the operands to
.T abut
are file names,
while those for
.T regress
are variable names,
which could be different if desired.
If they were always supposed to be the same,
then this constraint could be encoded in a shell script or batch file.
.P "Batch Files and Shell Scripts
Because the |STAT programs work well together,
and because most data analysis is routine,
it is often advantageous to save a series of commands
in a file for later analyses.
Both UNIX and MSDOS support this,
MSDOS with
.I batch
files and UNIX with
.I shell
scripts.
Batch files and shell scripts also support variables,
some set by command line calls and some set inside the command file.
They provide |STAT with a simple but effective programming facility.
.SH "Program Options"
.P
Program options allow the user to control how a program works
by requesting custom or extra analysis.
Without options,
|STAT programs provide the simplest or most common behavior by default.
Program options conform to the standard UNIX option parsing convention
(Hemenway & Armitage, 1984)
by using the
.T getopt
option parser.
In this standard,
all program options are single characters preceded by a minus sign.
For example,
.T -a
and
.T -X
are both options.
All program options must precede operands
(such as file names, variable names, or expressions).
Some options require values,
and these should follow the option.
For example, the
.T pair
plotting function allows setting the height
of the plot with the
.T -h
option:
.T "-h\ 30"
would set the plot height to 30 lines.
There should be a space between an option and its value.
Options that do not take values (logical options)
can be grouped or ``bundled'' to save typing.
For example, the descriptive statistics program,
.T desc ,
has options for requesting a histogram, a table of frequencies,
and a table of proportions.
These can be requested with the bundle of options:
.T -hfp
instead of the longer:
.T "-h\ -f\ -p" .
.P
There are some special conventions used with the
.T getopt
option parser.
A double dash,
.T -- ,
by itself signals the end of the options,
which can be useful when the first operand begins with
.T -
and it would be misinterpreted as an option.
For programs that take files as operands
(e.g.,
.T abut ,
.T calc ),
a solitary 
.T -
means to read from the standard input,
which can be useful to insert the output of a pipeline
in a set of files.
For example,
the
.T abut
program can read several files
with the standard input inserted with the following command line.
.(
series 1 20  |  abut file1 file2 - file3
.)
The output would be four columns,
the third of which would be the series 1 to 20.
.P
The same options can usually be specified more than once
on a command line.
For logical options (those that turn on or off a feature),
repetition usually has no effect.
For options that take values, such as the width of a plot,
respecifying an option resets it to a new value.
Exceptions to these rules for specific options are mentioned
in program manual entries.
.(
.if t .ft B
Table of Option Rules
.if t .ft P
.ta 1i
-x	\fRoptions are single letters preceded by minus\fP
-h 30	\fRoption values must follow the option after a space\fP
-nve	\fRlogical options can be bundled\fP
--	\fRsignals the end of the options\fP
-	\fRinsert standard input to operands of file-reading program\fP
.)
.P "Standard Options
All |STAT programs using the standard option parser,
.T getopt ,
have standard options to get information online.
The information reported by the program is always accurate,
while the printed documentation may not be up to date,
or may not apply to the particular version
(e.g., limits on MSDOS may be smaller than on UNIX).
.(
-L	\fRprints a list of program limits\fP
-O	\fRprints a summary of program options\fP
-V	\fRprints version information\fP
.)
.SH "File Inputs and Outputs"
.P
Most of the |STAT programs are filters.
That means they read from the standard input and write to the standard output.
By default, the standard input is the keyboard,
and the standard output is the screen.
The standard input and output can independently be ``redirected''
using the special characters:
.T < ,
to redirect the standard input from an immediately following file name,
.T > ,
to redirect the standard output to a file.
Also, the pipe character
.T | ,
can connect the output from one program to the input to another.
(Some of these features are not available on early versions of MSDOS
(before version 2.0).)
The following command says for the
.T anova
program to read from the file
.T anova.in .
.(
anova  <  anova.in
.)
The output would go to the screen, by default.
The following command saves the above output to the file
.T anova.out .
.(
anova  <  anova.in  >  anova.out
.)
Never do this:
.(
anova  <  data  >  data          # Never Do This!
.)
Never make the input file the same as the output file,
or you will lose the file;
the output file is created (and zeroed)
by the command line interpreter
before the input file is read.
Temporary files should be used instead.
Here is an example
of output redirection to save 50 random normal numbers.
.(
probdist random normal 50  >  numbers
.)
In English, this is read:
``A random sample of 50 numbers is created and saved in the file
.T numbers .
This file of numbers could be used as input to the descriptive statistics
program,
.T desc .
The intermediate file,
.T numbers ,
could be avoided by using a pipeline.
.(
probdist random normal 50  |  desc
.)
To save the result of the above analysis in a file called
.T results ,
output redirection would be used.
.(
probdist random normal 50  |  desc  >  results
.)
.P
Although pipes are supported on MSDOS,
they are not efficient
and they require that there is enough space
for temporary files to hold the contents of the pipes
(temporary files with names like
.T PIPE%1.$$$ ).
This can make input and output redirection without pipes a better
choice for speed, especially in command scripts,
called ``batch files'' on MSDOS.
.P "Keyboard Input
If a program is expecting input from the keyboard
(ie. the standard input has not been redirected from a file or pipe),
a prompt will be printed on the screen.
Often, input from the keyboard is a mistake;
most people do not type directly
into an analysis program but prepare a file with their preferred
editor and use that file as input.
.(
prompt: desc
desc: reading input from terminal:
.ds ^D \f(CW^D\fP
.ds ^Z \f(CW^Z\fP
.if t .ft I
user types input, followed by end of file: \*(^D on UNIX, \*(^Z on MSDOS
.if t .ft P
.)
In all examples of keyboard input,
the sequence
.T ^X
will be used for control characters like control-x
(hold down the
.T CTRL
key and type the letter
.T x ).
On UNIX, end of input from the keyboard is signaled by
typing
.T ^D .
MSDOS users type
.T ^Z .
.SH "Input Formats"
.P
|STAT programs have simple input formats.
Program input is read until the end of file,
.I EOF ,
is found.
End of file in disk files is done by the system;
no special marking characters are needed nor allowed.
.P
Input
.B fields
(visibly distinguishable
.B words )
are separated by
.B whitespace
(blank spaces, tabs, newlines).
For most programs,
fields in lines with embedded spaces can be enclosed by
single or double quotes.
Most |STAT analysis programs
ignore blank input lines used to improve the human-readability of the data.
However, blank lines are meaningful to some data manipulation programs,
so when there are unexpected results,
it is often instructive to run a file through
.T validata .
.P "Suggestion: Staged Analysis
It is usually a good idea to build a complex command,
such as a pipeline,
in stages.
At each stage,
a quick visual inspection of the output catches most errors you might make.
.P "Data Types
|STAT programs recognize several types of data:
label and variable names, numbers (integers and real numbers),
and some programs can deal with missing values, denoted by
.T NA .
Label and variable names begin with an alphabetic character (a-z or A-Z),
and can be followed by any number of alphanumerics (a-z, A-Z, 0-9)
and underscores.
There are three types of numbers:
integers, real numbers with a decimal point,
and numbers in exponential scientific notation.
Integers are positive or negative numbers with no decimal point,
or if they have a decimal point, they have no non-zero digits
after the decimal point.
Exponential notation numbers are numbers of the form
.T xxx.yyyEzz .
They may have digits before an optional decimal point or after it,
and the number after the
.T E
or
.T e
is a power of ten multiplier.
For example,
.T 1.2e-6
is 1.2 times the inverse of one million.
.P "Caveat: Appearances Can Be Deceiving
Inputs that look like they line up might not
appear so to |STAT programs.
For example,
the following data might appear to have four columns,
but have a variable number.
Also, the columns that look like they line up to a person,
do not line up to |STAT programs.
.(
a	b	c	d
e		f	g
h	i		j
.)
Here is how |STAT programs see this input:
.(
a	b	c	d
e	f	g
h	i	j
.)
This difference could be found with the
.T validata
utility program, which would report for both formats above:
.(
validata: Variable number of columns at line 2
Col   N  NA alnum alpha   int float other  type   min   max 
  1   3   0     3     3     0     0     0 alnum     0     0 
  2   3   0     3     3     0     0     0 alnum     0     0 
  3   3   0     3     3     0     0     0 alnum     0     0 
  4   1   0     1     1     0     0     0 alnum     0     0 
.)
.SH "Limits and Error Messages"
.P
There is a system-dependent limit on the count of characters
in an input line:
on small systems, 512 characters,
and on large ones, 1024.
Many programs use dynamic memory allocation
so the memory available on a machine will determine the
size of data sets that can be analyzed.
Integer overflow is not checked,
so numbers like data counts are limited on 16 bit machines to 32767;
in practice, this has not presented problems.
All calculations are done with double precision floating point numbers,
but overflow (exceeding the maximum allowed double precision number,
about 10 to the 38th power)
and underflow (loss of precision of a tiny non-zero result being rounded to 0.0)
are not checked.
Program specific limits can be found in most programs with the
.T -L
option.
The programs are not robust when used on highly variable data
(differences of several orders of magnitude),
very large numbers,
or large datasets (more than 10,000 values).
.P
All error and warning messages
(1) identify the program detecting the problem
(useful when pipelines or command scripts are used),
(2)
print diagnostic information,
(3)
sound a bell,
and for errors,
(4)
cause an exit.
All error and warning messages are printed on the diagnostic output
(that is
.T stderr
for C lovers),
so they will be seen even if the standard output is redirected to a file.
All |STAT programs exit with a non-zero exit status on error
and a zero exit status after a successful run.
.P "Common Error Messages
Some errors and messages are common to several programs.
They are explained below.
Other messages should be self-explanatory.
.(
Not enough (or no) input data
	\fRThere were no data points read, or not enough to make sense\fP
Too many xxxx's; at most N allowed
	\fRToo many of something were in the input (e.g., columns or variables)\fP
Cannot open 'file'
	\fRThe named file could not be opened for reading\fP
No storage space left for xxxx
	\fRThe program has run out of dynamic memory for internal storage\fP
\'string' (description) is not a number
	\fRThe described object whose input value was 'string' was non-numerical\fP
N operand(s) ignored on command line
	\fROperands (e.g., files) on the command line are ignored by this program\fP
VALUE is an illegal value for the TYPE
	\fRThe provided value was out of the legal range for the given type\fP
Ragged input file
	\fRThe program expects a uniform number of input columns\fP
.)
.SH "Manual Entries"
.P
|STAT manual entries contain detailed information about
each of the programs.
They describe the effects of all the options.
.P "On-Line Manuals
On UNIX systems, the manual entries for |STAT programs
are available online with the
.T manstat
program.
UNIX system administrators might prefer to install the |STAT manuals
in a public place,
so they might be available with the standard UNIX
.T man
program.
On MSDOS systems, manual entries might be available online
with a batch file that types pre-formatted manuals.
The following will print the online manual for the
.T anova
program.
.(
manstat anova
.)
Most programs print a summary of their options with the
.T -O
option.
The following will print a summary of the options available with the
.T desc
descriptive statistics program.
.(
desc -O
.)
.P "UNIX Manual Conventions
UNIX manual entries are often considered cryptic,
especially for new users.
It helps to know the conventions used in writing manual entries.
In the following table,
the contents of the different manual entry sections are summarized.
.(
ALGORITHMS
	\fRsources or descriptions of algorithms\fP
BUGS
	\fRlimitations or known deficiencies in the program\fP
DESCRIPTION
	\fRdetails about the workings of the program,
	and information about operands\fP
EXAMPLES
	\fRexamples of command lines showing expected use of the program\fP
FILES
	\fRfiles used by the program (e.g., temporary files)\fP
LIMITS
	\fRlimits built into the program should be determined with the -L option\fP
NAME
	\fRthe name and purpose of the program\fP
OPTIONS
	\fRdetailed information about command line options (see the -O option)\fP
SYNOPSIS
	\fRa short summary of the option/operand syntax for the program
	(items enclosed in square brackets are optional)\fP
.)
.TC
.if t .AA
