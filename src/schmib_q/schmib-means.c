#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "jval.h"
#include "jrb.h"
#include "dllist.h"
#include "simple_input.h"
#include "norm.h"
#include "schmib-means.h"

#define MAX_B (3)

int Max_bin = MAX_B;


int Use2;
char Fname[255];
int Means;
int Find_outlier;
int Verbose;
int UseNormal;
int UseUniform;
int Keep_going = 1;
int data_count;

int DoEdgeMerge(Bin**, int, int);
int DoCrossMerge(Bin**, int, int);
int DoSingletonMerge(Bin**, int);
     
char Line_buf[1024*1024];

Bin *InitBin(void *data)
{
	Bin *b;

	b = (Bin *)malloc(sizeof(Bin));
	if(b == NULL)
	{
		exit(1);
	}

	b->count = 0;
	b->list = new_dllist();
	if(b->list == NULL)
	{
		exit(1);
	}

	b->data = data;
	b->q_sum = 0.0;
	b->log_L = 0.0;

	return(b);
}

void FreeBin(Bin *b)
{
	Dllist d;

	free_dllist(b->list);

	free(b);

	return;
}

void BinMeanVarLogOrder(Bin *b, 
		double *order_vals, double *mean, double *var)
{
	double n = 0.0;
	double mu = 0.0;
	double sig_sq = 0.0;
	int index;
	Dllist d;

	if(b->count <= 0)
	{
		*mean = 0;
		*var = 0;
		return;
	}

	
	dll_traverse(d,b->list)
	{
		index = jval_i(dll_val(d));
		mu += order_vals[index];
		n += 1.0;
	}

	if(n == 0.0)
	{
		*mean = 0;
		*var = 0;
		return;
	}

	mu = mu / n;

	dll_traverse(d,b->list)
	{
		index = jval_i(dll_val(d));
		sig_sq += (order_vals[index] - mu) * (order_vals[index] - mu);
	}

	if(n == 1.0)
	{
		*var = 0;
	}
	else
	{
		*var = sig_sq / (n - 1.0);
	}

	*mean = mu;

	return;
}
/*
 * compute over the values specified in the field
 */
void BinMeanVar(Bin *b, int field, double *mean, double *var)
{
	Dllist d;
	double n = 0.0;
	double mu = 0.0;
	double sig_sq = 0.0;
	int index;
	double *vals;

	vals = GetFieldValues(b->data,field);

	if(vals == NULL)
	{
		*mean = 0;
		*var = 0;
		return;
	}

	dll_traverse(d,b->list)
	{
		index = jval_i(dll_val(d));
		mu += vals[index];
		n += 1.0;
	}

	if(n == 0.0)
	{
		*mean = 0;
		*var = 0;
		return;
	}

	mu = mu / n;

	dll_traverse(d,b->list)
	{
		index = jval_i(dll_val(d));
		sig_sq += (vals[index] - mu) * (vals[index] - mu);
	}

	if(n == 1.0)
	{
		*var = 0;
	}
	else
	{
		*var = sig_sq / (n - 1.0);
	}

	*mean = mu;

	return;
}

void BinMeanVarLog(Bin *b, int field, double *mean, double *var)
{
	Dllist d;
	double n = 0.0;
	double mu = 0.0;
	double sig_sq = 0.0;
	double *vals;
	int index;

	vals = GetFieldValues(b->data,field);
	if(vals == NULL)
	{
		*mean = 0;
		*var = 0;
		return;
	}
	
	dll_traverse(d,b->list)
	{
		index = jval_i(dll_val(d));
		mu += log(vals[index]);
		sig_sq += log(vals[index]) * log(vals[index]);
		n += 1.0;
	}
	if(n == 0.0)
	{
		*mean = 0;
		*var = 0;
		return;
	}

	mu = mu / n;
	sig_sq = sig_sq / n;
	sig_sq = sig_sq - (mu * mu);
	
	if(n == 1.0)
	{
		*var = 0.0000000000001;
	}
	else
	{
	  *var = sig_sq;
	}

	*mean = mu;

	return;
}

