#ifndef MIOREGRESS_H
#define MIOREGRESS_H

#include "mio.h"
#include "mioarray.h"

Array1D *RegressMatrix2D(Array2D *x, Array2D *y);
Array1D *RegressMatrix2DSimple(Array2D *x, Array2D *y);
double RSquared(Array2D *x, Array2D *b, Array2D *y);
double RMSE(Array2D *x, Array2D *b, Array2D *y);
Array2D *Residuals(Array2D *x, Array2D *b, Array2D *y);
Array2D *CenterScale(Array2D *x);
Array2D *CIBeta(Array2D *x, Array2D *b, Array2D *y, double alpha);


#endif

