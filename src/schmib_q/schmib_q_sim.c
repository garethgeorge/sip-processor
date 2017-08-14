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

#include "schmib_history.h"
#include "schmib_q_sim.h"

extern void UpdateDownTime(double ts, double value);

Qsim *InitQsim(double now, int fields)
{
	Qsim *q;

	q = (Qsim *)malloc(sizeof(Qsim));
	if(q == NULL)
	{
		return(NULL);
	}

	q->pending = MakeHistory(fields);
	if(q->pending == NULL)
	{
		free(q);
		return(NULL);
	}

	q->completed = MakeHistory(fields);
	if(q->completed == NULL)
	{
		FreeHistory(q->pending);
		free(q);
		return(NULL);
	}

#ifdef RUNNING
	q->running = MakeHistory(fields);
	if(q->running == NULL)
	{
		FreeHistory(q->pending);
		FreeHistory(q->completed);
		free(q);
		return(NULL);
	}
	q->running_node_count = 0;
#endif

	q->now = now;

	return(q);
}

void FreeQsim(Qsim *q)
{
	FreeHistory(q->pending);
	FreeHistory(q->completed);
#ifdef RUNNING
	FreeHistory(q->running);
#endif
	free(q);
	return;
}

double Last_job_out;
double Last_job_ts;

void FlushPendingQueue(Qsim *q)
{
	int err;
	double oldest_time;
	double oldest_value;
	double oldest_lowpred;
	double oldest_highpred;
	double oldest_exp_value;
	double oldest_node_count;
	double oldest_exec_time;

	err = GetOldestHistory(q->pending,&oldest_time,&oldest_value,
                &oldest_lowpred, &oldest_highpred, &oldest_exp_value,
                &oldest_node_count,&oldest_exec_time);

	while(err != 0)
	{
		RemoveOldestHistory(q->pending,&oldest_time,
				&oldest_value,
				&oldest_lowpred, &oldest_highpred, 
				&oldest_exp_value,
				&oldest_node_count,&oldest_exec_time);
		err = GetOldestHistory(q->pending,&oldest_time,&oldest_value,
                	&oldest_lowpred, &oldest_highpred, &oldest_exp_value,
                	&oldest_node_count,&oldest_exec_time);
	}


	return;
}

void UpdateQsimState(Qsim *q, double now)
{
	double oldest_time;
	double oldest_value;
	double oldest_lowpred;
	double oldest_highpred;
	double oldest_exp_value;
	double oldest_node_count;
	double oldest_exec_time;
	int err;

#ifdef RUNNING
	/*
	 * first, run any jobs that need to be run
	 */
	err = GetOldestHistory(q->running,&oldest_time,&oldest_value,
		&oldest_lowpred, &oldest_highpred, &oldest_exp_value,
		&oldest_node_count,&oldest_exec_time);

	/*
	 * age out jobs that are running
	 */
	if(err != 0)
	{
		while(now >= oldest_time)
		{
			RemoveOldestHistory(q->running,&oldest_time,
				&oldest_value,
				&oldest_lowpred, &oldest_highpred, 
				&oldest_exp_value,
				&oldest_node_count,&oldest_exec_time);
			q->running_node_count -= oldest_node_count;
			err = GetOldestHistory(q->running,&oldest_time,
					&oldest_value,
				&oldest_lowpred, &oldest_highpred, 
				&oldest_exp_value,
				&oldest_node_count,&oldest_exec_time);
			if(err == 0)
			{
				break;
			}
		}
	}
#endif
		
		
	/*
	 * move all jobs from the pending queue to the completed
	 * queue that have a ts >= now
	 */
	err = GetOldestHistory(q->pending,&oldest_time,&oldest_value,
		&oldest_lowpred, &oldest_highpred, &oldest_exp_value,
		&oldest_node_count,&oldest_exec_time);
	if(err == 0)
	{
		q->now = now;
		return;
	}
	while(now >= oldest_time)
	{
		RemoveOldestHistory(q->pending,&oldest_time,&oldest_value,
			&oldest_lowpred, &oldest_highpred, &oldest_exp_value,
			&oldest_node_count,&oldest_exec_time);
		AddToHistory(q->completed,oldest_time, oldest_value, 
				oldest_lowpred, oldest_highpred,
							oldest_exp_value,
				oldest_node_count,oldest_exec_time);
		if(oldest_exec_time != 0)
		{
//printf("adding running job %10.0f %f %f\n",oldest_time+oldest_exec_time,
//			oldest_value,
//			oldest_node_count,
//			oldest_exec_time);
//fflush(stdout);
#ifdef RUNNING
			AddToHistory(q->running,
				oldest_time+oldest_exec_time, 
				oldest_value, 
				oldest_lowpred, oldest_highpred,
							oldest_exp_value,
				oldest_node_count,oldest_exec_time);
			q->running_node_count += oldest_node_count;
#endif
		}

		/*
		 * take care of the duplicates
		 */
		if(Last_job_ts < oldest_time)
		{
			Last_job_out = oldest_time - Last_job_ts;
			Last_job_ts = oldest_time;
#ifdef DOWN_TIME
			UpdateDownTime(Last_job_ts, Last_job_out);
#endif
		}
		err = GetOldestHistory(q->pending,&oldest_time,&oldest_value,
			&oldest_lowpred, &oldest_highpred, &oldest_exp_value,
			&oldest_node_count,&oldest_exec_time);
		if(err == 0)
		{
			break;
		}
	}

	q->now = now;

	return;
}

void JobArrives(Qsim *q, double ts, double waiting_time, 
		double lowpred, double highpred, double exp_value,
		double node_count, double exec_time)
{
	/*
	 * first, add this job to the pending queue
	 * 
	 * the time stamp should be the time when the job will exit the
	 * queue
	 */
	AddToHistory(q->pending,ts+waiting_time,waiting_time, 
		lowpred, highpred, exp_value, node_count, exec_time);
	
	/*
	 * now update the simulation state assuming that the ts is the
	 * arrival now -- i.e. now
	 */
	UpdateQsimState(q,ts);

	return;
}	

#ifdef RUNNING
double TimeUntil(Qsim *q, double now, int node_count, int total_nodes)
{
	int i;
	double until;
	double elapsed;
	double count;
	int err;
	double ts;
	double value;
	double lowpred;
	double highpred;
	double exp_value;
	double nodes;
	double exec_time;

	i = 1;
	count = 0;
	until = 0;

	count = total_nodes - q->running_node_count;
	if(count >= node_count)
	{
		return(0.0);
	}

	

	while(count <= node_count)
	{
		err = GetHistoryEntry(q->running,i,
			&ts,
			&value,
			&lowpred,
			&highpred,
			&exp_value,
			&nodes,
			&exec_time);
		/*
		 * if there is nothing running, run now
		 */
		if(err == 0)
		{
			return(until);
		}
		/*
		 * sanity check
		 */
		if(ts < now)
		{
			i++;
			continue;
		}
		count += nodes;
		elapsed = ts - now;
		if(elapsed > until)
		{
			until = elapsed;
		}
		if(count > node_count)
		{
			break;
		}
		i++;
	} 

	return(until);
}
#endif