void BinMinMax(Bin *b, int field, double *min, double *max)
{
	Dllist d;
	double lmax = 0.0;
	double lmin = 9999999999999999999.99;
	int index;
	double *vals;

	vals = GetFieldValues(b->data,field);
	if(vals == NULL)
	{
		*min = 0;
		*max = 0;
		return;
	}
	

	dll_traverse(d,b->list)
	{
		index = jval_i(dll_val(d));
		if(vals[index] < lmin)
		{
			lmin = vals[index];
		}
		if(vals[index] > lmax)
		{
			lmax = vals[index];
		}
	}

	*min = lmin;
	*max = lmax;

	return;
}

void PrintBin(FILE *fd, Bin *b, int exp_field, int resp_field, int tag)
{
	Dllist d;
	double min;
	double max;
	double *exp_vals;
	double *resp_vals;
	int index;

	exp_vals = GetFieldValues(b->data,exp_field);
	if(exp_vals == NULL)
	{
		return;
	}
	resp_vals = GetFieldValues(b->data,resp_field);
	if(resp_vals == NULL)
	{
		return;
	}

	

	BinMinMax(b,exp_field,&min,&max);

	fprintf(fd,"[%f,%f] count: %d\n",
		min,
		max,
		b->count);
#ifdef GROT
	if(Verbose == 1)
	{
		dll_traverse(d,b->list)
		{
			index = jval_i(dll_val(d));
			fprintf(fd,"\t%f -- %f\n",
					exp_vals[index],
					resp_vals[index]);
		}
		fprintf(fd,"\n");
	}
#endif

	return;
}

void DumpBin(FILE *fd, Bin *b, int id, 
			int ts_field, int exp_field, int resp_field)
{
	Dllist d;
	double min;
	double max;
	double *exp_vals;
	double *resp_vals;
	double *ts_vals;
	int index;

	exp_vals = GetFieldValues(b->data,exp_field);
	if(exp_vals == NULL)
	{
		return;
	}
	resp_vals = GetFieldValues(b->data,resp_field);
	if(resp_vals == NULL)
	{
		return;
	}
	ts_vals = GetFieldValues(b->data,ts_field);
	if(ts_vals == NULL)
	{
		return;
	}

	dll_traverse(d,b->list)
	{
		index = jval_i(dll_val(d));
		fprintf(fd,"bin-%d %f %f %f\n",
				id,
				ts_vals[index],
				resp_vals[index],
				exp_vals[index]);
	}

	return;
}

void PrintBins(Bin **bins, int count, int exp_field, int resp_field)
{
	int i;
	for(i=0; i < count; i++)
	{
		PrintBin(stdout,bins[i],exp_field,resp_field,i);
	}
	return;
}

int yan()
{
	printf("yan\n");
	return(0);
}

void DumpBins(Bin **bins, int count, int ts_field, 
				int exp_field, int resp_field)
{
	int i;
	for(i=0; i < count; i++)
	{
		DumpBin(stdout,bins[i],
			i,ts_field,exp_field,resp_field);
	}
	return;
}
/*
 * adds object from b->data at index to bin
 */
void AddObjectToBin(Bin *b, int index)
{
	double *vals;

	dll_append(b->list,new_jval_i(index));
	b->count++;

	return;
}

int DeleteFirstObjectFromBin(Bin *b)
{
	int index;

	index = jval_i(dll_val(dll_first(b->list)));
	dll_delete_node(dll_first(b->list));
	b->count--;

	return(index);
}


Bin *MergeBins(Bin *a, Bin *b)
{
	Bin *new_bin;
	Dllist curr;
	int index;

	new_bin = InitBin(a->data);
	if(new_bin == NULL)
	{
		exit(1);
	}

	dll_traverse(curr,a->list)
	{
		index = jval_i(dll_val(curr));
		AddObjectToBin(new_bin,index);
	}

	dll_traverse(curr,b->list)
	{
		index = jval_i(dll_val(curr));
		AddObjectToBin(new_bin,index);
	}


	return(new_bin);

}

Bin *CopyBin(Bin *b)
{
	Bin *new_b;
	Dllist d;
	int index;

	new_b = InitBin(b->data);
	if(new_b == NULL)
	{
		exit(1);
	}
	dll_traverse(d,b->list)
        {
		index = jval_i(dll_val(d));
		AddObjectToBin(new_b,index);
	}
	new_b->q_sum = b->q_sum;

	return(new_b);
}




