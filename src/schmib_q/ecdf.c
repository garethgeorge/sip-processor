#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ecdf.h>

#ifdef RUN
main(int argc, char **argv) {
  int rc, i, numvals;
  FILE *FH;
  double val;
  double *vals, *x, *y;

  FH = fopen(argv[1], "r");
  if (FH == NULL) {
    perror("open");
    exit(1);
  }

  vals = NULL;
  i = 0;
  while(fscanf(FH, "%lf", &val) != EOF) {
    vals = realloc(vals, (i+1)*sizeof(double));
    vals[i] = val;
    i++;
  }
  fclose(FH);
  numvals = i;

  rc = ecdf(vals, numvals, &x, &y);
  for (i=0; i<numvals; i++) {
    printf("%f %f\n", x[i], y[i]);
  }
  free(vals);
  exit(0);
}
#endif

int ecdf(double *vals, int numvals, double **x, double **y) {
  int i;
  double *ex, *ey;

  *x = malloc(sizeof(double) * numvals);
  *y = malloc(sizeof(double) * numvals);
  ex = *x;
  ey = *y;

  memcpy(ex, vals, sizeof(double)*numvals);
  qsort(ex, numvals, sizeof(double), compdouble);
  
  for (i=0; i<numvals; i++) {
    ey[i] = (double)(i+1) / (numvals+1.0);
  }

  return(1);
}

int compdouble(const void *a, const void *b) {
  double da, db;
  da = *(double *)a;
  db = *(double *)b;

  if (da < db) {
    return(-1);
  } else if (da > db) {
    return(1);
  }
  return(0);
}
