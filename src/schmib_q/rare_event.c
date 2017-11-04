#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "simple_input.h"
#include "schmib_q_sim.h"
#include "schmib_pred.h"
#include "ecdf.h"
#include "norm.h"
#include "autoc.h"

int TheBin;


typedef struct tsval_t {
  int idx;
  double ts;
  double xval;
  double yval;
} tsval;

int comp_by_xval(const void *a, const void *b) {
  tsval tsa, tsb;
  tsa = *(tsval *)a;
  tsb = *(tsval *)b;

  if (tsa.xval < tsb.xval) {
    return(-1);
  } else if (tsa.xval > tsb.xval) {
    return(1);
  }

  /*
   * order by ts if equal
   */
  if(tsa.ts < tsb.ts) {
	return(-1);
  } else if(tsa.ts > tsb.ts) {
	return(1);
  }

  return(0);
}

int comp_by_idx(const void *a, const void *b) {
  tsval tsa, tsb;
  tsa = *(tsval *)a;
  tsb = *(tsval *)b;

  if (tsa.idx < tsb.idx) {
    return(-1);
  } else if (tsa.idx > tsb.idx) {
    return(1);
  }
  return(0);
}

int ConsecutiveWrong(History *h, double *retautoc) {
  return(ConsecutiveWrongNorm(h, retautoc));
}

int ConsecutiveRightOLD(History *h, double *retautoc) {
	return(RECOVERYHIGH);
}

int ConsecutiveRight(History *h, double *retautoc)
{
	JRB jr;
	JRB sorted;

  //  int rares[10] = {3,3,3,4,5,5,6,8,10,20};
  int rares[10] = {177, 179, 186, 194, 210, 233, 254, 305, 400, 629};
  double mu, var;
  float rarechks[10] = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};

  int i, count, start;

  double autoc, dummy, dummya, ts, value, dummyb;
  double dummyc, dummyd;
  void *data;

  if (GetHistorySize(h) <= 0) {
    return(rares[0]);
  }

  /*
   * sort by timestamp first
   */
  sorted = make_jrb();
  if(sorted == NULL)
  {
	exit(1);
  }

  for (i=start; i<=GetHistorySize(h); i++)
  {
    	GetHistoryEntry(h, i, &ts, &value, &dummy, &dummya, &dummyb,
		&dummyc, &dummyd);
	jrb_insert_dbl(sorted,ts,new_jval_i(i));
  }

  start = 1;
  count = 0; 
  //  printf("START: %d STOP: %d\n", start, GetHistorySize(h));
  InitDataSet(&data,2);
//  for (i=start; i<=GetHistorySize(h); i++) {
 jrb_traverse(jr,sorted)
 {
    i = jval_i(jrb_val(jr));
    GetHistoryEntry(h, i, &ts, &value, &dummy, &dummya, &dummyb,
		&dummyc, &dummyd);
/*    printf("TV: %f %f %d\n", ts, value, i); */
    WriteEntry(data, ts, value);
    count++;
  }
  jrb_free_tree(sorted);

  MeanVar(data, &mu, &var);
/*  printf("MV: %f %f\n", mu, var); */
  Rval(data, mu, var, 1, &autoc);
/*  printf("AUTOC: %f\n", autoc); */
  *retautoc = autoc;
  for(i=0; i < 9; i++) {
	if((autoc > rarechks[i]) &&
	   (autoc < rarechks[i+1])) {
		return(rares[i+1]);
        }
  }
  /*
   * autoc bigger than 0.9
   * use value for 0.99
   */
  return(3648);
}