Bin **CopyBins(Bin **bins, int count)
{
	Bin **new_bins;
	int i;
	Dllist d;
	int index;

	new_bins = (Bin **)malloc(count*sizeof(Bin *));
	if(new_bins == NULL)
	{
		exit(1);
	}
	memset(new_bins,0,count*sizeof(Bin *));

	for(i=0; i < count; i++)
	{
		new_bins[i] = InitBin(bins[i]->data);
		if(new_bins[i] == NULL)
		{
			exit(1);
		}
		dll_traverse(d,bins[i]->list)
		{
			index = jval_i(dll_val(d));
			AddObjectToBin(new_bins[i],index);
		}
	}

	return(new_bins);
}

double NBIC(Bin **bins, int bin_count, int field, double N)
{
	double prod;
	int i;
	double lam;
	double p;
	double var;
	double mu;
	Dllist d;
	double x_i;
	double bic;
	double log_L;
	double a_log_L;
	double b_log_L;
	int acount;
	double penalty;
	int index;
	double *vals;


	prod = 1.0;
	log_L = 0.0;
	for(i=0; i < bin_count; i++)
	{
		vals = GetFieldValues(bins[i]->data,field);
		if(vals == NULL)
		{
			return(-9999999999999.99);
		}
		p = (double)bins[i]->count / N;
		BinMeanVar(bins[i],field,&mu,&var);
		dll_traverse(d,bins[i]->list)
		{
			index = jval_i(dll_val(d));
			x_i = vals[index];
			if(x_i == 1.0)
			{
				x_i = 1.0000000001;
			}
			log_L = log_L +
				log(Normal(x_i,mu,sqrt(var)));
		}
	}

	penalty = (((double)(3*bin_count-1)*log(N)/2.0));
	bic = log_L - penalty;

/*
	if(Verbose)
	{
		fprintf(stdout,"bic: %e\n",bic);
		fflush(stdout);
	}
*/

	return(bic);
}

double BIC(Bin **bins, int bin_count, int field, double N)
{
	double prod;
	int i;
	double lam;
	double p;
	double var;
	double mu;
	Dllist d;
	double x_i;
	double bic;
	double log_L;
	double penalty;
	int index;
	double *vals;

	log_L = 0.0;
	for(i=0; i < bin_count; i++)
	{
		vals = GetFieldValues(bins[i]->data,field);
		if(vals == NULL)
		{
			return(-9999999999999999999.99);
		}
		p = (double)bins[i]->count / N;
		BinMeanVar(bins[i],field,&mu,&var);
		if(mu == 0)
		{
			fprintf(stderr,"mu == 0\n");
			fflush(stderr);
			BinMeanVar(bins[i],field,&mu,&var);
			exit(1);
		}
		lam = 1.0 / mu;
		dll_traverse(d,bins[i]->list)
		{
			index = jval_i(dll_val(d));
			x_i = vals[index];
			log_L = log_L + log(lam) + (-1.0*lam*x_i);
		}
	}

	penalty = (((double)(2*bin_count-1)*log(N)/2.0));
	bic = log_L - penalty;
/*
	if(Verbose)
	{
		fprintf(stdout,"bic: %e\n",bic);
		fflush(stdout);
	}
*/
	

	return(bic);
}

double UBIC(Bin **bins, int bin_count, int field, double N)
{
	double prod;
	int i;
	double lam;
	double p;
	double var;
	double mu;
	Dllist d;
	double x_i;
	double bic;
	double log_L;
	double penalty;
	int index;
	double *vals;
	double min;
	double max;
	double last_min;

	log_L = 0.0;
	last_min = 0;
	for(i=0; i < bin_count; i++)
	{
		vals = GetFieldValues(bins[i]->data,field);
		if(vals == NULL)
		{
			return(-9999999999999999999.99);
		}
		BinMinMax(bins[i],field,&min,&max);
		if(last_min == 0)
		{
			last_min = min;
		}

		if(max != last_min)
		{
			log_L = log_L + -1.0 * bins[i]->count * 
							log(max - last_min);
			last_min = max;
		} /* else do noting and get next bin */
	}

	penalty = (bin_count + 1)*log(N)/2.0;
	bic = log_L - penalty;
/*
	if(Verbose)
	{
		fprintf(stdout,"bic: %e\n",bic);
		fflush(stdout);
	}
*/
	

	return(bic);
}
JRB Quantiles;
int Q_init;
int Last_data;

/*
 * produces -1.0 * log(order stat) for exp BIC
 */
