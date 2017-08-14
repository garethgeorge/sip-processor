#ifndef INCLUDE_HYPEREXP_H
#define INCLUDE_HYPEREXP_H

#define HYPER 2

int hyp2mle_wrap(double *pa, double *pb, double *la, double *lb, double *x, int len);
double h2inv(double *dp, double y);
double hyppdf(double *dp, double x, double ign);
double hypcdf(double *dp, double x, double ign);
double hypcpdf(double *dp, double x, double t);
double hypccdf(double *dp, double x, double t);

#endif
