#define FULL_NITS
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

#include "schmib_q_sim.h"
#include "bmbp_pred.h"
#include "rare_event.h"
#include "schmib_pred.h"
#include "weibpred.h"
#include "hyppred.h"
#include "lognpred.h"

double Quantile;
double Confidence;
int Verbose;
int UseTrim;
int Mode;
int UseGLRT;
int UseDown;
int UseLog = 1;

int Recovery;

extern int Max_bin;
extern double Last_job_out;
extern double Last_job_ts;
double Last_down_high;
double Last_down_low;
double Prev_job_ts;

double Low_queued;
double High_queued;


#define RAND() (drand48())
#define SEED() (srand48())

#define ARGS "f:c:q:vX:TDP:J:M:wlurE:B:GdL"
char *Usage = "brevik_q_sim -f filename\n\
\t-c confidence level\n\
\t-B max_bins\n\
\t -E {1,2,3} <1=rtime, 2=procs, 3=rtime*proc>\n\
\t-q quantile\n\
\t-G get GLRTy\n\
\t-L use log transform\n\
\t-X cache_file <acceleration cache file>\n\
\t-T use_trimming\n\
\t-v <verbose>\n\
\t-D use distributions\n\
\t-d use downtime\n\
\t-P percentage of time for training/trimming\n\
\t-r in addition to trim, train with initial data\n\
\t-J number of jobs to skip in the beginning *do not use*\n\
\t-M number of days to trim off *do not use*\n\
\t-w use weibull in dist mode\n\
\t-l use log normal in dist mode\n\
\t-h use hyper 2 in dist mode\n";

void *Dt;
void *St;