double *MakeOrderStats(void *data, int field)
{
	double *vals;
	double *unsorted_vals;
	int i;
	int last_i;
	JRB curr;
	double this_beats;
	double count;
	double so_far;
	double total;

	vals = (double *)malloc(SizeOf(data)*sizeof(double));
	if(vals == NULL)
	{
		exit(1);
	}

	if(!Q_init)
	{
		Quantiles = make_jrb();
		Q_init = 1;
	}

	unsorted_vals = GetFieldValues(data,field);

	for(i=Last_data; i < SizeOf(data); i++)
	{
		jrb_insert_dbl(Quantiles,unsorted_vals[i],
			new_jval_i(i));
	}

	Last_data = i;

	so_far = 0.0;
	this_beats = so_far + 1;
	count = (double)SizeOf(data);
	last_i = -1;

	jrb_traverse(curr,Quantiles)
	{
		i = jval_i(jrb_val(curr));
		if(last_i != -1)
		{
			if(unsorted_vals[last_i] != unsorted_vals[i])
			{
				this_beats = so_far + 1;
			}
		}
		vals[i] = -1.0 * log(this_beats / count);
		last_i = i;
		so_far++;
	}

	return(vals);
		
}


double OrderBIC(Bin **bins, int bin_count, int field, double N)
{
	double prod;
	int i;
	double lam;
	double p;
	double var;
	double mu;
	Dllist d;
	double x_i;
	double bic;
	double log_L;
	double penalty;
	int index;
	double *vals;
	double *order_vals;

	log_L = 0.0;
	order_vals = MakeOrderStats(bins[0]->data,field);
	for(i=0; i < bin_count; i++)
	{
		vals = GetFieldValues(bins[i]->data,field);
		if(vals == NULL)
		{
			return(-9999999999999999999.99);
		}
		p = (double)bins[i]->count / N;
		BinMeanVarLogOrder(bins[i],order_vals,&mu,&var);
		if(mu == 0)
		{
			fprintf(stderr,"mu == 0 for bin %d\n",i);
			fflush(stderr);
			BinMeanVarLogOrder(bins[i],order_vals,&mu,&var);
			exit(1);
		}
		lam = 1.0 / mu;
		dll_traverse(d,bins[i]->list)
		{
			index = jval_i(dll_val(d));
			/*
			 * already converted to -log(order_stats)
			 */
			x_i = order_vals[index];
			log_L = log_L + log(lam) + (-1.0*lam*x_i);
		}
	}
	free(order_vals);

	penalty = (((double)(2*bin_count-1)*log(N)/2.0));
	bic = log_L - penalty;
/*
	if(Verbose)
	{
		fprintf(stdout,"bic: %e\n",bic);
		fflush(stdout);
	}
*/
	

	return(bic);
}

void SanityCheck(Bin **bins, int count, int field)
{
	double min;
	double max;
	double next_min;
	double next_max;
	int i;
	double *vals;

	vals = GetFieldValues(bins[0]->data,field);

	for(i=0; i < count-1; i++)
	{
		BinMinMax(bins[i],field,&min,&max);
		BinMinMax(bins[i+1],field,&next_min,&next_max);
		if(max > next_min)
		{
			yan();
		}
	}

	return;
}


