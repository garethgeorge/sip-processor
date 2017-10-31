/*
 * http://www.swox.com/gmp/manual
 */
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "jval.h"
#include "jrb.h"
#include "simple_input.h"

#include "norm.h"

#include "jbindex.h"
#include "bmbp_pred.h"
#include "rare_event.h"
#include "logupred.h"
#include "lognpred.h"
#include "weibpred.h"

#define DEBUG_SAVE_RESTORE 0

void *InitBMBPPred(double quantile, double confidence, int fields)
{
	BMBPPred *p;
	double autoc;

	p = (BMBPPred *)malloc(sizeof(BMBPPred));
	if(p == NULL)
	{
		return(NULL);
	}

	p->data = MakeHistory(fields);
	if(p->data == NULL)
	{
		free(p);
		return(NULL);
	}

	p->low_wrong = 0;
	p->high_wrong = 0;
	/*
	p->wrong_count = ConsecutiveWrong(p->data, &autoc);
	p->right_count = ConsecutiveRight(p->data, &autoc);
	*/
        ConsecutiveWrongRight(p->data,&autoc,&p->wrong_count,&p->right_count);
	/*
	 * FIX
	p->right_count = 99999999;
	 */

	p->quantile = quantile;
	p->confidence = confidence;
	if(p->use_low)
	{
		p->min_history = MinHistory(1.0-quantile,1.0 - confidence);
	}
	else
	{
		p->min_history = MinHistory(quantile,confidence);
	}
	p->recovery = p->min_history+1;
	p->recovery_high = p->right_count;
	p->no_trim = 0;
	p->use_cdf = 0;

	return((void *)p);
}

void SaveBMBPPred(void *ib, FILE *fd)
{
	BMBPPred *p = (BMBPPred *)ib;
#ifdef DEBUG_SAVE_RESTORE 
	// write out debug information / error checking information
	fputc(0x01, fd);
	fprintf(fd, "Begin BMBPPred");
#endif 

	BMBPPred copy;
	memcpy(&copy, p, sizeof(BMBPPred));
	copy.data = NULL;
	fwrite(&copy, sizeof(BMBPPred), 1, fd);

	SaveHistory(p->data, fd);

#ifdef DEBUG_SAVE_RESTORE 
	// write out debug information / error checking information
	fputc(0x01, fd);
	fprintf(fd, "End BMBPPred");
#endif 
}

void *LoadBMBPPred(double quantile, double confidence, int fields, FILE *fd)
{
#ifdef DEBUG_SAVE_RESTORE
	printf("Loading BMBPPred.\n");
	assert(fgetc(fd) == 0x01);
	fseek(fd, sizeof("Begin BMBPPred") - 1, SEEK_CUR);
#endif 

	BMBPPred *p;
	p = (BMBPPred *)malloc(sizeof(BMBPPred));
	if (p == NULL)
	{
		return NULL;
	}
	fread(p, sizeof(BMBPPred), 1, fd);
	
	p->data = LoadHistory(fields, fd);
	
#ifdef DEBUG_SAVE_RESTORE
	assert(fgetc(fd) == 0x01);
	fseek(fd, sizeof("End BMBPPred") - 1, SEEK_CUR);
#endif 
	
	return p;
}

void FreeBMBPPred(void *ib)
{
	BMBPPred *p = (BMBPPred *)ib;

	FreeHistory(p->data);
	free(p);

	return;
}



/*
 * need to do it this way if the values are sorted by the arrival
 * time and not the time they pop out of the pending queue
 */


