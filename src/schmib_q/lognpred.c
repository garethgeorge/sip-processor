#include <stdio.h>
#include <stdlib.h>
#include <hyperexp.h>
#include <schmib_history.h>
#include <lognpred.h>
#include <math.h>
#include <schmib_pred.h>

int Last_count;
double sum = 0.0;
double varsum = 0.0;

void LognPred(History *h, double quantile, int doit, double *ret) {
  
  int guttmanidx[9] = {59,
		       70,
		       80,
		       90,
		       100,
		       250,
		       500,
		       800,
		       1000};

  double guttman[9] = {2.026,
		       1.990,
		       1.964,
		       1.944,
		       1.927,
		       1.815,
		       1.763,
		       1.737,
		       1.727};
  double guttv, *logvals, ts, value, dummya, dummyb, dummyc;
  double dummyd, dummye;
  double mean, var, std;
  int i, count;
  int start;

  /*
  logvals = (double *)malloc(sizeof(double)*GetHistorySize(h));
   */
  i = 0;

  count = GetHistorySize(h);

  /*
   * if nothing has changed and we are not at the min window
   */
  if((count == Last_count) && (count != 20))
  {
	goto out;
  }
  if((count <= Last_count) || (count == 20))
  {
	start = 1;
	sum = 0.0;
	varsum = 0.0;
  }
  else
  {
	start = Last_count + 1;
  }

  /*
   * var = E(X^2) = (E(X)^2)
   */
  for (i=start; i<=count; i++) {
    GetHistoryEntry(h, i, &ts, &value, &dummya, &dummyb, &dummyc, &dummyd,
	&dummye);
/*
    logvals[i-1] = log(value);
*/
    sum += log(value);
    varsum += log(value)*log(value);
  }

out:
  mean = sum / (double)count;

  /*
   * MLE is N/(N-1) * sample var
   */
  var = ((varsum / ((double)count)) - (mean*mean)) * 
		((double)count / ((double)(count - 1)));
  std = sqrt(var);

  guttv = 1.645;
/*
  for (i=0; i<9; i++) {
    if (count <= guttmanidx[i]) {
      guttv = guttman[i];
      break;
    }
  }
*/

  *ret = exp(mean + guttv*std);
  Last_count = count;
  /*
  free(logvals);
   */
}
