/*
 * http://www.swox.com/gmp/manual
 */
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

#include "schmib_q_sim.h"
#include "bmbp_pred.h"
#include "schmib_pred.h"
#include "schmib-means.h"
#include "schmib-glrt.h"

int PrintCount;

extern int UseGLRT;
extern int Max_bin;


void *InitSchmibPred(double quantile, double confidence, int fields)
{
	SchmibPred *p;
	double autoc;

	p = (SchmibPred *)malloc(sizeof(SchmibPred));
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
	p->wrong_count = ConsecutiveWrong(p->data, &autoc);

	p->quantile = quantile;
	p->confidence = confidence;
	p->min_history = MinHistory(quantile,confidence);
	p->recovery = p->min_history+1;
	p->no_trim = 0;

	p->bins = NULL;
	p->preds = NULL;
	p->bin_count = 0;
	p->rebin_at = 100;
	p->since_rebin = 0;

	return((void *)p);
}

void FreeSchmibPred(void *ib)
{
	SchmibPred *p = (SchmibPred *)ib;
	int i;

	FreeHistory(p->data);
	if(p->bins != NULL)
	{
		for(i=0; i < p->bin_count; i++)
		{
			FreeBin(p->bins[i]);
			FreeBMBPPred(p->preds[i]);
		}
		free(p->bins);
		free(p->preds);
	}
	free(p);

	return;
}

void CleanBins(SchmibPred *p)
{
	int i;

	if(p->bins != NULL)
	{
		for(i=0; i < p->bin_count; i++)
		{
			FreeBin(p->bins[i]);
#if 0
			FreeBMBPPred(p->preds[i]);
#endif
		}
		free(p->bins);
#if 0
		free(p->preds);
#endif
	}

	return;
}

void RebinSchmib(SchmibPred *p)
{
	int i;
	void *data;
	Dllist d;
	int ndx;
	double ts;
	double resp_value;
	double exp_value;
	double lowpred;
	double highpred;
	JRB sorted;
	JRB jr;
	double *vals;
	double dummy;
	int old_count;

#if 0
	if(p->bin_count != 0)
	{
	printf("BEFOR REBIN\n");
	for(i=0; i < p->bin_count; i++)
	{
		PrintBin(stdout,p->bins[i],J_EXP,J_VALUE,i);
	}
	}
#endif
	data = p->data->data;		/* get the raw data from History */
	CleanBins(p);
	old_count = p->bin_count;
	if(UseGLRT)
	{
		p->bins = 
			SchmibGLRT(data,J_FIELDS,J_EXP,J_VALUE,
				Max_bin,&p->bin_count);
		if(p->bins == NULL)
		{
			p->bins = 
			   SchmibGLRT(data,J_FIELDS,J_EXP,J_VALUE,1,
				&p->bin_count);
			p->bin_count = 1;
		}
	}
	else
	{
		p->bins = 
			SchmibMeans(data,J_FIELDS,J_EXP,J_VALUE,&p->bin_count);
	}

//	printf("REBIN\n");
//	for(i=0; i < p->bin_count; i++)
//	{
//		PrintBin(stdout,p->bins[i],J_EXP,J_VALUE,i);
//	}

//	printf("NEW BIN COUNT: %d\n",p->bin_count);

	if(p->bins == NULL)
	{
		fprintf(stderr,"RebinSchmib: schmibmeans fails\n");
		fflush(stderr);
		exit(1);
	}

// if(p->bin_count != old_count)
	if(1)
	{
		for(i=0; i < old_count; i++)
		{
			FreeBMBPPred(p->preds[i]);
		}
		free(p->preds);	
		p->preds = (void **)malloc(p->bin_count * sizeof(void *));
		if(p->preds == NULL)
		{
			fprintf(stderr,"RebinSchmib: out of space\n");
			fflush(stderr);
			exit(1);
		}

		for(i=0; i < p->bin_count; i++)
		{
			p->preds[i] = InitBMBPPred(p->quantile,p->confidence,
						J_FIELDS);
			if(p->preds[i] == NULL)
			{
				fprintf(stderr,"RebinSchmib: out of space for pred\n");
				fflush(stderr);
				exit(1);
			}
			vals = GetFieldValues(data,J_TS);
			sorted = make_jrb();
			if(sorted == NULL)
			{
				fprintf(stderr,
			"no space for sorted list in rebin\n");
				fflush(stderr);
				exit(1);
			}
			/*
			 * sort them by timemstamp so they go into the predictor
			 * in the right order
			 */
			dll_traverse(d,p->bins[i]->list)
			{
				ndx = jval_i(dll_val(d));
				jrb_insert_dbl(sorted,vals[ndx],new_jval_i(ndx));
			}
				
			jrb_traverse(jr,sorted)
			{
				ndx = jval_i(jrb_val(jr));
				Seek(data,ndx);
				ReadEntry6(data,&ts,&resp_value,
						&lowpred,&highpred,&exp_value, &dummy);

				UpdateBMBPPred(p->preds[i],
						ts,
						resp_value,
						lowpred,
						highpred);
			}
			jrb_free_tree(sorted);
		}
	}
	return;
}

