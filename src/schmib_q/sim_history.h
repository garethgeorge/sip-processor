#ifndef SIM_HISTORY_H
#define SIM_HISTORY_H

#include "simple_input.h"

#include "jval.h"
#include "jrb.h"

struct sim_history_stc
{ 
        JRB age_list;
	int count;
};

typedef struct sim_history_stc SimHistory;

SimHistory *MakeSimHistory();
void AddToSimHistory(SimHistory *h, double ts, double value); 
int GetSimHistorySize(SimHistory *h);
double GetSimHistoryValue(SimHistory *h, int index);
double GetSimHistoryAge(SimHistory *h, int index);
int GetSimHistoryEntry(SimHistory *h, int index, double *ts, double *value);
void TrimSimHistoryValue(SimHistory *h, int count);
void TrimSimHistoryAge(SimHistory *h, double oldest, int min);
void FreeSimHistory(SimHistory *h);
int GetOldestSimHistory(SimHistory *h, double *ts, double *value);
void RemoveOldestSimHistory(SimHistory *h, double *ts, double *value);
void PrintSimHistoryValue(SimHistory *h);

#endif

