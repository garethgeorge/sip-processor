CC=gcc
MPATH=../mio
ECPATH=../euca-cutils
IPATHS=-I${MPATH} -I${ECPATH}
INC=${MPATH}/mio.h ${MPATH}/mymalloc.h ${ECPATH}/dlist.h ${ECPATH}/textlist.h
CFLAGS=-g ${IPATHS}
LPATHS=-L${ECPATH}
UOBJ=${MPATH}/mio.o ${MPATH}/mymalloc.o ${ECPATH}/libutils.a

all: spot-price-aggregate

spot-price-aggregate: spot-price-aggregate.c ${INC} ${UOBJ} 
	${CC} ${CFLAGS} -o spot-price-aggregate spot-price-aggregate.c ${UOBJ}

clean:
	rm -f *.o spot-price-aggregate


