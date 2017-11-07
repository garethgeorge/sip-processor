/*
 * http://www.swox.com/gmp/manual
 */
#include <assert.h>
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

#include "schmib_history.h"

// #define DEBUG_SAVE_RESTORE 1

History *MakeHistory(int fields)
{
	History *h;
	int err;

	h = (History *)malloc(sizeof(History));
	if(h == NULL)
	{
		exit(1);
	}
	bzero(h, sizeof(History));

	
	h->value_list = make_jrb();
	if(h->value_list == NULL)
	{
		exit(1);
	}

	h->age_list = make_jrb();
	if(h->age_list == NULL)
	{
		exit(1);
	}

	h->count = 0;	

	return(h);
}

void SaveHistory(History* h, FILE *fd)
{
#ifdef DEBUG_SAVE_RESTORE
	fputc(0x02, fd);
	fprintf(fd, "Begin History");
#endif

	// step 1: save out the struct itself
	History copy;
	memcpy(&copy, h, sizeof(History));
	// copy.data = NULL;
	copy.age_list = NULL;
	copy.value_list = NULL;
	fwrite(&copy, sizeof(History), 1, fd);
	
	// step 2: save out all of the history points
	JRB ptr;
	jrb_traverse(ptr, h->age_list)
	{
		HistoryPoint* hP = (HistoryPoint *)jval_v(jrb_val(ptr));
		fputc(1, fd);
		fwrite(hP, sizeof(HistoryPoint), 1, fd);
	}
	fputc(0, fd);

#ifdef DEBUG_SAVE_RESTORE
	fputc(0x02, fd);
	fprintf(fd, "End History");
#endif
}

History *LoadHistory(int fields, FILE *fd)
{
#ifdef DEBUG_SAVE_RESTORE 
	printf("Loading History\n");
	assert(fgetc(fd) == 0x02);
	fseek(fd, sizeof("Begin History") - 1, SEEK_CUR);
#endif

	// step 1: read in the struct itself
	History *h;
	h = (History *)malloc(sizeof(History));
	fread(h, sizeof(History), 1, fd);

	JRB age_list = make_jrb();
	JRB value_list = make_jrb();

	while (fgetc(fd))
	{
		HistoryPoint *hP = (HistoryPoint *)malloc(sizeof(HistoryPoint));
		fread(hP, sizeof(HistoryPoint), 1, fd);
		jrb_insert_dbl(age_list, hP->ts, new_jval_v((void *)hP));
		jrb_insert_dbl(value_list, hP->value, new_jval_v((void *)hP));
	}

	h->age_list = age_list;
	h->value_list = value_list;

#ifdef DEBUG_SAVE_RESTORE 
	assert(fgetc(fd) == 0x02);
	fseek(fd, sizeof("End History") - 1, SEEK_CUR);
#endif

	return h;
}

void FreeHistory(History *h)
{
	JRB curr;

	/*
	 * free all of the data points...
	 */
	jrb_traverse(curr, h->age_list)
	{
		HistoryPoint *p = (HistoryPoint *)jval_v(jrb_val(curr));
		free(p);
	}

	/*
	 * free the value list first
	 */
	jrb_free_tree(h->value_list);
	jrb_free_tree(h->age_list);

	// FreeDataSet(h->data);

	free(h);

	return;
}

