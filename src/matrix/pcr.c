#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "mioregress.h"
#include "redblack.h"

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

int PrincipalComponents(Array2D *x, Array2D **out_u, Array2D **out_ev)
{
	Array2D *vcv;
	Array2D *eigen;
	Array2D *u;
	Array2D *new_u;
	Array2D *ev;
	Array2D *new_ev;
	int curr;
	RB *list;
	RB *rb;
	int i;
	int j;

	vcv = VarCoVarArray2D(x);
	if(vcv == NULL) {
		return(-1);
	}
	eigen = EigenVectorArray2D(vcv);
	if(eigen == NULL) {
		FreeArray2D(vcv);
		return(-1);
	}

	ev = EigenValueArray2D(vcv);
	if(ev == NULL) {
		FreeArray2D(vcv);
		FreeArray2D(eigen);
		return(-1);
	}

	u = NormalizeColsArray2D(eigen);
	if(u == NULL) {
		FreeArray2D(vcv);
		FreeArray2D(eigen);
		FreeArray2D(ev);
		return(-1);
	}

	/*
	 * sort by eigen value
	 */
	list = RBInitD();
	if(list == NULL) {
		FreeArray2D(vcv);
		FreeArray2D(eigen);
		FreeArray2D(u);
		FreeArray2D(ev);
		return(-1);
	}
	for(i=0; i < ev->ydim; i++) {
		RBInsertD(list,ev->data[i*ev->xdim+0],
				(Hval)i);
	}

	/*
	 * make space for sorted eigenvectors
	 */
	new_u = MakeArray2D(u->ydim,u->xdim);
	if(new_u == NULL) {
		FreeArray2D(vcv);
		FreeArray2D(eigen);
		FreeArray2D(u);
		RBDestroyD(list);
		FreeArray2D(ev);
		return(-1);
	}

	/*
	 * make space for sorted eigenvalue
	 */
	new_ev = MakeArray2D(ev->ydim,ev->xdim);
	if(ev == NULL) {
		FreeArray2D(vcv);
		FreeArray2D(eigen);
		FreeArray2D(new_u);
		FreeArray2D(u);
		RBDestroyD(list);
		FreeArray2D(ev);
		return(-1);
	}

	/*
	 * walk the list reordering
	 */
	curr = 0;
	RB_BACKWARD(list,rb) {
		i = rb->value.i;
		for(j=0; j < new_u->ydim; j++) {
			new_u->data[j*new_u->xdim+curr] =
				u->data[j*u->xdim+i];
		}
		new_ev->data[curr*new_ev->xdim+0] = ev->data[i*ev->xdim+0];
		curr++;
	}
		
	*out_u = new_u;
	*out_ev = new_ev;
	FreeArray2D(u);
	FreeArray2D(eigen);
	FreeArray2D(vcv);
	RBDestroyD(list);
	FreeArray2D(ev);
	return(1);
}

Array2D *PCArray(Array2D *x)
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

int SignificantEV(Array2D *ev)
{
	/*
	 * sum of first ev constitutes 85% of variance
	 * assumes that data has been centered and scaled
	 */
	int i;
	double count;
	double acc;

	count = (double)ev->ydim;
	acc = 0;
	for(i=0; i < ev->ydim; i++) {
		acc += ev->data[i*ev->xdim+0]; 
		if((acc / count) > 0.85) {
			return(i+1);
		}
	}

	return(ev->ydim);
}


#ifdef STANDALONE

#define ARGS "x:y:Ec:C:ASRN"
char *Usage = "usage: pca -x xfile\n\
\t-A <automatically exclude co-linear values\n\
\t-C confidence_level\n\
\t-c count <number of components to use>\n\
\t-E <explain variation>\n\
\t-N <no intercept>\n\
\t-R <print residuals>\n\
\t-S <summary only>\n";

char Xfile[4096];
char Yfile[4096];
int Auto;
int Explain;
int Components;
int Summary;
int UseResiduals;
double Confidence;
int NoInt;

double UnscaleB0(double y_bar, Array2D *beta, Array2D *cs)
{
	double b0;
	int i;

	b0 = y_bar;
	for(i=0; i < beta->ydim; i++) {
		b0 -= ((beta->data[i*beta->xdim+0] * cs->data[0*cs->xdim+i])) /
				cs->data[1*cs->xdim+i];
	}

	return(b0);
}