int FindBin(Bin **bins, int count, double value)
{
	int i;
	double min;
	double max;
	double next_min;
	double next_max;

	for(i=0; i < count; i++)
	{
		BinMinMax(bins[i],J_EXP,&min,&max);
#if 0
		BinMinMax(bins[i+1],J_EXP,&next_min,&next_max);
		if((value >= min) && (value <= max))
		{
			break;
		}
		if((value >= max) && (value <= next_min))
		{
			break;
		}
#endif
		if(value < max)
		{
			return(i);
		}
	}

	/*
	 * if we didn't find anything
	 */
	return(i-1);
}


/*
 * need the predictions that were made when the job was submitted
 *
 * low pred and high pred are for the response variable
 */
void UpdateSchmibPred(void *ib, double ts, 
		double resp_value, double lowpred, double highpred,
		double exp_value)
{
	SchmibPred *p = (SchmibPred *)ib;
	int jbindex;
	double ratio;
	int i;
	int ndx;
	double min;
	double max;


	if(GetHistorySize(p->data) < p->min_history)
	{
		AddToHistory(p->data,ts,resp_value,
				lowpred,highpred,exp_value,0,0);
		return;
	}

	/*
	 * if it is time to rebin
	 */
	if((p->bins == NULL) || (p->since_rebin >= p->rebin_at))
	{
		RebinSchmib(p);
		p->since_rebin = 0;
	}
	else
	{
		p->since_rebin++;
	}

	
	AddToHistory(p->data,ts,resp_value,
					lowpred,highpred,exp_value,0,0);
	/*
	 * get the index where it is written in the history
	 */
	ndx = Current(p->data->data);
	/*
	 * add it to the right bin
	 */
	i = FindBin(p->bins,p->bin_count,exp_value);
	AddObjectToBin(p->bins[i],ndx);
	UpdateBMBPPred(p->preds[i],ts,resp_value,lowpred,highpred);

#ifdef GROT
	if(PrintCount >= 2000)
	{
		DumpBins(p->bins,p->bin_count,J_TS,J_EXP,J_VALUE);
		for(i=0; i < p->bin_count; i++)
		{
			ForcBMBPPred(p->preds[i],&lowpred,&highpred);
			fprintf(stdout,"pred-%d %f\n",i,highpred);
			PrintBMBPPred(p->preds[i]);
		}
	}
#endif

	PrintCount++;

	return;
}

void *BorrowedPred(SchmibPred *p, int b)
{
	void *pred;
	int i;
	int count;
	Bin *bin;
	JRB sorted;
	JRB jr;
	double *vals;
	int ndx;
	void *data;
	double ts;
	double resp_value;
	double lowpred;
	double highpred;
	double exp_value;
	double dummy;
	Dllist d;
	int min_history = p->min_history;

	pred = InitBMBPPred(p->quantile,p->confidence,J_FIELDS);
	if(pred == NULL)
	{
		fprintf(stderr,"out of space in Borrowedpred\n");
		fflush(stderr);
		exit(1);
	}

	/*
	 * sort the data by time stamp
	 */
	sorted = make_jrb();
	if(sorted == NULL)
	{
		fprintf(stderr,
			"no space for sorted list in borrowed pred\n");
		fflush(stderr);
		exit(1);
	}
	/*
	 * start by going upwards
	 */
	count = 0;
	for(i=b; i < p->bin_count; i++)
	{
		bin = p->bins[i];
		/*
	 	 * get the raw data
		 */
		data = bin->data;
		vals = GetFieldValues(data,J_TS);
		dll_traverse(d,bin->list)
		{
			ndx = jval_i(dll_val(d));
			jrb_insert_dbl(sorted,vals[ndx],new_jval_i(ndx));
			count++;
		}
		if(count >= min_history)
		{
			break;
		}
	}

	/*
	 * if we don't have enough data, work downwards
	 */
	if(count < min_history)
	{
		for(i=b-1; i >= 0; i--)
		{
			bin = p->bins[i];
			/*
			 * get the raw data
			 */
			data = bin->data;
			vals = GetFieldValues(data,J_TS);
			dll_traverse(d,bin->list)
			{
				ndx = jval_i(dll_val(d));
				jrb_insert_dbl(sorted,vals[ndx],
							new_jval_i(ndx));
				count++;
			}
			if(count >= min_history)
			{
				break;
			}
		}
	}

	/*
	 * now build the predictor
	 */
	jrb_traverse(jr,sorted)
	{
		ndx = jval_i(jrb_val(jr));
		Seek(p->data->data,ndx);
		ReadEntry6(p->data->data,&ts,&resp_value,&lowpred,
			&highpred,&exp_value,&dummy);
		UpdateBMBPPred(pred,ts,resp_value,lowpred,highpred);
	}

	jrb_free_tree(sorted);

	return(pred);

}

