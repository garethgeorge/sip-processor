CC=gcc
MPATH=../../mio
MEANPATH=../../meanvar
AUTOCPATH=../../autoc
DPATH=../../distributions
APATH=../../matrix
LPATH=${APATH}/lapack-3.5.0
FPATH=/usr/local/Cellar/gcc/4.9.2_1/lib/gcc/4.9/
LAPACK=${LPATH}/liblapacke.a ${LPATH}/liblapack.a ${LPATH}/librefblas.a ${LPATH}/libtmglib.a
IPATHS=-I/usr/local/include -I${MPATH} -I${MEANPATH} -I${AUTOCPATH} -I${APATH} -I${DPATH}
ABOXLIB=
ABOXINC=
INC=${MPATH}/mio.h ${MPATH}/mymalloc.h ${MEANPATH}/meanvar.h ${AUTOCPATH}/autoc.h ${APATH}/mioregress.h ${APATH}/mioarray.h
CFLAGS=-g ${IPATHS} 
LAPATHS=-L/usr/local/lib -L${LPATH} -L${FPATH}
LAPACK=-llapacke -llapack -lrefblas -ltmglib -lgfortran

OBJS=${MPATH}/mymalloc.o ${MPATH}/mio.o ${MEANPATH}/meanvar.o ${AUTOCPATH}/autoc.o ${DPATH}/normal.o ${APATH}/mioregress.o ${APATH}/mioarray.o

all: split-data

split-data: split-data.c ${INC} ${OBJS}
	${CC} ${CFLAGS} ${LAPATHS} -o split-data split-data.c ${OBJS}

clean:
	rm *.o
