#ifndef JBINDEX_H
#define JBINDEX_H

void InitJBCache(char *fname, double q, double c);
int JBIndex(int size, double quantile, double confidence, double *ratio);
int MinHistory(double q, double c);

#endif