void AddToHistory(History *h, double ts, 
	double value, double lowpred, double highpred, double exp_value,
	double node_count, double exec_time)
{
	int index;
	int last;
	double values[7];

	// last = SizeOf(h->data) - 1;
	// Seek(h->data,last);

	/*
	 * last argument is a dummy for expansion purposes
	 */
	// values[0] = ts;
	// values[1] = value;
	// values[2] = lowpred;
	// values[3] = highpred;
	// values[4] = exp_value;
	// values[5] = node_count;
	// values[6] = exec_time;
	/*
	WriteEntry7(h->data,ts,value,lowpred,highpred,exp_value,node_count,
		exec_time);
	*/
	// WriteData(h->data,7,values);
	// index = Current(h->data);

	HistoryPoint *p = (HistoryPoint *)malloc(sizeof(HistoryPoint));
	p->ts = ts;
	p->value = value;
	p->lowpred = lowpred;
	p->highpred = highpred;
	p->exp = exp_value;
	p->nodecount = node_count;
	p->exectime = exec_time;

	/*
	 * sort value list by values
	 */
	jrb_insert_dbl(h->value_list, value, new_jval_v((void *)p));
	/*
	 * sort age list by ts
	 */
	jrb_insert_dbl(h->age_list,ts,new_jval_v((void *)p));
	h->count++;

	return;
}

int GetHistorySize(History *h)
{
	return h->count;
}

double GetHistoryValue(History *h, int index)
{
	int count;
	double *vals;
	int i;
	double value;
	JRB curr;
	// uccess_percentage: 0.977571
	// 4.23 real         4.01 user         0.07 sys

	if (index <= h->count / 2)
	{
		count = 1;
		jrb_traverse(curr,h->value_list)
		{
			if(count >= index)
				break;
	
			count++;
		}
	} 
	else 
	{
		count = h->count;
		jrb_rtraverse(curr, h->value_list)
		{
			if (count <= index)
				break;
			count--;
		}
	}

	if (curr == NULL)
	{
		curr = jrb_last(h->value_list);
	}

	

	HistoryPoint *p = (HistoryPoint *)jval_v(jrb_val(curr));
	// vals = GetFieldValues(h->data,J_VALUE);
	// value = vals[i];

	return(p->value);
}

double GetHistoryAge(History *h, int index)
{
	int count;
	double *vals;
	int i;
	double ts;
	JRB curr;
	JRB temp;

	if (index <= h->count / 2)
	{
		count = 1;
		jrb_traverse(curr,h->age_list)
		{
			if(count >= index)
				break;
	
			count++;
		}
	} 
	else 
	{
		count = h->count;
		jrb_rtraverse(curr, h->age_list)
		{
			if (count <= index)
				break;
			count--;
		}
	}

	if (curr == NULL)
	{
		curr = jrb_last(h->age_list);
	}

	HistoryPoint *p = (HistoryPoint *)jval_v(jrb_val(curr));

	// i = jval_i(jrb_val(curr));
	// vals = GetFieldValues(h->data,J_TS);
	// ts = vals[i];
	return(p->ts);
}		

int GetHistoryEntry(History *h, int index, double *ts, double *value, 
double *lowpred, double *highpred, double *exp_value, double *node_count, 
double *exec_time)
{
	int count;
	JRB curr;
	double *vals;
	int i;
	double l_ts;
	double l_value;
	double l_lowpred;
	double l_highpred;
	double l_pcent;

	if(index > h->count)
	{
		return(0);
	}

	if(h->count == 0)
	{
		return(0);
	}

	if (index <= h->count / 2)
	{
		count = 1;
		jrb_traverse(curr,h->age_list)
		{
			if(count >= index)
				break;
	
			count++;
		}
	} 
	else 
	{
		count = h->count;
		jrb_rtraverse(curr, h->age_list)
		{
			if (count <= index)
				break;
			count--;
		}
	}

	if (curr == NULL)
	{
		curr = jrb_last(h->age_list);
	}
	

	// i = jval_i(jrb_val(curr));
	HistoryPoint *p = jval_v(jrb_val(curr));

	*ts = p->ts;
	*value = p->value;
	*highpred = p->highpred;
	*lowpred = p->lowpred;

	/*
	 * not used if we are not doing clustering
	 */
	if(exp_value != NULL)
	{
		// vals = GetFieldValues(h->data,J_EXP);
		// *exp_value = vals[i];
		*exp_value = p->value;
	}
	if(node_count != NULL)
	{
		// vals = GetFieldValues(h->data,J_NODECOUNT);
		// *node_count = vals[i];
		*node_count = p->nodecount;
	}
	if(exec_time != NULL)
	{
		// vals = GetFieldValues(h->data,J_EXECTIME);
		// *exec_time = vals[i];
		*exec_time = p->exectime;
	}
	return(1);
}