int ForcSchmibPred(void *ib, double exp_value, 
			double *lowpred, double *highpred)
{
	SchmibPred *p = (SchmibPred *)ib;
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

	/*
	 * if this is our first time without a rebin
	 */
	if(p->bins == NULL)
	{
		RebinSchmib(p);
		p->since_rebin = 0;
	}

	i = FindBin(p->bins,p->bin_count,exp_value);

	if(p->bins[i]->count < p->min_history)
	{
		pred = BorrowedPred(p,i);
		ForcBMBPPred(pred,lowpred,highpred);
#if 0
printf("FORC: %f from bin %d, size: %d, exp_val: %f\n",
		*highpred,i,SizeOf(((BMBPPred *)pred)->data->data),
			exp_value);
#endif
		FreeBMBPPred(pred);
	}
	else
	{
		ForcBMBPPred(p->preds[i],lowpred,highpred);
#if 0
printf("FORC: %f from bin %d, size: %d exp_val: %f\n",
		*highpred,i,SizeOf(((BMBPPred *)p->preds[i])->data->data),
			exp_value);
#endif
	}

	return(1);
}

void SetNoTrimSchmib(void *ib)
{
	SchmibPred *p = (SchmibPred *)ib;

	p->no_trim = 1;

	return;
}

int SizeOfSchmibPred(void *ib)
{
	SchmibPred *p = (SchmibPred *)ib;

	return(GetHistorySize(p->data));
}

History *SchmibHistory(void *ib)
{
	SchmibPred *p = (SchmibPred *)ib;

	return(p->data);
}

void SetRebinCount(void *ib, int count)
{
	SchmibPred *p = (SchmibPred *)ib;
	p->rebin_at = count;
	p->since_rebin = 0;

	return;
}

int ForcWeibullPred(void *ib, double exp_value, double *lowpred, double *highpred) {
    	SchmibPred *p = (SchmibPred *)ib;
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

	/*
	 * if this is our first time without a rebin
	 */
	if(p->bins == NULL)
	{
		RebinSchmib(p);
		p->since_rebin = 0;
	}

	i = FindBin(p->bins,p->bin_count,exp_value);

	if(p->bins[i]->count < p->min_history)
	{
		pred = BorrowedPred(p,i);
		//ForcBMBPPred(pred,lowpred,highpred);
		WeibullPred( ((SchmibPred *)pred)->data, 0.95, 1, highpred);
		FreeBMBPPred(pred);
	}
	else
	{
	  //	  ForcBMBPPred(p->preds[i],lowpred,highpred);
	  WeibullPred( ((SchmibPred *)(p->preds[i]))->data, 0.95, 1, highpred);
	}

	return(1);
}



int ForcLognPred(void *ib, double exp_value, double *lowpred, double *highpred) {
    	SchmibPred *p = (SchmibPred *)ib;
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

	/*
	 * if this is our first time without a rebin
	 */
	if(p->bins == NULL)
	{
		RebinSchmib(p);
		p->since_rebin = 0;
	}

	i = FindBin(p->bins,p->bin_count,exp_value);

	if(p->bins[i]->count < p->min_history)
	{
		pred = BorrowedPred(p,i);
		//ForcBMBPPred(pred,lowpred,highpred);
		LognPred( ((SchmibPred *)pred)->data, 0.95, 1, highpred);
		FreeBMBPPred(pred);
	}
	else
	{
	  //	  ForcBMBPPred(p->preds[i],lowpred,highpred);
	  LognPred( ((SchmibPred *)(p->preds[i]))->data, 0.95, 1, highpred);
	}

	return(1);
}

int ForcLoguPred(void *ib, double exp_value, double *lowpred, double *highpred) {
    	SchmibPred *p = (SchmibPred *)ib;
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

	/*
	 * if this is our first time without a rebin
	 */
	if(p->bins == NULL)
	{
		RebinSchmib(p);
		p->since_rebin = 0;
	}

	i = FindBin(p->bins,p->bin_count,exp_value);

	if(p->bins[i]->count < p->min_history)
	{
		pred = BorrowedPred(p,i);
		//ForcBMBPPred(pred,lowpred,highpred);
		LoguPred( ((SchmibPred *)pred)->data, 0.95, 1, highpred);
		FreeBMBPPred(pred);
	}
	else
	{
	  //	  ForcBMBPPred(p->preds[i],lowpred,highpred);
	  LoguPred( ((SchmibPred *)(p->preds[i]))->data, 0.95, 1, highpred);
	}

	return(1);
}

