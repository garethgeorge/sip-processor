##########################################################################
#                            MAKEFILE
#=========================================================================
#	This is the makefile for |STAT data analysis programs.
#	There are macros at the beginning of this file.  Mostly they
#	help you customize.  The # comment character is placed
#	immediately after the value to avoid unwanted spaces.
##########################################################################
#                            WARNINGS
#=========================================================================
# With my version of gcc,
#      the -O option caused incorrect results with anova.
#      you may need to compile with the -bsd option added to CFLAGS
##########################################################################

DESTDIR=../bin#                               destination directory
DESTDIR=../../../bin#                            destination directory
CFLAGS =-O#                                   C Compiler Options
LINK   =$(CC) $(CFLAGS) -o $(DESTDIR)/$@#     Generic link command
LIBS   =-lm#                                  libraries needed (math)
EDITOR =/usr/ucb/vi#                          editor to call on make edit
RM     =/bin/rm -f#                           remove forcefully
MV     =/bin/mv#                              move/rename files
MAKE   =/usr/bin/make#                            use for recursive makes

MAINS = abut.c anova.c calc.y colex.c contab.c desc.c dm.y dprime.c \
	dsort.c features.c ff.c fpack.c linex.c maketrix.c oneway.c \
	pair.c perm.c probdist.c rankind.c rankrel.c ranksort.c \
	regress.c repeat.c reverse.c series.c stats.c transpose.c \
	ts.c validata.c

PROGS = abut anova calc colex contab desc dm dprime \
	dsort features ff fpack linex maketrix oneway \
	pair perm probdist rankind rankrel ranksort \
	regress repeat reverse series stats transpose \
	ts validata
# add cat to the above list if you do not have one on your system

UTILS = barplot.c centile.c chisq.c chitest.c cistrcmp.c confirm.c cor.c \
	f.c fisher.c fiveplot.c getopt.c getword.c mdmat.c \
	number.c numcmp.c numline.c ordstat.c \
	parseline.c primes.c prodlist.c random.c readlines.c setint.c setreal.c \
	skipnumber.c specol.c strings.c tmpfile.c z.c

all:
	@/bin/echo "Making all the |STAT Programs"
	@/bin/echo "Using compiler=$(CC) with options=$(CFLAGS)"
	@/bin/echo "Installing in directory=$(DESTDIR)"
	@$(MAKE) -i $(PROGS)

edit:
	@$(EDITOR) $(MAINS)

clean:
	@-$(RM) *.o y.tab.* core a.out *.P *.X *mon.out cscope.out $(PROGS)

archive: makefile $(UTILS) $(MAINS)
	archive
	touch archive

.PRECIOUS: makefile $(UTILS) $(MAINS)


ABUT=abut.o number.o getopt.o setint.o
abut: $(ABUT)
	$(LINK) $(ABUT)

ANOVA=anova.o parseline.o number.o f.o tmpfile.o getopt.o numcmp.o errplot.o numline.o setint.o
anova: $(ANOVA)
	$(LINK) $(ANOVA) $(LIBS)

CALC=calc.o skipnumber.o
calc: $(CALC)
	$(LINK) $(CALC) $(LIBS)
calc.c: calc.y
	yacc calc.y
	$(MV) y.tab.c calc.c

cat:
	$(LINK) -DCAT filter.c

COLEX=colex.o parseline.o number.o getopt.o specol.o
colex: $(COLEX)
	$(LINK) $(COLEX)

CONTAB=contab.o number.o tmpfile.o parseline.o getopt.o chisq.o z.o \
	fisher.o prodlist.o primes.o numcmp.o setint.o
contab: $(CONTAB)
	$(CC) $(CFLAGS) -DI_DATA -c mdmat.c
	$(LINK) $(CONTAB) mdmat.o $(LIBS)
contab.o:
	$(CC) $(CFLAGS) -DI_DATA -c contab.c

DESC=desc.o getopt.o f.o setreal.o number.o getword.o centile.o
desc: $(DESC)
	$(LINK) $(DESC) $(LIBS)

DM=dm.o strings.o number.o random.o confirm.o skipnumber.o
dm: $(DM)
	$(LINK) $(DM) $(LIBS)
