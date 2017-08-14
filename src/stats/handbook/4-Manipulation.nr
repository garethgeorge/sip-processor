.de PG
.P "\\$1:  \\$2
..
.CH 4 "Data Manipulation"
.de AA
.br
All data manipulation programs are introduced,
showing some of their options.
Full documentation is in the manual entries.
|STAT data manipulation tools allow users to generate,
transform, format, extract, and validate data.
.T dm ,
the data manipulator,
is the most important tool for use with other |STAT programs.
A detailed manual for
.T dm
is the last section of this chapter.
.if t .sp 6p
.if n .sp
There are several classes of data manipulation programs.
.B "Generation
programs produce more data than their inputs
by repeating data, numbering data, or by creating new data.
.B "Transformation
programs allow algebraic conversion of data.
.B "Formatting
programs change the shape or order of the data.
.B "Extraction
programs produce subsets of datasets.
.B "Validation
programs check the consistency, data types, and ranges of data.
.br
.rm AA
..
.if n .AA
.SH "Data Generation/Augmentation"
.PG repeat "repeat a string or file
.T repeat
can repeat strings or lines in a file as many times as requested.
It helps generate labels for datasets,
or feed a program like
.T dm
that needs input to produce output.
The following will repeat the file
.T data
10 times.
.(
repeat  -n 10  data
.)
The following will repeat its input series of 20 numbers 15 times.
.(
series  1 20  |  repeat  -n 15
.)
Strings can be repeated using the
.T echo
command.
The following will repeat the string
.T hello
100 times.
.(
echo hello | repeat -n 100
.)
.PG series "generate a linear series
.T series
generates a linear series of numbers between two values.
By default, its values change by units,
but this can be modified.
The following
produces a series of 10 numbers, 1 to 10, one per line.
.(
series 1 10
.)
The following produces the same series, but in reverse order;
the start of the series can be greater than the end.
.(
series 10 1
.)
Non-integral series can be created by supplying an optional increment.
.(
series 0 1 .1
.)
produces the series:
.(
0  .1  .2  .3  .4  .5  .6  .7  .8  .9  1
.)
except that each value is on its own line.
The output from series can be transformed with
.T dm
to produce other than linear series.
Here is an exponential series:
.(
series 1 10 | dm "exp(x1)"
.)
.PG probdist "generate random numbers
.T probdist
can generate random numbers for several probability distributions.
The following
will generate 100 random numbers from the uniform distribution
(between 0 and 1).
.(
probdist random uniform 100
.)
This can be transformed using
.T dm
to get random numbers with other ranges.
The following
will produce 100 random integers uniformly distributed
between 10 and 29.
.(
probdist random uniform 100 | dm "floor(x1*20+10)"
.)
The following generates numbers from a one-trial binomial distribution
with probability 0.5.
.(
probdist random uniform 100 | dm "if x1 > .5 then 1 else 0"
.)
.T probdist
also has a binomial distribution built in,
so the following would be equivalent to the previous example:
.(
probdist rand binomial 1 1/2 100
.)
The random number generator can be seeded.
The following will seed the random number generator with 143
and generate 100 normally distributed z values.
.(
probdist -s 143 random normal 100
.)
The seeding option is useful when a random sequence must be repeated.
The random normal numbers have a mean of 0 and a standard deviation of 1,
so
.T dm
can help create different random normal distributions.
The following samples a normal distribution with mean 100
and standard deviation 15.
.(
probdist random normal 100  |  dm  "x1*15+100"
.)
.PG abut "number lines, recycle files
.T abut
can number input lines in files using the
.T -n
option,
or cycle through input files as many times
as is necessary to match the length of longer files.
The latter case is common in creating input files for programs
like
.T anova
and
.T contab ,
which have input data tagged with regular patterns of labels.
.(
.ta 1i 2i 3i 4i
.if t .ft I
File1	File2	Data
.if t .ft P
large	easy	12
small	easy	23
	hard	34
	hard	45
		56
		67
		78
		89
.)
For the above input file configuration,
the command
.(
abut -nc File1 File2 Data
.)
would produce the following by recycling the smaller files.
.(
.ta 1i 2i 3i 4i
1	large	easy	12
2	small	easy	23
3	large	hard	34
4	small	hard	45
5	large	easy	56
6	small	easy	67
7	large	hard	78
8	small	hard	89
.)
.PG dm "number lines
.T dm
can number its input lines with its special variables
.T INLINE ,
which always contains the input line number,
and
.T INPUT ,
which always contains the current input line.
.(
dm INLINE INPUT < data
.)
.SH "Data Transformation"
.PG dm "conditional algebraic combinations of columns
.T dm
can produce algebraic combinations of columns.
The following command reads from
.T data
and produces
the ratio of columns 2 and 1 with column 3 added on.
.(
dm  x2/x1+x3  <  data
.)
Transformations can be based on conditions.
For example, if
.T x1 ,
the value in column 1,
in the above example is 0, then
.T dm
will exit after producing an error message like:
.(
dm: division by zero. input line 12  expr[1].
.)
To avoid this problem,
the following will do the division only if
.T x1
is non-zero.
.(
dm "if x1 then x2/x1+x3 else 0" < data
.)
.PG probdist "probability/statistic conversion
.T probdist
can convert probabilities to distribution statistics and
.I "vice versa"
as seen in tables at the end of most statistics textbooks.
Many distributions are supported, including:
the normal z,
binomial,
chi-square,
F, and t.
The following will print the two-tailed probability of an
obtained t statistic of 2.5 with 20 degrees of freedom.
.(
probdist prob t 20 2.5
0.021234
.)
Similarly, the following will print the two-tailed probability of an
F ratio of 6.25 with 1 and 20 degrees of freedom.
.(
probdist prob F 1 20 6.25
0.021234
.)
These results are the same because of the relationship between
the t and F distributions.
.P
The following prints
the critical value
(also called the quantile)
in the chi-square distribution with 5 degrees of freedom
to obtain a significance level of .05.
.(
probdist crit chisq 5 .05
11.070498
.)
Both probabilities and critical values in the normal z distribution
use the lower one tail -\(if to +\(if distribution,
so the z value that produces the .05 level is obtained with the following.
.(
probdist crit z .05
-1.644854
.)
The critical value for the 99th percentile is found with the following.
.(
probdist crit z .99
2.326348
.)
Binomial distribution critical values are treated differently
than the other continuous distributions.
For the binomial distribution based on five trials,
and a probability of success of one half,
The critical value for a one-tailed test at the .05 level is:
.(
probdist crit binomial 5 1/2 .05
5
.)
even though the probability of 5 successes is proportionally much less than .05:
.(
probdist prob binomial 5 1/2 5
0.031250
.)
This is because the binomial distribution is discrete.
Not only are critical values conservative,
sometimes there may be no possible value;
there is no way to get a less probable event than
five out of five successes:
.(
probdist crit binomial 5 1/2 .01
6
.)
Here,
.T probdist
is returning an impossible value
(one with zero probability).
.PG ranksort "convert data to ranks
.T ranksort
can rank order data from numerical data columns.
For the input:
.(
1	95	4.3
2	113	5.2
3	89	4.5
4	100	5.0
5	89	4.5
.)
.T ranksort
would produce:
.(
1	3	1
2	5	5
3	1.5	2.5
4	4	4
5	1.5	2.5
.)
The ties in the second and third columns
get the average rank of the values for which they are tied.
Once data are ranksorted,
further ranksorting has no effect.
With rank orders within columns,
rank order statistics
(e.g., Spearman rank order correlation, average group rank)
can be computed by parametric programs like
.T pair
or
.T regress .
.SH "Data Formatting"
.PG maketrix "form a matrix format file
.T maketrix
reads its data, one whitespace separated string at a time
from its free format input,
and produces a multicolumn output.
.(
series 1 20 | maketrix 5
.)
The above produces a five column output.
.(
1	2	3	4	5
6	7	8	9	10
11	12	13	14	15
16	17	18	19	20
.)
.PG perm "permute lines
.T perm ,
with no options,
randomizes its input lines.
It can randomize output from programs like
.T series .
.(
series 1 20 | perm
.)
A subset of this permutation is a sample without replacement.
The following is a sample of size 10 from the file
.T data .
.(
perm < data | dm "if INLINE <= 10 then INPUT else EXIT"
.)
.T perm
can be supplied a seed for its random number generator,
to replicate a random permutation.
.(
series 1 20 | perm -s 5762 | maketrix 5
.)
The above produces (with my system's random number generator):
.(
18	7	10	13	2
14	11	19	15	20
1	3	9	6	16
8	17	12	5	4
.)
.P
.T perm
can also put its lines in alphabetical or numerical order.
For example, the output from the previous example could be put into
ascending order (according to the first number on each line) with:
.(
series 1 20 | perm -s 5762 | maketrix 5 | perm -n
.)
This produces:
.(
1	3	9	6	16
8	17	12	5	4
14	11	19	15	20
18	7	10	13	2
.)
.PG dsort "sort data lines by multiple keys
The last example of the
.T perm
filter showed how lines can be ordered according to the numerical value
in the first column.
.T dsort
can sort lines based on numerical or alphabetical values in any column.
For example,
the following command sorts the previous example matrix in
ascending order of the values in the third column.
.(
series 1 20 | perm -s 5762 | maketrix 5 | dsort -n 3
.)
This produces:
.(
1	3	9	6	16
18	7	10	13	2
8	17	12	5	4
14	11	19	15	20
.)
If there were ties in a column,
.T dsort
could sort by additional key columns.
.PG transpose "transpose matrix format file
.T transpose
flips rows and columns in its input.
For the input:
.(
1	2	3	4
5	6	7	8
9	10	11	12
.)
.T transpose
produces:
.(
1	5	9	
2	6	10	
3	7	11	
4	8	12	
.)
The input to
.T transpose
does not have to be regular,
nor does it have to be numerical.
.(
.ta 1i 2i 3i 4i 5i
one	two	three
four	five
six
seven	eight
nine	ten	eleven
.)
For the above input,
.T transpose
produces the following.
.(
.ta 1i 2i 3i 4i 5i
one	four	six	seven	nine	
two	five		eight	ten	
three				eleven	
.)
Note that with regular inputs,
the transposition of a transposition yields the original.
This is not necessarily so with data as in the above input and output.
The above output piped through another pass of
.T transpose
produces a result different from the original input.
.(
.ta 1i 2i 3i 4i 5i
one	two	three	
four	five	eleven	
six	eight		
seven	ten		
nine			
.)
.PG reverse "reverse lines, columns, characters
.T reverse
can reverse the lines, fields, or characters in its input.
It can provide easier access to the last lines in a file,
or the last columns on lines.
To get the last 10 lines in a file,
we can reverse the file, get the first 10 lines,
and then reverse those 10 lines.
.(
reverse < data | dm "if INLINE GT 10 then EXIT else INPUT" | reverse
.)
To get the last two
.I columns
in a file is easier.
.(
reverse -f < data | dm s2 s1
.)
Here,
.T dm
is used for column extraction,
and rather than call
.T reverse
a second time,
what were the last two columns before reversal are listed in the opposite order.
.PG colex "reorder columns, reformat columns
.T colex
is a column extraction program
that shares some of the functionality of
.T dm
and
.T reverse .
.T colex
is faster and has a simpler syntax than
.T dm
and has data formatting capabilities.
Suppose a matrix dataset with 10 columns is created with the following.
.(
series 1 50 | maketrix 10
.)
.T colex
can extract the last five columns followed by the first five
with the command:
.(
series 1 50 | maketrix 10 | colex 6-10 1 2 3 4 5
.)
Either ranges of columns or single columns can be given.
The above command produces:
.(
.if t .ne 5v
6	7	8	9	10	1	2	3	4	5	
16	17	18	19	20	11	12	13	14	15	
26	27	28	29	30	21	22	23	24	25	
36	37	38	39	40	31	32	33	34	35	
46	47	48	49	50	41	42	43	44	45	
.)
.P
Note in the previous example how the numbers line up on the left,
rather than the customary format to line up the unit digits.
This is because
.T colex
puts tabs between columns,
and it is not a problem because |STAT programs read data in free-format.
.T colex
can print its columns in several numerical formats
as well as the default string format.
The numerical formatting can round values to some
number of decimal places (like zero, for whole numbers).
The option:
.T "-F\ 4i"
would tell
.T colex
to format all the columns as integers, each four spaces wide,
and the
.T -t
option would tell
.T colex
to not place a tab between columns.
The format of columns can be assigned to individual columns
by placing the format before each range of columns.
For example,
the following variation on the previous command would print
columns 6-10
in a money format with two digits after the decimal place,
and
columns 1-5
as integers four wide.
.(
series 1 50 | maketrix 10 | colex -t 6.2n6-10 4i1-5
.)
.(
  6.00  7.00  8.00  9.00 10.00   1   2   3   4   5
 16.00 17.00 18.00 19.00 20.00  11  12  13  14  15
 26.00 27.00 28.00 29.00 30.00  21  22  23  24  25
 36.00 37.00 38.00 39.00 40.00  31  32  33  34  35
 46.00 47.00 48.00 49.00 50.00  41  42  43  44  45
.)
.PG dm "reorder columns
.T dm ,
like
.T colex ,
can reorder columns.
However, it does not allow the specification of ranges of columns.
The above example of
.T colex
could be done with
.T dm
with similar results.
.(
series 1 50 | maketrix 10 | dm s6 s7 s8 s9 s10 s1 s2 s3 s4 s5
.)
.PG abut "paste corresponding lines from files
.T abut
can join data in separate files beside one another.
In the usual case,
.T abut
takes N files with K lines
and produces 1 file with K lines.
Suppose the files
.T height
and
.T weight
contain the respective heights and weights of the same people.
Each line in each file contains one height or weight.
These could be plotted with the plotting option on the
.T pair
program with the following command.
.(
abut height weight | pair -p
.)
.SH "Data Extraction"
.PG dm "conditional data extraction
.T dm
can extract subsets of its input,
either by columns or by lines.
To extract columns of data,
each extracted column is specified with the number of the
column preceded by the letter
.T s .
The following extracts columns 8, 2, and 11,
in that order.
.(
dm s8 s2 s11
.)
.T dm
can extract lines of data by using its built-in line skipping expression
.T SKIP .
The following will extract lines 50 to 100.
.(
dm "if INLINE >= 50 & INLINE <= 100 then INPUT else SKIP"
.)
It is more awkward than column extraction,
but the latter is common.
.PG colex "quick column extraction
.T colex
can extract individual columns,
or ranges of columns.
For column extraction, it is easier to use and faster than
.T dm .
The following extracts, in order, columns 8, 2, and 11.
.(
colex  8  2  11
.)
.PG linex "line extraction
.T linex
can extract individual lines (by number), or ranges of lines.
The following extracts, in order, lines 8, 2, and 11.
.(
linex  8  2  11
.)
To extract lines 50 to 100,
you could type:
.(
linex  50-100
.)
or you could even extract them in reverse order:
.(
linex  100-50
.)
.SH "Data Validation"
.PG validata "data validation
.T validata
will report for its input the number of columns,
data-types of columns, and for columns with numerical values,
the maxima and minima.
.T validata
reports any inconsistencies in the number of columns
in its input.
Floating point numbers can be entered in scientific notation.
For the input:
.(
1	2	3
4	5	6
7	2E2	end
5		1e-3
.)
.T validata 's
output is:
.(
validata: Variable number of columns at line 4
Col   N  NA alnum alpha   int float other  type   min   max 
  1   4   0     4     0     4     4     0   int     1     7 
  2   4   0     3     0     2     4     0 float 0.001   200 
  3   3   0     3     1     2     2     0 alnum     3     6 
.)
.PG dm "conditional data validation
.T dm
can find exceptional cases in its input.
A simple case is non-numerical input,
which can be checked with
.T dm 's
.T number
function.
.(
dm  "if !number(s1) then 'bad input on line' else SKIP"  INLINE
.)
.T dm
can check for specific values, ranges of values,
or specific relations of values.
The following prints all lines in
.T data
with the string
.T "bad"
in them.
.(
dm "if 'bad' C INPUT then INPUT else SKIP"
.)
The input line number could be prepended.
.(
dm INLINE "if 'bad' C INPUT then INPUT else SKIP"
.)
This is possible because
.T dm
will produce no output for skipped lines,
regardless of expression order.
The following prints all lines where column 3 is greater than column 2.
.(
dm "if x3 > x2 then INPUT else SKIP"
.)
.T dm
can print lengths of strings and check for numerical fields:
.(
dm  len(s1)  number(s1)
.)
will print the length of column 1 strings,
and report if they are numerical
(0 for non-numbers, 1 for integers, 2 for real numbers,
3 for exponential scientific notation numbers).
.SH "DM: Tutorial and Manual"
.so DM.nr
.TC --
.if t .sp .2i
.if t .AA
