#ifndef BATCH_QUEUE_SIM_H
#define BATCH_QUEUE_SIM_H

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
#define J_HIGHPRED (2)
#define J_LOWPRED (3)
#define J_EXP (4)       /* explanatory var in clustering case */
#define J_NODECOUNT (5) /* nodes requested */
#define J_EXECTIME (6)  /* actual run time (for SPRUCE next-to-run sim) */


/*
 * total number of fields
 */
#define J_FIELDS (7)

#include "schmib_history.h"
        
struct q_sim_stc
{
	History *pending;
	History *completed; /* completed its waiting time */
	History *running;   /* needed for SPRUCE simulation */
	int running_node_count;
	double now;
};

typedef struct q_sim_stc Qsim;

Qsim *InitQsim(double now, int fields);
void FreeQsim(Qsim *q);
void UpdateQsimState(Qsim *q, double now);
void JobArrives(Qsim *q, double ts, double waiting_time, double lowpred,
double highpred, double pcent, double node_count, double exec_time);
double TimeUntil(Qsim *q, double now, int node_count, int total_nodes);

void FlushPendingQueue(Qsim *q);

#endif

