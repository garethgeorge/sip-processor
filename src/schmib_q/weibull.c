#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <weibull.h>
#include <string.h>

#define ZERO 0.0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001

int dcomp(const void *ina, const void *inb) {
  double a, b;
  
  a = *(double *)ina;
  b = *(double *)inb;
  
  if (a < b) {
    return(-1);
  } else if (a > b) {
    return(1);
  }
  return(0);
}

int weibmle_wrap(double *a, double *b, double *x, int len) {
  FILE *PH, *OFH;
  int i, rc;
  double shape;
  int scale;

  
  OFH = fopen("/tmp/simout.weib", "w");
  for (i=0; i<len; i++) {
    fprintf(OFH, "%f\n", x[i]);
  }
  fclose(OFH);
  

  PH = popen("./weibwrap /tmp/simout.weib", "r");
  rc = fscanf(PH, " weiba=%lf;weibb=%d;", &shape, &scale);
  if (rc != 2) {
    fprintf(stderr, "ERROR: in weibmle_wrap with popen output fscanf\n");
    *a = 0.5;
    *b = 4000;
    return(-1);
  }
  pclose(PH);
  *a = shape;
  *b = (double)scale;

  return(0);
}

int weibmle_sweep(double *a, double *b, double *x, int len) {
  double amin, amax, bmin, bmax, *xsort, rmin, ares, bres;
  double i, j, result, astep, bstep;
  int k;

  xsort = malloc(sizeof(double) * len);
  memcpy(xsort, x, sizeof(double) * len);
  qsort(xsort, len, sizeof(double), dcomp);  
  
  amin = 0.0;
  amax = 2.0;
  astep = (amax - amin) / 30.0;

  bmin = xsort[0];
  bmax = xsort[len-1];
  bstep = (bmax - bmin) / 30.0;
  
  result = wloglikelihood(amin, bmin, x, len);
  rmin = result;

  *a = amin;
  *b = bmin;

  for (k=0; k<4; k++) {
    for(i=amin; i<=amax; i += astep) {
      for (j=bmin; j<=bmax; j+=bstep) {
	result = wloglikelihood(i, j, x, len);
	if (result > rmin) {
	  rmin = result;
	  ares = i;
	  bres = j;
	}
      }
    }
    amin = ares - (astep*2.0);
    amax = ares + (astep*2.0);
    astep = (amax - amin) / 30.0;

    bmin = bres - (bstep * 2.0);
    bmax = bres + (bstep * 2.0);
    bstep = (bmax - bmin) / 30.0;
  }

  *a = ares;
  *b = bres;
  return(1);
}

double wloglikelihood(double a, double b, double *x, int len) {
  int i;
  double sum, dp[2];

  dp[0] = a;
  dp[1] = b;
  
  sum = 0.0;
  for (i=0; i<len; i++) {
    sum += log(wpdf(dp, x[i], 0.0) + 0.000000000000000001);
  }

  return(sum);
}

double nrec_wloglikelihood(double *dp, double *x, int len) {
  int i;
  double sum;

  sum = 0.0;
  for (i=0; i<len; i++) {
    sum += log(wpdf(dp, x[i], 0.0) + 0.000000000000000001);
  }

  return(-1.0 * sum);
}

double winv(double *dp, double y) {
  double a=dp[0], b=dp[1];

  return(b * pow((-1.0 * log(1.0 - y)), 1.0/a));
}

double wpdf(double *dp, double x, double ign) {

  double a=dp[0], b=dp[1];
  double tmpa, tmpb, tmpx, dret;

  tmpa = a;
  tmpb = b;
  tmpx = x;

  dret = tmpa * pow(tmpb, -1.0*tmpa) * exp(-1.0 * pow(tmpx/tmpb, tmpa )) * pow(tmpx, -1.0 + tmpa);

  return(dret);
  
}

double wcdf(double *dp, double x, double ign) {
  double a=dp[0], b=dp[1];
  double tmpa, tmpb, tmpx, dret;

  tmpa = a;
  tmpb = b;
  tmpx = x;

  dret = 1.0 - exp(-1.0 * pow(tmpx/tmpb, tmpa));
  return(dret);
}

double wcpdf(double *dp, double x, double t) {
  double ret, pdfr, cdfr;
  double a=dp[0], b=dp[1];

  pdfr = a * pow(b, -1.0*a) * exp(-1.0 * pow((x+t)/b, a )) * pow((x+t), -1.0 + a);
  cdfr = 1.0 - exp(-1.0 * pow(t/b, a));
  
  /*
  if (pdfr > 1.0) {
    pdfr = 1.0;
  }
  if (cdfr > 1.0) {
    cdfr = 1.0;
  }
  */

  ret = pdfr / (1.0 - cdfr);

  if (ret > 1.0) {
    ret = 0.0;
  }
  //  fprintf(stderr, "ret; %f\n", ret);
  /*  fprintf(stderr, "npdf %f\n", ret);
  ret = wpdf(dp, x+t, 0.0) / (1 - wcdf(dp, t, 0.0));
  fprintf(stderr, "opdf %f\n", ret);
  */
  return(ret);
}

double wccdf(double *dp, double x, double t) {
  double ret, cdfr, cdfr1, tmp;
  double a=dp[0], b=dp[1];
  
  //  fprintf(stderr, "%f %f %f %f CDF: exp %f pow %f\n", x, t, b, a, exp(-1.0 * pow((x+t)/b, a)), pow((x+t)/b, a));
  cdfr = 1.0 - exp(-1.0 * pow((x+t)/b, a));
  cdfr1 = 1.0 - exp(-1.0 * pow(t/b, a));
  
  //  fprintf(stderr, "CDF2: %f %f\n", cdfr, cdfr1);
  
  tmp = (cdfr - cdfr1);
  if (tmp == 0.0) {
    tmp = ZERO;
  }
  ret = tmp / (1.0 - cdfr1);
  if (ret > 1.0) {
    ret = 1.0;
  }

/*
  fprintf(stderr, "ncdf %f\n", ret);
  ret = (wcdf(dp, x+t, 0.0) - wcdf(dp, t, 0.0)) / (1.0 - wcdf(dp, t, 0.0));
  fprintf(stderr, "ocdf %f\n", ret);
*/
  return(ret);
}

/*
double wcpdf(double *dp, double x, double t) {
  double a=dp[0], b=dp[1];
  double tmpa, tmpb, tmpx, tmpt, dret;

  tmpa = a;
  tmpb = b;
  tmpx = x;
  tmpt = t;

  
  dret = (tmpa * exp(pow(tmpt/tmpb, tmpa) - pow((tmpt+tmpx)/tmpb,tmpa)) * pow((tmpt + tmpx)/tmpb, -1.0 + tmpa)) / tmpb; 

  
  return(dret);
}

double wccdf(double *dp, double x, double t) {
  double a=dp[0], b=dp[1];
  double tmpa, tmpb, tmpx, tmpt, dret;

  tmpa = a;
  tmpb = b;
  tmpx = x;
  tmpt = t;
  
  dret = 1.0 - exp( pow(tmpt/tmpb, tmpa) -pow( (tmpt+tmpx)/tmpb, tmpa));
  return(dret);
}
*/

