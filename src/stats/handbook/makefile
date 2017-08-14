# Makefile for printing the |STAT Handbook

CHAPTERS=\
	!Cover.nr \
	0-Preface.nr \
	1-Introduction.nr \
	2-Example.nr \
	3-Conventions.nr \
	4-Manipulation.nr \
	5-Analysis.nr \
	6-Manuals.nr

POSTSCRIPT=\
	!Cover.ps \
	0-Preface.ps \
	1-Introduction.ps \
	2-Example.ps \
	3-Conventions.ps \
	4-Manipulation.ps \
	5-Analysis.ps \
	6-Manuals.ps

TROFF=atroff         # whatever is used locally to troff
NROFF=/usr/bin/nroff # whatever is used locally to nroff

.SUFFIXES: .ps .nr

.nr.ps:
	${TROFF} macros $< > $@

all: postscript handbook.txt

postscript: ${POSTSCRIPT}

.PRECIOUS: contents.nr exam.dat DM.nr Calc.nr

# Each chapter is printed separately, with its own pagination
runoff: 
	${TROFF} !Cover.nr                # includes contents.nr, but no logo
	${TROFF} macros 0-Preface.nr      
	${TROFF} macros 1-Introduction.nr 
	${TROFF} macros 2-Example.nr      # includes exam.dat
	${TROFF} macros 3-Conventions.nr  
	${TROFF} macros 4-Manipulation.nr # includes DM.nr
	${TROFF} macros 5-Analysis.nr     # includes Calc.nr
	${TROFF} macros 6-Manuals.nr      # (append alphabetized man pages)

contents.nr: $(CHAPTERS)
	cat $(CHAPTERS) | egrep "^\.[CS]H" > $@

handbook.txt: macros.txt !Cover.txt $(CHAPTERS)
	${NROFF} macros.txt !Cover.txt $(CHAPTERS) | unctrl > handbook.txt

clean:
	/bin/rm -f handbook.txt *.ps
