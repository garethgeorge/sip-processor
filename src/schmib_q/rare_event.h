#ifndef RARE_EVENT_H
#define RARE_EVENT_H
#include "schmib_history.h"

#define RECOVERYHIGH (300)

int ConsecutiveWrong(History *h, double *);
int ConsecutiveWrongOld(History *h, double *);
int ConsecutiveWrongNorm(History *h, double *);
int ConsecutiveWrongRight(History *h, double *retautoc,
                int *wrongs, int *rights);
int IsRare(double ts, double value, double lowpred, double highpred, 
	double quantile, double confidence, 
	int wrong_count, int *low_wrong, int *high_wrong);

#endif

