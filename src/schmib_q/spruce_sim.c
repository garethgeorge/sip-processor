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

#include "schmib_q_sim.h"
#include "rare_event.h"
#include "bmbp_pred.h"
#include "weibpred.h"
#include "hyppred.h"
#include "lognpred.h"
#include "sim_history.h"

double Quantile;
double Confidence;
int Verbose;
int UseTrim;
int Mode;
int UseGLRT;
double SPRUCETime;
int SPRUCENodes;
int TotalNodesia64;
int TotalNodesia32;

int Recovery;

#define RAND() (drand48())
#define SEED() (srand48())

#define ARGS "f:c:q:vX:S:N:M:m:Ts:"
char *Usage = "spruce_sim -f filename\n\
\t-c confidence level\n\
\t-N nodes (for spruce jobs)\n\
\t-q quantile\n\
\t-X cache_file <acceleration cache file>\n\
\t-T turn of trimming\n\
\t-v <verbose>\n\
\t-M node_count <ia64 machine size>\n\
\t-m node_count <ia32 machine size>\n\
\t-s spruce file\n\
\t-S time interval (for spruce jobs)\n";

int yanyan()
{
	printf("yanyan\n");
	return(0);
}

struct mach_stc
{
	SimHistory *busy;
	SimHistory *jobs;
	int count;
	int procs;
};
typedef struct mach_stc Machine;

struct q_stc
{
	SimHistory *jobs;
	SimHistory *nodes;
	SimHistory *run_time;
};
typedef struct q_stc MachineQ;

Machine *IA32;
Machine *IA64;

MachineQ *ia_32_q;	/* jobs in queue */
MachineQ *ia_64_q;

int UseLow = 0;


Machine *InitMachineState(int procs)
{
	Machine *m;

	m = (Machine *)malloc(sizeof(Machine));
	if(m == NULL)
	{
		exit(1);
	}

	m->busy = MakeSimHistory();
	if(m->busy == NULL)
	{
		exit(1);
	}
	m->jobs = MakeSimHistory();
	if(m->jobs == NULL)
	{
		exit(1);
	}
	m->count = 0;
	m->procs = procs;

	return(m);
}

void PrintMachine(Machine *m)
{

	int i;
	int err;
	double ts;
	double orig_ts;
	double nodes;

	fprintf(stdout,"procs: %d\n",m->procs);
	fprintf(stdout,"busy: %d\n",m->count);

	i = 1;
	err = GetSimHistoryEntry(m->jobs, i, &ts, &orig_ts);
	while(err != 0)
	{
		GetSimHistoryEntry(m->busy,i,&ts,&nodes);
		fprintf(stdout,"\tts: %10.0f %f\n",orig_ts,nodes);
		i++;
		err = GetSimHistoryEntry(m->jobs, i, &ts, &orig_ts);
	}
	
	fflush(stdout);
	return;
}

MachineQ *InitMachineQ()
{
	MachineQ *q;

	q = (MachineQ *)malloc(sizeof(MachineQ));
	if(q == NULL)
	{
		exit(1);
	}

	q->nodes = MakeSimHistory();
	if(q->nodes == NULL)
	{
		exit(1);
	}

	q->run_time = MakeSimHistory();
	if(q->run_time == NULL)
	{
		exit(1);
	}
	q->jobs = MakeSimHistory();
	if(q->jobs == NULL)
	{
		exit(1);
	}

	return(q);
}

void AgeMachineState(Machine *m, double now)
{
	double ts;
	double job_ts;
	double nodes;
	int err;

	err = GetOldestSimHistory(m->busy,&ts,&nodes);

	if(err != 0)
	{
		while(now >= ts)
		{
			RemoveOldestSimHistory(m->busy,&ts,&nodes);
			m->count -= nodes;
			RemoveOldestSimHistory(m->jobs,&ts,&job_ts);
			err = GetOldestSimHistory(m->busy,&ts,&nodes);
			if(err == 0)
			{
				break;
			}
		}
	}

	return;
}


void AgeMachineQ(MachineQ *q, Machine *m, double now)
{
	int err;
	double ts;
	double l_ts;
	double job_ts;
	double nodes;
	double run_time;
	double ts32;
	double nodes32;
	int err32;

	err = GetOldestSimHistory(q->nodes,&ts,&nodes);
	if(err == 0)
	{
		return;
	}
	while(now >= ts)
	{
		RemoveOldestSimHistory(q->nodes,&ts,&nodes);
		RemoveOldestSimHistory(q->run_time,&l_ts,&run_time);
		RemoveOldestSimHistory(q->jobs,&l_ts,&job_ts);
/*
		if(m->count > m->procs)
		{
			err32 = GetOldestSimHistory(IA32->busy,&ts32,&nodes32);
			yanyan();
		}
*/
		AddToSimHistory(m->busy,ts+run_time,nodes);
		AddToSimHistory(m->jobs,ts+run_time,job_ts);
if(job_ts == 1074127773)
yanyan();
		m->count += nodes;
		if(m->count > m->procs)
		{
			fprintf(stdout,"now: %10.0f\n",now);
			PrintMachine(m);
		}
		err = GetOldestSimHistory(q->nodes,&ts,&nodes);
		if(err == 0)
		{
			break;
		}
	}

	return;

}

