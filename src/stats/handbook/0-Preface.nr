.CH 0 "Preface
.if t .bp 1
.P "Purpose and Intended Audience of the Handbook
This handbook is meant to be an introduction to the |STAT programs.
It is not written to teach students how to do data analysis,
although it has been used as a supplementary text in courses.
|STAT users should be familiar with using the hardware and utility
programs (e.g., a text editor) on their systems.
.P "Comparison With Other Packages
|STAT has advantages and disadvantages
compared to other statistical packages.
|STAT is not a comprehensive package because it was developed
as needs arose.
So there are deficits in many areas of analysis:
no multivariate analysis other than regression,
and only simple graphics.
Independent of these limitations,
the programs are not designed for use with large data sets or large values;
the programs are usually adequate for data up to a few thousand points.
Also, |STAT is unsupported,
so if you have problems installing or using the programs,
you may be on your own.
Despite these limitations,
|STAT provides you with most analyses reported in research.
|STAT programs run on UNIX and MSDOS, operating systems
popular in educational and research institutions, government, and industry.
The liberal copyright of the programs
allows free copies to be made for multiple machines
provided the programs are not copied for material gain.
|STAT programs integrate easily with other programs,
and this makes it possible for new programs to be added later.
.P "Distribution Conditions
CAREFULLY READ THE FOLLOWING CONDITIONS.  IF YOU DO NOT FIND THEM
ACCEPTABLE, YOU SHOULD NOT USE |STAT.
.P
|STAT IS PROVIDED "AS IS," WITHOUT ANY EXPRESS OR IMPLIED
WARRANTY.  THE USER ASSUMES ALL RISKS OF USING |STAT.  THERE IS NO
CLAIM OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
|STAT MAY NOT BE SUITED TO YOUR NEEDS.  |STAT MAY NOT RUN ON YOUR
PARTICULAR HARDWARE OR SOFTWARE CONFIGURATION.  THE AVAILABILITY OF
AND PROGRAMS IN |STAT MAY CHANGE WITHOUT NOTICE.  NEITHER
MANUFACTURER NOR DISTRIBUTOR BEAR RESPONSIBILITY FOR ANY MISHAP OR
ECONOMIC LOSS RESULTING THEREFROM OF THE USE OF |STAT EVEN IF THE
PROGRAMS PROVE TO BE DEFECTIVE.  |STAT IS NOT INTENDED FOR CONSUMER
USE.
.P
CASUAL USE BY USERS NOT TRAINED IN STATISTICS, OR BY USERS NOT
SUPERVISED BY PERSONS TRAINED IN STATISTICS, MUST BE AVOIDED.
USERS MUST BE TRAINED AT THEIR OWN EXPENSE TO LEARN TO USE THE
PROGRAMS.  DATA ANALYSIS PROGRAMS MAKE MANY ASSUMPTIONS ABOUT DATA,
THESE ASSUMPTIONS AFFECT THE VALIDITY OF CONCLUSIONS MADE BASED ON
THE PROGRAMS.  REFERENCES TO SOME APPROPRIATE STATISTICAL SOURCES
ARE MADE IN THE |STAT HANDBOOK AND IN THE MANUAL ENTRIES FOR
SPECIFIC PROGRAMS.  |STAT PROGRAMS HAVE NOT BEEN VALIDATED FOR
LARGE DATASETS, HIGHLY VARIABLE DATA, NOR VERY LARGE NUMBERS.
.P
You may make copies of any tangible forms of |STAT programs,
provided that there is no material gain involved, and provided that
the information in this notice accompanies every copy.  You may not
copy printed documentation unless such duplication is for non-
profit educational purposes.  You may not provide |STAT as an
inducement to buy your software or hardware or any products or
services.  You may distribute copies of |STAT, provided that mass
distribution (such as electronic bulletin boards or anonymous ftp)
is not used.  You may not modify the source code for any purposes
other than getting the programs to work on your system.  Any costs
in compiling or porting |STAT to your system are your's alone, and
not any other parties.  You may not distribute any modified source
code or documentation to users at any sites other than your own.
.ft R
.if t .bp
.P "References
.in .5i
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRBradley, J. V.
\fR(1968)
\fIDistribution-Free Statistical Tests\fR.
Englewood Cliffs, NJ: Prentice-Hall.
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRCoombs, C. H.,
Dawes, R. M.,
&
Tversky, A.
\fR(1970)
\fIMathematical Psychology: An Elementary Introduction\fR.
Englewood Cliffs, NJ: Prentice-Hall.
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRDixon, W. J.
\fR(1975)
\fIBMD-P Biomedical Computer Programs\fR.
Berkeley, CA: University of California Press.
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRGuilford, J. P.,
&
Fruchter, B.
\fR(1978)
\fIFundamental Statistics in Psychology and Education\fR.
(6th Edition).
New York: McGraw-Hill.
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRHays, W. L.
\fR(1973)
\fIStatistics for the Social Sciences\fR.
(2nd Edition).
New York, NY: Holt Rinehart Winston.
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRHemenway, K.,
&
Armitage, H.
\fR(1984)
\fRProposed Syntax Standard for UNIX System Commands\fR.
In
\fR\fR\fISummer USENIX Conference\fR.
El Cerito, CA: Usenix Association.
(Washington, DC.)
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRKeppel, G.
\fR(1973)
\fIDesign and Analysis: A Researcher's Handbook\fR.
Englewood Cliffs, NJ: Prentice-Hall.
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRKerlinger, F. N.,
&
Pedhazur, E. J.
\fR(1973)
\fIMultiple Regression in Behavioral Research\fR.
New York, NY: Holt Rinehart Winston.
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRKernighan, B. W.,
&
Ritchie, D. M.
\fR(1979)
\fIThe C Programming Language\fR.
Englewood Cliffs, NJ: Prentice-Hall.
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRNie, H. H.,
Jenkins, J. G.,
Steinbrenner, K.,
&
Bent, D. H.
\fR(1975)
\fISPSS: Statistical Package for the Social Sciences\fR.
New York: McGraw-Hill.
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRPerlman, G.
\fR(1980)
\fRData Analysis Programs for the UNIX Operating System\fR.
\fIBehavior Research Methods & Instrumentation\fR,
\fB12:5\fR,
554-558.
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRPerlman, G.
\fR(1982)
\fRData Analysis in the UNIX Environment: Techniques for Automated Experimental Design Specification\fR.
In
\fRK. W. Heiner,
R. S. Sacher,
&
J. W. Wilkinson
(Eds.),
\fR\fIComputer Science and Statistics: Proceedings of the 14th Symposium on the Interface\fR.
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRPerlman, G.,
&
Horan, F. L.
\fR(1986)
\fRReport on |STAT Release 5.1 Data Analysis Programs for UNIX and MSDOS\fR.
\fIBehavior Research Methods, Instruments, & Computers\fR,
\fB18.2\fR,
168-176.
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRPerlman, G.,
&
Horan, F. L.
\fR(1986)
|STAT: Compact Data Manipulation and Analysis Programs for MSDOS and UNIX - A Tutorial Overview.
Tyngsboro, MA: Wang Institute of Graduate Studies.
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRRitchie, D. M.,
&
Thompson, K.
\fR(1974)
\fRThe UNIX Time-Sharing System\fR.
\fICommunications of the Association for Computing Machinery\fR,
\fB17:7\fR,
365-375.
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRRyan, T. A.,
Joiner, B. L.,
&
Ryan, B. F.
\fR(1976)
\fIMINITAB Student Handbook\fR.
North Scituate, MA: Duxbury Press.
.if \n(.L=1 .if n .sp
.if \n(.L=1 .if t .sp 0.5v
.ne .5i
.ti 0
\fRSiegel, S.
\fR(1956)
\fINonparametric Methods for the Behavioral Sciences\fR.
New York: McGraw-Hill.
.in 0
.TC