int IsRareBMBP(BMBPPred *p,
	   double ts, 
	   double value, 
	   double lowpred, 
	   double highpred)
{
	JRB jr;
	JRB start_jr;
	int highcount;
	int lowcount;
	double t;
	int ndx;
	double *vals;
	double old_ts;
	int found;
	double old_v;
	double old_hp;
	double old_lp;
	double max_diff;
	double diff;
	double max_ts;
	double total;
	double total_v;
	double count;
	double mean;
	double sd;

	if((p->use_low == 0) && (value <= highpred))
	{
		return(0);
	} else if((p->use_low == 1) && (value >= lowpred)) {
		return(0);
	}

	/*
	 * walk backward in the history list and find the spot
	 */
	found = 0;
	vals = GetFieldValues(p->data->data,J_TS);
	jrb_rtraverse(jr,p->data->age_list)
	{
		ndx = jval_i(jrb_val(jr));
		old_ts = vals[ndx];
		if(old_ts <= ts)
		{
			found = 1;
			break;
		}
	}

	if(found == 0)
	{
		return(0);
	}

	start_jr = jr;

	/*
	 * now count how many misses from this point
	 */
	highcount = 1;
	lowcount = 1;
	max_diff = -1;
	max_ts = -1;
	total = 0;
	count = 0;

	jr = start_jr;
	while(jr != jrb_nil(p->data->age_list))
	{
		ndx = jval_i(jrb_val(jr));
		vals = GetFieldValues(p->data->data,J_TS);
		old_ts = vals[ndx];
		vals = GetFieldValues(p->data->data,J_VALUE);
		old_v = vals[ndx];
		vals = GetFieldValues(p->data->data,J_HIGHPRED);
		old_hp = vals[ndx];
		vals = GetFieldValues(p->data->data,J_LOWPRED);
		old_lp = vals[ndx];

//		if((UseLow == 0) && ((old_v > old_hp) || (old_v < old_lp)))
		if((p->use_low == 0) && (old_v > old_hp))
		{
			highcount++;
			diff = old_v - old_hp;
			if(diff > max_diff) {
				max_diff = diff;
				max_ts = old_ts;
			}
		}
		else if((p->use_low == 1) && (old_v < old_lp))
		{
			diff = old_lp - old_v;
			if(diff > max_diff) {
				max_diff = diff;
				max_ts = old_ts;
			}
			lowcount++;
		}
		else
		{
			break;
		}
		jr = jrb_prev(jr);
	}
		

	if((p->use_low == 0) && (highcount >= p->wrong_count)) {
		total = 0;
		count = 0;
		jr = start_jr;
		vals = GetFieldValues(p->data->data,J_VALUE);
		while(jr != jrb_nil(p->data->age_list))
		{
			ndx = jval_i(jrb_val(jr));
			old_v = vals[ndx];
			total += old_v;
			count++;
			jr = jrb_prev(jr);
		}
		mean = total / count;
		total_v = 0;
		jr = start_jr;
		vals = GetFieldValues(p->data->data,J_VALUE);
		while(jr != jrb_nil(p->data->age_list))
		{
			ndx = jval_i(jrb_val(jr));
			old_v = vals[ndx];
			total_v += (old_v - mean) * (old_v - mean);
			jr = jrb_prev(jr);
		}
		sd = sqrt(total_v / count);
		if(value > (mean + 5*sd)) {
			printf("sd rare: %10.0f %f (%10.0f)\n" ,ts,value,max_ts);
			return(2); /* discard as outlier */
		} else {
			printf("bmbp rare: %10.0f %f (%10.0f)\n" ,ts,value,max_ts);
			return(1);
		}
	}
	else if((p->use_low == 1) && (lowcount >= p->wrong_count)) {

printf("lowr: %10.0f %d %f %d %10.0f %f\n",max_ts,
		lowcount,max_diff,p->wrong_count,
		ts,value);
		return(1);

	} else {
		return(0);
	}
}