int ConsecutiveWrongRight(History *h, double *retautoc,
		int *wrongs, int *rights)
{
  // dan and john method calculated ~nurmi/AR
  // based on 0.0012 (3 wrongs in a row at 0.05)
  double arvals[11] = {0.0, .1, .2, .3, .4, .5, .6, .7, .8, .9, 1.0};
  int arwrongs[11] = {3, 3, 4, 4, 5, 6, 8, 10, 14, 28, 28};
  int arrights[11] = {174, 179, 186, 194, 210, 233, 254, 305, 400, 629, 3648};
  //  int arwrongs[11] = {3,3,3,4,5,5,6,8,10,20,20};
  int i, lag, j, rc, l, count, start;
  int k;
  double ts;
  JRB sorted;
  JRB jr;
  double value;

  double *vals, *x, *y, sum, suma, sumb, n, *covs, autoc, *tss, dummy, dummya, dummyb, pcent;
  double dummyc, dummyd;
  tsval *records;
  // idea is to take the history, make a list of empirical percentiles, find corresponding quantiles to percentiles from a normal(0,1), and calculate AR, lag 1, on that data. 

  if (GetHistorySize(h) <= 0) {
    *wrongs = arwrongs[0];
    *rights = arrights[0];
    return(1);
  }

  records = malloc(sizeof(tsval)*GetHistorySize(h));
  //  tss = malloc(sizeof(double)*GetHistorySize(h));
  

  if (GetHistorySize(h) <= 100) {
    start = 1;
  } else {
    start = GetHistorySize(h) - 100;
  }

  sorted = make_jrb();
  if(sorted == NULL)
  {
	  exit(1);
  }

  for (i=start; i<=GetHistorySize(h); i++)
  {
    GetHistoryEntry(h, i, &ts, &value, &dummy, &dummya, &dummyb,
		    &dummyc, &dummyd);
	  jrb_insert_dbl(sorted,ts,new_jval_i(i));
  }
  count = 0; 
  //  printf("START: %d STOP: %d\n", start, GetHistorySize(h));
//  for (i=start; i<=GetHistorySize(h); i++)
  jrb_traverse(jr,sorted)
  {
    i = jval_i(jrb_val(jr));
    GetHistoryEntry(h, i, &records[count].ts, &records[count].xval, &dummy,
&dummya, &pcent, &dummyc, &dummyd);
    //    vals[i] = GetHistoryValue(h, i);
    //    tss[i] = GetHistoryAge(h, i);
    records[count].idx = i;
//            printf("id: %d HIST %f %f\n", i,records[count].ts, records[count].xval);
    count++;
  }
  jrb_free_tree(sorted);

  /*
   * okay -- the way in which qsort() handles ties matters.  If the x values
   * are the same, qsort could assign different CDF values to different
   * timestamps (which is true on OSX and Linux).  When autoc is computed
   * it matters
   *
   * FIX this with double sort
   */

  qsort(records, count, sizeof(tsval), comp_by_xval);
  for (i=0; i < count; i++) {
    //    printf("RSORT: %d %f %f\n", records[i].idx, records[i].ts, records[i].xval);
    records[i].yval = (double)(i+1) / ((double)count + 1.0);
//        printf("RSORT: %d %f %f %f\n", records[i].idx, records[i].ts, 
//		records[i].xval, records[i].yval);
  } 
  qsort(records, count, sizeof(tsval), comp_by_idx);
//  for(i=0; i < count; i++) {
//        printf("YSORT: %d %f %f %f\n", records[i].idx, records[i].ts, 
//		records[i].xval, records[i].yval);
 // }
  //  for (i=0; i < GetHistorySize(h); i++) {
  //    printf("FOOHIST: %f %f\n", records[i].ts, records[i].xval);
  //  }

  vals = malloc(sizeof(double)*count);
  for (i=0; i<count; i++) {
    //    printf("%f %f\n", y[i], InvNormal(y[i], 0.0, 1.0));
    vals[i] = InvNormal(records[i].yval, 0.0, 1.0);
//        printf("AUTO HIST %f %f %f\n", records[i].ts, vals[i], records[i].yval);
  }

  lag=1;
  covs = malloc(sizeof(double)*(lag+1));

  for (l=0; l<=lag; l++) {
    sum = 0.0;
    suma = 0.0;
    sumb = 0.0;

    n = (double)count - (double)l;

    for (j=0; j<(count - l); j++) {
      i = (j+l);
      /*
      if (i >= count) {
	sum += vals[j] * vals[count-1];
	suma += vals[j];
	sumb += vals[count-1];
      } else {
      }
      */
      sum += vals[j] * vals[i];
      suma += vals[j];
      sumb += vals[i];
      
    }
    covs[l] = (sum/n) - (suma/n)*(sumb/n);
//        printf("COV %d: %f %f\n", l, covs[l], covs[l] / covs[0]);
  }

  autoc = covs[lag] / covs[0];
//  autoc = (double)((int)(autoc * 10.0))/10.0;
  //  autoc -= .2;

  /*
  if (*retautoc < autoc) {
    autoc = *retautoc + 0.1;
  }
  */

  *retautoc = autoc;
  for (i=0; i<10; i++) {
    if ((autoc >= arvals[i]) &&
	(autoc <= arvals[i+1])) {
      //      printf("%d, %f == %d wrongs!\n", GetHistorySize(h), autoc, arwrongs[i]);
      free(records);
      free(covs);
      free(vals);
      /*
       * be conservative
       */
      *wrongs = arwrongs[i]; 
      /*
       * choose the one that is closer
       */
      if(fabs(autoc-arvals[i]) < fabs(autoc-arvals[i+1])) {
		k=i;
      } else {
		k=i+1;
      }
      k=i+1;
      *rights = arrights[k];
      return(1);
    }
  }
  free(records);
  free(covs);
  free(vals);
  *retautoc = 0.0;
  *wrongs = arwrongs[0];
  *rights = arrights[0];
//printf("RIGHTWRONG ERROR: autoc: %f\n",autoc);
  return(1);
}

