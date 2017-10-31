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

#define DEBUG_SAVE_RESTORE 0

History *MakeHistory(int fields)
{
	History *h;
	int err;

	h = (History *)malloc(sizeof(History));
	if(h == NULL)
	{
		exit(1);
	}

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
	err = InitDataSet(&h->data,fields);
	if(err == 0)
	{
		jrb_free_tree(h->value_list);
		jrb_free_tree(h->age_list);
		free(h);
		return(NULL);
	}

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
	copy.data = NULL;
	copy.age_list = NULL;
	copy.value_list = NULL;
	fwrite(&copy, sizeof(History), 1, fd);

	// step 2: save out the age list red black tree
	JRB ptr;
    jrb_traverse(ptr, h->age_list)
    {
        fputc(1, fd);
        fwrite(&ptr->key, sizeof(Jval), 1, fd);
        fwrite(&ptr->val, sizeof(Jval), 1, fd);
    }
    fputc(0, fd);

	// step 3: save out the value list red black tree
    jrb_traverse(ptr, h->value_list)
    {
        fputc(1, fd);
        fwrite(&ptr->key, sizeof(Jval), 1, fd);
        fwrite(&ptr->val, sizeof(Jval), 1, fd);
    }
	fputc(0, fd);
	
	// step 4: write out the history data set
	SaveBinaryDataSet(h->data, fd);

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

	// step 2: read the age list
	JRB age_list = make_jrb();
	while (fgetc(fd)) 
	{
		Jval key, val;
		fread(&key, sizeof(Jval), 1, fd);
		fread(&val, sizeof(Jval), 1, fd);
		jrb_insert_dbl(age_list, jval_d(key), val);
	}

	h->age_list = age_list;

	// step 3: read the value list
	JRB value_list = make_jrb();
	while (fgetc(fd)) 
	{
		Jval key, val;
		fread(&key, sizeof(Jval), 1, fd);
		fread(&val, sizeof(Jval), 1, fd);
		jrb_insert_dbl(value_list, jval_d(key), val);
	}
	h->value_list = value_list;

	// step 4: read in the history data set
	LoadBinaryDataSet(&(h->data), fields, fd);
	assert(h->data != NULL);

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
	 * free the value list first
	 */
	jrb_free_tree(h->value_list);
	jrb_free_tree(h->age_list);

	FreeDataSet(h->data);

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

	last = SizeOf(h->data) - 1;
	Seek(h->data,last);

	/*
	 * last argument is a dummy for expansion purposes
	 */
	values[0] = ts;
	values[1] = value;
	values[2] = lowpred;
	values[3] = highpred;
	values[4] = exp_value;
	values[5] = node_count;
	values[6] = exec_time;
	/*
	WriteEntry7(h->data,ts,value,lowpred,highpred,exp_value,node_count,
		exec_time);
	*/
	WriteData(h->data,7,values);
	index = Current(h->data);

	/*
	 * sort value list by values
	 */
	(void) jrb_insert_dbl(h->value_list,value,new_jval_i(index));
	/*
	 * sort age list by ts
	 */
	jrb_insert_dbl(h->age_list,ts,new_jval_i(index));
	h->count++;

	return;
}

int GetHistorySize(History *h)
{
	int list_size;
	
	list_size = h->count;

	return(list_size);

}

double GetHistoryValue(History *h, int index)
{
	int count;
	double *vals;
	int i;
	double value;
	JRB curr;

	count = 1;
	jrb_traverse(curr,h->value_list)
	{
		if(count >= index)
			break;

		count++;
	}

	if(curr == NULL)
	{
		curr = jrb_last(h->value_list);
	}

	i = jval_i(jrb_val(curr));
	vals = GetFieldValues(h->data,J_VALUE);
	value = vals[i];

	return(value);
}