int IsRareBMBPHigh(BMBPPred *p,
	   double ts, 
	   double value, 
	   double lowpred, 
	   double highpred)
{
	JRB jr;
	JRB start_jr;
	int highcount;
	int lowcount;
	double t;
	int ndx;
	double *vals;
	double old_ts;
	int found;
	double old_v;
	double old_hp;
	double old_lp;
	double max_diff;
	double diff;
	double max_ts;
	double total;
	double total_v;
	double count;
	double mean;
	double sd;
	double last_ts;

	if((p->use_low == 0) && (value > highpred))
	{
		return(0);
	} else if((p->use_low == 1) && (value < lowpred)) {
		return(0);
	}

	/*
	 * walk backward in the history list and find the spot
	 */
	found = 0;
	vals = GetFieldValues(p->data->data,J_TS);
	jrb_rtraverse(jr,p->data->age_list)
	{
		ndx = jval_i(jrb_val(jr));
		old_ts = vals[ndx];
		if(old_ts <= ts)
		{
			found = 1;
			break;
		}
	}

	if(found == 0)
	{
		return(0);
	}

	start_jr = jr;

	/*
	 * now count how many successs from this point
	 */
	highcount = 1;
	lowcount = 1;
	max_diff = -1;
	max_ts = -1;
	total = 0;
	count = 0;

	jr = start_jr;
	last_ts = -1;
	while(jr != jrb_nil(p->data->age_list))
	{
		ndx = jval_i(jrb_val(jr));
		vals = GetFieldValues(p->data->data,J_TS);
		old_ts = vals[ndx];
		vals = GetFieldValues(p->data->data,J_VALUE);
		old_v = vals[ndx];
		vals = GetFieldValues(p->data->data,J_HIGHPRED);
		old_hp = vals[ndx];
		vals = GetFieldValues(p->data->data,J_LOWPRED);
		old_lp = vals[ndx];

//		if((UseLow == 0) && ((old_v > old_hp) || (old_v < old_lp)))
		/*
		 * ties count as correct for high
		 */
		if((p->use_low == 0) && (old_v <= old_hp))
		{
			highcount++;
			diff = old_v - old_hp;
			if(diff > max_diff) {
				max_diff = diff;
				max_ts = old_ts;
			}
		}
		else if((p->use_low == 1) && (old_v >= old_lp))
		{
			diff = old_lp - old_v;
			if(diff > max_diff) {
				max_diff = diff;
				max_ts = old_ts;
			}
			lowcount++;
		}
		else
		{
			/*
			 * need to break ties
			 */
			if(last_ts == -1) {
				last_ts = old_ts;
				jr = jrb_prev(jr);
				continue;
			} else if(last_ts == old_ts) {
				jr = jrb_prev(jr);
				continue;
			}
			break;
		}
		last_ts = old_ts;
		jr = jrb_prev(jr);
	}

#ifndef OUTLIER
	if((p->use_low == 0) && (highcount >= p->right_count)) {
		return(1);
	}
#endif
		

	if((p->use_low == 0) && (highcount >= p->right_count)) {
		total = 0;
		count = 0;
		jr = start_jr;
		vals = GetFieldValues(p->data->data,J_VALUE);
		while(jr != jrb_nil(p->data->age_list))
		{
			ndx = jval_i(jrb_val(jr));
			old_v = vals[ndx];
			total += old_v;
			count++;
			jr = jrb_prev(jr);
		}
		mean = total / count;
		total_v = 0;
		jr = start_jr;
		vals = GetFieldValues(p->data->data,J_VALUE);
		while(jr != jrb_nil(p->data->age_list))
		{
			ndx = jval_i(jrb_val(jr));
			old_v = vals[ndx];
			total_v += (old_v - mean) * (old_v - mean);
			jr = jrb_prev(jr);
		}
		sd = sqrt(total_v / count);
		if(value > (mean + 5*sd)) {
			return(2); /* discard as outlier */
		} else {
			return(1);
		}
	}
	else if((p->use_low == 1) && (lowcount >= p->right_count)) {
printf("low right cp: %d\n",lowcount);
		return(1);
	} else {
printf("low right no cp: %d %d\n",highcount,lowcount);
		return(0);
	}
}

/*
 * need the predictions that were made when the job was submitted
 */
