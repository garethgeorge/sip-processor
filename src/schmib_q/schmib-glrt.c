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

extern int Max_bin;
#define MIN_SIZE 2



double *MakeQuants(Bin *b, int field)
{
	void *data = b->data;
	Dllist list = b->list;
	double *vals;
	double *unsorted_vals;
	int i;
	int last_i;
	JRB curr;
	double this_beats;
	double count;
	double so_far;
	double total;
	JRB sorted;
	Dllist d;
	double q_sum;


	sorted = make_jrb();
	if(sorted == NULL)
	{
		exit(1);
	}

	unsorted_vals = GetFieldValues(data,field);
	count = 0.0;
	dll_traverse(d,list)
	{
		i = jval_i(dll_val(d));
		jrb_insert_dbl(sorted,unsorted_vals[i],
			new_jval_i(i));
		count++;
	}

	if(count == 0.0)
	{
		return(NULL);
	}

	/*
	 * always compute quants relative to whole data set even if we are
	 * only looking at a subset in a bin
	 */
	vals = (double *)malloc(SizeOf(data)*sizeof(double));
	if(vals == NULL)
	{
		exit(1);
	}


	so_far = 0.0;
	this_beats = so_far + 1;
	last_i = -1;
	q_sum = 0.0;

	jrb_traverse(curr,sorted)
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
		q_sum += vals[i];
		last_i = i;
		so_far++;
	}

	jrb_free_tree(sorted);
	b->q_sum = q_sum;
	return(vals);
}

double LLExp(Bin *bin, int field)
{
	Dllist list = bin->list;
	Dllist d;
	int i;
	double mu;
	double var;
	double x_i;
	double log_L;
	double lam;
	double size = (double)(bin->count);
	double *vals;

	vals = GetFieldValues(bin->data,field);
	if(vals == NULL)
	{
		exit(1);
	}

	mu = bin->q_sum / size;

	lam = 1.0 / mu;
	log_L = 0;

	/*
	 * okay -- here is the deal.  This computes SUM(log(lam) - lam*x_i)
	 * which factors into SUM(log(lam) - SUM(lam * x_i) which then becomes
	 * size * log(lam) - lam * SUM(x_i)
	 * which we can update incrementally
	 */
#if 0
	dll_traverse(d,list)
	{
		i = jval_i(dll_val(d));
		x_i = vals[i];
		log_L = log_L + log(lam) + (-1.0*lam*x_i);
	}


#endif
	log_L = size * log(lam) - (lam * bin->q_sum); 

	bin->log_L = log_L;

	return(log_L);
}
/*
 * computes the log likelihood function for the quantiles in the dll
 */
double LLExpQ(double *quants, Bin *bin, int field)
{
	Dllist list = bin->list;
	Dllist d;
	int i;
	double mu;
	double x_i;
	double log_L;
	double lam;
	double lq;
	double size = (double)(bin->count);
	double q_sum = bin->q_sum;
	double lam_term;
	double x_term;

	/*
	size = 0;
	dll_traverse(d,list)
	{
		i = jval_i(dll_val(d));
		lq = quants[i];
		mu += lq;
		size++;
	}

	mu = mu / (double)size;
	*/

	lam = 1.0 / (q_sum / size);

#if 0
	/*
	 * much faster the other way
	 */
	log_L = 0.0;
	dll_traverse(d,list)
	{
		i = jval_i(dll_val(d));
		/*
		 * already converted to -log(order_stats)
		 */
		x_i = quants[i];
		log_L = log_L + log(lam) + (-1.0*lam*x_i);
		log_L = log_L + (-1.0*lam*x_i);
	}
#endif

	x_term = -1.0 * q_sum * lam;
	lam_term = size * log(lam);
	log_L = lam_term + x_term;

	return(log_L);
}


void ShiftBinsLeft(Bin *dst, Bin *src, double *vals)
{
	int index;

	
	index = DeleteFirstObjectFromBin(src);
	src->q_sum -= vals[index];

	AddObjectToBin(dst,index);
	dst->q_sum += vals[index];
	return;
}