# if dm is compiled without the -DPTREE option, then there will be
# no facility for printing the parsings of expressions and dm will be smaller.
dm.o: dm.c
	$(CC) $(CFLAGS) -DPTREE -c dm.c
dm.c: dm.y
	yacc dm.y
	$(MV) y.tab.c dm.c

DPRIME=dprime.o number.o z.o parseline.o getopt.o 
UNTESTED = "This program has not been rigorously tested"
dprime: $(DPRIME)
	$(LINK) $(DPRIME) $(LIBS)
	@echo "dprime:" $(UNTESTED)

DSORT=dsort.o getopt.o number.o parseline.o specol.o \
	numcmp.o readlines.o setint.o cistrcmp.o readmatrix.o
dsort: $(DSORT)
	$(LINK) $(DSORT)

FEATURES=features.o getopt.o setint.o number.o
features: $(FEATURES)
	$(LINK) $(FEATURES)

FF=ff.o number.o getopt.o setint.o filter.o fls.o
ff: $(FF)
	$(LINK) $(FF)

FPACK=fpack.o filter.o getopt.o
fpack: $(FPACK)
	$(LINK) $(FPACK)

LINEX=linex.o number.o getopt.o specol.o readlines.o
linex: $(LINEX)
	$(LINK) $(LINEX)

MAKETRIX=maketrix.o number.o getword.o getopt.o
maketrix: $(MAKETRIX)
	$(LINK) $(MAKETRIX)

ONEWAY=oneway.o number.o getopt.o getword.o f.o \
	setint.o setreal.o numline.o
oneway: $(ONEWAY)
	$(LINK) $(ONEWAY) $(LIBS)

PAIR=pair.o getopt.o setint.o setreal.o number.o \
	f.o parseline.o numline.o
pair: pair.o $(PAIR)
	$(LINK) $(PAIR) $(LIBS)

PERM=perm.o random.o number.o getopt.o setint.o \
	readlines.o numcmp.o cistrcmp.o
perm: $(PERM)
	$(LINK) $(PERM)

PROBDIST=probdist.o z.o chisq.o f.o number.o parseline.o random.o \
	getopt.o setint.o binomial.o prodlist.o primes.o
probdist: $(PROBDIST)
	$(LINK) $(PROBDIST) $(LIBS)

RANKIND=rankind.o getword.o centile.o getopt.o setreal.o number.o \
	fisher.o prodlist.o primes.o chisq.o z.o setint.o fiveplot.o \
	numline.o ranksort.o ordstat.o chitest.o
rankind: $(RANKIND)
	$(LINK) $(RANKIND) $(LIBS)

RANKREL=rankrel.o parseline.o getopt.o number.o setint.o binomial.o \
	centile.o prodlist.o primes.o chisq.o z.o ranksort.o ordstat.o cor.o f.o
rankrel: $(RANKREL)
	$(LINK) $(RANKREL) $(LIBS)

RANKSORT=setint.o number.o parseline.o getopt.o 
ranksort: $(RANKSORT)
	$(CC) $(CFLAGS) -c -DRANKSORT ranksort.c
	$(LINK) ranksort.o $(RANKSORT)
	-$(RM) ranksort.o

REGRESS=regress.o parseline.o number.o f.o setint.o getopt.o 
regress: $(REGRESS)
	$(LINK) $(REGRESS) $(LIBS)

REPEAT=repeat.o number.o getopt.o setint.o readlines.o filter.o
repeat: $(REPEAT)
	$(LINK) $(REPEAT)

REVERSE=reverse.o getopt.o parseline.o readlines.o
reverse: $(REVERSE)
	$(LINK) $(REVERSE)

SERIES=series.o number.o
series: $(SERIES)
	$(LINK) $(SERIES) $(LIBS)

STATS=stats.o number.o getword.o getopt.o 
stats: $(STATS)
	$(LINK) $(STATS) $(LIBS)

TRANSPOSE=transpose.o parseline.o number.o getopt.o setint.o
transpose: $(TRANSPOSE)
	$(LINK) $(TRANSPOSE)

TS=ts.o setint.o setreal.o number.o cor.o f.o barplot.o getword.o \
	getopt.o numline.o
ts: $(TS)
	$(LINK) $(TS) $(LIBS)

VALIDATA=validata.o parseline.o number.o getopt.o 
validata: $(VALIDATA)
	$(LINK) $(VALIDATA)