void UpdateBMBPPred(void *ib, double ts, double value, double lowpred,
		double highpred)
{
	BMBPPred *p = (BMBPPred *)ib;
	int jbindex;
	double ratio;
	double autoc;
	int rare;
	int rare_high;

	if(p->use_cdf == 1) {
		AddToHistory(p->data,ts,value,lowpred,highpred,0.0,0.0,0.0);
		return;
	}

	p->recovery++;
	p->recovery_high++;
	if(GetHistorySize(p->data) < p->min_history)
	{
		/*
		 * last arg is extraneous without clustering
	 	 */
		AddToHistory(p->data,ts,value,lowpred,highpred,0.0,0.0,0.0);
		return;
	}
	/*
        p->wrong_count = ConsecutiveWrong(p->data,&autoc);
        p->right_count = ConsecutiveRight(p->data,&autoc);
	*/
	ConsecutiveWrongRight(p->data,&autoc,&p->wrong_count,&p->right_count);
	printf("cwrong: %d cright: %d, autoc: %f\n",p->wrong_count,p->right_count,autoc);
	
#if 0
	if((p->no_trim == 0) 
			&& (p->recovery > (p->min_history - p->wrong_count))
				&& IsRare(ts,value,lowpred,highpred,
					p->quantile,
					p->confidence,
					p->wrong_count,
					&p->low_wrong,
					&p->high_wrong))
#endif
	
	rare = IsRareBMBP(p,ts,value,lowpred,highpred);

	if(rare != 0) {
		printf("CHANGEPOINT %10.0f %f %f %f\n",
				ts,value,lowpred,highpred);
	}

	if((p->no_trim == 0) 
			&& (p->recovery > (p->min_history - p->wrong_count))
				&& (rare > 0))
	  {
	    p->recovery = 0;
	    p->recovery_high = 0;
//	    TrimHistoryValue(p->data,p->min_history);
	    TrimHistoryAge(p->data,ts,p->min_history);
	  }
	
	if((p->no_trim == 0) && 
		(p->recovery <= (p->min_history - p->wrong_count)))
	{
// 	  TrimHistoryValue(p->data,p->min_history);
	  TrimHistoryAge(p->data,ts,p->min_history);
	}

	if(rare == 0) {
		rare_high = IsRareBMBPHigh(p,ts,value,lowpred,highpred);
		if(rare_high != 0) {
			printf("CHANGEPOINT %10.0f %f %f %f\n",
				ts,value,lowpred,highpred);
		}
		if((p->no_trim == 0) 
                        && (p->recovery_high >= p->right_count)
                                && (rare_high > 0)) {
	  		TrimHistoryAge(p->data,ts,p->min_history);
			p->recovery_high = 0;
	    		p->recovery = 0;
//printf("TRIM RARE: %d\n",p->right_count);
		} else if((p->no_trim == 0) && 
//			(p->recovery > (p->min_history - p->wrong_count)) && 
//						(p->recovery_high < p->right_count)) {
						(p->recovery_high < p->min_history)) {
			TrimHistoryAge(p->data,ts,p->min_history);
//printf("TRIM RECOVER: %d\n",p->right_count);
		}
	}

	/*
	 * < 2 throws away outliers
	 */
	if(rare <= 2) {
		AddToHistory(p->data,ts,value,lowpred,highpred,0.0,0.0,0.0);
	}

	return;
}

int ForcBMBPPred(void *ib, double *lowpred, double *highpred)
{
	BMBPPred *p = (BMBPPred *)ib;
	double ratio;
	int jbindex;

	if(ib == NULL)
	{
		return(0);
	}

	if(GetHistorySize(p->data) < 1)
	{
	  *lowpred = -1.0;
	  *highpred = -1.0;
	  return(0);
	}

	if(p->use_cdf == 1) {
		jbindex = GetHistorySize(p->data)*p->quantile;
	} else {
        	jbindex = JBIndex(GetHistorySize(p->data),
				  p->quantile,p->confidence, &ratio);
	}
	p->last_index = jbindex;
	if(p->use_low == 1) {
		*lowpred = GetHistoryValue(p->data,jbindex);
		*highpred = GetHistoryValue(p->data, GetHistorySize(p->data) - jbindex);
	} else {
		*highpred = GetHistoryValue(p->data,jbindex);
		*lowpred = GetHistoryValue(p->data, GetHistorySize(p->data) - jbindex);
	}
// printf("COMP %d %f %d\n",jbindex,*highpred,GetHistorySize(p->data));
// fflush(stdout);

	return(1);
}

