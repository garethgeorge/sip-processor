#! /bin/sh
ECHO="echo --------------------"
DELIM="echo  --------------------"
RM="/bin/rm -f regress.eqn scores.dat"

trap "echo interrupted...; $RM; exit 1" 2

echo '$ ff -dc -w 79 example.txt'
ff -dc -w 79 example.txt

$DELIM "Section 2.1    Data in exam.dat"
echo '$ cat exam.dat'
cat exam.dat

$ECHO "Section 2.2    Computing Final Scores"
echo '$ dm INPUT ".2*x4 + .3*x5 + .5*x6" < exam.dat > scores.dat'
dm INPUT ".2*x4 + .3*x5 + .5*x6" < exam.dat > scores.dat

$DELIM "Examine Scores File: scores.dat"
echo '$ cat scores.dat'
cat scores.dat

$ECHO "Sort Records by Final Scores"
echo '$ reverse -f < scores.dat | sort'
reverse -f < scores.dat | sort

$DELIM "Another Way Using dsort"
echo '$ dsort n7 < scores.dat'
dsort n7 < scores.dat

$ECHO "Section 2.3    Summary of Final Scores"
echo '$ dm  s7  <  scores.dat | desc  -o  -t 75  -h  -i 10  -m 0'
dm  s7  <  scores.dat | desc  -o  -t 75  -h  -i 10  -m 0

$ECHO "Section 2.4    Predicting Final Exam Scores"
echo '$ dm x6 x4 x5 < scores.dat | regress -e final midterm1 midterm2'
dm x6 x4 x5 < scores.dat | regress -e final midterm1 midterm2

$ECHO "Predicted Plot From Regression Equation in regress.eqn"
echo '$ dm x6 x4 x5 < scores.dat | dm Eregress.eqn |
	pair -p -h 10 -w 30 -x final -y predicted'
dm x6 x4 x5 < scores.dat | dm Eregress.eqn |
	pair -p -h 10 -w 30 -x final -y predicted

$DELIM "Residual Plot"
echo '$ dm x6 x4 x5 < scores.dat | dm Eregress.eqn | dm x2 x1-x2 |
	pair -p -h 10 -w 30 -x predicted -y residuals'
dm x6 x4 x5 < scores.dat | dm Eregress.eqn | dm x2 x1-x2 |
	pair -p -h 10 -w 30 -x predicted -y residuals

$ECHO "Section 2.5    Failures by Assistant and Gender"
echo '$ dm s2 s3 "if x7 GE 75 then' "'pass' else 'fail'\"" '1 < scores.dat |
	contab assistant gender success count'
dm s2 s3 "if x7 GE 75 then 'pass' else 'fail'" 1 < scores.dat |
	contab assistant gender success count

$ECHO "Section 2.6    Effects of Assistant and Gender"
echo '$ dm s1 s2 s3' "\"'m1'\"" s4 s1 s2 s3 "\"'m2'\"" s5 s1 s2 s3 "\"'final'\"" 's6 < scores.dat |
	maketrix 5 | anova student assistant gender exam score'
dm s1 s2 s3 "'m1'" s4 s1 s2 s3 "'m2'" s5 s1 s2 s3 "'final'" s6 < scores.dat |
	maketrix 5 | anova student assistant gender exam score

df1=2
df2=32
critf=`probdist crit F $df1 $df2 .05`
MSerror=32.1215
N=10
$DELIM "Scheffe 95% Confidence Interval:"
echo '$ echo "sqrt ($df1 * $critf * $MSerror * 2 / $N)" | calc'
echo "sqrt ($df1 * $critf * $MSerror * 2 / $N)" | calc

$RM
