#include <stdio.h>
#include <stdlib.h>
#include <weibull.h>
#include <schmib_history.h>
#include <weibpred.h>
#include <matlabhack.h>
#include <string.h>
#include <schmib_pred.h>

int weibinit=0, wfds[2], rfds[2], pid;

void WeibullPred(History *h, double quantile, int doit, double *ret) {
  int i, len, msize;
  double *list, dummy, dummya, dummyb, dp[2], foo, dummyc, dummyd, dummye;
  double min;
  char mbuf[4096], *retstr, ofile[256];
  FILE *FH;

  len = GetHistorySize(h);
  list = NULL;

  if (weibinit == 0) {
    printf("doing weibinit\n");
    weibinit = 1;
    pid = fork_matlab(wfds, rfds);
  }

  if (*ret == 0.0 || doit == 1) {
    list = malloc(sizeof(double) * len);
    
    min = 99999999999.0;
    for (i=1; i<=len; i++) {
      GetHistoryEntry(h, i, &dummy, &list[i-1], &dummya, &dummyb, &dummyc,
		&dummyd, &dummye);
      if (list[i-1] < min) {
	min = list[i-1];
      }
    }
    for (i=0; i<len; i++) {
      list[i] -= (min - 1.0);
    }

    /*
    if (weibmle_wrap(&dp[0], &dp[1], list, len) < 0) {
      *ret = -1.0;
    } else {
      *ret = winv(dp, quantile) + (min - 1.0);
    }
    */
    //    printf("MIN: %d\n", (int)min);
    msize = 4096;
    bzero(mbuf, msize);
    bzero(ofile, 256);

    sprintf(ofile, "/tmp/o.%d", rand() % 32768);
    FH = fopen(ofile, "w");
    for (i=0; i<len; i++) {
      //      rc = sstack_inspect(s, i, &retobj);
      fprintf(FH, "%d\n", (int)list[i]);
    }
    fclose(FH);
    snprintf(mbuf, msize, "load %s\np=wblfit(o);\na=wblinv(%.2f,p(1),p(2));\n",ofile, quantile);
    retstr = NULL;
    retstr = exec_matlab(wfds, rfds, mbuf);
    *ret = atof(retstr);
    if (list != NULL) {
      free(list);
    }
    if (retstr != NULL) {
      free(retstr);
    }
    unlink(ofile);
    printf("WLEN: %d\n", len);
  }
}


void WeibullPredOrig(History *h, double quantile, int doit, double *ret) {
  int i, len;
  double *list, dummy, dummya, dummyb, dp[2], foo, dummyc, dummyd, dummye;
  double min;

  len = GetHistorySize(h);

  if (*ret == 0.0 || doit == 1) {
    list = malloc(sizeof(double) * len);
    
    min = 99999999999.0;
    for (i=1; i<=len; i++) {
      GetHistoryEntry(h, i, &dummy, &list[i-1], &dummya, &dummyb, &dummyc,
		&dummyd, &dummye);
      if (list[i-1] < min) {
	min = list[i-1];
      }
    }
    for (i=0; i<len; i++) {
      list[i] -= (min - 1.0);
    }

    if (weibmle_wrap(&dp[0], &dp[1], list, len) < 0) {
      *ret = -1.0;
    } else {
      *ret = winv(dp, quantile) + (min - 1.0);
    }
  }
}