int InvForcBMBPPred(void *ib, double value, double *lowp, double *highp)
{
	BMBPPred *p = (BMBPPred *)ib;
	double ratio;
	int jbindex;
	double q;
	double highv;
	double lowv;

	if(ib == NULL)
	{
		return(0);
	}

	if(GetHistorySize(p->data) < 1)
	{
	  *lowp = -1.0;
	  *highp = -1.0;
	  return(0);
	}

#define STEP (0.01)

	for(q=0.99; q >= 0.01; q -= STEP) {
        	jbindex = JBIndex(GetHistorySize(p->data),q,p->confidence,
                          &ratio);
        	highv = GetHistoryValue(p->data,jbindex);
		/*
		 * if we walk down below the value, the p was the last
		 * one above
		 */
		if(highv < value) {
			*highp = q;
			break;
		}
	}

	for(q=0.01; q <= 0.99; q += STEP) {
        	jbindex = JBIndex(GetHistorySize(p->data),q,p->confidence,
                          &ratio);
        	lowv = GetHistoryValue(p->data,
			GetHistorySize(p->data) - jbindex);
		/*
		 * if we walk up past the value, the p was the last
		 * one below
		 */
		if(lowv > value) {
			*lowp = q;
			break;
		}
	}
// printf("COMP %d %f %d\n",jbindex,*highpred,GetHistorySize(p->data));
// fflush(stdout);

	return(1);
}

int ForcWeibullNoBinsPred(void *ib, double *lowpred, double *highpred) {
  //    	SchmibPred *p = (SchmibPred *)ib;
	BMBPPred *p = (BMBPPred *)ib;
	void *pred;
	int i;
	
	if(ib == NULL)
	{
		return(0);
	}

	if(GetHistorySize(p->data) < 1)
	{
	  *lowpred = -1.0;
	  *highpred = -1.0;
	  return(0);
	}

	WeibullPred(((BMBPPred *)p)->data, 0.95, 1, highpred);

	return(1);
}


int ForcLognNoBinsPred(void *ib, double *lowpred, double *highpred) {
  //    	SchmibPred *p = (SchmibPred *)ib;
	BMBPPred *p = (BMBPPred *)ib;
	void *pred;
	int i;
	
	if(ib == NULL)
	{
		return(0);
	}

	if(GetHistorySize(p->data) < 1)
	{
	  *lowpred = -1.0;
	  *highpred = -1.0;
	  return(0);
	}

	LognPred( ((BMBPPred *)p)->data, 0.95, 1, highpred);

	return(1);
}

int ForcLoguNoBinsPred(void *ib, double *lowpred, double *highpred) {
  //    	SchmibPred *p = (SchmibPred *)ib;
	BMBPPred *p = (BMBPPred *)ib;
	void *pred;
	int i;
	
	if(ib == NULL)
	{
		return(0);
	}

	if(GetHistorySize(p->data) < 1)
	{
	  *lowpred = -1.0;
	  *highpred = -1.0;
	  return(0);
	}
	
	LoguPred( ((BMBPPred *)p)->data, 0.95, 1, highpred);

	return(1);
}



void SetNoTrimBMBP(void *ib)
{
	BMBPPred *p = (BMBPPred *)ib;

	p->no_trim = 1;

	return;
}

void SetUseCDFBMBP(void *ib)
{
	BMBPPred *p = (BMBPPred *)ib;

	p->use_cdf = 1;

	return;
}

int SizeOfBMBPPred(void *ib)
{
	BMBPPred *p = (BMBPPred *)ib;

	return(GetHistorySize(p->data));
}

History *BMBPHistory(void *ib)
{
	BMBPPred *p = (BMBPPred *)ib;

	return(p->data);
}

void PrintBMBPPred(void *ib)
{
	BMBPPred *p = (BMBPPred *)ib;

	fprintf(stdout,"low_wrong: %d\n",p->low_wrong);
	fprintf(stdout,"high_wrong: %d\n",p->high_wrong);
	fprintf(stdout,"last_index: %d\n",p->last_index);

	PrintHistoryValue(p->data,J_TS,J_VALUE);

	return;
}

void SetUseLowBMBP(void *ib)
{
	BMBPPred *p = (BMBPPred *)ib;
	p->use_low = 1;
}