void ResetBICList(JRB *list, JRB work_list)
{
	JRB bic_list = *list;
	JRB jr;
	Bin *b;
	Bin *new_b;
	double key;

	if(bic_list != NULL)
	{
		jrb_traverse(jr,bic_list)
		{
			b = (Bin *)jval_v(jrb_val(jr));
			FreeBin(b);
		}
		jrb_free_tree(bic_list);
	}

	bic_list = make_jrb();
	if(bic_list == NULL)
	{
		exit(1);
	}

	jrb_traverse(jr,work_list)
	{
		b = (Bin *)jval_v(jrb_val(jr));
		key = jval_d(jr->key);
		new_b = CopyBin(b);
		jrb_insert_dbl(bic_list,key,new_jval_v(new_b));
	}

	*list = bic_list;

	return;
}
			



Bin **SchmibGLRTQ(void *data,
	       int fields,	/* number of fields in input */
	       int exp_f,	/* field number for exp variable */
	       int resp_f,	/* field number of response variable */
	       int bin_count,
	       int *means)
{
	int i;
	Bin **bins;
	double N;
	JRB sorted;
	JRB jr;
	Bin *left;
	Bin *right;
	Bin *both;
	int curr;
	int index;
	double *vals;
	double ratio;
	double max_ratio;
	int split;
	int best_split;
	double *quants;
	double all_L;
	double both_L;
	double left_L;
	double right_L;
	double min;
	double max;
	JRB work_list;
	JRB blist;
	int curr_bin_count;
	Bin *b;
	Bin *next_b;
	Bin *new_b;
	Dllist d;
	double best_bic;
	double new_bic;
	double bic_N;
	int best_bin_count;
	JRB bic_list;
	double penalty;
	double min_L;
	double min_R;
	double max_L;
	double max_R;
	int fails;

	N = (double)SizeOf(data);
	bic_N = N;

	if(N == 0)
	{
		FreeDataSet(data);
		return(NULL);
	}
	/*
	 * start with 1  bin 
	 */
	
	bins = (Bin **)malloc(bin_count*sizeof(Bin *));
	if(bins == NULL)
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

	work_list = make_jrb();
	if(work_list == NULL)
	{
		exit(1);
	}

	blist = make_jrb();
	if(blist == NULL)
	{
		exit(1);
	}

	/*
	 * read them into a list sorted by explanatory variable
	 *
	 * stick them in a single bin to start
	 */
	both = InitBin(data);
	for(i=0; i < N; i++)
	{
		jrb_insert_dbl(sorted,vals[i],new_jval_i(i));
		AddObjectToBin(both,i);
	}
	jrb_free_tree(sorted);

	curr_bin_count = 1;
	best_bic = -99999999999999999999999.99;
	quants = MakeQuants(both,resp_f);
	all_L = LLExpQ(quants, both, resp_f);
	free(quants);
	bic_list = NULL;

	jrb_insert_dbl(work_list,0.0,new_jval_v(both));
	while(curr_bin_count < bin_count)
	{
		/*
		 * work_list sorted by Log Likelihood value
		 */
		both = jval_v(jrb_val(jrb_first(work_list)));
		jrb_delete_node(jrb_first(work_list));

		N = both->count;

		/*
		 * sort the values in this bin by their exp value
		 */
		sorted = make_jrb();
		if(sorted == NULL)
		{
			exit(1);
		}
		dll_traverse(d,both->list)
		{
			i = jval_i(dll_val(d));
			jrb_insert_dbl(sorted,vals[i],new_jval_i(i));
		}

		quants = MakeQuants(both,resp_f);



		split = MIN_SIZE;
		max_ratio = -999999999999999999999999.99;
		best_split = -1;

		curr = 0;
		left = InitBin(data);
		right = InitBin(data);
		jrb_traverse(jr,sorted)
		{
			index = jval_i(jrb_val(jr));
			if(curr < split)
			{
				AddObjectToBin(left,index);
				left->q_sum += quants[index];
			}
			else
			{
				AddObjectToBin(right,index);
				right->q_sum += quants[index];
			}
			curr++;
		}
		both_L = LLExpQ(quants, both, resp_f);

		for(split=MIN_SIZE; split < (N-MIN_SIZE); split++)
		{
			left_L = LLExpQ(quants,
			       left,
			       resp_f);

			right_L = LLExpQ(quants,
			       right,
			       resp_f);

			ratio = both_L - (left_L + right_L);
// printf("%f\n",ratio);

			if((-2*ratio) > max_ratio)
			{
				max_ratio = -2*ratio;
				best_split = split;
			}
			ShiftBinsLeft(left,right,quants);
		}
		
		FreeBin(left);
		FreeBin(right);

		if(best_split == -1)
		{
			jrb_free_tree(sorted);
			/*
			 * no split works, retain "both" and end
			 */
			jrb_insert_dbl(work_list,both_L,new_jval_v(both));
			break;
		}
		else
		{
			FreeBin(both);
		}

		

		left = InitBin(data);
		right = InitBin(data);
		curr = 0;
		jrb_traverse(jr,sorted)
		{
			index = jval_i(jrb_val(jr));
			if(curr < best_split)
			{
				AddObjectToBin(left,index);
				left->q_sum += quants[index];
			}
			else
			{
				AddObjectToBin(right,index);
				right->q_sum += quants[index];
			}
			curr++;
		}

		left_L = LLExpQ(quants,
			       left,
			       resp_f);
		jrb_insert_dbl(work_list,left_L,new_jval_v(left));

		right_L = LLExpQ(quants,
			       right,
			       resp_f);
		jrb_insert_dbl(work_list,right_L,new_jval_v(right));
//printf("left: %f, right: %f, both: %f\n",left_L,right_L,both_L);
//fflush(stdout);

		penalty = ((2*(double)(curr_bin_count+1)) - 1)*log(bic_N)/2.0;
		new_bic = (all_L - both_L + left_L + right_L) - penalty;

		if(new_bic > best_bic)
		{
			best_bic = new_bic;
			best_bin_count = curr_bin_count+1;
			ResetBICList(&bic_list,work_list);
		}

		all_L = all_L - both_L + left_L + right_L;

		free(quants);

		curr_bin_count++;


//printf("best_split: %d\n",best_split);

		jrb_free_tree(sorted);
	}

	if(bic_list == NULL)
	{
		jrb_traverse(jr,work_list)
		{
			b = (Bin *)jval_v(jrb_val(jr));
			BinMinMax(b,exp_f,&min,&max);
			jrb_insert_dbl(blist,min,new_jval_v(b));
		}
	}
	else
	{
		jrb_traverse(jr,bic_list)
		{
			b = (Bin *)jval_v(jrb_val(jr));
			BinMinMax(b,exp_f,&min,&max);
			jrb_insert_dbl(blist,min,new_jval_v(b));
		}
		jrb_free_tree(bic_list);
	}
	jrb_free_tree(work_list);

	i = 0;
	jrb_traverse(jr,blist)
	{
		b = (Bin *)jval_v(jrb_val(jr));
		while(jrb_next(jr) != jrb_nil(blist))
		{
			next_b = (Bin *)jval_v(jrb_val(jrb_next(jr)));
			BinMinMax(b,exp_f,&min_L,&max_L);
			BinMinMax(next_b,exp_f,&min_R,&max_R);

			if(min_L == min_R)
			{
				new_b = MergeBins(b,next_b);
				FreeBin(b);
				b = new_b;
				FreeBin(next_b);
				jr = jrb_next(jr);
			}
			else
			{
				break;
			}
		}
		bins[i] = b;
		i++;
	}
	jrb_free_tree(blist);

//printf("best_bic: %f, best_count: %d, final count: %d\n",
//		best_bic,best_bin_count,i);
		
	
	*means = i;
	return(bins);
}


