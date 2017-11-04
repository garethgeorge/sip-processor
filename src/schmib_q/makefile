CC = gcc
#INC = -I. -Igmp-4.2.1/include
INC = -I. -Igmp-6.0.0/include
OBJS = jrb.o jval.o fbuff.o simple_input.o ecdf.o weibull.o weibpred.o hyperexp.o hyppred.o autoc.o lognpred.o matlabhack.o logupred.o
SCHMIBOBJS = schmib_history.o schmib_q_sim.o schmib-means.o \
	bmbp_pred.o dllist.o sim_history.o
BMBPOBJS = schmib_history.o bmbp_pred.o schmib_q_sim.o
INCLS = fbuff.h jrb.h jval.h norm.h simple_input.h jbindex.h\
	rare_event.h ecdf.h weibull.h weibpred.h \
	schmib_history.h bmbp_pred.h schmib-means.h sim_history.h
#GMPLIB = gmp-4.2.1/lib/libgmp.a
GMPLIB = gmp-6.0.0/lib/libgmp.a
GMPOBJS = jbindex.o mpf_comb.o rare_event.o
LIBS = ${GMPLIB} ./norm.o -lm
NGLIBS = ./norm.o -lm
CFLAGS = -g $(INC) -O3 -finline-functions -fPIC

all: bmbp_sim glrt_cluster exp_cluster bmbp_ts bmbp_index bmbp_steady cluster_ts\
	 machine_analysis logn_ts

# schmib_sim: schmib_sim.c $(OBJS) $(INCLS) ${GMPLIB} ${GMPOBJS} $(SCHMIBOBJS)\
# 	 ./norm.o ./schmib-glrt.o
# 	rm -f schmib_q_sim.o
# 	${CC} ${CFLAGS} -g -c -DDOWN_TIME schmib_q_sim.c
# 	${CC} ${CFLAGS} -DDOWN_TIME -o schmib_sim schmib_sim.c $(SCHMIBOBJS) $(OBJS) schmib-glrt.o ${GMPOBJS} $(LIBS)
# 	${CC} ${CFLAGS} -g -c schmib_q_sim.c

spruce_sim: spruce_sim.c $(OBJS) $(INCLS) ${GMPLIB} ${GMPOBJS} $(SCHMIBOBJS)\
	 ./norm.o ./schmib-glrt.o
	${CC} ${CFLAGS} -o spruce_sim spruce_sim.c $(SCHMIBOBJS) $(OBJS) schmib-glrt.o ${GMPOBJS} $(LIBS)

bmbp_sim: bmbp_sim.c $(OBJS) $(INCLS) ${GMPLIB} ${GMPOBJS} $(BMBPOBJS)\
	 ./norm.o
	${CC} ${CFLAGS} -o bmbp_sim bmbp_sim.c $(BMBPOBJS) $(OBJS) ${GMPOBJS} $(LIBS)

bmbp_ts: bmbp_ts.c $(INCLS) $(INCLS) ${GMPLIB} ${GMPOBJS} $(BMBPOBJS) norm.o
	${CC} ${CFLAGS} -o bmbp_ts bmbp_ts.c $(BMBPOBJS) $(OBJS) ${GMPOBJS} $(LIBS)

bmbp_index: bmbp_index.c $(INCLS) $(INCLS) ${GMPLIB} ${GMPOBJS} $(BMBPOBJS) norm.o
	${CC} ${CFLAGS} -o bmbp_index bmbp_index.c $(BMBPOBJS) $(OBJS) ${GMPOBJS} $(LIBS)

bmbp_steady: bmbp_steady.c $(INCLS) $(INCLS) ${GMPLIB} ${GMPOBJS} $(BMBPOBJS) norm.o
	${CC} ${CFLAGS} -o bmbp_steady bmbp_steady.c $(BMBPOBJS) $(OBJS) ${GMPOBJS} $(LIBS)

logn_ts: logn_ts.c $(INCLS) $(INCLS) ${GMPLIB} ${GMPOBJS} $(BMBPOBJS) norm.o
	${CC} ${CFLAGS} -o logn_ts logn_ts.c $(BMBPOBJS) $(OBJS) ${GMPOBJS} $(LIBS)

machine_analysis: machine_analysis.c $(INCLS) $(INCLS) ${GMPLIB}\
	 ${GMPOBJS} $(BMBPOBJS) norm.o
	${CC} ${CFLAGS} -o machine_analysis machine_analysis.c $(BMBPOBJS) $(OBJS) ${GMPOBJS} $(LIBS)

glrt_cluster: schmib-glrt.c $(OBJS) $(INCLS) schmib-means.o ${SCHMIBOBJS}
	${CC} ${CFLAGS} -DTEST -o glrt_cluster schmib-glrt.c schmib-means.o jval.o jrb.o dllist.o simple_input.o $(LIBS)

exp_cluster: schmib-means.c $(OBJS) $(INCLS) ${SCHMIBOBJS}
	${CC} ${CFLAGS} -DTEST -o exp_cluster schmib-means.c jval.o jrb.o dllist.o simple_input.o $(NGLIBS)

cluster_ts: schmib-means.o $(OBJS) $(INCLS) cluster_ts.c ${SCHMIBOBJS}
	${CC} ${CFLAGS} cluster_ts.c -o cluster_ts schmib-means.o jval.o jrb.o dllist.o simple_input.o $(NGLIBS)

jrb.o: jrb.c jval.h jrb.h
	${CC} ${CFLAGS} -c jrb.c

jval.o: jval.c jval.h
	${CC} ${CFLAGS} -c jval.c

fbuff.o: fbuff.c fbuff.h
	${CC} ${CFLAGS} -c fbuff.c

norm.o: norm.c norm.h
	${CC} ${CFLAGS} -c norm.c

mpf_comb.o: mpf_comb.c mpf_comb.h
	${CC} ${CFLAGS} -c mpf_comb.c

ecdf.o: ecdf.c ecdf.h
	${CC} ${CFLAGS} -c ecdf.c

schmib_q_sim.o: schmib_q_sim.c schmib_q_sim.h simple_input.h simple_input.o
	${CC} ${CFLAGS} -c schmib_q_sim.c

schmib_history.o: schmib_history.c schmib_history.h simple_input.h
	${CC} ${CFLAGS} -c schmib_history.c

sim_history.o: sim_history.c sim_history.h simple_input.h
	${CC} ${CFLAGS} -c sim_history.c

schmib-means.o: schmib-means.c schmib-means.h simple_input.h
	${CC} ${CFLAGS} -c schmib-means.c

schmib-glrt.o: schmib-glrt.c schmib-means.h simple_input.h\
		schmib-glrt.h
	${CC} ${CFLAGS} -c schmib-glrt.c

dllist.o: dllist.c dllist.h
	${CC} ${CFLAGS} -c dllist.c

jbindex.o: jbindex.c jbindex.h
	${CC} ${CFLAGS} -c jbindex.c

rare_event.o: rare_event.c rare_event.h jbindex.h
	${CC} ${CFLAGS} -c rare_event.c

simple_input.o: simple_input.c simple_input.h
	${CC} ${CFLAGS} -c simple_input.c

${GMPLIB}:
#	tar -xf gmp-4.2.1.tar
	tar -xf gmp-6.0.0a.tar
	cd gmp-6.0.0; ./configure --prefix=`pwd` --disable-shared; make install
clean:
	rm -f *.o
