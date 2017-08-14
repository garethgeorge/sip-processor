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



char *ARGS = "f:LT:ECe";
int UseLow;
double Threshold;
int DoCensored = 1;
int Empht;
int UseCorrect;
int UseEqual;

char *Usage = "pred-duration -f filename\n\
\t-E <produce empht output>\n\
\t-e <use less than equal to>\n\
\t-L <use low>\n\
\t-C <use correct>\n\
\t-T minimum threshold\n";

int main(int argc,char **argv)
{

	char fname[255];
	int c;
	int ierr;
	MIO *data;
	MIO *d_mio;
	unsigned long size;
	RB *expiring;
	RB *censored;
	RB *rb;
	double now;
	unsigned long i;
	int f;
	double *series;
	double ts;
	double value;
	double pred;
	double oldpred;
	int found;
	double duration;
	unsigned long recs;
	double max_censored;
	double last_ts;
	double incr;

	UseLow = 0;
	Threshold = 0;
	Empht = 0;
	UseCorrect = 0;
	UseEqual = 0;
	while((c = getopt(argc,argv,ARGS)) != EOF)
	{
		switch(c)
		{
			case 'E':
				Empht = 1;
				break;
			case 'e':
				UseEqual = 1;
				break;
			case 'C':
				UseCorrect = 1;
				break;
			case 'f':
				strcpy(fname,optarg);
				break;
			case 'L':
				UseLow = 1;
				break;
			case 'T':
				Threshold = atof(optarg);
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

	expiring = RBInitD();
	if(expiring == NULL) {
		exit(1);
	}

	censored = RBInitD();
	if(censored == NULL) {
		exit(1);
	}

	series = (double *)MIOAddr(d_mio);

	last_ts = 0;
	incr = 0.1;
	for(i=0; i < recs; i++) {
		now = series[i*f+0];
#if 0
		if(now == last_ts) {
			now = now + incr;
			incr = incr + 0.1;
		} else {
			incr = 0.1;
		}
#endif
		value = series[i*f+1];
		pred = series[i*f+2];
		rb = RB_FIRST(expiring);
		while(rb != NULL) {
			found = 0;
			oldpred = K_D(rb->key);
			ts = rb->value.d; 
			if(UseLow == 0) {
				if((UseEqual == 0) && (value > oldpred)) {
					duration = now - ts;
					found = 1;
				} else if((UseEqual == 1) && 
					(value >= oldpred)) {
					duration = now - ts;
					found = 1;
				}
			} else {
				if((UseEqual == 0) && (value < oldpred)) {
					duration = now - ts;
					found = 1;
				} else if((UseEqual == 1) &&
						(value <= oldpred)) {
					duration = now - ts;
					found = 1;
				}
			}
			if(found == 1) {
				if(Empht == 0) {
					printf("%10.1f %10.0f\n",now,duration);
				} else {
					printf("1 %f %10.1f\n",duration,now);
				}
				RBDeleteD(expiring,rb);
				rb = RB_FIRST(expiring);
			} else {
				rb = rb->next;
			}
		}
		if(UseCorrect == 1) {
			if((UseLow == 0) && (pred >= value)) {
				RBInsertD(expiring,pred,(Hval)now);
			} else if ((UseLow == 1) && (pred <= value)) {
				RBInsertD(expiring,pred,(Hval)now);
			}
		} else {
			RBInsertD(expiring,pred,(Hval)now);
		}
		last_ts = series[i*f+0];;
	}

	/*
 	 * handle those still on the expiring list.  if they haven't expired yet just count them as ending
 	 * now (subject to the threshold)
 	 */
	RB_FORWARD(expiring,rb) {
		ts = rb->value.d;
		RBInsertD(censored,ts,(Hval)NULL);
	}
	max_censored = -1;
	RB_FORWARD(censored,rb) {
		ts = K_D(rb->key);
		duration = now - ts;	
		if(max_censored == -1) {
			max_censored = duration;
		}
		if(duration > Threshold) {
			if(Empht == 0) {
				printf("%10.1f %10.0f\n",now,duration);
			} else {
				printf("0 %f %10.1f\n",duration,now);
			}
		}
	}

#if 0
	max_censored = 0;
	RB_FORWARD(expiring,rb) {
		ts = rb->value.d;
		duration = now - ts;	
		if(duration > max_censored) {
			max_censored = duration;
		}
	}
	printf("%10.0f %f\n",now,max_censored);
#endif
	RBDestroyD(expiring);
	RBDestroyD(censored);
	MIOClose(d_mio);

	return(0);
}
				
			 
