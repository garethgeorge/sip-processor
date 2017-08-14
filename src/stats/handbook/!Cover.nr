.if t .po 1i
.if t .ne 1i
.ll 6.5i
.lt 6.5i
.if n .ll 79n
.if n .lt 79n
.nf
.nh
.if t .sp |1i
.if t .vs 30p
.ps 24p
.ft B
.tl '''The |STAT Handbook'
.ft R
.ps 24
.tl '''Data Analysis Programs'
.tl '''on UNIX and MSDOS'

.ps 24
.ft B
.tl '''Gary Perlman'
.ce 0
.if t .vs 12
.ps 11
.ft R
.if t .sp |10.5i
.if n .sp 3
.if t .tl '\(co 1986 Gary Perlman'''
.if t .bp 0
.if t .sp |1i
.if n .sp 3
.nf
.if t \{
UNIX is a trademark of AT&T Bell Laboratories.
|STAT is not a product of any company or organization.

|STAT is used at your own risk.
|STAT should not be used for decision making by non-statisticians.
|STAT is not robust for large datasets or for highly variable data.
|STAT is unsupported, but known bugs are removed.

.fi
This handbook was typeset by the author using the \fBtroff\fR
text formatting system on the UNIX operating system.
The cover design,
also by the author,
shows data flowing though pipes and filters to produce displays,
summaries, and inferences.
.if t .sp |5i
.if n .sp 3
.ce
Dedicated to Caroline.
.if t .sp |7.5i
.if n .sp 3
.\}
.ce
.if t Copyright \(co 1986 Gary Perlman
.if n Copyright 1986 Gary Perlman
.sp 2
.ce 0
.fi
All rights reserved.
No part of this handbook may be reproduced or transmitted
in any form or by any means
without prior written permission from the author.
Non-profit organizations may make copies
provided such copies are not made for resale.
.if t \{

.nf
.ta 1.5i
First Edition:	March 1986
Second Edition:	September 1986
Third Edition:	March 1987

Printed in the United States of America
.\}
.bp
.nf
.sp |1i
.ce 2
.ft B
.ps 14
.vs 18
|STAT Handbook
Table of Contents
.ps
.vs
.ft

.de CH
.sp
.ft B
Chapter \\$1: \\$2
.ft
.nr SH 0
.nr CH \\$1
.ta .5iR .75i 6i
..
.de SH
.nr SH +1
	\\n(SH	\\$1  \\n(CH-\\$2
..
.so contents.nr
