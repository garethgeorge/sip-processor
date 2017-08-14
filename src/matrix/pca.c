#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "mioregress.h"

Array2D *VarCoVarArray2D(Array2D *x)
{
	/*
	 * assumes that the variables x_i are in rows
	 */
	Array2D *vcv;
	double acc;
	double count;
	double mu_i;
	double mu_j;
	int i;
	int j;
	int k;
	double *xdata;
	int c;

	vcv = MakeArray2D(x->xdim,x->xdim);
	if(vcv == NULL) {
		return(NULL);
	}

	xdata = x->data;
	c = x->xdim;

	for(i=0; i < x->xdim; i++) {
		for(j=0; j < x->xdim; j++) {
			/*
			 * get mu from each column i and j
			 */
			mu_i = 0;
			mu_j = 0;
			count = 0;
			for(k=0; k < x->ydim; k++) {
				mu_i += xdata[k*c+i];
				mu_j += xdata[k*c+j];
				count++;
			}
			mu_i = mu_i / count;
			mu_j = mu_j / count;
			acc = 0;
			count = 0;
			for(k=0; k < x->ydim; k++) {
				acc += ((xdata[k*c+i] - mu_i) *
				        (xdata[k*c+j] - mu_j));
				count++;
			}
			vcv->data[i*x->xdim+j] = acc / count;
		}
	}

	return(vcv);
}

Array2D *PCArray2D(Array2D *x)
{
	Array2D *vcv;
	Array2D *eigen;
	Array2D *u;

	vcv = VarCoVarArray2D(x);
	if(vcv == NULL) {
		return(NULL);
	}
	eigen = EigenVectorArray2D(vcv);
	if(eigen == NULL) {
		FreeArray2D(vcv);
		return(NULL);
	}

	u = NormalizeColsArray2D(eigen);
	if(u == NULL) {
		FreeArray2D(vcv);
		FreeArray2D(eigen);
		return(NULL);
	}

	FreeArray2D(vcv);
	FreeArray2D(eigen);
	return(u);
}

#ifdef STANDALONE

#define ARGS "x:E"
char *Usage = "usage: pca -x xfile\n\
\t-E <explain variation>\n";

char Xfile[4096];
int Explain;

int main(int argc, char *argv[])
{
	int c;
	int size;
	MIO *d_mio;
	Array2D *x;
	Array2D *xt;
	Array2D *xtx;
	MIO *xmio;
	Array2D *y;
	Array2D *ev;
	Array2D *u;
	Array2D *vcv;
	double acc;
	double frac;
	int i;
	int j;

	Explain = 0;
	while((c = getopt(argc,argv,ARGS)) != EOF) {
		switch(c) {
			case 'x':
				strncpy(Xfile,optarg,sizeof(Xfile));
				break;
			case 'E':
				Explain = 1;
				break;
			default:
				fprintf(stderr,
			"unrecognized command: %c\n",(char)c);
				fprintf(stderr,"%s",Usage);
				exit(1);
		}
	}

	if(Xfile[0] == 0) {
		fprintf(stderr,"must specify xfile\n");
		fprintf(stderr,"%s",Usage);
		exit(1);
	}

	size = MIOSize(Xfile);
	d_mio = MIOOpenText(Xfile,"r",size);
	if(d_mio == NULL) {
		fprintf(stderr,"couldn't open %s\n",Xfile);
		exit(1);
	}
	xmio = MIODoubleFromText(d_mio,NULL);
	if(xmio == NULL) {
		fprintf(stderr,"no valid data in %s\n",Xfile);
		exit(1);
	}

	x = MakeArray2DFromMIO(xmio);

	u = PCArray2D(x);
	if(u == NULL) {
		fprintf(stderr,"couldn't get principal components\n");
		exit(1);
	}
	/*
	 * measurements are in rows of x
	 */
	y = MultiplyArray2D(x,u);
	if(y == NULL) {
		fprintf(stderr,"couldn't compute PC transform\n");
		exit(1);
	}

	for(i=0; i < y->ydim; i++) {
		for(j=0; j < y->xdim; j++) {
			printf("%10.10f ",y->data[i*y->xdim+j]);
		}
		printf("\n");
	}

	if(Explain == 1) {
		vcv = VarCoVarArray2D(x);
		if(vcv == NULL) {
			fprintf(stderr,
				"couldn't compute var-co-var matrix\n");
			exit(1);
		}
		printf("var-covar matrix\n");
		PrintArray2D(vcv);

		printf("eigen vectors (in columns)\n");
		PrintArray2D(u);

		ev = EigenValueArray2D(vcv);
		if(ev == NULL) {
			fprintf(stderr,"couldn't get eigen values\n");
			exit(1);
		}
		printf("eigen values\n");
		PrintArray1D(ev);
		printf("\n");
		acc = 0;
		for(i=0; i < ev->ydim; i++) {
			/*
			acc += (ev->data[i*ev->xdim+0] *
				ev->data[i*ev->xdim+0]);
			*/
			acc += ev->data[i*ev->xdim+0];
		}
		printf("varfrac: ");
		for(i=0; i < ev->ydim; i++) {
			/*
			frac = (ev->data[i*ev->xdim+0] *
				ev->data[i*ev->xdim+0]) / acc;
			*/
			frac = ev->data[i*ev->xdim+0] / acc;
			printf("%f ",frac);
		}
		printf("\n");
		FreeArray2D(ev);
		FreeArray2D(vcv);

		xt = TransposeArray2D(x);
		xtx = MultiplyArray2D(xt,x);
		ev = EigenValueArray2D(xtx);
		if(ev == NULL) {
			fprintf(stderr,"no eigen values for xtx\n");
			exit(1);
		}
		printf("eigen values for x^t * x\n");
		PrintArray1D(ev);
		FreeArray2D(ev);
		FreeArray2D(xt);
		FreeArray2D(xtx);
	}
	

	FreeArray2D(u);
	FreeArray2D(x);
	FreeArray2D(y);

	return(0);
}

#endif
