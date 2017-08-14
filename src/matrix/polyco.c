#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "mymalloc.h"

int MPolyCo(double *a, int adim, 
	    double *b, int bdim, 
	    double **r, 
	    int *rdim)
{
	double *ca;
	int ad;
	double *cb;
	int bd;
	double *or;
	int rd;
	int i;
	int j;
	int k;
	double e;
	double ae;
	double be;
	double oe;
	double acc;

	/*
	 * make sure a is bigger
	 */
	if(bdim > adim) {
		ca = b;
		cb = a;
		ad = bdim;
		bd = adim;
	} else {
		ca = a;
		cb = b;
		ad = adim;
		bd = bdim;
	}

	rd = (ad-1)+(bd-1) + 1;
	or = (double *)Malloc(rd*sizeof(double));
	if(or == NULL) {
		return(-1);
	}

	/*
	 * index backwards from polynomial degree
	 *
	 * e is the exponent of the current term
	 */
	for(k=0; k < rd; k++) {
		acc = 0;
		for(i=0; i < ad; i++) {
			for(j=0; j < bd; j++) {
				oe = rd - k;
				ae = ad - i;
				be = bd - j;
				if((oe-1) == ((ae-1)+(be-1))) {
					acc += (ca[i] * cb[j]);
				}
			}
		}
		or[k] = acc;
	}

	*r = or;
	*rdim = rd;

	return(1);
}

#ifdef STANDALONE
int main(int argc, char **argv)
{
	double *a;
	double *b;
	double *r;
	double *r1;
	int rdim;
	int rdim1;
	int i;
	int err;

	a = (double *)Malloc(2*sizeof(double));
	if(a == NULL) {
		exit(1);
	}

	b = (double *)Malloc(3*sizeof(double));
	if(b == NULL) {
		exit(1);
	}

	/*
	 * try (z-3)(z^2 - 4z + 13)
	 */
	a[0] = 1.0;
	a[1] = +3.0;

	b[0] = 1.0;
	b[1] = -4.0;
	b[2] = 13.0;

	err = MPolyCo(a,2,b,3,&r,&rdim);
	if(err < 0) {
		fprintf(stderr,"PolyCo failed\n");
		exit(1);
	}

	printf("a: "); 
	for(i=0; i < 2; i++) {
		printf("%f ",a[i]);
	}
	printf("\n");

	printf("b: "); 
	for(i=0; i < 3; i++) {
		printf("%f ",b[i]);
	}
	printf("\n");

	printf("r: "); 
	for(i=0; i < rdim; i++) {
		printf("%f ",r[i]);
	}
	printf("\n\n");

	a[0] = 1;
	a[1] = -4;

	err = MPolyCo(a,2,r,rdim,&r1,&rdim1);
	if(err < 0) {
		fprintf(stderr,"second MPolyCo failed\n");
		exit(1);
	}

	printf("a: "); 
	for(i=0; i < 2; i++) {
		printf("%f ",a[i]);
	}
	printf("\n");

	printf("r1: "); 
	for(i=0; i < rdim1; i++) {
		printf("%f ",r1[i]);
	}
	printf("\n\n");

	

	exit(0);
}
		

#endif
			 
		


