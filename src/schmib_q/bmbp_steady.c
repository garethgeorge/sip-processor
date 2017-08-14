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

double Quantile;
double Confidence;
int Verbose;
int UseTrim;
int Training;
int Window;
int UseLow;


int Recovery;

#define RAND() (drand48())
#define SEED() (srand48())

#define ARGS "f:c:q:vX:TDP:J:M:wlhrt:W:"
char *Usage = "bmbp_stead -f filename\n\
\t-c confidence level\n\
\t-q quantile\n\
\t-X cache_file <acceleration cache file>\n\
\t-T use_trimming\n\
\t-t training count\n\
\t-W window (to take one per window size)\n\
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
	double last_ts;
	double value;
	int min_history;
	void *bp_high;
	void *bp_low;
	int fields;
	double *values;
	double dummy;

	Quantile = 0.95;
	Confidence = 0.05;
	Verbose = 0;
	UseTrim = 0;
	Window = 0;
	memset(fname,0,sizeof(fname));
	memset(cname,0,sizeof(cname));
	UseLow = 0;
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
	    case 'W' :
		Window = atoi(optarg);
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

	fields = GetFieldCount(fname);
	if(fields <= 0) {
		fprintf(stderr,"couldn't get field count\n");
		exit(1);
	}

	err = InitDataSet(&data,fields);
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

	bp_high = InitBMBPPred(Quantile,Confidence,J_FIELDS);
	if(bp_high == NULL)
	{
		exit(1);
	}
	bp_low = InitBMBPPred(Quantile,Confidence,J_FIELDS);
	if(bp_low == NULL)
	{
		exit(1);
	}
	SetUseLowBMBP(bp_low);

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
		SetNoTrimBMBP(bp_high);
		SetNoTrimBMBP(bp_low);
	}

	high_success = 0.0;
	low_success = 0.0;
	pred_value = 0.0;	
	total = 0.0;

	curr_count = 0;
	values = (double *)malloc(fields*sizeof(double));
	if(values == NULL) {
		exit(1);
	}
	last_ts = -1;
	while(ReadData(data,fields,values))
	{
		value = values[fields-1];
	  	if (value == 0.0) 
		{
	    		value = 1.0;
	  	}
		if(fields > 1) {
			ts = values[0];
		}
		/*
		 * if we are taking only one per window size
		 */
		if(Window > 0) {
			if((ts - Window) < last_ts) {
				continue;
			}
			last_ts = ts;
		} 
		if((curr_count > min_history) && (curr_count > Training))
		{
	    		ForcBMBPPred(bp_high,&dummy,&pred_value);
	    		ForcBMBPPred(bp_low,&low_pred_value,&dummy);
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
	      		fprintf(stdout,"high_pred: %f ",pred_value);
	      		fprintf(stdout,"low_pred: %f ",low_pred_value);
	      		fprintf(stdout,"count: %d ",SizeOfBMBPPred(bp_high));
	      		fprintf(stdout,"success: %f ",high_success);
	      		fprintf(stdout,"total: %f ",total);
			fprintf(stdout,"high_percent: %f\n",high_success/total);
			fprintf(stdout,"low_percent: %f\n",low_success/total);
	      		fprintf(stdout,"\n");
		}
		curr_count++;
		UpdateBMBPPred(bp_high,ts,value,low_pred_value,pred_value);
		UpdateBMBPPred(bp_low,ts,value,low_pred_value,pred_value);
	}
	
	fprintf(stdout,"high_success_percentage: %f\n",
			high_success / total);
	fprintf(stdout,"low_success_percentage: %f\n",
			low_success / total);
	free(values);
	exit(0);
}

