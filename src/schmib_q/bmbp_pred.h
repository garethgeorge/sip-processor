#ifndef BREVIK_PRED_H
#define BREVIK_PRED_H

#include "rare_event.h"

struct bmbp_stc
{
	History *data;
	int low_wrong;		/* for rare event triming */
	int high_wrong;
	int wrong_count;
	int right_count;
	int recovery;
	int recovery_high;
	double quantile;
	double confidence;
	int min_history;
	int no_trim;
	int last_index;
	int use_low;
	int use_cdf;
};

typedef struct bmbp_stc BMBPPred;


void *InitBMBPPred(double quantile, double confidence, int fields);
void *LoadBMBPPred(double quantile, double confidence, int fields, FILE *fd);
void SaveBMBPPred(void *ib, FILE *fd);
void FreeBMBPPred(void *ib);
void UpdateBMBPPred(void *ib, double ts, double value, double lowpred, double highpred);
int ForcBMBPPred(void *ib, double *lowpred, double *highpred);
int InvForcBMBPPred(void *ib, double value, double *lowp, double *highp);
void SetNoTrimBMBP(void *ib);
void SetUseCDFBMBP(void *ib);
int SizeOfBMBPPred(void *ib);
History *BMBPHistory(void *ib);
int ForcWeibullNoBinsPred(void *ib, double *lowpred, double *highpred);
int ForcLognNoBinsPred(void *ib, double *lowpred, double *highpred);
int ForcLoguNoBinsPred(void *ib, double *lowpred, double *highpred);
void SetUseLowBMBP(void *ib);
#endif


