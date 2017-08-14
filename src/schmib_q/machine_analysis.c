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
int UseLow;
int TotalProcs = 100;
double StartTime = 0;
double EndTime;
int Mode = 0; /* 0 == measure procs, 1 == node-secs */


int Recovery;

#define RAND() (drand48())
#define SEED() (srand48())

#define ARGS "f:c:q:vX:TDP:J:M:wlhrt:"
char *Usage = "bmbp_ts -f filename\n\
\t-c confidence level\n\
\t-q quantile\n\
\t-X cache_file <acceleration cache file>\n\
\t-T use_trimming\n\
\t-t training count\n\
\t-P total procs\n\
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
	double wait_time;
	double procs;
	double max_exec;
	double exec_time;
	double total_ns;
	double job_ns;	/* job node-secs */
	double small_job_ns;
	double big_job_ns;
	double total_job_ns;
	int min_history;
	void *bp;
	void *low_bp;
	void *high_bp;
	double mach_frac;
	double upper_mach_frac;
	double total_frac;
	double value;
	double low_wait_small;
	double low_wait_big;
	double high_wait_small;
	double high_wait_big;

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
	    case 'P':
		TotalProcs = atof(optarg);
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

	err = InitDataSet(&data,5);
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
	low_bp = InitBMBPPred(0.95,0.05,J_FIELDS);
	if(low_bp == NULL)
	{
		exit(1);
	}
	high_bp = InitBMBPPred(0.95,0.05,J_FIELDS);
	if(high_bp == NULL)
	{
		exit(1);
	}

	if(UseLow == 1) {
		SetUseLowBMBP(bp);
		SetUseLowBMBP(low_bp);
		SetUseLowBMBP(high_bp);
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
	small_job_ns = 0;
	big_job_ns = 0;
	total_job_ns = 0.0;
	mach_frac = 0.0;
	upper_mach_frac = 0.0;
	total_frac = 0.0;
	while(ReadEntry5(data,&ts,&wait_time,&procs,&max_exec,&exec_time))
	{
		switch(Mode)
		{
			case 0:
				value = procs;
				break;
			case 1:
				value = procs * exec_time;
				break;
			default:
				fprintf(stderr,"bad mode: %d\n",
					Mode);
				exit(1);
				break;
		}
	  	if (value == 0.0) 
		{
	    		value = 1.0;
	  	}
		if((curr_count > min_history) && (curr_count > Training))
		{
			job_ns = procs * exec_time;
	    		ForcBMBPPred(bp,&low_pred_value,&pred_value);
	    		ForcBMBPPred(low_bp,&low_wait_small,
				&high_wait_small);
	    		ForcBMBPPred(high_bp,&low_wait_big,
				&high_wait_big);
			if(pred_value >= value)
			{
				high_success++;
				small_job_ns += job_ns;
			}
			else
			{
				big_job_ns += job_ns;
			}
			total_job_ns += job_ns;
			if(low_pred_value <= value)
			{
				low_success++;
			}
			total++;
			if(StartTime == 0)
			{
				StartTime = ts+exec_time;
			}
			else
			{
			  mach_frac = 
			   small_job_ns / ((((ts+exec_time) - StartTime) * TotalProcs) * 1.0);
			  upper_mach_frac = 
			   big_job_ns / ((((ts+exec_time) - StartTime) * TotalProcs) * 1.0);
			  total_frac = total_job_ns /
				((((ts+exec_time) - StartTime) * TotalProcs) * 1.0);
			}
			if((mach_frac < 1.0) && (mach_frac > 0.0))
			{
	      		fprintf(stdout,"time: %10.0f ",ts);
			fprintf(stdout,"mach_frac: %f ",mach_frac);
			fprintf(stdout,"upper_frac: %f ",upper_mach_frac);
	      		fprintf(stdout,"value: %f ",value);
	      		fprintf(stdout,"pred: %f ",pred_value);
	      		fprintf(stdout,"count: %d ",SizeOfBMBPPred(bp));
	      		fprintf(stdout,"success: %f ",high_success);
	      		fprintf(stdout,"total: %f ",total);
			fprintf(stdout,"percent: %f ",high_success/total);
			fprintf(stdout,"total_frac: %f  ",total_frac);
			fprintf(stdout,"small_wait: %f  ",high_wait_small);
			fprintf(stdout,"big_wait: %f  ",high_wait_big);
	      		fprintf(stdout,"\n");
			}
		}
		curr_count++;
		UpdateBMBPPred(bp,ts,value,lowpred,highpred);
		if(pred_value >= value)
		{
			UpdateBMBPPred(low_bp,ts,wait_time,
				low_wait_small,high_wait_small);
		}
		else
		{
			UpdateBMBPPred(high_bp,ts,wait_time,
				low_wait_big,high_wait_big);
		}
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

