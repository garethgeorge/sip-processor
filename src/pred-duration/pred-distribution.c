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

char *ARGS = "f:F:I:q:c:V";
double Frac;
double Interval;
double Quantile;
double Confidence;
int Verbose;

void PrintDistribution(RB **distributions, int dsize, 
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

	for(i=0; i < dsize; i++) {
		total = 0;
		lsize = 0;
		RB_FORWARD(distributions[i],rb) {
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
		ndx = JBIndex(lsize,q,c,&ratio);
		j = 0;
		RB_FORWARD(distributions[i],rb) {
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
	}

	return;
}
		



char *Usage = "pred-distribution -f filename\n\
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
	RB **expiring;
	int ecount;
	unsigned long i;
	int j;
	RB **distributions;
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
	while((c = getopt(argc,argv,ARGS)) != EOF)
	{
		switch(c)
		{
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

	expiring = malloc(ecount*sizeof(RB *));
	if(expiring == NULL) {
		exit(1);
	}
	distributions = malloc(ecount*sizeof(RB *));
	if(distributions == NULL) {
		exit(1);
	}

	j=0;
	if(Interval > 0) {
		for(incr=0.0; incr <= Frac; incr += Interval) {
			expiring[j] = RBInitD();
			if(expiring[j] == NULL) {
				exit(1);
			}
			distributions[j] = RBInitD();
			if(distributions[j] == NULL) {
				exit(1);
			}
			j++;
		}
	} else {
		for(incr=Frac; incr > ENDF; incr += Interval) {
			expiring[j] = RBInitD();
			if(expiring[j] == NULL) {
				exit(1);
			}
			distributions[j] = RBInitD();
			if(distributions[j] == NULL) {
				exit(1);
			}
			j++;
		}
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
	for(i=0; i < recs; i++) {
		now = series[i*f+0];
		value = series[i*f+1];
		pred = series[i*f+2];
		j = 0;
		if(Interval > 0) {
			for(incr=0.0; incr <= Frac; incr += Interval) {
				rb = RB_FIRST(expiring[j]);
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
						RBInsertD(distributions[j],
							duration, (Hval)incr);
						RBDeleteD(expiring[j],rb);
						rb = RB_FIRST(expiring[j]);
					} else {
						rb = rb->next;
					}
				}
				over = pred+(pred*incr);
				RBInsertD(expiring[j],over,(Hval)now);
				j++;
			}
		} else {
			for(incr=Frac; incr > ENDF; incr += Interval) {
				rb = RB_FIRST(expiring[j]);
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
						RBInsertD(distributions[j],
							duration, (Hval)incr);
						RBDeleteD(expiring[j],rb);
						rb = RB_FIRST(expiring[j]);
					} else {
						rb = rb->next;
					}
				}
				over = pred-(pred*incr);
				if((over >= 0) && (over >= last_value)) {
					RBInsertD(expiring[j],over,(Hval)now);
				}
				j++;
			}
		}
		last_value = value;
	}

	/*
 	 * handle those still on the expiring list.  if they haven't expired yet just count them as ending
 	 * now (subject to the threshold)
 	 */
	j = 0;
	if(Interval > 0) {
		for(incr=0.0; incr <= Frac; incr += Interval) {
			RB_FORWARD(expiring[j],rb) {
				duration = now - rb->value.d;
				RBInsertD(distributions[j],duration,(Hval)incr);
			}
			j++;
		}
	} else {
		for(incr = Frac; incr > ENDF; incr += Interval) {
			RB_FORWARD(expiring[j],rb) {
				duration = now - rb->value.d;
				RBInsertD(distributions[j],duration,(Hval)incr);
			}
			j++;
		}
	}

	PrintDistribution(distributions,j,pred,Quantile,Confidence);

	j=0;
	if(Interval > 0) {
		for(incr=0.0; incr <= Frac; incr += Interval) {
			RBDestroyD(expiring[j]);
			RBDestroyD(distributions[j]);
			j++;
		}
	} else {
		for(incr=Frac; incr > ENDF; incr += Interval) {
			RBDestroyD(expiring[j]);
			RBDestroyD(distributions[j]);
			j++;
		}
	}
	free(expiring);
	free(distributions);
	MIOClose(d_mio);

	return(0);
}
				
			 
