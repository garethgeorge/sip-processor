CC=gcc
MPATH=../mio
EPATH=../euca-cutils
JPATH=../schmib_q
GPATH=${JPATH}/gmp-6.0.0
INC=${MPATH}/mio.h ${MPATH}/mymalloc.h ${EPATH}/redblack.h ${JPATH}/jbindex.h
CFLAGS=-g -I${MPATH} -I${EPATH} -I${JPATH} -I${GPATH}/include -L${GPATH}/lib

LIBS=${MPATH}/mymalloc.o ${MPATH}/mio.o ${EPATH}/redblack.o

all: pred-duration in-a-row pred-distribution pred-distribution-fast

pred-duration: pred-duration.c ${INC} ${LIBS}
	${CC} ${CFLAGS} -o pred-duration pred-duration.c ${LIBS}

pred-distribution: pred-distribution.c ${INC} ${LIBS}
	${CC} ${CFLAGS} -o pred-distribution pred-distribution.c ${LIBS} ${JPATH}/jbindex.o ${JPATH}/norm.o ${JPATH}/mpf_comb.o -lgmp -lm

pred-distribution-fast: pred-distribution-fast.c ${INC} ${LIBS}
	${CC} ${CFLAGS} -o pred-distribution-fast pred-distribution-fast.c ${LIBS} ${JPATH}/jbindex.o ${JPATH}/norm.o ${JPATH}/mpf_comb.o -lgmp -lm

in-a-row: in-a-row.c ${INC} ${LIBS}
	${CC} ${CFLAGS} -o in-a-row in-a-row.c ${LIBS}

clean:
	rm *.o pred-duration in-a-row