void TrimHistoryValue(History *h, int count)
{
	int list_size;
	JRB curr;
	int i;
	JRB value_entry;
	JRB tempj;
	int ndx;
	double *vals;
	double *time_vals;
	double value;

	/*
	 * possible optimization: if the values in the data
	 * are already in time series order, a faster way to do this
	 * is to delete all of the relevant entries from the age list
	 * and then to walk through the value list and throw out any
	 * value with a lower index that the smallest index in the age list
	 */

	if(jrb_empty(h->age_list))
		return;

	/*
	 * compute the list size as the difference between the smallest and
	 * biggest index
	 */

	list_size = GetHistorySize(h);

	if(count >= list_size)
	{
		return;
	}

	i = list_size;
	// vals = GetFieldValues(h->data,J_VALUE);
	// time_vals = GetFieldValues(h->data,J_TS);
	jrb_traverse(curr,h->age_list)
	{
		HistoryPoint *p = (HistoryPoint *)jval_v(jrb_val(curr));
		// ndx = jval_i(jrb_val(curr));
		// value = vals[ndx];
		/*  FIX ME!
		value_entry = jrb_find_dbl(h->value_list,value);
		vindex = jval_i(jrb_val(value_entry));
		 */

		value_entry = jrb_first(h->value_list);
		while (value_entry != NULL && jval_v(jrb_val(value_entry)) != p)
		{
			value_entry = value_entry->flink;
		}

		if(value_entry == NULL)
		{
			fprintf(stderr,
				"WARNING: missing value in trim\n");
			fflush(stderr);
			exit(1);
		}

		/*
		 * delete the value node from the value list
		 */
		jrb_delete_node(value_entry);

		/*
		 * delete the node from the age list
		 */
		tempj = curr->blink;

		/*
		 * free the HistoryPoint struct associated with the node being trimmed
		 */
		free((HistoryPoint *)jval_v(jrb_val(curr)));

		/*
		 * delete the age record
		 */
		jrb_delete_node(curr);
		curr = tempj;
		h->count--;

		i--;

		if(i <= count)
			break;

		if(i <= 0)
			break;
	}

	return;
}

void TrimHistoryAge(History *h, double oldest, int min)
{
	JRB curr;
	JRB value_entry;
	JRB tempj;
	int ndx;
	double *vals;
	double value;
	double ts;

	if(jrb_empty(h->age_list))
		return;

	if(h->count <= min)
		return;


	jrb_traverse(curr,h->age_list)
	{
		HistoryPoint *p = (HistoryPoint *)jval_v(jrb_val(curr));

		value_entry = jrb_first(h->value_list);
		while(value_entry != NULL && jval_v(jrb_val(value_entry)) != p)
		{
			value_entry = value_entry->flink;
		}

		if (value_entry == NULL)
		{
			fprintf(stderr, "WARNING: missing value in trim\n");
			fflush(stderr);
			exit(1);
		} 

		// vals = GetFieldValues(h->data,J_VALUE);
		// ndx = jval_i(jrb_val(curr));
		// value = vals[ndx];

		// value_entry = jrb_first(h->value_list);
		// vindex = jval_i(jrb_val(value_entry));
		// while(vindex != ndx)
		// {
		// 	value_entry = value_entry->flink;
		// 	vindex = jval_i(jrb_val(value_entry));
		// }
		// if(vindex != ndx)
		// {
		// 	fprintf(stderr,"error: couldn't find value entry\n");
		// 	fflush(stderr);
		// 	exit(1);
		// }
		// if(value_entry == NULL)
		// {
		// 	fprintf(stderr,
		// 		"WARNING: missing value in trim\n");
		// 	fflush(stderr);
		// }

		// vals = GetFieldValues(h->data,J_TS);
		// ts = vals[ndx];

		/*
		 * ties don't count
		 */
		if(p->ts > oldest)
			break;

		/*
		 * delete the node from the value list
		 */
		jrb_delete_node(value_entry);
		
		/*
		 * free the HistoryPoint struct associated with the node being trimmed
		 */
		free((HistoryPoint *)jval_v(jrb_val(curr)));
		
		/*
		 * delete the node from the age list
		 */
		tempj = curr->blink;
		jrb_delete_node(curr);
		curr = tempj;
	
		h->count--;

		if(h->count <= min)
			break;

	}

	return;
}

