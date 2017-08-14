#ifndef INCLUDE_WEIBULL_H
#define INCLUDE_WEIBULL_H

#define WEIBULL 1
#define WEIBULLNOCOND 3

int dcomp(const void *a, const void *b);
int weibmle_sweep(double *a, double *b, double *x, int len);
double nrec_wloglikelihood(double *dp, double *x, int len);
double wloglikelihood(double a, double b, double *x, int len);
double winv(double *dp, double y);
double wpdf(double *dp, double x, double ign);
double wcdf(double *dp, double x, double ign);
double wcpdf(double *dp, double x, double t);
double wccdf(double *dp, double x, double t);

#endif