Bin **SchmibMeans(void *data,
	       int fields,	/* number of fields in input */
	       int exp_f,	/* field number for exp variable */
	       int resp_f,	/* field number of response variable */
	       int *means)
{
	int i;
	int j;
	int k;
	int best_i;
	Bin **bins;
	Bin **new_bins;
	Bin *new_bin;
	Dllist d;
	int curr_bin_count;
	double best_bic;
	double max_bic;
	double N;
	JRB sorted;
	JRB jr;
	Bin **best_bins;
	int best_count;
	double best_ever;
	double w, x, y, z;
	int err;
	int index;
	int last_index;
	double last_value;
	double *vals;
	double *ts_vals;

	N = (double)SizeOf(data);

	if(N == 0)
	{
		FreeDataSet(data);
		return(NULL);
	}
	/*
	 * start with max possible bins (could be fixed)
	 */

	curr_bin_count = N;
	printf("N = %f\n", N);
	//	curr_bin_count = SizeOf(data);
	
	bins = (Bin **)malloc(curr_bin_count*sizeof(Bin *));
	if(bins == NULL)
	{
		exit(1);
	}
	new_bins = (Bin **)malloc(curr_bin_count*sizeof(Bin *));
	if(new_bins == NULL)
	{
		exit(1);
	}

	sorted = make_jrb();
	if(sorted == NULL)
	{
		exit(1);
	}

	vals = GetFieldValues(data,exp_f);
	if(vals == NULL)
	{
		return(NULL);
	}

	ts_vals = GetFieldValues(data,0);

	/*
	 * read them into a list sorted by explanatory variable
	 */
	for(i=0; i < N; i++)
	{
		jrb_insert_dbl(sorted,vals[i],new_jval_i(i));
	}
	/*
	 * stick them in the bins in ascending order, but
	 *  keep duplicates together
	 */
	memset(bins,0,sizeof(Bin *)*curr_bin_count);

	curr_bin_count = 0;
	last_index = jval_i(jrb_val(jrb_first(sorted)));
	last_value = vals[last_index];
	jrb_traverse(jr,sorted)
	{
	  index = jval_i(jrb_val(jr));
	  /*
	   * if we don't have a bin at this index or
	   * we do, but the last object we saw had a smaller
	   * value, we need a new bin
	   */
	  if((bins[curr_bin_count] == NULL) ||
	     (vals[index] > last_value))
	    {
	      if(vals[index] > last_value)
		{
		  curr_bin_count++;
		}
	      bins[curr_bin_count] = InitBin(data);
	      AddObjectToBin(bins[curr_bin_count],index);
	    }
	  else /* the last object and this object have the same value */
	    {
	      AddObjectToBin(bins[curr_bin_count],index);
	    }
	  last_index = index;
	  last_value = vals[index];
	}
	
	jrb_free_tree(sorted);
	
	curr_bin_count++;
	
	printf("B4: %d\n", curr_bin_count);
/*
	curr_bin_count = DoEdgeMerge(bins, curr_bin_count, 59);
	curr_bin_count = DoSingletonMerge(bins, curr_bin_count);
*/
	printf("AF: %d\n", curr_bin_count);
	/*
	 * compute an initial bic
	 */

	if(UseNormal)
	{
		best_bic = NBIC(bins,curr_bin_count,resp_f,N);
	}
	else
	{
		best_bic = OrderBIC(bins,curr_bin_count,resp_f,N);
	}
	max_bic = best_bic;
	if(Keep_going)
	{
		best_ever = -99999999999999999999999999999.99;
		best_count = curr_bin_count;
		best_bins = CopyBins(bins,curr_bin_count);
	}
	best_i = -1;
	while(best_i == -1)
	{
		if(Verbose)
		{
			printf("*** starting %d ****\n",
				curr_bin_count);
			fflush(stdout);
		}
		/*
		 * if we want to run all the wat through, reset our
		 * notion of best each time
		 */
		for(i=0; i < curr_bin_count-1; i++)
		{
			/*
			 * make a new merged bin
			 */
			new_bin = MergeBins(bins[i],bins[i+1]);
			/*
			 * copy all the bins before the first merge
			 */
			for(j=0; j < i; j++)
			{
				new_bins[j] = bins[j];
			}
			new_bins[j] = new_bin;
			/*
			 * now copy the others
			 */
			for(j=i+1; j < curr_bin_count-1; j++)
			{
				new_bins[j] = bins[j+1];
			}
			/*
			 * now compute new BIC value
			 */
			if(UseNormal)
			{
				max_bic = 
				     NBIC(new_bins,curr_bin_count-1,
					resp_f,N);
				/*
				 * can't compute variance if all things in
				 * the same bin are the same
				 *
				 * if we see a nan, just merge this with the 
				 * one above it and try again
				 */
				if(isnan(max_bic))
				{
					if(Verbose)
					{
						fprintf(stdout,
				"force merge at %d\n",i);
					}
					best_i = i;
					FreeBin(new_bin);
					goto merge;
				}
			}
			else if(UseUniform)
			{
				max_bic = 
				     UBIC(new_bins,curr_bin_count-1,
					resp_f,N);
				/*
				 * can't compute variance if all things in
				 * the same bin are the same
				 *
				 * if we see a nan, just merge this with the 
				 * one above it and try again
				 */
				if(isnan(max_bic))
				{
					if(Verbose)
					{
						fprintf(stdout,
				"force merge at %d\n",i);
					}
					best_i = i;
					FreeBin(new_bin);
					goto merge;
				}
			}	
			else
			{
				max_bic = 
				OrderBIC(new_bins,curr_bin_count-1,resp_f,N);
			}
			if((max_bic > best_bic) ||
				(Keep_going && (best_i == -1)))
			{
				if(Verbose)
				{
					printf("found good bic: %e at %d\n",
						max_bic,i);
					fflush(stdout);
				}
				best_bic = max_bic;
				best_i = i;
				/*
				 * if we are running the table,
				 * remember any bic that is better than
				 * the best we've seen so far
				if(Keep_going && (max_bic > best_ever))
				 */
				if(Keep_going && (max_bic > best_ever)	
					&& ((curr_bin_count-1) <= Max_bin))
				{
					for(k=0; k < best_count; k++)
					{
						FreeBin(best_bins[k]);
					}
					free(best_bins);
					best_bins = CopyBins(new_bins,
							curr_bin_count-1);
					best_count = curr_bin_count-1;
					best_ever = max_bic;
				}
			}
			/*
			 * dump the merged bin and try again
			 */
			FreeBin(new_bin);
		}

		/*
		 * if we have a winner
		 */
merge:
		if(best_i != -1)
		{
			/*
			 * merge the two best
			 */
			new_bin = MergeBins(bins[best_i],bins[best_i+1]);
			/*
			 * dump the old ones
			 */
			if(Verbose)
			{
				printf("merging %d (%d) and %d (%d) with %d bins\n",
					best_i,
					bins[best_i]->count,
					best_i+1,
					bins[best_i+1]->count,
					curr_bin_count);
			}
			FreeBin(bins[best_i]);
			FreeBin(bins[best_i+1]);
			/*
			 * insert the new one
			 */
			bins[best_i] = new_bin;
			/*
			 * shift all bins ahead of insertion back by 1
			 */
			for(i=best_i+1; i < curr_bin_count-1; i++)
			{
				bins[i] = bins[i+1];
			}
			curr_bin_count--;


			/*
			 * if we are running the table, check to see if we are
			 * down to one bin
			 */
			if(Keep_going && (curr_bin_count == 1))
			{
				break;
			}
			/*
			 * reset and try again
			 */
			best_i = -1;
		}
		else
		{
			best_i = -2;
		}
	}

	/*
	 * here we are done
	 */
	if(Keep_going)
	{
		*means = best_count;
		free(new_bins);
		free(bins);
		return(best_bins);
	}
	else
	{
		*means = curr_bin_count;
		free(new_bins);
		return(bins);
	}
				
}


