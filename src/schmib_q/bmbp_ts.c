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
int UseCDF;
int Training;
int Window;
int UseLow;
double InvertValue;
double Conditional;
int Durations;
double Interval;


int Recovery;

#define RAND() (drand48())
#define SEED() (srand48())

#define ARGS "f:c:C:q:vX:TDP:J:M:whrt:W:LI:i:E"
char *Usage = "bmbp_ts -f filename\n\
\t-c confidence level\n\
\t-C conditional time value\n\
\t-D <values are durations>\n\
\t-E <use Empirical CDF instead of binomial method -- implies NoTrim>\n\
\t-q quantile\n\
\t-i implied interval value (last measurement persists)\n\
\t-I value <inverted value to use>\n\
\t-L use low\n\
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
	double ds;
	double last_ts;
	double last_value;
	double value;
	int min_history;
	void *bp;
	int fields;
	double *values;
	double lowp;
	double highp;
	double remaining;
	int no_update;

	Quantile = 0.95;
	Confidence = 0.05;
	Verbose = 0;
	UseTrim = 0;
	Window = 0;
	Interval = 0;
	UseCDF = 0;
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
	    case 'C':
	      Conditional = atof(optarg);
	      break;
	    case 'D':
	      Durations = 1;
	      break;
	    case 'E':
	      UseCDF = 1;
	      break;
	    case 'I':
	      InvertValue = atof(optarg);
	      break;
	    case 'i':
		Interval = atoi(optarg);
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
	    case 'L':
		UseLow = 1;
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
		if(UseLow == 0) {
			min_history = MinHistory(Quantile,Confidence);
		} else {
			min_history = MinHistory(1.0-Quantile,1.0-Confidence);
		}
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

	if(UseCDF == 1) {
		SetUseCDFBMBP(bp);
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
		} else {
			ts = curr_count+1;
		}

		/*
		 * walk forward by intervals from last ts if Interval
		 * specified
		 */
		if((last_ts != -1) && (Interval != 0)) {
			for(ds=last_ts+Interval; ds < ts; ds = ds + Interval) {
				UpdateBMBPPred(bp,
				   ds,last_value,low_pred_value,pred_value);
	    			ForcBMBPPred(bp,&low_pred_value,&pred_value);
				if(pred_value >= last_value)
				{
					high_success++;
				}
				if(low_pred_value <= last_value)
				{
					low_success++;
				}
				total++;
	      			fprintf(stdout,"itime: %10.0f ",ds);
	      			fprintf(stdout,"value: %f ",last_value);
				if(UseLow == 1) {
	      				fprintf(stdout,
						"pred: %f ",low_pred_value);
				} else {
	      				fprintf(stdout,"pred: %f ",pred_value);
				}
	      			fprintf(stdout,"count: %d ",SizeOfBMBPPred(bp));
				if(UseLow == 1) {
	      				fprintf(stdout,"success: %f ",low_success);
				} else {
	      				fprintf(stdout,"success: %f ",high_success);
				}
	      			fprintf(stdout,"total: %f ",total);
				if(UseLow == 1) {
					fprintf(stdout,"percent: %f\n",
						low_success/total);
				} else {
					fprintf(stdout,"percent: %f\n",
						high_success/total);
				}
				if(InvertValue != 0) {
					fprintf(stdout,"lowp: %f\n",lowp);
					fprintf(stdout,"highp: %f\n",highp);
				}
	      			fprintf(stdout,"\n");
			}
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
		if(Conditional != 0) {
			if(value < Conditional) {
				continue;
			} else {
				if(Durations == 1) {
					remaining = value-Conditional;
					value = remaining;
				}
			}
		}
		if((curr_count > min_history) && (curr_count > Training))
		{
			if(InvertValue != 0) {
				InvForcBMBPPred(bp,InvertValue,&lowp,&highp);
			}
	    		ForcBMBPPred(bp,&low_pred_value,&pred_value);
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
			if(UseLow == 1) {
	      			fprintf(stdout,"pred: %f ",low_pred_value);
			} else {
	      			fprintf(stdout,"pred: %f ",pred_value);
			}
	      		fprintf(stdout,"count: %d ",SizeOfBMBPPred(bp));
			if(UseLow == 1) {
	      			fprintf(stdout,"success: %f ",low_success);
			} else {
	      			fprintf(stdout,"success: %f ",high_success);
			}
	      		fprintf(stdout,"total: %f ",total);
			if(UseLow == 1) {
				fprintf(stdout,"percent: %f\n",
						low_success/total);
			} else {
				fprintf(stdout,"percent: %f\n",
						high_success/total);
			}
			if(InvertValue != 0) {
				fprintf(stdout,"lowp: %f\n",lowp);
				fprintf(stdout,"highp: %f\n",highp);
			}
	      		fprintf(stdout,"\n");
		}
		curr_count++;
		UpdateBMBPPred(bp,ts,value,low_pred_value,pred_value);
		last_ts = ts;
		last_value = value;
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
	free(values);
	exit(0);
}