int ConsecutiveWrongNorm(History *h, double *retautoc)
{
  // dan and john method calculated ~nurmi/AR
  double arvals[11] = {0.0, .1, .2, .3, .4, .5, .6, .7, .8, .9, 1.0};
  int arwrongs[11] = {3, 3, 4, 4, 5, 6, 8, 10, 14, 28, 28};
  //  int arwrongs[11] = {3,3,3,4,5,5,6,8,10,20,20};
  int i, lag, j, rc, l, count, start;
  double ts;
  JRB sorted;
  JRB jr;
  double value;

  double *vals, *x, *y, sum, suma, sumb, n, *covs, autoc, *tss, dummy, dummya, dummyb, pcent;
  double dummyc, dummyd;
  tsval *records;
  // idea is to take the history, make a list of empirical percentiles, find corresponding quantiles to percentiles from a normal(0,1), and calculate AR, lag 1, on that data. 

  if (GetHistorySize(h) <= 0) {
    return(3);
  }

  records = malloc(sizeof(tsval)*GetHistorySize(h));
  //  tss = malloc(sizeof(double)*GetHistorySize(h));
  

  if (GetHistorySize(h) <= 100) {
    start = 1;
  } else {
    start = GetHistorySize(h) - 100;
  }

#if 1
  sorted = make_jrb();
  if(sorted == NULL)
  {
	exit(1);
  }

  for (i=start; i<=GetHistorySize(h); i++)
  {
    	GetHistoryEntry(h, i, &ts, &value, &dummy, &dummya, &dummyb,
		&dummyc, &dummyd);
	jrb_insert_dbl(sorted,ts,new_jval_i(i));
  }
  count = 0; 
  //  printf("START: %d STOP: %d\n", start, GetHistorySize(h));
//  for (i=start; i<=GetHistorySize(h); i++)
  jrb_traverse(jr,sorted)
#else
  count = 0; 
  for (i=start; i<=GetHistorySize(h); i++)
#endif
  {
#if 1
    i = jval_i(jrb_val(jr));
#endif
    GetHistoryEntry(h, i, &records[count].ts, &records[count].xval, &dummy,
&dummya, &pcent, &dummyc, &dummyd);
    //    vals[i] = GetHistoryValue(h, i);
    //    tss[i] = GetHistoryAge(h, i);
    records[count].idx = i;
    //        printf("id: %d HIST %f %f\n", i,records[count].ts, records[count].xval);
    count++;
  }
#if 1
  jrb_free_tree(sorted);
#endif

/*
  if (pcent <= 0.96) {
    // rich thing
    return(3);
  }
*/

  //  rc = ecdf_tsval(vals, GetHistorySize(h), &x, &y);
  qsort(records, count, sizeof(tsval), comp_by_xval);
  for (i=0; i < count; i++) {
    //    printf("RSORT: %d %f %f\n", records[i].idx, records[i].ts, records[i].xval);
    records[i].yval = (double)(i+1) / ((double)count + 1.0);
  } 
  qsort(records, count, sizeof(tsval), comp_by_idx);
  //  for (i=0; i < GetHistorySize(h); i++) {
  //    printf("FOOHIST: %f %f\n", records[i].ts, records[i].xval);
  //  }

  vals = malloc(sizeof(double)*count);
  for (i=0; i<count; i++) {
    //    printf("%f %f\n", y[i], InvNormal(y[i], 0.0, 1.0));
    vals[i] = InvNormal(records[i].yval, 0.0, 1.0);
    //    printf("AUTO HIST %f %f\n", records[i].ts, vals[i]);
  }

  lag=1;
  covs = malloc(sizeof(double)*(lag+1));

  for (l=0; l<=lag; l++) {
    sum = 0.0;
    suma = 0.0;
    sumb = 0.0;

    n = (double)count - (double)l;

    for (j=0; j<(count - l); j++) {
      i = (j+l);
      /*
      if (i >= count) {
	sum += vals[j] * vals[count-1];
	suma += vals[j];
	sumb += vals[count-1];
      } else {
      }
      */
      sum += vals[j] * vals[i];
      suma += vals[j];
      sumb += vals[i];
      
    }
    covs[l] = (sum/n) - (suma/n)*(sumb/n);
    //    printf("COV %d: %f %f\n", l, covs[l], covs[l] / covs[0]);
  }

  autoc = covs[lag] / covs[0];
  autoc = (double)((int)(autoc * 10.0))/10.0;
  //  autoc -= .2;

  /*
  if (*retautoc < autoc) {
    autoc = *retautoc + 0.1;
  }
  */

  *retautoc = autoc;
  for (i=0; i<11; i++) {
    if ((arvals[i] + 0.05) > autoc) {
      //      printf("%d, %f == %d wrongs!\n", GetHistorySize(h), autoc, arwrongs[i]);
      free(records);
      free(covs);
      free(vals);
      return(arwrongs[i]);
    }
  }
  free(records);
  free(covs);
  free(vals);
  *retautoc = 0.0;
  return(7);
}

int IsRare(double ts, 
	   double value, 
	   double lowpred, 
	   double highpred, 
	   double quantile, 
	   double confidence, 
	   int wrong_count, 
	   int *low_wrong,
	   int *high_wrong)
{

	if(value <= highpred)
	{
		*high_wrong = 0;
		if(value <= lowpred)
		{
			(*low_wrong)++;
		}
		else
		{
			*low_wrong = 0;
		}
	}
	else
	{
		(*high_wrong)++;
		*low_wrong = 0;
	}

	if((*high_wrong >= wrong_count))
	{
printf("AT %f RARE hw: %d, wc: %d, bin: %d\n",ts,*high_wrong,wrong_count,TheBin);
		return(1);
	}
	else
	{
printf("AT %f NOT_RARE hw: %d, wc: %d, bin: %d\n",ts,*high_wrong,wrong_count,TheBin);
		return(0);
	}
}
