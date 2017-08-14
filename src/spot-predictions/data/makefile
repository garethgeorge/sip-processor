CC=gcc
MPATH=../../mio
ECPATH=../../euca-cutils
IPATHS=-I${MPATH} -I${ECPATH}
INC=${MPATH}/mio.h ${MPATH}/mymalloc.h ${ECPATH}/dlist.h ${ECPATH}/textlist.h
CFLAGS=-g ${IPATHS}
LPATHS=-L${ECPATH}
UOBJ=${MPATH}/mio.o ${MPATH}/mymalloc.o ${ECPATH}/libutils.a

all: an-file-test find-no-dropout

an-file-test: an-file.c ${INC} ${UOBJ} 
	${CC} ${CFLAGS} -DSTANDALONE -o an-file-test an-file.c ${UOBJ}

find-no-dropout: find-no-dropout.c ${INC} ${UOBJ} an-file.o
	${CC} ${CFLAGS} -o find-no-dropout find-no-dropout.c an-file.o ${UOBJ}

an-file.o: an-file.c an-file.h ${INC}
	${CC} ${CFLAGS} -c an-file.c

clean:
	rm -f *.o an-file-test find-no-dropout