int DoSingletonMerge(Bin **bins, int curr_bin_count) {
  Bin *new_bin;
  int i, j;

  for (i=0; i<curr_bin_count-1; i++) {
    if ( ((Bin *)bins[i])->count == 1 ) {
      new_bin = MergeBins(bins[i], bins[i+1]);
      FreeBin(bins[i]);
      FreeBin(bins[i+1]);
      bins[i] = new_bin;
      for (j=i+1; j<curr_bin_count-1; j++) {
	bins[j] = bins[j+1];
      }
      curr_bin_count--;
    }
  }
  if ( ((Bin *)bins[curr_bin_count - 1])->count == 1 ) {
    new_bin = MergeBins(bins[curr_bin_count - 1], bins[curr_bin_count - 2]);
    FreeBin(bins[curr_bin_count-1]);
    FreeBin(bins[curr_bin_count-2]);
    bins[curr_bin_count-2] = new_bin;
    curr_bin_count--;
  }		  
  return(curr_bin_count);
}
 
int DoEdgeMerge(Bin **bins, int curr_bin_count, int min_bin_count) {
  Bin *currbin, *new_bin;
  Dllist curr;
  int i,done, j;
  
  done=0;
  i=0;
  while(!done) {
    currbin = bins[i];
    if (currbin->count < min_bin_count) {
      new_bin = MergeBins(bins[i], bins[i+1]);
      FreeBin(bins[i]);
      FreeBin(bins[i+1]);
      bins[i] = new_bin;
      for (j=i+1; j<curr_bin_count-1; j++) {
	bins[j] = bins[j+1];
      }
      curr_bin_count--;
    } else {
      done++;
    }
  }
  done=0;
  i=curr_bin_count-1;
  while(!done) {
    currbin = bins[i];
    if (currbin->count < min_bin_count) {
      new_bin = MergeBins(bins[i], bins[i-1]);
      FreeBin(bins[i]);
      FreeBin(bins[i-1]);
      bins[i-1] = new_bin;
      curr_bin_count--;
      i = curr_bin_count - 1;
    } else {
      done++;
    }
  }

  return(curr_bin_count);
}

