/****************************************************************/
/*                                                              */
/*      Global include files                                    */
/*                                                              */
/****************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include "mio.h"
#include "hval.h"
#include "redblack.h"

#include "jbindex.h"

#define ENDF 0.0

char *ARGS = "Ef:F:I:q:c:V";
double Frac;
double Interval;
double Quantile;
double Confidence;
int UseCDF;
int Verbose;

void PrintDistribution(RB *distribution,
		double pred, double q, double c)
{
	int i;
	int j;
	double incr;
	RB *rb;
	int ndx;
	double ratio;
	double over;
	int lsize;
	double total;

	total = 0;
	lsize = 0;
	RB_FORWARD(distribution,rb) {
		lsize++;
		if(Verbose == 1) {
			if(Interval > 0) {
				over = pred+(rb->value.d*pred);
			} else {
				over = pred-(rb->value.d*pred);
			}
			total += over;
		}
			
	}
	if(UseCDF == 0) {
		ndx = JBIndex(lsize,q,c,&ratio);
	} else {
		ndx = (int)(lsize * q);
	}
	j = 0;
	RB_FORWARD(distribution,rb) {
		if(ndx == j) {
			/*
			 * incr carried in the value
			 */
			if(Interval > 0) {
				over = pred+(rb->value.d*pred);
			} else {
				over = pred-(rb->value.d*pred);
			}
			if(Verbose == 1) {
				printf("%f %f avg: %f count: %d\n",K_D(rb->key),
						    over,
						    total / (double)lsize,
						    lsize);
			} else {
				printf("%f %f\n",K_D(rb->key),over);
			}
			break;
		}
		j++;
	}

	return;
}
		



char *Usage = "pred-distribution -f filename\n\
\t-E <use empirical CDF>\n\
\t-F maximum fraction of DAFTS min\n\
\t-I interval between points in distribution\n\
\t-q quantile\n\
\t-c confidence\n";

int main(int argc,char **argv)
{

	char fname[255];
	int c;
	int ierr;
	MIO *data;
	MIO *d_mio;
	unsigned long size;
	RB *expiring;
	int ecount;
	unsigned long i;
	int j;
	RB *distribution;
	RB *rb;
	double now;
	double *series;
	double ts;
	double value;
	double pred;
	double oldpred;
	int found;
	double duration;
	unsigned long recs;
	double incr;
	int f;
	double over;
	double interv;
	double last_value;

	fname[0] = 0;

	Frac = 0.0;
	Interval = 0.1;
	UseCDF=0;
	while((c = getopt(argc,argv,ARGS)) != EOF)
	{
		switch(c)
		{
			case 'E':
				UseCDF=1;
				break;
			case 'c':
				Confidence = atof(optarg);
				break;
			case 'q':
				Quantile = atof(optarg);
				break;
			case 'F':
				Frac = atof(optarg);
				break;
			case 'f':
				strcpy(fname,optarg);
				break;
			case 'I':
				Interval = atof(optarg);
				break;
			case 'V':
				Verbose = 1;
				break;
			default:
				fprintf(stderr,"unrecognized argument %c\n",
						c);
				fprintf(stderr,"%s",Usage);
				fflush(stderr);
				break;
		}
	}

	if(fname[0] == 0)
	{
		fprintf(stderr,"%s",Usage);
		fflush(stderr);
		exit(1);
	}

	if(Interval > 0) {
		interv = Interval;
	} else {
		interv = -1 * Interval;
	}
	ecount = (int)((Frac+interv) / Interval) + 1;
	if(ecount < 0) {
		ecount = ecount * -1;
	}

	if(ecount <= 0) {
		fprintf(stderr,"bad fraction or interval: f: %f i: %f\n",
			Frac,Interval);
		fprintf(stderr,"%s",Usage);
		exit(1);
	}

	if((Confidence <= 0) || (Confidence > 1.0)) {
		fprintf(stderr,"bad confidence: %f",Confidence);
		fprintf(stderr,"%s",Usage);
		exit(1);
	}

	if((Quantile <= 0) || (Quantile > 1.0)) {
		fprintf(stderr,"bad quantile: %f",Quantile);
		fprintf(stderr,"%s",Usage);
		exit(1);
	}

	size = MIOSize(fname);
	data = MIOOpenText(fname,"r",size);
	if(data == NULL) {
		fprintf(stderr,"autoc: couldn't init data for file %s\n",
				fname);
		fflush(stderr);
		exit(1);
	}

	f = MIOTextFields(data);
	if(f != 3) {
		fprintf(stderr,
		"file %s must have 'timestamp value prediction' format\n",
			fname);
		MIOClose(data);
		exit(1);
	}
	recs = MIOTextRecords(data);
	d_mio = MIODoubleFromText(data,NULL);
	if(d_mio == NULL) {
		MIOClose(data);
		exit(1);
	}
	MIOClose(data);

	series = (double *)MIOAddr(d_mio);

	last_value = 99999;
	
	for(incr=0.0; incr <= Frac; incr += Interval) {
		expiring = RBInitD();
		if(expiring == NULL) {
			exit(1);
		}
		distribution = RBInitD();
		if(distribution == NULL) {
			exit(1);
		}
		for(i=0; i < recs; i++) {
			now = series[i*f+0];
			value = series[i*f+1];
			pred = series[i*f+2];
			rb = RB_FIRST(expiring);
			while(rb != NULL) {
				found = 0;
				oldpred = K_D(rb->key);
				ts = rb->value.d; 
				if(value >= oldpred) {
					duration = now - ts;
					found = 1;
				} else {
					break;
				}
				if(found == 1) {
					RBInsertD(distribution,
						duration, (Hval)incr);
					RBDeleteD(expiring,rb);
					rb = RB_FIRST(expiring);
				} else {
					rb = rb->next;
				}
			}
			over = pred+(pred*incr);
			RBInsertD(expiring,over,(Hval)now);
		}
		/*
		 * handle the remaining ones on the expiring list
		 */
		RB_FORWARD(expiring,rb) {
			duration = now - rb->value.d;
			RBInsertD(distribution,duration,(Hval)incr);
		}
		PrintDistribution(distribution,pred,Quantile,Confidence);
		RBDestroyD(expiring);
		RBDestroyD(distribution);
	}
			
	MIOClose(d_mio);

	return(0);
}
				
			 
