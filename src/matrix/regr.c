#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "mioregress.h"

#define ARGS "x:y:C:"
char *Usage = "usage: regr -x xfile\n\
\t-y yfile\n\
\t-C confidence_level\n";

char Xfile[4096];
char Yfile[4096];
double Confidence;


int main(int argc, char *argv[])
{
	int i;
	int j;
	int c;
	MIO *d_mio;
	Array2D *x;
	Array2D *cx;
	MIO *xmio;
	Array2D *b;
	Array2D *y;
	MIO *ymio;
	Array2D *f;
	unsigned long size;
	double rsq;
	double rmse;
	Array2D *ci;

	Confidence = 0;
	while((c = getopt(argc,argv,ARGS)) != EOF) {
		switch(c) {
			case 'x':
				strncpy(Xfile,optarg,sizeof(Xfile));
				break;
			case 'y':
				strncpy(Yfile,optarg,sizeof(Yfile));
				break;
			case 'C':
				Confidence = atof(optarg);
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
	if(Yfile[0] == 0) {
		fprintf(stderr,"must specify yfile\n");
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

	size = MIOSize(Yfile);
	d_mio = MIOOpenText(Yfile,"r",size);
	if(d_mio == NULL) {
		fprintf(stderr,"couldn't open %s\n",Yfile);
		exit(1);
	}
	ymio = MIODoubleFromText(d_mio,NULL);
	if(ymio == NULL) {
		fprintf(stderr,"no valid data in %s\n",Yfile);
		exit(1);
	}

	if((Confidence < 0) || (Confidence > 1)) {
		fprintf(stderr,"must enter confidence level between 0 and 1\n");
		fprintf(stderr,"%s",Usage);
		exit(1);
	}


	x = MakeArray2DFromMIO(xmio);
	y = MakeArray2DFromMIO(ymio);

	cx = CopyArray2D(x);
	if(cx == NULL) {
		fprintf(stderr,"couldn't make copy of x\n");
		exit(1);
	}

	/*
	 * destructive of x so must use copy
	 */
	b = RegressMatrix2D(cx,y);
	if(b == NULL) {
		fprintf(stderr,"regression failed\n");
		exit(1);
	}

	rsq = RSquared(x,b,y);
	rmse = RMSE(x,b,y);


	printf("b: ");
	PrintArray2D(b);

	printf("\n");
	printf("R^2: %f RMSE: %f\n",rsq,rmse);

	if(Confidence != 0) {
		ci = CIBeta(x,b,y,Confidence);
		if(ci != NULL) {
			printf("%0.2f CI b:\n",1-Confidence);
			PrintArray2D(ci);
			FreeArray2D(ci);
		}
	}

	FreeArray2D(x);
	FreeArray2D(cx);
	FreeArray2D(y);
	FreeArray2D(b);

	return(0);
}


	

	