double TimeUntilAvailable(Machine *m, double now, int node_count)
{
	int i;
	double until;
	double elapsed;
	double count;
	int err;
	double ts;
	double nodes;

	count = m->procs - m->count;

	if(count >= node_count)
	{
		return(0.0);
	}

	i = 1;
	until = 0.0;
	while(count <= node_count)
	{
		err = GetSimHistoryEntry(m->busy,i,&ts,&nodes);
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
		if(count >= node_count)
		{
			break;
		}
		i++;
	}

	return(until);
}
		
		

int main(int argc, char *argv[])
{
	int c;
	char fname[255];
	char cname[255];
	char sname[255];
	int curr_count;
	double lowpred,highpred;
	int predcount;
	int err;
	void *data;
	void *sdata;
	double value;
	double s_value;
	Qsim *queue;
	int min_history;
	void *bp;
	double exp_value;
	double rtime;
	double s_rtime;
	double procs;
	double s_procs;
	double etime;
	double s_etime;
	double new_ts;
	double s_ts;
	double dummy_1, dummy_2;	/* for SPRUCE -- not here */
	double next_spruce;
	double now;
	double spruce_wait32;
	double spruce_wait64;
	double success;
	double total;
	double n_32;
	double n_64;
	double s_32;
	double s_64;
	int err32;
	double ts32;
	double nodes32;

	Quantile = 0.95;
	Confidence = 0.05;
	Verbose = 0;
	UseTrim = 1;
	memset(fname,0,sizeof(fname));
	memset(cname,0,sizeof(cname));
	memset(sname,0,sizeof(cname));
	SPRUCETime = 3600;
	SPRUCENodes = 16;
	TotalNodesia64 = 64;
	TotalNodesia32 = 96;

	while((c = getopt(argc,argv,ARGS)) != EOF)
	{
	  switch(c)
	    {
	    case 'f':
	      strncpy(fname,optarg,sizeof(fname));
	      break;
	    case 'q':
	      Quantile = atof(optarg);
	      break;
	    case 'c':
	      Confidence = atof(optarg);
	      break;
	    case 'X':
	      strncpy(cname,optarg,sizeof(cname));
	      break;
	    case 'v':
	      Verbose = 1;
	      break;
	    case 'S':
		SPRUCETime = atof(optarg);
		break;
	    case 's':
	      strncpy(sname,optarg,sizeof(sname));
	      break;
	    case 'N':
		SPRUCENodes = atoi(optarg);
		break;
	    case 'M':
		TotalNodesia64 = atoi(optarg);
		break;
	    case 'm':
		TotalNodesia32 = atoi(optarg);
		break;
	    case 'T':
		UseTrim = 0;
		break;
	    default:
	      fprintf(stderr,
		      "unrecognized command %c\n",(char)c);
	      fprintf(stderr,"%s",Usage);
	      exit(1);
	    }
	}

	if(fname[0] == 0)
	{
		fprintf(stderr,"must specify file name\n");
		fprintf(stderr,"%s",Usage);
		exit(1);
	}

	if(cname[0] != 0)
	{
		InitJBCache(cname,0.95,0.05);
	}

	err = InitDataSet(&data,7);
	if(err == 0)
	{
		exit(1);
	}
	err = LoadDataSet(fname,data);
	if(err == 0)
	{
		exit(1);
	}

	if(sname[0] != 0)
	{
		err = InitDataSet(&sdata,7);
		if(err == 0)
		{
			fprintf(stderr,"spruce file init failed\n");
			fflush(stderr);
			exit(1);
		}
		err = LoadDataSet(sname,sdata);
		if(err == 0)
		{
			fprintf(stderr,"spruce file load failed\n");
			fflush(stderr);
			exit(1);
		}
	}
	else
	{
		sdata = NULL;
	}

	bp = InitBMBPPred(Quantile,Confidence,J_FIELDS);
	if(bp == NULL)
	{
		exit(1);
	}

	min_history = MinHistory(Quantile,Confidence);

	/*
	 * turn off trimming if needed
	 */
	if(UseTrim == 0)
	{
		SetNoTrimSchmib(bp);
	}

	ia_32_q = InitMachineQ();
	if(ia_32_q == NULL)
	{
		exit(1);
	}
	ia_64_q = InitMachineQ();
	if(ia_64_q == NULL)
	{
		exit(1);
	}

	IA32 = InitMachineState(TotalNodesia32);
	IA64 = InitMachineState(TotalNodesia64);

	success = 0;
	total = 0;


	/*
	 * here is the main loop
	 */
	predcount = 0;
	curr_count = 0;
	next_spruce = 0;
	now = 0;
	/*
	 * if we have a spruce file
	 */
	if(sdata != NULL)
	{
		err = ReadEntry7(sdata,&s_ts,&s_value,&s_procs,&s_rtime,
				&s_etime,&s_32,&s_64);
		if(err == 0)
		{
			FreeDataSet(sdata);
			sdata = NULL;
		}
	}
	while(ReadEntry7(data,&new_ts,&value,&procs,&rtime,&etime,&n_32,&n_64))
	{
		if(value <= 0)
		{
			continue;
		}

		if(next_spruce == 0)
		{
//			next_spruce = new_ts + (SPRUCETime * RAND());
			next_spruce = new_ts;
			now = new_ts;
		}

		/*
		if(curr_count >= min_history)
		{
			ForcBMBPPred(bp,&lowpred,&highpred);
			predcount++;
		}
		*/

		/*
		 * if we are validating, check to see if the next ts
		 * is an actual spruce job
		if((sdata != NULL) && (s_ts <= next_spruce))
		{
			next_spruce = s_ts;
		}
		 */

		while(now < new_ts)
		{
			curr_count++;
			/*
			 * bump the clock forward to next_spruce time
			 * since it is before the arrival of the next job
			 */
			AgeMachineState(IA32,now);
			AgeMachineState(IA64,now);
			AgeMachineQ(ia_32_q,IA32,now);
			AgeMachineQ(ia_64_q,IA64,now);
			AgeMachineState(IA32,now);
			AgeMachineState(IA64,now);
			spruce_wait32 = TimeUntilAvailable(IA32,now,
					SPRUCENodes);
			spruce_wait64 = TimeUntilAvailable(IA64,now,
					SPRUCENodes);
			/*
			 * make sure we have run some jobs
			if(spruce_wait >= 0.0)
			{
				if(spruce_wait <= highpred)
				{
					success++;
				}
				total++;
				UpdateBMBPPred(bp,
					next_spruce,spruce_wait,lowpred,
						highpred);
			}
			 */

			/*
			 * if we are not in validation mode and there are
			 * predictions
			 */
			if((predcount > 0) && (sdata == NULL))
			{
				fprintf(stdout,"%10.0f %f %f\n",
						now,
						spruce_wait32,
						highpred);
			}

			if(sdata != NULL)
			{
				if(now == s_ts)
				{
					fprintf(stdout,
						"%10.0f %f %f %f\n",
						s_ts,
						s_value,
						spruce_wait32,
						spruce_wait64);
					/*
					 * add in the spruce job
					JobArrives(queue,
						s_ts,
						s_value,
						lowpred,
						highpred,
						exp_value,
						s_procs,
						s_etime);
					 */
					err = ReadEntry7(sdata,
						&s_ts,
						&s_value,
						&s_procs,
						&s_rtime,
						&s_etime,
						&s_32,
						&s_64);
					if(err == 0)
					{
						FreeDataSet(sdata);
						exit(1);
					}
				}
			} 
//			next_spruce = next_spruce + (SPRUCETime * RAND());
//			next_spruce = s_ts;
//			if((sdata != NULL) && (s_ts <= next_spruce))
//			{
//				next_spruce = s_ts;
//			}
			/*
			 * trace contains duplicates at the same time stamp
			 */
			if((now+1) <= s_ts)
			{
				now++;
			}
		}

		/*
		 * now add the non spruce to the machine state
		 */
		AgeMachineState(IA32,new_ts);
		AgeMachineState(IA64,new_ts);
		AgeMachineQ(ia_32_q,IA32,new_ts);
		AgeMachineQ(ia_64_q,IA64,new_ts);
		AgeMachineState(IA32,new_ts);
		AgeMachineState(IA64,new_ts);

/*
if(IA32->count > IA32->procs)
{
err32 = GetOldestSimHistory(IA32->busy,&ts32,&nodes32);
PrintSimHistory(IA32->jobs);
yanyan();
}
if(IA64->count > IA64->procs)
{
yanyan();
}
*/

/*
		if(Verbose == 1)
		{
			printf("adding %10.0f %f %f %f (%10.0f)\n",
				new_ts,
				value,
				n_32,
				n_64, 
				new_ts+value);
			fflush(stdout);
		}
*/
		if(etime > rtime)
			etime = rtime;
  		/*
   		 * crank the simulator forward to the next job arrival time
   		 */
		if(n_32 > 0)
		{
			AddToSimHistory(ia_32_q->nodes,new_ts+value,n_32);
			AddToSimHistory(ia_32_q->run_time,new_ts+value,etime);
			AddToSimHistory(ia_32_q->jobs,new_ts+value,new_ts);
		}
		if(n_64 > 0)
		{
			AddToSimHistory(ia_64_q->nodes,new_ts+value,n_64);
			AddToSimHistory(ia_64_q->run_time,new_ts+value,etime);
			AddToSimHistory(ia_64_q->jobs,new_ts+value,new_ts);
		}
	}
	
	if(total > 0)
	{
		printf("success rate: %f\n",success/total);
	}
	exit(0);
}

