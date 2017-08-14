#if !defined AUTOC_H
#define AUTOC_H
int Rval(char *data_set, double mu, double var, int tau, double *rval);
int AutoCor(char *data_set,double mu, double var, int lags, double *rvals);
int ParCor(char *data_set, int K, double *avals);
int AIC(char *data_set, int K, double *aic);
int MeanVar(char *data_set,double *mean, double *var);
int GetBias(char *fname, double *bias);
#endif

