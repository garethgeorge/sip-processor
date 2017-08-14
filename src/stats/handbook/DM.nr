.P
.T dm
is a data manipulating program with many
operators for manipulating columnated files of
numbers and strings.
.T dm
helps avoid writing little BASIC or C
programs every time some transformation
to a file of data is wanted.
To use
.T dm ,
a list of expressions is entered, and
for each line of data,
.T dm
prints the result of evaluating each expression.
.P
.B "Introductory Examples."
Usually, the input to
.T dm
is a file of lines,
each with the same number of fields.
Put another way,
.T dm 's
input is a file with some set number of columns.
.P
.I "Column Extraction:"
.T dm
can be used to extract columns.
If
.T data
is the name of a file of five columns,
then the following will extract
the 3rd string followed by the 1st, followed by the 4th,
and print them to the standard output.
.(
dm  s3  s1  s4  <  data
.)
Thus
.T dm
is useful for putting data in a correct format for input to many programs,
notably the |STAT data analysis programs.
.B Warning:
If a column is missing (e.g., you access column 3 and there
is no third column in the input), then the value of the access
will be taken from the previous input line.
This
.I feature
must be considered if there are blank lines in the input;
it may be best to remove blank lines, with
.T dm
or some other filter program.
.P
.I "Simple Expressions:"
In the preceding example, columns were accessed by typing the letter
.T s
(for string) followed by a column number.
The numerical value of a column can be accessed by typing
.T x
followed by a column number.
This is useful to form simple expressions based on columns.
Suppose
.T data
is a file of four numerical columns,
and that the task is to print the sum of the first two columns
followed by the difference of the second two.
The easiest way to do this is with:
.(
dm  x1+x2  x3-x4  <  data
.)
Almost all arithmetic operations are available and
expressions can be of arbitrary complexity.
Care must be taken because many of the symbols
used by
.T dm
(such as
.T *
for multiplication) have special meaning when used in UNIX (though not MSDOS).
Problems can be avoided by putting expressions in quotes.
For example, the following will print the sum of the squares of
the first two columns followed by the square of the third,
a simple Pythagorean program.
.(
dm  "x1*x1+x2*x2"  'x3*x3'  <  data
.)
.P
.I "Line Extraction Based on Conditions:"
.T dm
allows printing values that depend on conditions.
The
.T dm
call
.(
dm  "if x1 >= 100 then INPUT else NEXT"  <  data
.)
will print only those lines that have first columns with values
greater than or equal to 100.
The variable
.T INPUT
refers to the whole input line.
The special variable
.T NEXT
instructs
.T dm
to stop processing on the current line and go to the next.
.P "Data Types"
.B "String Data."
To access or print a column in a file,
the string variable,
.T s ,
is provided.
.T s i
(the letter
.T s
followed by a column number, such as
.T 5 )
refers to the ith column of the input, treated as a string.
The most simple example is to use an
.T s i
as the only part
of an expression.
.(
dm  s2  s3  s1
.)
will print the second, third and first columns of the input.
One special string is called
.T INPUT ,
and is the current input line of data.
String constants in expressions are delimited by single or double quotes.
For example:
.(
"I am a string"
.)
.P
.B "Numerical Data."
Constant numbers like
.T 123
or
.T 14.6
can be used alone or with other expressions.
Two general numerical variables are available
To refer to the input columns, there is
.T x i
and for the result of evaluated expressions, there is
.T y i.
.T x i
refers to the ith column of the input, treated as a number.
.T x i
is the result of converting
.T s i
to a number.
If
.T s i
contains non-numerical characters,
.T x i
may have strange values.
A common use of the
.T x i
is in algebraic expressions.
.(
dm  x1+x2  x1/x2
.)
will print out two columns,
first the sum of the first two input columns,
then their ratio.
.P
The value of a previously evaluated expression can be accessed
to avoid evaluating the same sub-expression more than once.
.T y i
refers to the numerical value of the ith expression.
Instead of writing:
.(
dm  x1+x2+x3  (x1+x2+x3)/3
.)
the following would be more efficient:
.(
dm  x1+x2+x3     y1/3
.)
.T y1
is the value of the first expression,
.T x1+x2+x3 .
String values of expressions are unfortunately inaccessible.
.P
Indexing numerical variables
is usually done by putting the index after
.T x
or
.T y ,
but if value of the index is to depend on the input,
such as when there are a variable number of columns,
and only the last column is of interest, the index value
will depend on the number of columns.
If a computed index is desired for
.T x
or
.T y
the index should be an expression in square brackets following
.T x
or
.T y .
For example,
.T x[N]
is the value of the last column of the input.
.T N
is a special variable equal to the number of columns in
.T INPUT .
There is the option to use
.T x1
or
.T x[1]
but
.T x1
will execute faster
so computed indexes should not be used unless necessary.
.P
.B "Special Variables."
.T dm
offers some special variables and control primitives for
commonly desired operations.
Many of the special variables have more than one name
to allow more readable expressions.
Many can be abbreviated,
and the short forms will be shown in square brackets.
.(
.ta 1i
.de BI\"builtin
\\$1	\fR\\$2\fP
..
.BI N             "the number of columns in the current input line
.BI SUM           "the sum of the numbers on the input line
.BI INLINE        "the line number of the input (initially 1.0)
.BI OUTLINE       "the number of lines so far output (initially 0.0)
.BI "RAND [R]"    "a random number uniform in [0,1) (may be followed by a seed)
.BI "INPUT [I]"   "the original input line, all spaces, etc. included
.BI NIL           "the empty expression (often used with a test)
.BI "KILL [K]"    "stop processing the current line and produce no output
.BI NEXT          "synonym for \f(CBKILL\fP
.BI SKIP          "synonym for \f(CBKILL\fP
.BI "EXIT [E]"    "exit immediately (useful after a search)
.)
.P "User Interface"
.B "Expressions."
Expressions are written in common computer language syntax,
and can have spaces or underscores inserted for readability anywhere except
(1) in the middle of constants, and
(2) in the middle of multicharacter operators such as
.T <=
(less than or equal to).
Four modes are available for specifying expressions to
.T dm .
They provide the choice of entering expressions from the terminal or a file,
and the option to use
.T dm
interactively or in batch mode.
.P
.I "Argument Mode:"
The most common but restrictive mode is to supply expressions as arguments
on the command line call to
.T dm ,
as featured in previous examples.
The main problem with this mode is that many special characters
in UNIX and MSDOS are used as operators, requiring that many
expressions be quoted.
The main advantage is that this mode is most useful in
constructing pipelines and shell scripts.
.P
.I "Expression File Mode:"
Another non-interactive method is to supply
.T dm
with a file with
expressions in it (one to each line) by calling
.T dm
with:
.(
dm  Efilename
.)
where
.T filename
is a file of expressions.
This mode makes it easier to use
.T dm
with pipelines and redirection.
.P
.I "Interactive Mode:"
.T dm
can also be used interactively by calling
.T dm
with no arguments.
In interactive mode,
.T dm
will first ask for a file of expressions.
If the expressions are not in a file,
type
.T RETURN
when asked for the expression file,
and they can be entered interactively.
A null filename tells
.T dm
to read expressions from the terminal.
In terminal mode,
.T dm
will prompt with the expression number,
and print out how it interprets what is typed in if it has correct syntax,
otherwise it allows corrections.
When the last expression has been entered,
an empty line informs
.T dm
there are no more.
If the expressions are in a file, type in the name of the file,
and
.T dm
will read them from there.
.P
.B "Input."
If
.T dm
is used in interactive mode, it will prompt for an input file.
A file name can be supplied
or a
.T RETURN
without a file name tells
.T dm
to read data from the terminal.
Out of interactive mode,
.T dm
will read from the standard input.
.P
.T dm
reads data a line at a time and stores that line
in a string variable called
.T INPUT .
.T dm
then takes each column in
.T INPUT ,
separated by spaces or tabs,
and stores each in the string variables,
.T s i.
.T dm
then tries to convert these strings to numbers and stores the
result in the number variables,
.T x i.
If a column is not a number (e.g., it is a string)
then its numerical value will be inaccessible,
and trying to refer to such a column will cause an error message.
The number of columns in a line is stored in a special variable called
.T N ,
so variable numbers of columns can be dealt with gracefully.
The general control structure of
.T dm
is summarized in the following display.
.(
.ta .5i 1i 1.5i 2i
read in n expressions; e1, e2, ..., en.
repeat while there is some input left
	INPUT  = <next line from input file>
	N      = <number of fields in INPUT>
	SUM    = 0
	RAND   = <a new random number in [0,1)>
	INLINE = INLINE + 1
	for i  = 1 until N do
		si  = <ith string in INPUT>
		xi  = <si converted to a number>
		SUM = SUM + xi
	for i = 1 until n do
		switch on <value of ei>
			case EXIT: <stop the program>
			case KILL: <go to get new INPUT>
			case NIL : <go to next expression>
			default  :
					OUTLINE = OUTLINE + 1
					yi = <value of ei>
					if (ei not X'd) print yi
	<print a newline character>
.)
.P
.B "Output."
In interactive mode,
.T dm
will ask for an output file (or pipe, on UNIX only).
.(
Output file or pipe: 
.)
A filename, a ``pipe command,'' or just
.T RETURN
can be entered.
A null filename tells
.T dm
to print to the terminal.
If output is being directed to a file,
the output file should be different from the input file.
.T dm
will ask permission to overwrite any
file that contains anything, but that does not mean
it makes sense to write the file it is reading from.
.P
On UNIX,
the output from
.T dm
can be redirected
to another program by having the first character of the
output specification be a pipe symbol, the vertical bar:
.T | .
For example, the following line tells
.T dm
to pipe its output to
.T tee
which prints a copy of its output to the terminal,
and a copy to the named file.
.(
Output file or pipe: | tee dm.save
.)
.P
Out of interactive mode,
.T dm
prints to the standard output.
.P
.T dm
prints the values of all its expressions
in
.T %.6g
format for numbers (maintaining at most six digits of precision
and printing in the fewest possible characters), and
.T %s
format for strings.
A tab is printed after every column to insure separation.
.P "Operations"
.T dm
offers many numerical, logical, and string operators.
The operators are evaluated in the usual order (e.g., times before plus)
and expressions tend be evaluated from left to right.
Parentheses can be used to make the order of operations clear.
The way
.T dm
interprets expressions can be verified by entering them
interactively on UNIX, in which case
.T dm
prints a fully parenthesized form.
.P
An assignment operator is not directly available.
Instead, variables can be evaluated but not printed
by using the expression suppression flag,
.T X .
If the first character of an expression is
.T X ,
it will
be evaluated, but not printed.
The value of a suppressed expression can later be accessed with
the expression value variable,
.T y i.
.P
.B "String Operations."
Strings can be lexically compared with several comparators:
.T <
or
.T LT
(less-than),
.T <=
or
.T LE
(less-than or equal),
.T =
or
.T EQ
(equal),
.T !=
or
.T NE
(not equal),
.T >= 
or
.T GE
greater-than or equal),
and
.T >
or
.T GT
(greater than).
They return
.T 1.0
if their condition holds, and
.T 0.0
otherwise.
For example,
.(
"abcde"  <=  'eeek!'
.)
is equal to
.T 1.0.
The length of strings can be found with the
.T len
operator.
.(
len  'five'
.)
evaluates to four, the length of the string argument.
The character
.T #
is a synonym for the
.T len
operator.
The numerical type of a string can be checked with the
.T number
function, which returns 0 for non-numerical strings,
1 for integer strings, and 2 for real numbers
(scientific notation or strings with non-zero digits after
the decimal point).
.P
Individual characters inside strings can be accessed by following a
string with an index in square brackets.
.(
"abcdefg"[4]
.)
is the ASCII character number (164.0) of the 4th character in
.T "abcdefg" .
Indexing a string is mainly useful for comparing characters because
it is not the character that is printed, but the character number.
A warning is appropriate here:
.(
s1[1]  =  '*'
.)
will result in an error because the left side of the
.T =
is a number, and the right hand side is a string.
The correct (although inelegant) form is:
.(
s1[1]  =  '*'[1]
.)
.P
A substring test is available. The expression:
.(
string1  C  string2
.)
will return
.T 1.0
if
.T string1
is somewhere in
.T string2 .
This can be used as a test for character
membership if string1 has only one character.
Also available is
.T !C
which returns
.T 1.0
if
.T string1
is NOT in
.T string2 .
.P
.B "Numerical Operators."
The numerical comparators are:
.(
<  <=  =  !=  >=  >
LT LE  EQ NE  GE GT
.)
and have
the analogous meanings as their string counterparts.
.P
The binary operators,
.T +
(addition),
.T -
(subtraction or "change-sign"),
.T *
(multiplication), and
.T /
(division)
are available.
Multiplication and division are evaluated
before addition and subtraction,
and are all evaluated left to right.
Exponentiation,
.T ^ ,
is the binary operator of highest precedence
and is evaluated right to left.
Modulo division,
.T % ,
has the same properties as division,
and is useful for tests of even/odd and the like.
NOTE: Modulo division truncates its operands to integers before
dividing.
.P
Several unary functions are available:
.T l
(natural log
.T [log] ),
.T L
(base ten log
.T [Log] ),
.T e
(exponential
.T [exp] ),
.T a
(absolute value
.T [abs] ),
.T f
(floor
.T [floor] ),
.T c
(ceiling
.T [ceil] ).
Their meaning can be verified in the UNIX Programmer's Manual.
Single letter names for these functions
or the more mnemonic strings bracketed after their names can be used.
Also available are trigonometric functions
that work on degrees in radians:
.T "sin cos tan asin acos atan"  .
.P
.B "Logical Operators."
Logical operators are of lower precedence than any other operators.
Both logical AND,
.T &
and OR
.T |
can be used to form complicated tests.
For example, to see if the first three columns are
in either increasing or decreasing order,
one could test if
.T x2
was between
.T x1
and
.T x3 :
.(
x1<x2  &  x2<x3  |  x1>x2  &  x2>x3
.)
would equal
.T 1.0
if the condition was satisfied.
Parentheses are unnecessary because
.T <
and
.T >
are of
higher precedence than
.T &
which is of higher precedence than
.T | .
The above expression could be written as:
.(
x1 LT x2  AND  x2 LT x3  OR  x1 GT x2  AND  x2 GT x3
.)
by using synonyms for the special character operators.
This is useful to avoid the special meaning of characters in command lines.
The unary logical operator,
.T !
(NOT), evaluates to
1.0
if its operand is
.T 0.0 ,
otherwise it equals
.T 0.0 .
Many binary operators can be immediately preceded by
.T !
to negate their value.
.T !=
is "not equal to,"
.T !|
is "neither,"
.T !&
is "not both,"
and
.T !C
is "not in."
.P
.B "Conditional Expressions."
The expressions:
.(
if expression1 then expression2 else expression3
   expression1  ?   expression2   :  expression3
.)
evaluate to
.T expression2
if
.T expression1
is non-zero, otherwise
they evaluate to
.T expression3 .
The first form is more mnemonic than the second which is
consistent with C syntax.
Upper case names can be used in their stead.
Both forms have the same meaning.
.T expression1
has to be numerical,
.T expression2
or
.T expression3
can be numerical or string.
For example, The following expression will filter out lines
with the word
.T bad
in them.
.(
if  'bad'  C  INPUT  then  KILL  else  INPUT
.)
As another example, the following expression will print the
ratio of columns two and three if (a) there are at least three
columns, and (b) column three is not zero.
.(
if  (N >= 3)  &  (x3 != 0)  then  x2/x3  else  'bad line'
.)
These are the only expressions, besides
.T s i
or a string constant
that can evaluate to a string.
If a conditional expression does evaluate to a string,
then it CANNOT be used in some other expression.
The conditional expression is of lowest precedence
and groups left to right,
however parentheses are recommended to make the semantics obvious.
.P "Expression Syntax"
Arithmetic expressions may be formed using
variables (with
.T x i
and
.T y i)
and constants
and can be of arbitrary complexity.
In the following table,
unary and binary operators are listed along with their
precedences and a brief description.
All unary operators are prefix except string indexing,
.T [] ,
which is postfix.  All binary operators are infix.
.P
Operators of higher precedence are executed first.
All binary operators are left associative except exponentiation,
which groups to the right.
An operator,
.T O ,
is left associative if
.T xOxOx
is parsed as
.T (xOx)Ox ,
while one that is right associative is parsed as
.T xO(xOx) .
.(
.ta .25i 1.25iR 1.5i
.ft R
Unary Operators:
.de BI
.ft R
.ft CW
	\\$1	\fP\\$2	\\$3
..
.if t .ul
	op	prec	description
.BI sin     10          "sine of argument degrees in radians
.BI cos     10          "cosine of argument degrees in radians
.BI tan     10          "tangent of argument degrees in radians
.BI asin    10          "arc (inverse) sine function
.BI acos    10          "arc (inverse) cosine function
.BI atan    10          "arc (inverse) tangent function
.BI sqrt    10          "square root function
.BI log     10          "base e logarithm [\fPl\fP]
.BI Log     10          "base 10 logarithm [\fPL\fP]
.BI exp     10          "exponential [\fPe\fP]
.BI abs     10          "absolute value [\fPa\fP]
.BI ceil    10          "ceiling (rounds up to next integer) [\fPc\fP]
.BI floor   10          "floor (rounds down to last integer) [\fPf\fP]
.BI len     10          "number of characters in string [\fP#\fP]
.BI number  10          "report if string is a number (0 non, 1 int, 2 real)
.BI []      10          "ASCII number of indexed string character
.BI -       9           "change sign
.BI !       4           "logical not (also \fPNOT\fP, \fPnot\fP)
.if t .sp .5v
.if n .sp
.ft R
Binary Operators:
.if t .ul
	op	prec	description
.BI ^           8           "exponentiation
.BI *           7           "multiplication
.BI /           7           "division
.BI %           7           "modulo division
.BI +           6           "addition
.BI -           6           "subtraction
.BI =           5           "test for equality (also \fPEQ\fP; opposite \fP!=\fP, \fPNE\fP)
.BI >           5           "test for greater-than (also \fPGT\fP; opposite \fP<=\fP, \fPLE\fP)
.BI <           5           "test for less-than (also \fPLT\fP; opposite, \fP>=\fP, \fPGE\fP)
.BI C           5           "substring (opposite \fP!C\fP)
.BI &           4           "logical AND (also \fPAND\fP, \fPand\fP; opposite \fP!&\fP)
.BI |           3           "logical OR (also \fPOR\fP, \fPor\fP; opposite \fP!|\fP)
.)
.P "Some Examples
To print lines 10-20 from an input file
.T dm.dat ,
you could run the following command (note that
.T x
is the same as
.T x0 ,
which is the same as
.T INLINE ,
the input line number).
.(
dm  "if x >= 20 and x <= 20 then INPUT else SKIP"  < dm.dat
.)
.P
To print all the lines longer than 100 characters,
you could run the following:
.(
dm  "if len(INPUT) > 100 then INPUT else SKIP"  < dm.dat
.)
.P
To print the running sums of values in a column,
you need to use the
.T y
variables.
The following will print the running sum of values in the first column.
.(
dm y1+x1
.)
To print the running sum of the data in the 5th column is a bit unintuitive.
.T y1
is the value from the previous line of the first expression,
and
.T x5
is the value of the fifth column on the current line.
To get the running sum of column 5,
you would type:
.(
dm y1+x5
.)
If the running sum is to come out in the third column,
then you would run:
.(
dm <something> <something> y3+x5
.)
.P
.T dm
is good at making tables of computed values.
In the following example,
the
.T echo
command prints headings for the columns,
and
.T colex
reformats the output of
.T dm .
.T colex
sets the default format to 10.3n
(numbers 10 wide, with 3 decimal places),
and prints column 1 in 2i format (2-wide integer)
and column 6 in 6i format (6-wide integer).
The
.T -t
option to
.T colex
stops the printing of tabs after columns.
.(
echo " x       1/x  x**2   sqrt(x)    log(x)"
series 1 10 | dm x1 1/x1 "x1*x1" "sqrt(x1)" "log(x1)" |
	colex -t -F 10.3n 2i1 2 6i3 4-5
.)
.(
 x       1/x  x**2   sqrt(x)    log(x)
 1     1.000     1     1.000     0.000
 2     0.500     4     1.414     0.693
 3     0.333     9     1.732     1.099
 4     0.250    16     2.000     1.386
 5     0.200    25     2.236     1.609
 6     0.167    36     2.449     1.792
 7     0.143    49     2.646     1.946
 8     0.125    64     2.828     2.079
 9     0.111    81     3.000     2.197
10     0.100   100     3.162     2.303
.)