int main(int argc, char *argv[])
{
	int c;
	int size;
	MIO *d_mio;
	Array2D *x;
	Array2D *y;
	Array2D *xt;
	Array2D *xtx;
	MIO *xmio;
	MIO *ymio;
	Array2D *w;
	Array2D *ev;
	Array2D *u;
	Array2D *u_s;
	Array2D *vcv;
	Array2D *gamma;
	Array2D *b;
	Array2D *b_star;
	Array2D *rx;
	Array2D *cs;
	Array2D *sx;
	Array2D *Z;
	double y_bar;
	double acc;
	double count;
	double frac;
	int i;
	int j;
	double rsq;
	double rmse;
	Array2D *resid;
	double b0;
	int err;
	Array2D *Zc;
	Array2D *ci;
	Array2D *bc;
	Array2D *bci;
	Array2D *bv;

	Explain = 0;
	Auto = 0;
	Summary = 0;
	UseResiduals = 0;
	Confidence = 0;
	NoInt = 0;
	while((c = getopt(argc,argv,ARGS)) != EOF) {
		switch(c) {
			case 'x':
				strncpy(Xfile,optarg,sizeof(Xfile));
				break;
			case 'y':
				strncpy(Yfile,optarg,sizeof(Yfile));
				break;
			case 'A':
				Auto = 1;
				break;
			case 'c':
				Components = atoi(optarg);
				break;
			case 'C':
				Confidence = atof(optarg);
				break;
			case 'E':
				Explain = 1;
				break;
			case 'N':
				NoInt = 1;
				break;
			case 'R':
				UseResiduals = 1;
				break;
			case 'S':
				Summary = 1;
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

	if((Confidence < 0) || (Confidence > 1.0)) {
		fprintf(stderr,"must enter confidence level between 0 and 1\n");
		fprintf(stderr,"%s",Usage);
		exit(1);
	}

	x = MakeArray2DFromMIO(xmio);
	y = MakeArray2DFromMIO(ymio);

	if(Components == 0) {
		Components = x->xdim;
	}

	/*
	 * get center/scale mean and s
	 */
	cs = CenterScale(x);
	if(cs == NULL) {
		exit(1);
	}

	/*
	 * scale and center the data
	 */
	sx = MakeArray2D(x->ydim,x->xdim);
	if(sx == NULL) {
		exit(1);
	}
	/*
	 * scale and center the data
	 */
	for(i=0; i < sx->ydim; i++) {
		for(j=0; j < sx->xdim; j++) {
			sx->data[i*sx->xdim+j] =
			(x->data[i*x->xdim+j] - cs->data[0*cs->xdim+j]) /
				cs->data[1*cs->xdim+j];
		}
	}

	err = PrincipalComponents(sx,&u,&ev);
	if(err <= 0) {
		fprintf(stderr,"couldn't get principal components\n");
		exit(1);
	}

		

	/*
	 * measurements are in rows of x
	 */
	w = MultiplyArray2D(sx,u);
	if(y == NULL) {
		fprintf(stderr,"couldn't compute PC transform\n");
		exit(1);
	}

	/*
	for(i=0; i < w->ydim; i++) {
		for(j=0; j < w->xdim; j++) {
			printf("%10.10f ",w->data[i*w->xdim+j]);
		}
		printf("\n");
	}
	*/

	if(Auto == 1) {
		Components = SignificantEV(ev);
	}

	if(Explain == 1) {
		vcv = VarCoVarArray2D(sx);
		if(vcv == NULL) {
			fprintf(stderr,"couldn't get var-co var matrix\n");
			exit(1);
		}
		printf("correlation matrix\n");
		PrintArray2D(vcv);
		printf("eigen vectors (in columns)\n");
		PrintArray2D(u);
		printf("eigen values\n");
		PrintArray1D(ev);
		printf("varfrac: ");
		acc = 0;
		for(i=0; i < ev->ydim; i++) {
			acc += ev->data[i*ev->xdim+0];
		}
		for(i=0; i < ev->ydim; i++) {
			frac = ev->data[i*ev->xdim+0] / acc;
			printf("%f ",frac);
		}
		printf("\n");
		printf("components: %d\n",Components);
		FreeArray2D(vcv);
	}



	/*
	 * make eigenvalue subset for regression
	 */
	u_s = MakeArray2D(u->ydim,Components);
	if(u_s == NULL) {
		exit(1);
	}

	for(i=0; i < u_s->ydim; i++) {
		for(j=0; j < u_s->xdim; j++) {
			u_s->data[i*u_s->xdim+j] =
			  u->data[i*u->xdim+j];
		}
	}

	/*
	 * make Z matrix from significant principal components
	 */
	Z = MultiplyArray2D(sx,u_s);
	if(Z == NULL) {
		fprintf(stderr,"couldn't make Z matrix\n");
		exit(1);
	}

	gamma = RegressMatrix2D(Z,y);
        if(gamma == NULL) {
                fprintf(stderr,"regression failed\n");
                exit(1);
        }
	FreeArray2D(Z);

	/*
	 * create PCR estimator from PCA eigen vectors
	 * (without intercept)
	 */
	b_star = MultiplyArray2D(u_s,gamma);
	if(b_star == NULL) {
		fprintf(stderr,"couldn't form PCR estimator\n");
		exit(1);
	}

	/*
	 * get y_bar
	*/
	y_bar = 0;
	count = 0;
	for(i=0; i < y->ydim; i++) {
		y_bar += y->data[i*y->xdim+0];
		count++;
	}
	y_bar = y_bar / count;

	if(NoInt == 1) {
		b = MakeArray1D(b_star->ydim);
		if(b == NULL) {
			exit(1);
		}
		for(i=0; i < b->ydim; i++) {
			b->data[i*b->xdim+0] = 
				b_star->data[i*b_star->xdim+0]
			 		/ cs->data[1*cs->xdim+i];
		}
		rx = MakeArray2D(x->ydim,x->xdim);
		if(rx == NULL) {
			exit(1);
		}
		for(i=0; i < rx->ydim; i++) {
			for(j=0; j < rx->xdim; j++) {
				rx->data[i*rx->xdim+j] = 
				 x->data[i*x->xdim+j];
			}
		}
	} else {
		/*
		 * estimate of intercept for unscaled data
		 */
		b0 = UnscaleB0(y_bar,b_star,cs);

		b = MakeArray1D(b_star->ydim+1);
		if(b == NULL) {
			exit(1);
		}
		b->data[0*b->xdim+0] = b0;
		for(i=1; i < b->ydim; i++) {
			b->data[i*b->xdim+0] = 
				b_star->data[(i-1)*b_star->xdim+0]
				 / cs->data[1*cs->xdim+(i-1)];
		}

		/*
		 * now make regrssion array to compute fit
		 */
		rx = MakeArray2D(x->ydim,x->xdim+1);
		if(rx == NULL) {
			exit(1);
		}
		for(i=0; i < rx->ydim; i++) {
			rx->data[i*rx->xdim+0] = 1;
			for(j=1; j < rx->xdim; j++) {
				rx->data[i*rx->xdim+j] = 
					x->data[i*x->xdim+(j-1)];
			}
		}
	}

	if(Summary == 0) {
		printf("b: (PCR estimator)\n");
		PrintArray2D(b);
	}
        rsq = RSquared(rx,b,y);
	rmse = RMSE(rx,b,y);
	printf("R^2: %f RMSE: %f\n",rsq,rmse);

	if(Confidence != 0) {
		ci = CIBeta(rx,b,y,Confidence);
		if(ci != NULL) {
			printf("%0.2f CI on PCR est.\n",1-Confidence);
			PrintArray2D(ci);
			FreeArray2D(ci);
		}
	}
	if(UseResiduals == 1) {
		resid = Residuals(rx,b,y);
		if(resid != NULL) {
			printf("residuals\n");
			PrintArray1D(resid);
			FreeArray1D(resid);
		}
	}

	FreeArray2D(u);
	FreeArray2D(u_s);
	FreeArray2D(x);
	FreeArray2D(w);
	FreeArray2D(y);
	FreeArray2D(b);
	FreeArray2D(gamma);
	FreeArray2D(cs);
	FreeArray2D(sx);
	FreeArray2D(rx);
	FreeArray2D(b_star);
	FreeArray2D(ev);

	return(0);
}

#endif