double GetHistoryAge(History *h, int index)
{
	int count;
	double *vals;
	int i;
	double ts;
	JRB curr;
	JRB temp;

	count = 1;
	jrb_traverse(curr,h->age_list)
	{
		if(count >= index)
			break;

		count++;
	}

	if(curr == NULL)
	{
		curr = jrb_last(h->age_list);
	}

	i = jval_i(jrb_val(curr));
	vals = GetFieldValues(h->data,J_TS);
	ts = vals[i];


	return(ts);
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

	count = 1;
	jrb_traverse(curr,h->age_list)
	{
		if(count >= index)
			break;

		count++;
	}

	if(curr == NULL)
	{
		curr = jrb_last(h->age_list);
	}

	i = jval_i(jrb_val(curr));
	

	vals = GetFieldValues(h->data,J_TS);
	*ts = vals[i];
	vals = GetFieldValues(h->data,J_VALUE);
	*value = vals[i];
	vals = GetFieldValues(h->data,J_HIGHPRED);
	*highpred = vals[i];
	vals = GetFieldValues(h->data,J_LOWPRED);
	*lowpred = vals[i];
	/*
	 * not used if we are not doing clustering
	 */
	if(exp_value != NULL)
	{
		vals = GetFieldValues(h->data,J_EXP);
		*exp_value = vals[i];
	}
	if(node_count != NULL)
	{
		vals = GetFieldValues(h->data,J_NODECOUNT);
		*node_count = vals[i];
	}
	if(exec_time != NULL)
	{
		vals = GetFieldValues(h->data,J_EXECTIME);
		*exec_time = vals[i];
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
	int vindex;

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
	vals = GetFieldValues(h->data,J_VALUE);
	time_vals = GetFieldValues(h->data,J_TS);
	jrb_traverse(curr,h->age_list)
	{
		
		ndx = jval_i(jrb_val(curr));
		value = vals[ndx];
		/*  FIX ME!
		value_entry = jrb_find_dbl(h->value_list,value);
		vindex = jval_i(jrb_val(value_entry));
		 */
		value_entry = jrb_first(h->value_list);
		vindex = jval_i(jrb_val(value_entry));
		while(vindex != ndx)
		{
			value_entry = value_entry->flink;
			vindex = jval_i(jrb_val(value_entry));
		}
		if(vindex != ndx)
		{
			fprintf(stderr,"error: couldn't find value entry\n");
			fflush(stderr);
			exit(1);
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
	int vindex;

	if(jrb_empty(h->age_list))
		return;

	if(h->count <= min)
		return;


	jrb_traverse(curr,h->age_list)
	{
		vals = GetFieldValues(h->data,J_VALUE);
		ndx = jval_i(jrb_val(curr));
		value = vals[ndx];

		value_entry = jrb_first(h->value_list);
		vindex = jval_i(jrb_val(value_entry));
		while(vindex != ndx)
		{
			value_entry = value_entry->flink;
			vindex = jval_i(jrb_val(value_entry));
		}
		if(vindex != ndx)
		{
			fprintf(stderr,"error: couldn't find value entry\n");
			fflush(stderr);
			exit(1);
		}
		if(value_entry == NULL)
		{
			fprintf(stderr,
				"WARNING: missing value in trim\n");
			fflush(stderr);
		}

		vals = GetFieldValues(h->data,J_TS);
		ts = vals[ndx];

		/*
		 * ties don't count
		 */
		if(ts > oldest)
			break;

		/*
		 * delete the node from the value list
		 */
		jrb_delete_node(value_entry);
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
	ndx = jval_i(jrb_val(first));

	vals = GetFieldValues(h->data,J_TS);
	*ts = vals[ndx];

	vals = GetFieldValues(h->data,J_VALUE);
	*value = vals[ndx];

	vals = GetFieldValues(h->data,J_HIGHPRED);
	*highpred = vals[ndx];

	vals = GetFieldValues(h->data,J_LOWPRED);
	*lowpred = vals[ndx];

	/*
	 * not used if we are not doing clustering
	 */
	if(exp_value != NULL)
	{
		vals = GetFieldValues(h->data,J_EXP);
		*exp_value = vals[ndx];
	}
	if(node_count != NULL)
	{
		vals = GetFieldValues(h->data,J_NODECOUNT);
		*node_count = vals[ndx];
	}
	if(exec_time != NULL)
	{
		vals = GetFieldValues(h->data,J_EXECTIME);
		*exec_time = vals[ndx];
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
	ndx = jval_i(jrb_val(first));
	vals = GetFieldValues(h->data,J_VALUE);
	v = vals[ndx];
	value_entry = jrb_find_dbl(h->value_list,v);
	if(value_entry == NULL)
	{
		fprintf(stderr,
			"missing value entry in RemoveOldestHistory\n");
		fflush(stderr);
	}
	
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
	int ndx;

	ts_vals = GetFieldValues(h->data, ts_field);
	vals = GetFieldValues(h->data,val_field);
	jrb_traverse(jr,h->value_list)
	{
		ndx = jval_i(jrb_val(jr));
		fprintf(stdout,"\t%f %f %d\n",ts_vals[ndx],vals[ndx],ndx);
	}

	return;
}
	


