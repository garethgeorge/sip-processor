#include <stdio.h>
#include <stdlib.h>
#include <hyperexp.h>
#include <schmib_history.h>
#include <lognpred.h>
#include <math.h>
#include <schmib_pred.h>

void LoguPred(History *h, double quantile, int doit, double *ret) {
  int i, count;
  double ts, value, dummya, dummyb, dummyc;
  double dummyd, dummye, max, min, tmp;

  count = GetHistorySize(h);

  min = log(GetHistoryValue(h, 0)+1);
  max = log(GetHistoryValue(h, count)+1);
  *ret = exp(min + ((max - min) * quantile));
  return;
  min = 99999999999999999999.99;
  max = 0.0;

  for (i=1; i<=count; i++) {
    GetHistoryEntry(h, i, &ts, &value, &dummya, &dummyb, &dummyc, &dummyd,
	&dummye);
    value = log(value+1);
    if (value > max) {
      max = value;
    }
    if (value < min) {
      min = value;
    }
  }

  *ret = exp(min + ((max - min) * quantile));
  tmp = GetHistoryValue(h, count);
  printf("COUNT: %d %f %f %f %f\n", count, min, max, *ret, tmp);
}