int GetOldestHistory(History *h, double *ts, double *value, double *lowpred,
double *highpred, double *exp_value, double *node_count, double *exec_time)
{
	JRB first;
	int ndx;
	double f;
	double *vals;

	if(h->count == 0)
	{
		return(0);
	}

	first = jrb_first(h->age_list);
	HistoryPoint *p = (HistoryPoint *)jval_v(jrb_val(first));
	// ndx = jval_i(jrb_val(first));

	// vals = GetFieldValues(h->data,J_TS);
	// *ts = vals[ndx];
	*ts = p->ts;

	// vals = GetFieldValues(h->data,J_VALUE);
	// *value = vals[ndx];
	*value = p->value;

	// vals = GetFieldValues(h->data,J_HIGHPRED);
	// *highpred = vals[ndx];
	*highpred = p->highpred;

	// vals = GetFieldValues(h->data,J_LOWPRED);
	// *lowpred = vals[ndx];
	*lowpred = p->lowpred;

	/*
	 * not used if we are not doing clustering
	 */
	if(exp_value != NULL)
	{
		// vals = GetFieldValues(h->data,J_EXP);
		// *exp_value = vals[ndx];
		*exp_value = p->exp;
	}
	if(node_count != NULL)
	{
		// vals = GetFieldValues(h->data,J_NODECOUNT);
		// *node_count = vals[ndx];
		*node_count = p->nodecount;
	}
	if(exec_time != NULL)
	{
		// vals = GetFieldValues(h->data,J_EXECTIME);
		// *exec_time = vals[ndx];
		*exec_time = p->exp;
	}

	return(1);
}

void RemoveOldestHistory(History *h, double *ts, double *value, double
*lowpred, double *highpred, double *exp_value, double *node_count,
double *exec_time)
{
	JRB first;
	JRB value_entry;
	double *vals;
	int ndx;
	double v;

	if(h->count == 0)
	{
		return;
	}

	GetOldestHistory(h,ts,value,lowpred,highpred,exp_value,node_count,
		exec_time);
	first = jrb_first(h->age_list);
	
	HistoryPoint *p = (HistoryPoint *)jval_v(jrb_val(first));
	// ndx = jval_i(jrb_val(first));
	// vals = GetFieldValues(h->data,J_VALUE);
	// v = vals[ndx];
	value_entry = jrb_find_dbl(h->value_list, p->value);
	if(value_entry == NULL)
	{
		fprintf(stderr,
			"missing value entry in RemoveOldestHistory\n");
		fflush(stderr);
	}
	
	free(p);
	jrb_delete_node(value_entry);
	jrb_delete_node(first);

	h->count--;

	return;
}
	
void PrintHistoryValue(History *h, int ts_field, int val_field)
{
	JRB jr;
	double *ts_vals;
	double *vals;
	HistoryPoint *p;

	jrb_traverse(jr,h->value_list)
	{
		p = jval_v(jrb_val(jr));
		fprintf(stdout,"\t%f %f\n",p->ts,p->value);
	}

	return;
}
	


