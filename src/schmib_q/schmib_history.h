#ifndef SCHMIB_HISTORY_H
#define SCHMIB_HISTORY_H

#include "simple_input.h"

#include "jval.h"
#include "jrb.h"

/*
struct job_rec
{ 
  double ts;
  double value;
  double highpred;
  double lowpred;
  double pcent;
  int index;
};
*/

/*
 * fields for data set of job histories
 */
#define J_TS (0)
#define J_VALUE (1)
#define J_LOWPRED (2)
#define J_HIGHPRED (3)
#define J_EXP (4)	/* explanatory var in clustering case */
#define J_NODECOUNT (5)	/* nodes requested */
#define J_EXECTIME (6)  /* actual run time (for SPRUCE next-to-run sim) */

/*
 * total number of fields
 */
#define J_FIELDS (7)
        
int JobIndex;
  
struct history_stc
{ 
	void *data;		/* data set defined by simple_input */
	JRB value_list;
	JRB age_list;
	int count;
};

typedef struct history_stc History;

History *MakeHistory(int fields);
History *LoadHistory(int fields, FILE *fd);
void SaveHistory(History *h, FILE *fd);
void AddToHistory(History *h, double ts, double resp_value,
		double lowpred, double highpred, double exp_value,
		double node_count, double exec_time);
int GetHistorySize(History *h);
double GetHistoryValue(History *h, int index);
double GetHistoryAge(History *h, int index);
int GetHistoryEntry(History *h, int index, double *ts, double *value, double
	*lowpred, double *highpred, double *exp_value, double *node_count,
	double *exec_time);
void TrimHistoryValue(History *h, int count);
void TrimHistoryAge(History *h, double oldest, int min);
void FreeHistory(History *h);
int GetOldestHistory(History *h, double *ts, double *value, 
		double *lowpred, double *highpred, double *exp_value,
		double *node_count, double *exec_time);
void RemoveOldestHistory(History *h, double *ts, double *value, 
		double *lowpred, double *highpred, double *exp_value,
		double *node_count, double *exec_time);
void PrintHistoryValue(History *h, int ts_field, int val_field);

#endif

