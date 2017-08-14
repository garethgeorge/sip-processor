.P
.T calc
is a program for mathematical calculations for which you might use a
hand-held calculator.
.T calc
supplies most of the operations common to programming languages
and variables with constraint properties much like those in spreadsheets.
.P
The arithmetical operators
.T calc
offers are
.(
+	\fRaddition\fP
-	\fRsubtraction and change-sign\fP
*	\fRmultiplication\fP
/	\fRdivision\fP
%	\fRmodulo division\fP
^	\fRexponentiation\fP
.)
Arithmetical expressions can be arbitrarily complex
and are generally evaluated left to right.
That is,
.(
a + b - c
.)
is the same as
.(
(a + b) - c
.)
Exponentiation is evaluated before multiplication and division
which are evaluated before addition and subtraction.
For example, the expression
.(
a + b - c * d / e ^ 2
.)
is parsed as
.(
(a + b) - ((c * d) / (e ^ 2))
.)
This default order of operations can be overridden by using parentheses.
.P
.T calc
supplies some transcendental functions:
.T sqrt ,
.T log ,
.T exp ,
and
.T abs ,
and the following trigonometric functions:
.T sin ,
.T asin ,
.T cos ,
.T acos ,
.T tan ,
and
.T atan ,
for which
degrees are measured in radians.
.P "Using CALC"
To use
.T calc ,
begin by typing
.(
calc
.)
at the command level, and
.T calc
will prompt you with
.(
CALC:
.)
You can supply inputs to
.T calc
from files specified by command line arguments.
For example, typing
.(
calc foo
.)
will read from the file
.T foo
and then ask for input from you.
Type in each of your expressions followed by
.T RETURN
and
.T calc
will respond with how it parsed your expression followed by the result.
In all following examples, what you would type in is preceded by the
.T calc
prompt
.(
CALC:
.)
and what
.T calc
responds with is immediately after.
A simple calculation is:
.(
CALC: sqrt (12^2 + 5^2)
sqrt(((12 ^ 2) + (5 ^ 2)))     = 13
.)
.P
Expressions can be stored by assigning them to variables.
For example you could type:
.(
CALC: pi = 22/7
(22 / 7)      = 3.14286
CALC: pi
pi          = 3.14286
.)
Variables can be used in expressions.
.(
CALC: area = pi * r^2
(pi * (r ^ 2))      = UNDEFINED
CALC: area
area     = UNDEFINED
.)
.T area
is undefined because
.T r
has not been set.
Once
.T r
is set,
.T area
will have a value because
.T area
is set to an equation rather than a particular value.
This can be observed by printing all the variables
so far introduced with
.T ^V ,
which may have to be typed twice
as
.T ^V
is used in some UNIX versions to quote characters.
.(
CALC: ^V
pi       =      3.14286 = (22 / 7)
area     =    UNDEFINED = (pi * (r ^ 2))
r        =    UNDEFINED =
.)
The variable table is formatted so that each variable's name is
on the left, followed by its current value, followed by its
current definition.
If
.T r
is set to 5, the value of
.T area
is now defined.
.(
CALC: r = 5
5        = 5
CALC: ^V
pi       =      3.14286 = (22 / 7)
area     =      78.5714 = (pi * (r ^ 2))
r        =            5 = 5
.)
The effect of changing
.T r
on
.T area
can be observed because of the way
.T area
is defined.
.(
CALC: r = 2
2         = 2
CALC: area
area      = 12.5714
.)
.P
A special variable named
.T $
is always equal to the most recent result printed.
.P "Setting Constant Values
Of course, there are times when you want to set a variable
to a value and not have it depend on the values of variables
at a later time.
To do this, you precede an expression with the number operator
.T # .
For example,
.(
CALC: area2 = # area
12.5716      = 12.5716
CALC: ^V
pi       =      3.14286 = (22 / 7)
area     =      12.5716 = (pi * (r ^ 2))
r        =            2 = 2
area2    =      12.5716 = 12.5716
.)
.T area2
does not depend on the variable to which it was set because
the number operator
.T #
only lets numbers through it rather than
expressions.
If
.T area2
was set without the
.T #
operator,
it would be subject to any changes in
.T area
or to any changes in variables on which
.T area
depends.
.(
CALC: area2 = area
area      = 12.5716
CALC: ^V
pi       =      3.14286 = (22 / 7)
area     =      12.5716 = (pi * (r ^ 2))
r        =            2 = 2
area2    =      12.5716 = area
.)
.P "Testing Conditions
Variables can be set based on a tested condition.
For example, you may want a variable
.T max
to always be the maximum of
.T a
and
.T b .
.(
CALC: max = if a > b then a else b
(if (a > b) then a else b)    = UNDEFINED
.)
.T max
is undefined because
.T a
and
.T b
have not been set.
.(
CALC: a = 21
21     = 21
CALC: b = 3^3
(3 ^ 3)    = 27
CALC: max
max      = 27
CALC: a = 50
50   = 50
CALC: max
max      = 50
.)
The if-then-else expression allows variables to be set based on
conditions.
This condition can be made up with relational and logical operators.
The relational operators available with
.T calc
are:
.(
==	\fRtest equality\fP
!=	\fRtest inequality\fP
>=	\fRgreater than or equal\fP
<=	\fRless than or equal\fP
>	\fRgreater than\fP
<	\fRless than\fP
.)
while the logical operators are:
.(
&	\fRand\fP
|	\fRor\fP
!	\fRnot\fP
.)
A more complicated expression involving these is:
.(
if a > b & b > c then b
.)
The
.T else
part of the conditional is optional, and if not present
and the condition is false, the conditional is undefined.
.P "Undefined Variables
Variables are undefined if they have not been set,
if they depend on variables that are undefined,
or if they are set to an expression involving an illegal operation.
.(
CALC: 1/0
(1 / 0)     = UNDEFINED
.)
You can be confident that no operations will result in
.T calc
blowing up.
Thus you could write the equation for the roots of a quadratic formula
with the following definitions and always get reasonable answers.
.(
x = 0
a = b = 1
c = -1
radical = sqrt (b^2 - 4*a*c)
equation = a*x^2 + b*x + c
derivative = 2*a*x + b
root1 = (-b + radical) / (2 * a)
root2 = (-b - radical) / (2 * a)
.)
.P "Control Characters
Non-mathematical operations are accomplished with control characters.
To type a control character, say CTRL-p, while you hold down the
key labeled CTRL you type a
.T p .
This will appear as
.T ^P .
Some control characters have special meanings, such as
"stop the program" so you must be careful with them.
On UNIX, you can avoid some problems with control characters
by typing a
.T ^V
before them.
This character removes any special meaning associated with the
character immediately following it.
So to type
.T ^P
you could be extra safe and type
.T ^V^P .
To type a
.T ^V ,
you may have to type it twice.
Unfortunately, these conventions are not universal.
.P
The following control operations are available with
.T calc .
.(
^P	\fRtoggle the printing of expressions (UNIX only)\fP
^Rf	\fRread the input from file f and return to current state\fP
^V	\fRprint the variable table\fP
^Wf	\fRwrite the variable table to file \fPf\fP
	(\fP^W\fP is a synonym for \fP^V\fP)\fP
.)
If you forget any of these commands,
you can type a
.T ?
to get
.T calc
to remind you.
.if t .bp
.(
.ps 12p
.B "Table of \f(CWcalc\fP Operations
.sp
.ps
.ft R
.ta .5iC +.75iC +.75iC +.5i
.ta .5iC +.75iC +.75iC +.5i
.if n .ta 8nC +10nC +10nC +8n
	Operator		Associativity
		Precedence		Description
.if t .sp .5v
.if n .sp
.if t .ft CW
.ft R
	\fP$	\fPconst	none	numerical value of previous calculation
	\fP#\fPa	1	none	numerical value of a
	a\fP=\fPb	2	right	a is set to expression b
	\fPif\fP a \fPthen\fP b	3	left	if a != 0 then b else \fPUNDEFINED\fP
	\fPelse\fP	4	left
	a\fP|\fPb	5	left	true if a or b is true
	a\fP&\fPb	6	left	true is a and b are true
	\fP!\fPa	7	none	true is a is false
	a\fP==\fPb	8	none	true if a equals b
	a\fP!=\fPb	8	none	true if a is not equal b
	a\fP<\fPb	8	none	true if a is less than b
	a\fP>\fPb	8	none	true if a greater than b
	a\fP>=\fPb	8	none	true if a > b | a == b
	a\fP<=\fPb	8	none	true if a < b | a == b
	a\fP+\fPb	9	left	a plus b
	a\fP-\fPb	9	left	a minus b
	a\fP*\fPb	10	left	a times b
	a\fP/\fPb	10	left	a divided by b
	a\fP%\fPb	10	left	a modulo b
	a\fP^\fPb	11	right	a to the b
	\fP-\fPa	12	none	change sign
	\fPabs(\fPa\fP)\fP	12	none	absolute value
	\fPexp(\fPa\fP)\fP	12	none	e to the power a
	\fPlog(\fPa\fP)\fP	12	none	natural logarithm of a
	\fPsqrt(\fPa\fP)\fP	12	none	square root of a
	\fPsin(\fPa\fP)\fP	12	none	sine of a in radians (\fPcos\fP & \fPtan\fP)
	\fPasin(\fPa\fP)\fP	12	none	arc sine of a (\fPacos\fP & \fPatan\fP)
.)