Bin **SchmibGLRT(void *data,
	       int fields,	/* number of fields in input */
	       int exp_f,	/* field number for exp variable */
	       int resp_f,	/* field number of response variable */
	       int bin_count,
	       int *means)
{
	int i;
	Bin **bins;
	double N;
	JRB sorted;
	JRB jr;
	Bin *left;
	Bin *right;
	Bin *both;
	int curr;
	int index;
	double *vals;
	double ratio;
	double max_ratio;
	int split;
	int best_split;
	double all_L;
	double both_L;
	double left_L;
	double right_L;
	double min;
	double max;
	JRB work_list;
	JRB blist;
	int curr_bin_count;
	Bin *b;
	Bin *next_b;
	Bin *new_b;
	Dllist d;
	double best_bic;
	double new_bic;
	double bic_N;
	int best_bin_count;
	JRB bic_list;
	double penalty;
	double min_L;
	double min_R;
	double max_L;
	double max_R;
	int fails;

	N = (double)SizeOf(data);
	bic_N = N;

	if(N == 0)
	{
		FreeDataSet(data);
		return(NULL);
	}
	/*
	 * start with 1  bin 
	 */
	
	bins = (Bin **)malloc(bin_count*sizeof(Bin *));
	if(bins == NULL)
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

	work_list = make_jrb();
	if(work_list == NULL)
	{
		exit(1);
	}

	blist = make_jrb();
	if(blist == NULL)
	{
		exit(1);
	}

	/*
	 * read them into a list sorted by explanatory variable
	 * exp variables for a sorted range (1-5 procs, 6 - 10 proces, etc.)
	 *
	 * stick them in a single bin to start
	 */
	both = InitBin(data);
	for(i=0; i < N; i++)
	{
		jrb_insert_dbl(sorted,vals[i],new_jval_i(i));
		AddObjectToBin(both,i);
		both->q_sum += vals[i];
	}
	jrb_free_tree(sorted);

	curr_bin_count = 1;
	best_bic = -99999999999999999999999.99;
	all_L = LLExp(both, resp_f);
	bic_list = NULL;

	jrb_insert_dbl(work_list,all_L,new_jval_v(both));
	while(curr_bin_count < bin_count)
	{
		/*
		 * work_list sorted by Log Likelihood value
		 */
		both = jval_v(jrb_val(jrb_first(work_list)));
		jrb_delete_node(jrb_first(work_list));

		N = both->count;
		/*
		 * sort the values in this bin by their exp value
		 *
		 * may be sanity check since bins are probably
		 * sorted from above, but what the heck?
		 */
		sorted = make_jrb();
		if(sorted == NULL)
		{
			exit(1);
		}
		dll_traverse(d,both->list)
		{
			i = jval_i(dll_val(d));
			jrb_insert_dbl(sorted,vals[i],new_jval_i(i));
		}


		split = MIN_SIZE;
		max_ratio = -999999999999999999999999.99;
		best_split = -1;

		/*
		 * now try and split
		 */
		curr = 0;
		left = InitBin(data);
		right = InitBin(data);
		jrb_traverse(jr,sorted)
		{
			index = jval_i(jrb_val(jr));
			if(curr < split)
			{
				AddObjectToBin(left,index);
				vals = GetFieldValues(data,resp_f);
				left->q_sum += vals[index];
			}
			else
			{
				AddObjectToBin(right,index);
				vals = GetFieldValues(data,resp_f);
				right->q_sum += vals[index];
			}
			curr++;
		}
		both_L = LLExp(both, resp_f);
		vals = GetFieldValues(both->data,resp_f);

		for(split=MIN_SIZE; split < (N-MIN_SIZE); split++)
		{
			left_L = LLExp(left, resp_f);

			right_L = LLExp(right, resp_f);

			ratio = both_L - (left_L + right_L);
// printf("%f\n",ratio);

			/*
			 * -2 * log(Lambda) ~ ChiSq -- biggest is
			 * best candidate for a split since it is the
			 * thing most likely to fail hyp. test
			 */
			if((-2*ratio) > max_ratio)
			{
				max_ratio = -2*ratio;
				best_split = split;
			}
			ShiftBinsLeft(left,right,vals);
		}
		
		FreeBin(left);
		FreeBin(right);

		if(best_split == -1)
		{
			jrb_free_tree(sorted);
			/*
			 * no split works, retain "both" and end
			 */
			jrb_insert_dbl(work_list,both_L,new_jval_v(both));
			break;
		}
		else
		{
			FreeBin(both);
		}

		

		/*
		 * now go ahead and make the best split permanent
		 */
		left = InitBin(data);
		right = InitBin(data);
		curr = 0;
		jrb_traverse(jr,sorted)
		{
			index = jval_i(jrb_val(jr));
			if(curr < best_split)
			{
				AddObjectToBin(left,index);
				vals = GetFieldValues(data,resp_f);
				left->q_sum += vals[index];
			}
			else
			{
				AddObjectToBin(right,index);
				vals = GetFieldValues(data,resp_f);
				right->q_sum += vals[index];
			}
			curr++;
		}

		left_L = LLExp(left, resp_f);
		jrb_insert_dbl(work_list,left_L,new_jval_v(left));

		right_L = LLExp(right, resp_f);
		jrb_insert_dbl(work_list,right_L,new_jval_v(right));
//printf("left: %f, right: %f, both: %f\n",left_L,right_L,both_L);
//fflush(stdout);

		penalty = ((2*(double)(curr_bin_count+1)) - 1)*log(bic_N)/2.0;
		new_bic = (all_L - both_L + left_L + right_L) - penalty;

		if(new_bic > best_bic)
		{
			best_bic = new_bic;
			best_bin_count = curr_bin_count+1;
			ResetBICList(&bic_list,work_list);
//BinMinMax(left,exp_f,&min_L,&max_L);
//BinMinMax(right,exp_f,&min_R,&max_R);
//printf("lmin: %f, lmax: %f, rmin: %f rmax: %f\n",
//	min_L,max_L,min_R,max_R);
		
		}

		all_L = all_L - both_L + left_L + right_L;

		curr_bin_count++;


//printf("best_split: %d\n",best_split);

		jrb_free_tree(sorted);
	}

	if(bic_list == NULL)
	{
		jrb_traverse(jr,work_list)
		{
			b = (Bin *)jval_v(jrb_val(jr));
			BinMinMax(b,exp_f,&min,&max);
			jrb_insert_dbl(blist,min,new_jval_v(b));
		}
	}
	else
	{
		jrb_traverse(jr,bic_list)
		{
			b = (Bin *)jval_v(jrb_val(jr));
			BinMinMax(b,exp_f,&min,&max);
//printf("min: %f, max:%f\n",min,max);
//fflush(stdout);
			jrb_insert_dbl(blist,min,new_jval_v(b));
		}
		jrb_free_tree(bic_list);
	}
	jrb_free_tree(work_list);


	i = 0;
	jrb_traverse(jr,blist)
	{
		b = (Bin *)jval_v(jrb_val(jr));
		while(jrb_next(jr) != jrb_nil(blist))
		{
			next_b = (Bin *)jval_v(jrb_val(jrb_next(jr)));
			BinMinMax(b,exp_f,&min_L,&max_L);
			BinMinMax(next_b,exp_f,&min_R,&max_R);

			if(min_L == min_R)
			{
				new_b = MergeBins(b,next_b);
				FreeBin(b);
				b = new_b;
				FreeBin(next_b);
				jr = jrb_next(jr);
			}
			else
			{
				break;
			}
		}
		bins[i] = b;
		i++;
	}
	jrb_free_tree(blist);

//printf("best_bic: %f, best_count: %d, final count: %d\n",
//		best_bic,best_bin_count,i);
		
	
	*means = i;
	return(bins);
}

#ifdef TEST

char *ARGS = "f:vKNB:";
char *Usage = "glr_cluster -f filename\n\
\t-B bins\n\
\t-K <keep going>\n\
\t-N <use log normal>\n\
\t-v <verbose>\n";

#define DATA_FIELDS (4)
#define EXP_FIELD (3)
#define RESP_FIELD (1)

int Verbose = 0;
int Keep_going;
int UseNormal = 0;
char Fname[255];

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
	int bin_count;
	
	Verbose = 0;
	Keep_going = 0;
	UseNormal = 0;
	while((c = getopt(argc,argv,ARGS)) != EOF)
	{
		switch(c)
		{
			case 'f':
				strncpy(Fname,optarg,sizeof(Fname));
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
			case 'B':
				Max_bin = atoi(optarg);
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

	bins = SchmibGLRT(data,
			   DATA_FIELDS,
			   EXP_FIELD,
			   RESP_FIELD,
			   Max_bin,
			   &bin_count);

	if(bins == NULL)
	{
		exit(1);
	}
	for(i=0; i < bin_count; i++)
	{
	    PrintBin(stdout,bins[i],
			EXP_FIELD,
			RESP_FIELD,
			Max_bin);
	}
	
	return(0);
}

#endif
