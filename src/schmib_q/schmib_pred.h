#ifndef SCHMIB_PRED_H
#define SCHMIB_PRED_H

#include "schmib_q_sim.h"
#include "schmib-means.h"
#include "rare_event.h"

struct schmib_stc
{
	History *data;
	int low_wrong;		/* for rare event triming */
	int high_wrong;
	int wrong_count;
	int recovery;
	double quantile;
	double confidence;
	int min_history;
	int no_trim;
	int rebin_at;
	int since_rebin;
	Bin **bins;
	void **preds;
	int bin_count;
};

typedef struct schmib_stc SchmibPred;

void *InitSchmibPred(double quantile, double confidence, int fields);
void FreeSchmibPred(void *ib);
void UpdateSchmibPred(void *ib, double ts, double resp_value, 
			double lowpred, double highpred, double exp_value);
int ForcSchmibPred(void *ib, double exp_value, 
				double *lowpred, double *highpred);
void SetNoTrim(void *ib);
int SizeOfSchmibPred(void *ib);
History *SchmibHistory(void *ib);
void *BorrowedPred(SchmibPred *p, int b);
int ForcWeibullPred(void *ib, double exp_value, double *lowpred, double *highpred);
int ForcLognPred(void *ib, double exp_value, double *lowpred, double *highpred);
int ForcLoguPred(void *ib, double exp_value, double *lowpred, double *highpred);

#endif


