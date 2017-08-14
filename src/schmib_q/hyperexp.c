#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <hyperexp.h>
#define ZERO 0.0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001

int hyp2mle_wrap(double *pa, double *pb, double *la, double *lb, double *x, int len) {
  FILE *PH, *OFH;
  int i, rc;
  double rpa, rpb, rla, rlb;

  OFH = fopen("/tmp/simout.hyp2", "w");
  for (i=0; i<len; i++) {
    fprintf(OFH, "%f\n", x[i]);
  }
  fclose(OFH);
  

  PH = popen("./hyp2wrap /tmp/simout.hyp2", "r");
  rc = fscanf(PH, "hypphases=2;hypp = [%lf,%lf];hypl = [%lf,%lf];", &rpa, &rpb, &rla, &rlb);
  if (rc != 4) {
    fprintf(stderr, "ERROR: in hyp2mle_wrap with popen output fscanf\n");
    *pa = 0.0;
    *pb = 0.0;
    *la = 0.0;
    *lb = 0.0;
    return(-1);
  }
  pclose(PH);
  
  *pa = rpa;
  *pb = rpb;
  *la = rla;
  *lb = rlb;

  return(0);
}

double h2inv(double *dp, double y) {
  FILE *PH, *OFH;
  int i, rc;
  double rpa, rpb, rla, rlb, res;

  OFH = fopen("/tmp/simoutinv", "w");
  fprintf(OFH, "%f\n%f\n%f\n%f\n%f\n", dp[0], dp[1], dp[2], dp[3], y);
  fclose(OFH);
  

  PH = popen("./h2invwrap /tmp/simoutinv", "r");
  rc = fscanf(PH, "%lf", &res);
  if (rc != 1) {
    fprintf(stderr, "ERROR: in h2inv with popen output fscanf\n");
    res = 0.0;
    return(res);
  }
  pclose(PH);
  
  return(res);
}



double hyppdf(double *dp, double x, double ign){
  double ret;
  int i, nph, lidx, pidx;

  nph = (int)dp[0];

  ret = 0.0;

  for (i=0; i<nph; i++) {
    lidx = 1+nph+i;
    pidx = 1+i;

    ret = ret + (exp(-dp[lidx]*x) * dp[lidx] * dp[pidx]);
  }
  if (ret > 1.0) {
    ret = 0.0;
  }
  return(ret);

}

double hypcdf(double *dp, double x, double ign){
  double ret;
  int i, lidx, pidx, nph;

  
  nph = (int)dp[0];

  //  ret = 0.0;
  ret = 1.0;

  for (i=0; i<nph; i++) {
    lidx = 1+nph+i;
    pidx = 1+i;
    ret = ret - (exp(-dp[lidx]*x) * dp[pidx]);
  }
  if (ret > 1.0) {
    ret = 1.0;
  }
  //  printf("HCDF: %f %f\n", x, 1.0-ret);
  return(ret);

}


double hypcpdf(double *dp, double x, double t) {
  double ret;

  ret = hyppdf(dp, x+t, 0.0) / (1.0 - hypcdf(dp, t, 0.0));

  if (ret > 1.0) {
    //    ret = 1.0;
  }
  return(ret);
}

double hypccdf(double *dp, double x, double t) {
  double ret;

  ret = (hypcdf(dp, x+t, 0.0) - hypcdf(dp, t, 0.0)) / (1.0 - hypcdf(dp, t, 0.0));
  if (ret > 1.0) {
    //    ret = 0.0;
  }
  return(ret);

}
/*
double hypcpdf(double *dp, double x, double t) {
  double ret, denom, numer;
  int i, lidx, pidx, nph;

  nph = (int)dp[0];

  denom = 0.0;
  numer = 0.0;

  for (i=0; i<nph; i++) {
    lidx = 1+nph+i;
    pidx = 1+i;

    denom = denom - (exp(-dp[lidx]*(t+x))*dp[lidx]*dp[pidx]);
    numer = numer + (exp(-dp[lidx]*t)*dp[pidx]);
  }

  ret = -(denom/numer);
  return(ret);

}
double hypccdf(double *dp, double x, double t) {
  double ret, denom, numer;
  int i, lidx, pidx, nph;
  

  nph = (int)dp[0];

  denom = 0.0;
  numer = 0.0;

  for (i=0; i<nph; i++) {
    lidx = 1+nph+i;
    pidx = 1+i;

    denom = denom + (exp(-dp[lidx]*(t+x))*dp[pidx]);
    numer = numer + (exp(-dp[lidx]*t)*dp[pidx]);
  }

  ret = 1.0-(denom/numer);
  return(ret);
}
*/
