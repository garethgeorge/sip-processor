/*
 * http://www.swox.com/gmp/manual
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "jval.h"
#include "jrb.h"
#include "simple_input.h"

#include "norm.h"

#include "bmbp_q_sim.h"
#include "rare_event.h"
#include "bmbp_pred.h"

void LognPred(History *h, double quantile, int doit, double *ret);

double Quantile;
double Confidence;
int Verbose;
int UseTrim;
int Training;
int UseLow;


int Recovery;

#define RAND() (drand48())
#define SEED() (srand48())

#define ARGS "f:c:q:vX:TDP:J:M:wlhrt:"
char *Usage = "logn_ts -f filename\n\
\t-c confidence level\n\
\t-q quantile\n\
\t-X cache_file <acceleration cache file>\n\
\t-T use_trimming\n\
\t-t training count\n\
\t-v <verbose>\n";


int main(int argc, char *argv[])
{
	int c;
	char fname[255];
	char cname[255];
	int curr_count;
	double high_success,low_success;
	double total;
	double pred_value,low_pred_value,lowpred,highpred;
	int err;
	void *data;
	double ts;
	double value;
	int min_history;
	void *bp;
	History *hist;

	Quantile = 0.95;
	Confidence = 0.05;
	Verbose = 0;
	UseTrim = 0;
	memset(fname,0,sizeof(fname));
	memset(cname,0,sizeof(cname));
	UseLow = 1;
	Training = 0;

	while((c = getopt(argc,argv,ARGS)) != EOF)
	{
	  switch(c)
	    {
	    case 'f':
	      strncpy(fname,optarg,sizeof(fname));
	      break;
	    case 'q':
	      Quantile = atof(optarg);
	      break;
	    case 'c':
	      Confidence = atof(optarg);
	      break;
	    case 'T':
	      UseTrim = 1;
	      break;
	    case 'X':
	      strncpy(cname,optarg,sizeof(cname));
	      break;
	    case 'v':
	      Verbose = 1;
	      break;
	    case 't':
		Training = atoi(optarg);
		break;
	    default:
	      fprintf(stderr,
		      "unrecognized command %c\n",(char)c);
	      fprintf(stderr,"%s",Usage);
	      exit(1);
	    }
	}

	if(fname[0] == 0)
	{
		fprintf(stderr,"must specify file name\n");
		fprintf(stderr,"%s",Usage);
		exit(1);
	}

	if(cname[0] != 0)
	{
		InitJBCache(cname,0.95,0.05);
	}

	err = InitDataSet(&data,2);
	if(err == 0)
	{
		exit(1);
	}
	err = LoadDataSet(fname,data);
	if(err == 0)
	{
		exit(1);
	}

	curr_count = 0;

	bp = InitBMBPPred(Quantile,Confidence,J_FIELDS);
	if(bp == NULL)
	{
		exit(1);
	}
	if(UseLow == 1) {
		SetUseLowBMBP(bp);
	}

	if(Training == 0)
	{
		min_history = MinHistory(Quantile,Confidence);
	}
	else
	{
		min_history = 1;
	}

	/*
	 * turn off trimming if needed
	 */
	if(UseTrim == 0)
	{
		SetNoTrimBMBP(bp);
	}

	high_success = 0.0;
	low_success = 0.0;
	pred_value = 0.0;	
	total = 0.0;

	curr_count = 0;
	while(ReadEntry(data,&ts,&value))
	{
	  	if (value == 0.0) 
		{
	    		value = 1.0;
	  	}
		if((curr_count > min_history) && (curr_count > Training))
		{
			hist = ((BMBPPred *)bp)->data;
	    		LognPred(hist,Quantile,0,&pred_value);
			if(pred_value >= value)
			{
				high_success++;
			}
			if(low_pred_value <= value)
			{
				low_success++;
			}
			total++;
	      		fprintf(stdout,"time: %10.0f ",ts);
	      		fprintf(stdout,"value: %f ",value);
	      		fprintf(stdout,"pred: %f ",pred_value);
	      		fprintf(stdout,"count: %d ",SizeOfBMBPPred(bp));
	      		fprintf(stdout,"success: %f ",high_success);
	      		fprintf(stdout,"total: %f ",total);
			fprintf(stdout,"percent: %f\n",high_success/total);
	      		fprintf(stdout,"\n");
		}
		curr_count++;
		UpdateBMBPPred(bp,ts,value,lowpred,highpred);
	}
	
	if(UseLow == 0)
	{
		fprintf(stdout,"success_percentage: %f\n",
			high_success / total);
	}
	else
	{
		fprintf(stdout,"success_percentage: %f\n",
			low_success / total);
	}
	exit(0);
}

