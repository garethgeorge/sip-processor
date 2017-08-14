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
#include "weibpred.h"
#include "hyppred.h"
#include "lognpred.h"

double Quantile;
double Confidence;
int Verbose;
int UseTrim;

int Recovery;

#define RAND() (drand48())
#define SEED() (srand48())

#define ARGS "f:c:q:vX:TDP:J:M:wlurL"
char *Usage = "bmbp_sim -f filename\n\
\t-c confidence level\n\
\t-q quantile\n\
\t-X cache_file <acceleration cache file>\n\
\t-T use_trimming\n\
\t-L use log mode\n\
\t-v <verbose>\n\
\t-D use distributions\n\
\t-P percentage of time for training/trimming\n\
\t-r in addition to trim, train with initial data\n\
\t-J number of jobs to skip in the beginning *do not use*\n\
\t-M number of days to trim off *do not use*\n\
\t-w use weibull in dist mode\n\
\t-l use log normal in dist mode\n\
\t-u use log uniform in dist mode\n";


int main(int argc, char *argv[])
{
	int c;
	char fname[255];
	char cname[255];
	int curr_count;
	int size, autofuse;
	History *h;
	double success;
	double total;
	double total_err;
	double pred_value,low_pred_value,lowpred,highpred;
	int err;
	void *data;
	double ts;
	double value;
	double new_ts;
	double new_value;
	double ratio;
	double rare_count;
	Qsim *queue;
	int min_history;
	int i, doit, predcount;
	int oldest_index, UseDist;
	void *bp;
	double autoc, oldautoc;
	double ptrim;
	int jtrim;
	double mtrim;
	double mdeadline;
	double mse_right, mse_wrong;
	double l_mse_right, l_mse_wrong;
	double first_time, last_time, pcent;
	int distmode, trimtrain, start;
	int UseLog;

	trimtrain = 0;
	distmode = 0;
	UseDist = 0;
	Quantile = 0.95;
	Confidence = 0.05;
	Verbose = 0;
	UseTrim = 0;
	ptrim = 0.0;
	jtrim = 0;
	mtrim = 0.0;
	UseLog = 0;
	memset(fname,0,sizeof(fname));
	memset(cname,0,sizeof(cname));

	while((c = getopt(argc,argv,ARGS)) != EOF)
	{
	  switch(c)
	    {
	    case 'w':
	      distmode = 0;
	      break;
	    case 'u':
	      distmode = 1;
	      break;
	    case 'l':
	      distmode = 2;
	      break;
	    case 'D':
	      UseDist=1;
	      break;
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
	    case 'L':
	      UseLog = 1;
	      break;
	    case 'X':
	      strncpy(cname,optarg,sizeof(cname));
	      break;
	    case 'P':
	      ptrim = atof(optarg);
	      break;
	    case 'J':
	      jtrim = atoi(optarg);
	      break;
	    case 'v':
	      Verbose = 1;
	      break;
	    case 'M':
	      mtrim = atof(optarg) * 86400.0;
	      break;
	    case 'r':
	      trimtrain = 1;
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

	size = SizeOf(data);

	bp = InitBMBPPred(Quantile,Confidence,J_FIELDS);
	if(bp == NULL)
	{
		exit(1);
	}

	/*
	if (UseDist) {
	  ((BMBPPred *)bp)->min_history = 20;
	  min_history = 20;
	} else {
	  min_history = MinHistory(Quantile,Confidence);
	}
	*/
	min_history = MinHistory(Quantile,Confidence);

	/*
	 * turn off trimming if needed
	 */
	if(UseTrim == 0)
	{
		SetNoTrimBMBP(bp);
	}

	queue = InitQsim(1.0,J_FIELDS);
	if(queue == NULL)
	{
		exit(1);
	}

	/* go through data to find first and last timestamp */
	first_time = 0;
	last_time = 0;
	while(ReadEntry(data, &ts, &value)) {
	  if (first_time == 0) {
	    first_time = ts;
	  }
	  last_time = ts;
	}
	Rewind(data);

	/*
	 * prime the pump to at least 59 completed jobs
	 */
	if (ptrim != 0.0) {
	  mdeadline = first_time + ( (last_time - first_time) * ptrim );
	} else if(mtrim != 0.0)
	{
	    /* stole the -M option so that we only trim, no train */
	    mdeadline = first_time + mtrim;
	}

	while(ReadEntry(data,&ts,&value))
	{
	  if((mtrim != 0.0) && (ts <= mdeadline))
	  {
		continue;
	  }
	  if (value == 0.0) {
	    value = 1.0;
	  }
	  curr_count++;
	  JobArrives(queue,ts,value,value,value,1.0);
	  curr_count = GetHistorySize(queue->completed);
	  //	  if(curr_count >= min_history && curr_count >= (ptrim * size))
	  //  if(curr_count >= min_history && curr_count >= jtrim)
	  //	  printf("primin %f\n", value);
//	  if(curr_count >= min_history && ts >= mdeadline)
	  if(curr_count >= 1000)
	    {
	      break;
	    }
	  
	}
	

	if (trimtrain || (mtrim != 0.0)) {
	  start = 1;
	} else {
	  start = (GetHistorySize(queue->completed) - min_history + 1);
	}
	for(i=start; i <= GetHistorySize(queue->completed); i++)

	{
		err = GetHistoryEntry(queue->completed,i,&ts,
				&value,&lowpred,&highpred,NULL,NULL,NULL);
		if(err == 0)
		{
			fprintf(stderr,
				"can't get history entry for index %d when priming\n",i);
			fflush(stderr);
			exit(1);
		}
		if(UseLog)
		{
			value = log(value);
		}
		UpdateBMBPPred(bp,ts,value,value,value);
	}

	((BMBPPred *)bp)->wrong_count = ConsecutiveWrong(((BMBPPred *)bp)->data, &oldautoc);

	if(curr_count < min_history)
	{
		fprintf(stderr,"not enough data to prime the pump\n");
		fflush(stderr);
		exit(1);
	}

	/*
	 * here is the main loop
	 */
	success = 0.0;
	total = 0.0;
	rare_count = 0.0;
	mse_right = 0.0;
	mse_wrong = 0.0;
	l_mse_right = 0.0;
	l_mse_wrong = 0.0;

	pred_value = 0.0;	
	doit = 0;
	predcount = 0;

	autofuse = 0;

	while(ReadEntry(data,&new_ts,&new_value))
	{

	  if (new_value == 0.0) {
	    new_value = 1.0;
	  }

	  /*
	   * record the last thing the predictor has seen
	   */
	  oldest_index = GetHistorySize(queue->completed);
	  /*
	   * time has advanced
	   *
	     * move any jobs through the queues that need to be moved
	     */
	  UpdateQsimState(queue,new_ts);
	  /*
	   * now update the predictor data
	   */
	  for(i=oldest_index+1; i <= GetHistorySize(queue->completed); 
	      i++)
	    {
	      err = GetHistoryEntry(queue->completed,i,&ts,&value,
				    &lowpred,&highpred,NULL,
				    NULL,NULL);
	      if(err == 0)
		{
		  fprintf(stderr,"can't get history entry for pred\n");
		  fflush(stderr);
		  exit(1);
		}
	      /*
	       * convert again since value is reset in GetHistoryEntry()
	       * call
	       */
	      if(UseLog)
		{
		  value = log(value);
		  lowpred = log(lowpred);
		  highpred = log(highpred);
		}
	      UpdateBMBPPred(bp,ts,value,lowpred,highpred);
	    }
	  
	  if (UseDist) {
	    if (distmode == 0) {
	      ForcWeibullNoBinsPred(bp,&low_pred_value,&pred_value);
	    } else if (distmode == 1) {
	      ForcLoguNoBinsPred(bp,&low_pred_value,&pred_value);
	    } else if (distmode == 2) {
	      ForcLognNoBinsPred(bp,&low_pred_value,&pred_value);
	    }
	  } else {
	    ForcBMBPPred(bp,&low_pred_value,&pred_value);
	  }

	  if(UseLog)
	    {
	      low_pred_value = exp(low_pred_value);
	      pred_value = exp(pred_value);
	    }
	  
	  if ((GetHistorySize(((BMBPPred *)bp)->data) % 100) == 0) {
	    ((BMBPPred *)bp)->wrong_count = ConsecutiveWrong(((BMBPPred *)bp)->data, &autoc);
	    oldautoc = autoc;
	  }
	  
	  if(pred_value >= new_value)
	    {
	      success++;
	      mse_right += pow(fabs(pred_value - new_value), 2);
	      l_mse_right += pow(fabs(log(pred_value) - log(new_value)), 2);
	    } else {
	      mse_wrong += pow(fabs(pred_value - new_value), 2);
	      l_mse_wrong += pow(fabs(log(pred_value) - log(new_value)), 2);
	    }

	  total++;
	  total_err += fabs(pred_value - new_value);
	  if(Verbose)
	    {
	      fprintf(stdout,"value: %f ",new_value);
	      fprintf(stdout,"pred: %f ",pred_value);
	      fprintf(stdout,"count: %d ",SizeOfBMBPPred(bp));
	      fprintf(stdout,"success: %f ",success);
	      fprintf(stdout,"rare: %f ",rare_count);
	      fprintf(stdout,"total: %f ",total);
	      fprintf(stdout,"total_err: %f ",total_err);
	      if(total > 0.0)
		{
		  fprintf(stdout,"success percent: %f ",success/total);
		  fprintf(stdout,"rare percent: %f ",rare_count/total);
		}
	      else
		{
		  fprintf(stdout,"success percent: %f ",0.0);
		  fprintf(stdout,"rare percent: %f ",0.0);
		}
	      fprintf(stdout,"time: %f ",new_ts);
	      fprintf(stdout,"\n");
	    }

	  
	  /*
	   * now, update the sim state
	   *
	   * get the index of oldest thing the predictor has seen
	   */
	  oldest_index = GetHistorySize(queue->completed);
	  /*
	   * crank the simulator (last two not needed here -- SPRUCE)
	   */
	  JobArrives(queue,new_ts,new_value,low_pred_value,pred_value,success/total);
	  /*
	   * add any newly completed jobs to the brevik pred
	   */
	  for(i=oldest_index+1; i <= GetHistorySize(queue->completed); 
	      i++)
	    {
	      err = GetHistoryEntry(queue->completed,i,&ts,&value,
						&lowpred,&highpred,NULL,
						NULL,NULL);
	      if(err == 0)
		{
		  fprintf(stderr,"can't get history entry for pred\n");
		  fflush(stderr);
		  exit(1);
		}
	      /*
	       * convert again since value is reset in GetHistoryEntry()
	       * call
	       */
	      if(UseLog)
	      {
		value = log(value);
		lowpred = log(lowpred);
		highpred = log(highpred);
	      }
	      UpdateBMBPPred(bp,ts,value,lowpred,highpred);
	    }
	}
	
	fprintf(stdout,"total_preds: %f\n",total);
	if(total == 0.0)
	  {
		exit(1);
	}
	fprintf(stdout,"success_percentage: %f\n",
			success / total);
	fprintf(stdout,"total_err: %e\n",total_err);
	fprintf(stdout,"RMS right: %f\n", sqrt(mse_right / success));
	fprintf(stdout,"RMS wrong: %f\n", sqrt(mse_wrong / (total - success)));
	fprintf(stdout,"logRMS right: %f\n", sqrt(l_mse_right / success));
        fprintf(stdout,"logRMS wrong: %f\n", 
		sqrt(l_mse_wrong / (total - success)));        
	fprintf(stdout,"RMS total: %f\n", sqrt((mse_right + mse_wrong) /
			total));
        fprintf(stdout,"logRMS total: %f\n", 
			sqrt((l_mse_right + l_mse_wrong) / total));
	exit(0);
}

