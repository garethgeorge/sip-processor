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

#include "sim_history.h"

struct element
{
	double ts;
	double value;
};
typedef struct element Element;

Element *GetElement()
{
	Element *e;

	e = (Element *)malloc(sizeof(Element));
	if(e == NULL)
	{
		exit(1);
	}

	return(e);
}


SimHistory *MakeSimHistory()
{
	SimHistory *h;
	int err;

	h = (SimHistory *)malloc(sizeof(SimHistory));
	if(h == NULL)
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

void FreeSimHistory(SimHistory *h)
{
	JRB curr;

	/*
	 * free the value list first
	 */
	jrb_free_tree(h->age_list);

	free(h);

	return;
}

void AddToSimHistory(SimHistory *h, double ts, double value)
{
	int index;
	int last;
	Element *e;

	e = GetElement();
	e->ts = ts;
	e->value = value;

	jrb_insert_dbl(h->age_list,ts,new_jval_v(e));


	h->count++;

	return;
}

int GetSimHistorySize(SimHistory *h)
{
	int list_size;

	list_size = h->count;

	return(list_size);

}

double GetSimHistoryAge(SimHistory *h, int index)
{
	int count;
	double *vals;
	int i;
	double ts;
	JRB curr;
	JRB temp;
	Element *e;

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

	e = (Element *)jval_v(jrb_val(curr));
	ts = e->ts;


	return(ts);
}		

int GetSimHistoryEntry(SimHistory *h, int index, double *ts, double *value)
{
	int count;
	JRB curr;
	double *vals;
	int i;
	double l_ts;
	double l_value;
	Element *e;

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

	e = (Element *)jval_v(jrb_val(curr));
	*ts = e->ts;
	*value = e->value;
	return(1);
}


int GetOldestSimHistory(SimHistory *h, double *ts, double *value)
{
	JRB first;
	int ndx;
	double f;
	double *vals;
	Element *e;

	if(h->count == 0)
	{
		return(0);
	}

	first = jrb_first(h->age_list);
	e = (Element *)jval_v(jrb_val(first));

	*ts = e->ts;

	*value = e->value;

	return(1);
}

void RemoveOldestSimHistory(SimHistory *h, double *ts, double *value)
{
	JRB first;
	JRB value_entry;
	double *vals;
	int ndx;
	double v;
	Element *e;

	if(h->count == 0)
	{
		return;
	}

	GetOldestSimHistory(h,ts,value);
	first = jrb_first(h->age_list);
	e = (Element *)jval_v(jrb_val(first));
	
	jrb_delete_node(first);
	free(e);
	h->count--;

	return;
}

void PrintSimHistory(SimHistory *h)
{
	JRB curr;
	Element *e;

	jrb_traverse(curr,h->age_list)
	{
		e = (Element *)jval_v(jrb_val(curr));
		printf("ts: %10.0f valeu: %f\n",
				e->ts,
				e->value);
		fflush(stdout);
	}

	return;
}
		
	

