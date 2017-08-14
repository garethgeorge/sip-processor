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
#define J_LOWPRED (2)
#define J_HIGHPRED (3)
#define J_PCENT (4)
#define J_INDEX (5)

/*
 * total number of fields
 */
#define J_FIELDS (7)

#include "schmib_history.h"
        
struct q_sim_stc
{
	History *pending;
	History *completed;
	double now;
};

typedef struct q_sim_stc Qsim;

Qsim *InitQsim(double now, int fields);
void FreeQsim(Qsim *q);
void UpdateQsimState(Qsim *q, double now);
void JobArrives(Qsim *q, double ts, double waiting_time, double lowpred, double highpred, double pcent);

#endif