int DoCrossMerge(Bin **bins, int curr_bin_count, int field) {
  int i, j, done;
  double mua, mub, var;
  Bin *new_bin;

  done=0;
  for (i=0; i<curr_bin_count-1; i++) {
    BinMeanVar(bins[i], field, &mua, &var);
    BinMeanVar(bins[i+1], field, &mub, &var);
    if ( mua > mub ) {
      new_bin = MergeBins(bins[i], bins[i+1]);
      FreeBin(bins[i]);
      FreeBin(bins[i+1]);
      bins[i] = new_bin;
      for (j=i+1; j<curr_bin_count-1; j++) {
	bins[j] = bins[j+1];
      }
      curr_bin_count--;
    }
  }
  return(curr_bin_count);
}

#ifdef TEST

char *ARGS = "f:vKNUB:";
char *Usage = "bic_means -f filename\n\
\t-B max bins\n\
\t-K <keep going>\n\
\t-N <use log normal>\n\
\t-v <verbose>\n";

#define DATA_FIELDS (4)
#define EXP_FIELD (3)
#define RESP_FIELD (1)

int main(int argc, char *argv[])
{
	int c;
	void *data;
	void *raw_data;
	int err;
	int means;
	Bin **bins;
	double bic;
	int i;
	double w;
	double x;
	double y;
	double z;
	
	Verbose = 0;
	Keep_going = 0;
	data_count = 0;
	UseNormal = 0;
	UseUniform = 0;
	while((c = getopt(argc,argv,ARGS)) != EOF)
	{
		switch(c)
		{
			case 'f':
				strncpy(Fname,optarg,sizeof(Fname));
				break;
			case 'B':
				Max_bin = atoi(optarg);
				break;
			case 'v':
				Verbose = 1;
				break;
			case 'K':
				Keep_going = 1;
				break;
			case 'N':
				UseNormal = 1;
				break;
			case 'U':
				UseUniform = 1;
				break;
			default:
				fprintf(stderr,"unrecognized arg %c\n",c);
				fprintf(stderr,"%s",Usage);
				fflush(stderr);
				exit(1);
		}
	}

	if(Fname[0] == 0)
	{
		fprintf(stderr,"must specify filename\n");
		fprintf(stderr,"%s",Usage);
		fflush(stderr);
		exit(1);
	}

	err = InitDataSet(&raw_data,DATA_FIELDS);
	if(err == 0)
	{
		fprintf(stderr,"couldn't open file %s\n",
				Fname);
		fflush(stderr);
		exit(1);
	}

	err = LoadDataSet(Fname,raw_data);
	if(err == 0)
	{
		fprintf(stderr,"couldn't read data from file %s\n",
				Fname);
		fflush(stderr);
		exit(1);
	}

	/*
	 * now purify the data
	 */
	err = InitDataSet(&data,DATA_FIELDS);
	if(err == 0)
	{
		exit(1);
	}

	while(ReadEntry4(raw_data,&w,&x,&y,&z))
	{
		if((x <= 0) || (z <= 0))
		{
			continue;
		}
		WriteEntry4(data,w,x,y,z);
	}

	FreeDataSet(raw_data);
	Rewind(data);

	bins = SchmibMeans(data,
			   DATA_FIELDS,
			   EXP_FIELD,
			   RESP_FIELD,
			   &means);
	if(UseNormal)
	{
		bic = NBIC(bins,(double)means,RESP_FIELD,(double)SizeOf(data));
	}
	else if(UseUniform)
	{
		bic = UBIC(bins,(double)means,RESP_FIELD,(double)SizeOf(data));
	}
	else
	{
		bic = OrderBIC(bins,(double)means,RESP_FIELD,(double)SizeOf(data));
	}
	fprintf(stdout,"BIC: %f\n",bic);
	for(i=0; i < means; i++)
	  {
	    PrintBin(stdout,bins[i],
			EXP_FIELD,
			RESP_FIELD,
			i);
	  }
	
	return(0);
}

#endif