void UpdateDownTime(double ts, double value)
{

	/*
	 * if we are doing partial nits, then just return
	 */
#ifndef FULL_NITS
	return;
#endif
	if(UseDown == 0)
		return;

	UpdateBMBPPred(Dt,ts,value,Last_down_low,Last_down_high);

	return;
}

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
	double exp_value;
	int distmode, trimtrain, start;
	double rtime;
	double procs;
	double new_ts;
	double new_value;
	double dummy_1, dummy_2;	/* for SPRUCE -- not here */
	int down_count = 0;
	int down = 0;	/* will change if UseDown set */
	double total_down_time = 0.0;
	double down_ts;
	int skip_count = 0;
	int latest_queued = 0;
	int queued_count = 0;

	trimtrain = 0;
	distmode = 0;
	UseDist = 0;
	UseGLRT = 0;
	Quantile = 0.95;
	Confidence = 0.05;
	Verbose = 0;
	UseTrim = 0;
	UseDown = 0;
	ptrim = 0.0;
	jtrim = 0;
	mtrim = 0.0;
	Mode = 1;	/* rtime by default */
	UseLog = 0;
	memset(fname,0,sizeof(fname));
	memset(cname,0,sizeof(cname));

	while((c = getopt(argc,argv,ARGS)) != EOF)
	{
	  switch(c)
	    {
	    case 'B':
		Max_bin = atoi(optarg);
		break;
	    case 'E':
		Mode = atoi(optarg);
		break;
	    case 'G':
		UseGLRT = 1;
		break;
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
	    case 'd':
	      UseDown = 1;
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
	    case 'L':
		UseLog = 1;
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

	if((Mode < 1) || (Mode > 3))
	{
		fprintf(stderr,"bad mode\n");
		fprintf(stderr,"%s",Usage);
		fflush(stderr);
		exit(1);
	}

	if(cname[0] != 0)
	{
		InitJBCache(cname,0.95,0.05);
	}

	err = InitDataSet(&data,4);
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

	bp = InitSchmibPred(Quantile,Confidence,J_FIELDS);
	if(bp == NULL)
	{
		exit(1);
	}

	/*
	if (UseDist) {
	  ((SchmibPred *)bp)->min_history = 20;
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
		SetNoTrimSchmib(bp);
	}

	queue = InitQsim(1.0,J_FIELDS);
	if(queue == NULL)
	{
		exit(1);
	}

	if(UseDown == 1)
	{
#ifdef FULL_NITS
		Dt = InitBMBPPred(0.99,0.05,J_FIELDS);
		if(Dt == NULL)
		{
			exit(1);
		}
#endif
		St = InitBMBPPred(0.99,0.05,J_FIELDS);
		if(St == NULL)
		{
			exit(1);
		}
	}
	else
	{
		Dt = NULL;
		St = NULL;
	}

	/* go through data to find first and last timestamp */
	first_time = 0;
	last_time = 0;
	while(ReadEntry4(data, &new_ts, &value, &procs, &rtime)) 
	{
		if(value <= 0)
		{
			continue;
		}
		if(Mode == 1)
		{
			exp_value = rtime;
		} else if (Mode == 2)
		{
			exp_value = procs;
		}
		else
		{
			exp_value = rtime*procs;
		}
	  if (first_time == 0) {
	    first_time = new_ts;
	  }
	  last_time = new_ts;
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

	while(ReadEntry4(data,&new_ts,&new_value, &procs, &rtime))
	{
		if(new_value <= 0)
		{
			continue;
		}
		if(Mode == 1)
		{
			exp_value = rtime;
		} else if (Mode == 2)
		{
			exp_value = procs;
		}
		else
		{
			exp_value = rtime*procs;
		}
	  if((mtrim != 0.0) && (new_ts <= mdeadline))
	  {
		continue;
	  }
	  if (new_value == 0.0) {
	    new_value = 1.0;
	  }
	  curr_count++;
	  /*
	   * stub out last two -- needed for SPRUCE */
	  JobArrives(queue,new_ts,new_value,new_value,new_value,exp_value,0,0);
	  curr_count = GetHistorySize(queue->completed);
          /*
	   * need to do this after JobArrives since it uses value to determine
	   * when a job pops out fo the queue
	   */
	  //	  if(curr_count >= min_history && curr_count >= (ptrim * size))
	  //  if(curr_count >= min_history && curr_count >= jtrim)
	  //	  printf("primin %f\n", value);
//	  if(curr_count >= min_history && new_ts >= mdeadline)
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
					&value,&lowpred,&highpred,&exp_value,
					&dummy_1,&dummy_2);
		if(err == 0)
		{
			fprintf(stderr,
				"can't get history entry for index %d when priming\n",i);
			fflush(stderr);
			exit(1);
		}
		if((UseDown == 0) || (down == 0))
		{
			/*
			 * if working with logs, convert just before updating
			 * state
			 */
			if(UseLog)
			{
				value = log(value);
				lowpred = log(lowpred);
				highpred = log(highpred);
			}
			UpdateSchmibPred(bp,ts,value,lowpred,highpred,exp_value);
		}
	}

//printf("BT rebin: %f\n",ts);
	RebinSchmib((SchmibPred *)bp);
#if 0
	for(i=0; i < ((SchmibPred *)bp)->bin_count; i++)
	{
		PrintBin(stdout,((SchmibPred *)bp)->bins[i],
			J_EXP,J_VALUE,i);
	}
#endif

	((SchmibPred *)bp)->wrong_count = 
		ConsecutiveWrong(((SchmibPred *)bp)->data, &oldautoc);

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
	l_mse_right = 0;
	l_mse_wrong = 0;

	pred_value = 0.0;	
	doit = 0;
	predcount = 0;

	autofuse = 0;

	while(ReadEntry4(data,&new_ts,&new_value,&procs,&rtime))
	{
		if(new_value <= 0)
		{
			continue;
		}
		if(Mode == 1)
		{
			exp_value = rtime;
		} else if (Mode == 2)
		{
			exp_value = procs;
		}
		else
		{
			exp_value = rtime*procs;
		}
		if (value == 0.0) 
		{
	    		value = 1.0;
	  	}

		/*
		if (UseDist) 
		{
			if (predcount > 
				GetHistorySize(((SchmibPred *)bp)->data)) 
			{
				predcount = 0;
				doit = 0;
				pred_value = 0.0;
			}
			predcount = GetHistorySize(((SchmibPred *)bp)->data);
	    
			if (predcount >= 20) 
			{
				if (predcount >= 100) 
				{
					if (doit == 2) 
					{
						doit = 1;
					} 
					else 
					{
						doit = 3;
					}
				} 
				else 
				{
					if (doit == 0) 
					{
						doit = 1;
					} 
					else 
					{
						doit = 2;
					}
				}
			} 
			if(distmode == 0) 
			{
				WeibullPred(((SchmibPred *)bp)->data, 
					Quantile, doit, &pred_value);
				if (pred_value == -1.0) 
				{
					printf("ERROR doing weibpred\n");
					exit(1);
				}
			} 
			else if (distmode == 1) 
			{
				HyperPred(((SchmibPred *)bp)->data, 
						Quantile, doit, &pred_value);
			} 
			else if (distmode == 2) 
			{
				LognPred(((SchmibPred *)bp)->data, 
						Quantile, doit, &pred_value);
			}
		}
		else 
		*/

		/*
		 * now, update the sim state
		 *
		 * get the index of oldest thing the predictor has seen
		 */
		oldest_index = GetHistorySize(queue->completed);
		UpdateQsimState(queue,new_ts);
		for(i=oldest_index+1; 
		    i <= GetHistorySize(queue->completed); i++)
		  {
		    err = GetHistoryEntry(queue->completed,i,&ts,
					  &value,&lowpred,&highpred,&exp_value,
					  &dummy_1,&dummy_2);
		    if(err == 0)
		      {
			fprintf(stderr,
				"can't get history entry for pred\n");
			fflush(stderr);
			exit(1);
		      }
		    if(UseLog)
		      {
			value = log(value);
			lowpred = log(lowpred);
			highpred = log(highpred);
		      }
		    UpdateSchmibPred(bp,ts,value,lowpred,
				     highpred,exp_value);
		  }
		
		  if (UseDist) 
		    {
		      
		      if (distmode == 0) 
			{
			  ForcWeibullPred(bp,exp_value,&low_pred_value,&pred_value);
			} 
		      else if (distmode == 1) 
			{
			  ForcLoguPred(bp,exp_value,&low_pred_value,&pred_value);
			}
		      else if (distmode == 2) 
			{
			  ForcLognPred(bp,exp_value,&low_pred_value,&pred_value);
			}
		    } 
		  else 
		{
		  ForcSchmibPred(bp,exp_value,&low_pred_value,&pred_value);
		}
		  if(UseLog)
		    {
		      low_pred_value = exp(low_pred_value);
		      pred_value = exp(pred_value);
		    }
//printf("MACHINE: bins: %d\n",
//	((SchmibPred *)bp)->bin_count);
////fflush(stdout);
			
		if ((GetHistorySize(((SchmibPred *)bp)->data) % 100) == 0) 
		{
	    		((SchmibPred *)bp)->wrong_count = 
			    ConsecutiveWrong(((SchmibPred *)bp)->data, &autoc);
	    		oldautoc = autoc;
	  	}
	  
		if(down == 0)
		{
			if(pred_value >= new_value)
			{
				success++;
				mse_right += pow(fabs(pred_value - new_value), 2);
				l_mse_right +=
					pow(fabs(log(pred_value) -
							log(new_value)), 2);
			} 
			else 
			{
				mse_wrong += pow(fabs(pred_value - new_value), 2);
				l_mse_wrong +=
					pow(fabs(log(pred_value) -
							log(new_value)), 2);
			}

			total++;
			total_err += fabs(pred_value - new_value);
			if(Verbose)
			{
				fprintf(stdout,"value: %f ",new_value);
				fprintf(stdout,"pred: %f ",pred_value);
				fprintf(stdout,"count: %d ",SizeOfSchmibPred(bp));
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
				fprintf(stdout,"procs: %f\n",procs);
				fprintf(stdout,"\n");
			}
		}
		else
		{
			skip_count++;
		}
	  
	  /*
	   * now, update the sim state
	   *
	   * get the index of oldest thing the predictor has seen
	   */
	  	oldest_index = GetHistorySize(queue->completed);
	  /*
	   * crank the simulator
	   */
#ifdef FULL_NITS
		if((UseDown == 1) && (down_count >= 59))
		{
			ForcBMBPPred(Dt,&Last_down_low,&Last_down_high);
		}
#endif

		if((UseDown == 1) && (queued_count >= 59))
		{
			ForcBMBPPred(St,&Low_queued,&High_queued);
		}
		/*
		 * here we are still in non log space -- need to convert
		 * back after job arrives since timig is determined by
		 * value
		 */
		JobArrives(queue,new_ts,new_value,low_pred_value,pred_value,
		exp_value,0,0);
		if((UseDown == 1) && (down_count > 59) && (down == 0))
		{
//			fprintf(stdout,"MACHINE: %f %f\n",
//				Last_job_out,
//				Last_down_high);
//			fflush(stdout);
//			if((new_ts - Last_job_ts) > Last_down_high) 
			if(Prev_job_ts == Last_job_ts)
			{
				latest_queued++;
			}
			else
			{
				if(latest_queued > 0)
				{
//printf("MACHINE adding %f\n",(double)latest_queued);
				  fflush(stdout);
					UpdateBMBPPred(St,
						new_ts,
						(double)latest_queued,
						Low_queued,
						High_queued);
					queued_count++;
				}
				latest_queued = 0;
			}
			Prev_job_ts = Last_job_ts;
#ifdef FULL_NITS
			if( ((queued_count < 59) && (latest_queued > 50)) ||
				((queued_count >= 59) &&
				    ((double)latest_queued > High_queued)) ||
				((down_count >= 59) && 
					(Last_job_out > Last_down_high)))
#else
			if( ((queued_count < 59) && (latest_queued > 50)) ||
				((queued_count >= 59) &&
				    ((double)latest_queued > High_queued)) )
#endif
			{
//				fprintf(stdout,
//				"MACHINE DOWN at %10.0f for %f ",
//					Last_job_ts,
//					Last_job_out);
//				fprintf(stdout,"MACHINE (%f %f)\n",
//					new_ts - Last_job_ts, Last_down_high);
				if(Verbose)
				{
				fprintf(stdout,
		"MACHINE DOWN queued: %d high: %f count: %d skipped: %d\n",
					latest_queued,
					High_queued,
					queued_count,
					skip_count);
				}
				down_ts = Last_job_ts;
//				FlushPendingQueue(queue);
				down = 1;
			}
		}
		if(UseDown == 1)
		{
			down_count++;

			if((down == 1) && (down_ts < Last_job_ts))
			{
				down = 0;
				if(Verbose)
				{
				fprintf(stdout,
				"MACHINE UP at %10.0f skipped : %d\n",
					Last_job_ts,
					skip_count);
				}
				total_down_time += (Last_job_ts - down_ts);
			}

		}	

#if 0
printf("AFTER ARRIVE\n");
PrintBins(((SchmibPred *)bp)->bins, ((SchmibPred *)bp)->bin_count,J_EXP,J_VALUE);
#endif
	  /*
	   * OBSOLETE
	   * this next piece of code was here when we didn't do a
	   * UpdateQsimState() before the forecast.  Now it happens
	   * just before the forecast is generated and here, it never
	   * has an effect since the only change in state is that
	   * a job has been added to the pending queue by
	   * JobArrives
	   */
		for(i=oldest_index+1; 
			i <= GetHistorySize(queue->completed); i++)
		{
			err = GetHistoryEntry(queue->completed,i,&ts,
			&value,&lowpred,&highpred,&exp_value,
			&dummy_1,&dummy_2);
			if(err == 0)
			{
				  fprintf(stderr,
			"can't get history entry for pred\n");
				  fflush(stderr);
				  exit(1);
			}
			if(UseLog)
			{
				value = log(value);
				lowpred = log(lowpred);
				highpred = log(highpred);
			}
			UpdateSchmibPred(bp,ts,value,lowpred,
					highpred,exp_value);
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
	fprintf(stdout,"logRMS wrong: %f\n", sqrt(l_mse_wrong / (total - success)));
	fprintf(stdout,"RMS total: %f\n", sqrt((mse_right + mse_wrong) / total));
	fprintf(stdout,"logRMS total: %f\n", 
				sqrt((l_mse_right + l_mse_wrong) / total));
	if(UseDown == 1)
	{
		fprintf(stdout,"%d jobs skipped due to machine down\n",
				skip_count);
		fprintf(stdout,"total_down_time: %f\n",total_down_time);
	}

	exit(0);
}

