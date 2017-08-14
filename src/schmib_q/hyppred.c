#include <stdio.h>
#include <stdlib.h>
#include <hyperexp.h>
#include <schmib_history.h>
#include <hyppred.h>

void HyperPred(History *h, double quantile, int doit, double *ret) {
  int i, len;
  double *list, dummy, dummya, dummyb, dp[4], foo, dummyc, dummyd, dummye;

  len = GetHistorySize(h);

  if (*ret == 0.0 || doit == 1) {
    list = malloc(sizeof(double) * len);
    
    for (i=1; i<=len; i++) {
      GetHistoryEntry(h, i, &dummy, &list[i-1],&dummya, &dummyb, &dummyc,
		&dummyd, &dummye);
    }
    
    hyp2mle_wrap(&dp[0], &dp[1], &dp[2], &dp[3], list, len);
    *ret = h2inv(dp, quantile);
  }
}
