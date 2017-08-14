#ifndef SCHMIB_MEANS_H
#define SCHMIB_MEANS_H

#include "dllist.h"

struct bin_stc
{
  double x_bar;
  double y_bar;
  Dllist list;  /* list of indexes into data */
  void *data;   /* as defined in simple_input */
  int count;
  double q_sum;	/* optimization for glrt */
  double log_L;	/* for glrt */
};

typedef struct bin_stc Bin;

Bin **SchmibMeans(void *data,
               int fields,      /* number of fields in input */
               int exp_f,       /* field number for exp variable */
               int resp_f,      /* field number of response variable */
               int *means);

void PrintBins(Bin **bins, int count, int exp_field, int resp_field);
void DumpBins(Bin **bins, int count, int ts_field, 
				int exp_field, int resp_field);
Bin *InitBin(void *data);
void FreeBin(Bin *b);
Bin *MergeBins(Bin *a, Bin *b);
double *MakeOrderStats(void *data, int field);
void AddObjectToBin(Bin *b, int index);
int DeleteFirstObjectFromBin(Bin *b);
void BinMinMax(Bin *b, int field, double *min, double *max);
Bin *CopyBin(Bin *b);


#endif

