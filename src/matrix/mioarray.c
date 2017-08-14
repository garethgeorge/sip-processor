#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "mymalloc.h"
#include "mio.h"

#include "mioarray.h"


Array2D *MakeArray2D(int rows, int cols)
{
	Array2D *a;
	MIO *a_mio;

	a = (Array2D *)Malloc(sizeof(Array2D));
	if(a == NULL)
	{
		return(NULL);
	}

	a_mio = MIOMalloc(rows*cols*sizeof(double));
	if(a_mio == NULL) {
		Free(a);
		return(NULL);
	}

	a->data = (double *)MIOAddr(a_mio);
	a->mio = a_mio;

	a->xdim = cols;
	a->ydim = rows;

	return(a);
}

Array2D *MakeArray2DFromMIO(MIO *a_mio)
{
	Array2D *a;

	a = (Array2D *)Malloc(sizeof(Array2D));
	if(a == NULL)
	{
		return(NULL);
	}

	a->data = (double *)MIOAddr(a_mio);
	a->mio = a_mio;

	a->xdim = a_mio->fields;
	a->ydim = (int)a_mio->recs;

	return(a);
}

void FreeArray2D(Array2D *a)
{
	MIOClose(a->mio);
	Free(a);
	return;
}

void PrintArray2D(Array2D *a)
{
	int i;
	int j;

	for(i=0; i < a->ydim; i++)
	{
		fprintf(stdout,"\t[");
		for(j=0; j < a->xdim; j++)
		{
			if(j < a->xdim-1)
			{
				fprintf(stdout,"%f ",a->data[i*a->xdim+j]);
			}
			else
			{
				fprintf(stdout,"%f]\n",a->data[i*a->xdim+j]);
			}
		}
	}

	return;
}

Array2D *CopyArray2D(Array2D *a)
{
	Array2D *b;
	int i;

	b = MakeArray2D(a->ydim,a->xdim);
	if(b == NULL)
	{
		return(NULL);
	}

	for(i=0; i < ((a->xdim)*(a->ydim)); i++)
	{
		b->data[i] = a->data[i];
	}

	return(b);
}

Array2D *TransposeArray2D(Array2D *a)
{
	Array2D *t;
	int i;
	int j;

	t = MakeArray2D(a->xdim,a->ydim);
	if(t == NULL)
	{
		return(NULL);
	}

	for(i=0; i < a->ydim; i++)
	{
		for(j=0; j < a->xdim; j++)
		{
			t->data[j*t->xdim+i] =
				a->data[i*a->xdim+j];
		}
	}

	return(t);
}

#define EL(a,i,j,r) ((a)[(i)*(r)+(j)])

/*
 * adapted from Gauss-Jordon w/o pivoting
 */
#ifndef USELAPACK
Array2D *InvertArray2D(Array2D *a)
{
	int i;
	int j;
	int row;
	int pivot;
	double *temp;
	double *orig_a;
	MIO *or_mio;
	double *new_a;
	MIO *na_mio;
	double mult;
	double *new_b;
	int r;
	int c;
	double *inverse;
	MIO *in_mio;
	double *new_inverse;
	MIO *nin_mio;
	double t;
	Array2D *result;

	/*
	 * rows and columns from a
	 */
	r = a->ydim;
	c = a->xdim;

	/*
	 * allocate space for build
	 */
	na_mio = MIOMalloc(r*c*sizeof(double));
	if(na_mio == NULL)
	{
		fprintf(stderr,"no space for swap in Invert2D\n");
		fflush(stderr);
		exit(1);
	}
	new_a = (double *)MIOAddr(na_mio);

	or_mio = MIOMalloc(r*c*sizeof(double));
	if(or_mio == NULL)
	{
		fprintf(stderr,"no space for orig in Invert2D\n");
		fflush(stderr);
		exit(1);
	}
	orig_a = (double *)MIOAddr(or_mio);

	/*
	 * get space for the identity to make into the inverse
	 */
	in_mio = MIOMalloc(r*c*sizeof(double));
	if(in_mio == NULL)
	{
		fprintf(stderr,"no space for inverse in Invert2D\n");
		fflush(stderr);
		exit(1);
	}
	inverse = (double *)MIOAddr(in_mio);
	memset(inverse,0,r*c*sizeof(double));

	/*
	 * the idea here is to run the identity matrix into the inverse
	 * while turninhg a into the identity matrix
	 */
	for(pivot = 0; pivot < r; pivot++)
	{
		EL(inverse,pivot,pivot,c) = 1.0;
	}

	nin_mio = MIOMalloc(r*c*sizeof(double));
	if(nin_mio == NULL)
	{
		fprintf(stderr,"no space for new inverse in Invert2D\n");
		fflush(stderr);
		exit(1);
	}
	new_inverse = (double *)MIOAddr(nin_mio);

	for(i=0; i < (r*c); i++)
	{
		orig_a[i] = a->data[i];
	}

	for(pivot=0; pivot < r; pivot++)
	{
		for(row=0; row < r; row++)
		{
			t = EL(orig_a,pivot,pivot,c);
			/*
			 * check here for 0 pivot element
			 */
			if(t == 0)
			{
				MIOClose(or_mio);
				MIOClose(na_mio);
				MIOClose(in_mio);
				MIOClose(nin_mio);
				return(NULL);
			}
			mult = EL(orig_a,row,pivot,c) / t;

			if(row == pivot)
			{
				for(j=0; j < c; j++)
				{
					EL(new_a,row,j,c) =
						EL(orig_a,row,j,c) / 
						   EL(orig_a,pivot,pivot,c);
					EL(new_inverse,row,j,c) =
						EL(inverse,row,j,c) /
                                                   EL(orig_a,pivot,pivot,c);
				}
			}
			else
			{
				for(j=0; j < c; j++)
				{
					EL(new_a,row,j,c) =
						EL(orig_a,row,j,c) - mult *
							EL(orig_a,pivot,j,c);
					EL(new_inverse,row,j,c) = 
						EL(inverse,row,j,c) - mult *
                                                        EL(inverse,pivot,j,c);
				}
			}
		}

		temp = new_a;
		new_a = orig_a;
		orig_a = temp;
		or_mio->addr = (void *)orig_a;
		na_mio->addr = (void *)new_a;

		temp = new_inverse;
		new_inverse = inverse;
		inverse = temp;
		in_mio->addr = (void *)inverse;
		nin_mio->addr = (void *)new_inverse;
	}

	MIOClose(or_mio);
	MIOClose(na_mio);
	MIOClose(nin_mio);

	result = MakeArray2D(c,r);
	if(result == NULL)
	{
		MIOClose(in_mio);
		return(NULL);
	}
	else
	{
		/*
		 * throw away the mio in the array
		 */
		MIOClose(result->mio);
		/*
		 * keep the inverse mio
		 */
		result->mio = in_mio;
		result->data = (double *)MIOAddr(in_mio);
		return(result);
	}

}

Array2D *EigenVectorArray2D(Array2D *a)
{
	fprintf(stderr,"EigenVectorArray2D not implemented\n");
	return(NULL);
}

Array1D *EigenValueArray2D(Array2D *a)
{
	fprintf(stderr,"EigenValueArray2D not implemented\n");
	return(NULL);
}
#else

#include "lapacke.h"

Array2D *InvertArray2D(Array2D *a)
{
	Array2D *result;
	MIO *i_mio;
	lapack_int *ipiv;
	lapack_int lda;
	lapack_int m;
	lapack_int n;
	lapack_int info;

	result = CopyArray2D(a); /* these routines are destructive */
	if(result == NULL) {
		return(NULL);
	}

	i_mio = MIOMalloc(a->ydim*sizeof(lapack_int));
	if(i_mio == NULL) {
		FreeArray2D(result);
		return(NULL);
	}
	ipiv = (lapack_int *)MIOAddr(i_mio);

	lda = a->xdim;
	m = a->ydim;
	n = a->xdim;

	info = LAPACKE_dgetrf(LAPACK_ROW_MAJOR,m,n,result->data,lda,
				ipiv);

	if(info != 0) {
		fprintf(stderr,
		"lapacke_dgetrf failed: %d\n",info);
		FreeArray2D(result);
		MIOClose(i_mio);
		return(NULL);
	}

	info = LAPACKE_dgetri(LAPACK_ROW_MAJOR,n,result->data,lda,ipiv);

	if(info != 0) {
		fprintf(stderr,
		"lapacke_dgetri inverse failed: %d\n",info);
		FreeArray2D(result);
		MIOClose(i_mio);
		return(NULL);
	}

	MIOClose(i_mio);

	/*
	 * transpose the result
	 */
	result->ydim = (int)n;
	result->xdim = (int)m;

	return(result);
}

Array1D *EigenValueArray2D(Array2D *a)
{
	/*
	 * use DGHERD, DHSEQR, and DHSEIN to compute eigenvector of a
	 */ 
	lapack_int info;
	lapack_int n;
	lapack_int ilo;
	lapack_int ihi;
	lapack_int lda;
	double *tau;
	Array2D *wr;
	double *wi;
	Array2D *tempa;
	double *z;

	tempa = CopyArray2D(a);
	if(tempa == NULL) {
		return(NULL);
	}

	n = a->ydim;
	lda = a->xdim;
	ilo = 1;
	ihi = n;
	tau = Malloc(n*sizeof(double));
	if(tau == NULL) {
		FreeArray2D(tempa);
		return(NULL);
	}

	/*
	 * call dgehrd to get Hessenberg in upper triangle
	 */
	info = LAPACKE_dgehrd(LAPACK_ROW_MAJOR,
			      n,ilo,ihi,
			      tempa->data,lda,
			      tau);

	if(info < 0) {
		fprintf(stderr,"lapacke_dgehrd failed: %d\n",
			info);
		FreeArray2D(tempa);
		Free(tau);
		return(NULL);
	}

	/*
	 * get eigenvalues from Schur transformation
	 */
	wr = MakeArray1D(a->ydim);
	if(wr == NULL) {
		FreeArray2D(tempa);
		Free(tau);
		return(NULL);
	}
	wi = Malloc(n*sizeof(double));
	if(wi == NULL) {
		FreeArray2D(tempa);
		Free(tau);
		FreeArray1D(wr);
		return(NULL);
	}

	z = (double *)Malloc(n*n*sizeof(double));
	if(z == NULL) {
		FreeArray2D(tempa);
		Free(tau);
		FreeArray1D(wr);
		Free(wi);
		return(NULL);
	}
			
	info = LAPACKE_dhseqr(LAPACK_ROW_MAJOR,
			      'E','N',n,ilo,ihi,
			      tempa->data,lda,
			      wr->data,wi,z,n);
	if(info < 0) {
		fprintf(stderr,"lapacke_dhseqr failed: %d\n",
			info);
		FreeArray2D(tempa);
		Free(tau);
		Free(wi);
		FreeArray1D(wr);
		Free(z);
		return(NULL);
	}

	FreeArray2D(tempa);
	Free(tau);
	Free(wi);
	Free(z);

	return(wr);
}

Array2D *EigenVectorArray2D(Array2D *a)
{
	/*
	 * use DGHERD, DHSEQR, and DHSEIN to compute eigenvector of a
	 */ 
	lapack_int info;
	lapack_int n;
	lapack_int ilo;
	lapack_int ihi;
	lapack_int lda;
	double *tau;
	double *wr;
	double *wi;
	int *select;
	int *ifailr;
	Array2D *tempa;
	Array2D *Q;
	Array2D *eigen;
	double *z;
	int i;
	int j;
	int m;

	tempa = CopyArray2D(a);
	if(tempa == NULL) {
		return(NULL);
	}

	n = a->ydim;
	lda = a->xdim;
	ilo = 1;
	ihi = n;
	tau = Malloc(n*sizeof(double));
	if(tau == NULL) {
		FreeArray2D(tempa);
		return(NULL);
	}

	/*
	 * call dgehrd to get Hessenberg in upper triangle
	 */
	info = LAPACKE_dgehrd(LAPACK_ROW_MAJOR,
			      n,ilo,ihi,
			      tempa->data,lda,
			      tau);

	if(info < 0) {
		fprintf(stderr,"lapacke_dgehrd failed: %d\n",
			info);
		FreeArray2D(tempa);
		Free(tau);
		return(NULL);
	}

	/*
	 * now make Q array so that we can recover a
	 */
	Q = CopyArray2D(tempa);
	if(Q == NULL) {
		FreeArray2D(tempa);
		Free(tau);
		return(NULL);
	}
	/*
	 * get eigenvalues from Schur transformation
	 */
	wr = Malloc(n*sizeof(double));
	if(wr == NULL) {
		FreeArray2D(tempa);
		FreeArray2D(Q);
		Free(tau);
		return(NULL);
	}
	wi = Malloc(n*sizeof(double));
	if(wi == NULL) {
		FreeArray2D(tempa);
		FreeArray2D(Q);
		Free(tau);
		Free(wr);
		return(NULL);
	}

	z = (double *)Malloc(n*n*sizeof(double));
	if(z == NULL) {
		FreeArray2D(tempa);
		FreeArray2D(Q);
		Free(tau);
		Free(wr);
		Free(wi);
		return(NULL);
	}
			
	info = LAPACKE_dhseqr(LAPACK_ROW_MAJOR,
			      'E','N',n,ilo,ihi,
			      tempa->data,lda,
			      wr,wi,z,n);
	if(info < 0) {
		fprintf(stderr,"lapacke_dhseqr failed: %d\n",
			info);
		FreeArray2D(tempa);
		FreeArray2D(Q);
		Free(tau);
		Free(wi);
		Free(wr);
		Free(z);
		return(NULL);
	}

	/*
	use HSEIN to compute eigen vectors of H and then ormhr to
	multiply by Q (in the decomposed form returned by dgehdr)
	to get eigen vectors of a
	*/

	/*
	 * shur transform destroys hessenberg in tempa
	 */
	FreeArray2D(tempa);

	/*
	 * upper triangle of Q (output from dgehrd) is hessenberg
	 */
	tempa = CopyArray2D(Q);
	if(tempa == NULL) {
		FreeArray2D(Q);
		Free(tau);
		Free(wi);
		Free(wr);
		Free(z);
		return(NULL);
	}

	/*
	 * make an array to hold right eigen vectors
	 */
	eigen = MakeArray2D(a->ydim,a->xdim);
	if(eigen == NULL) {
		FreeArray2D(tempa);
		FreeArray2D(Q);
		Free(tau);
		Free(wr);
		Free(wi);
		Free(z);
		return(NULL);
	}

	/*
	 * make a local array to hold selectors
	 */
	select = (int *)Malloc(sizeof(int)*n);
	if(select == NULL) {
		FreeArray2D(tempa);
		FreeArray2D(Q);
		Free(tau);
		Free(wr);
		Free(wi);
		Free(z);
		return(NULL);
	}

	/*
	 * tell lapack to get them all
	 */
	for(i=0; i < n; i++) {
		select[i] = 1;
	}

	/*
	 * make space for right fail vector
	 */
	ifailr = (int *)Malloc(eigen->xdim * sizeof(int));
	if(ifailr == NULL) {
		FreeArray2D(tempa);
		FreeArray2D(Q);
		Free(tau);
		Free(wi);
		Free(wr);
		Free(z);
		Free(select);
		return(NULL);
	}

	/*
	 * R => right eigen vector
	 * Q => shur tranform was used
	 * N => no initial vectors for convergence
	 *
	 * CHECK: fortran specified columns for eigen vectors.  Is it the same
	 * for C?
	 */
	info = LAPACKE_dhsein(LAPACK_ROW_MAJOR,
				'R','Q','N',
				select,n,tempa->data,lda,
			wr,wi,z,n,eigen->data,eigen->xdim,eigen->xdim,&m,
				NULL,ifailr);
	if(info < 0) {
		fprintf(stderr,"lapacke_dhsein failed: %d\n",info);
		FreeArray2D(tempa);
		FreeArray2D(Q);
		Free(tau);
		Free(wi);
		Free(wr);
		Free(z);
		Free(select);
		Free(ifailr);
		return(NULL);
	}

	for(i=0; i < Q->ydim; i++) {
		for(j=i; j < Q->xdim; j++) {
			if(i == j) {
				Q->data[i*Q->xdim+j] = 1;
				if((j-1) >= 0) {
					Q->data[i*Q->xdim+(j-1)] = 0;
				}
			} else {
				Q->data[i*Q->xdim+j] = 0;
			}
		}
	}
				
		
	info = LAPACKE_dormhr(LAPACK_ROW_MAJOR,
				'L','T',eigen->ydim,eigen->xdim,
				ilo,ihi,
				Q->data,Q->xdim,
				tau,
				eigen->data,eigen->xdim);

	if(info < 0) {
		fprintf(stderr,"lapack_dormhr failed %d\n",info);
		FreeArray2D(tempa);
		FreeArray2D(Q);
		FreeArray2D(eigen);
		Free(tau);
		Free(wi);
		Free(wr);
		Free(z);
		Free(select);
		Free(ifailr);
		return(NULL);
	}

	FreeArray2D(tempa);
	FreeArray2D(Q);
	Free(tau);
	Free(wi);
	Free(wr);
	Free(z);
	Free(select);
	Free(ifailr);

	return(eigen);
}

#endif
Array2D *NormalizeRowsArray2D(Array2D *a)
{
	Array2D *result;
	int i;
	int j;
	double acc;

	result = CopyArray2D(a);
	if(result == NULL) {
		return(NULL);
	}

	for(i=0; i < a->ydim; i++) {
		acc = 0.0;
		for(j=0; j < a->xdim; j++) {
			acc += (a->data[i*a->xdim+j] * 
			       a->data[i*a->xdim+j]);
		}
		acc = sqrt(acc);
		for(j=0; j < result->xdim; j++) {
			result->data[i*result->xdim+j] =
				a->data[i*a->xdim+j]/acc;
		}
	}

	return(result);
}

Array2D *NormalizeColsArray2D(Array2D *a)
{
	Array2D *result;
	int i;
	int j;
	double acc;

	result = CopyArray2D(a);
	if(result == NULL) {
		return(NULL);
	}

	for(j=0; j < a->xdim; j++) {
		acc = 0.0;
		for(i=0; i < a->ydim; i++) {
			acc += (a->data[i*a->xdim+j] * 
			       a->data[i*a->xdim+j]);
		}
		acc = sqrt(acc);
		for(i=0; i < result->ydim; i++) {
			result->data[i*result->xdim+j] =
				a->data[i*a->xdim+j]/acc;
		}
	}

	return(result);
}

	

	

Array2D *MultiplyArray2D(Array2D *a, Array2D *b)
{
	Array2D *c;	/* result */
	int i;
	int j;
	int k;
	double acc;
	int new_r;
	int new_c;
	double *data;


	if(a->xdim != b->ydim)
	{
		return(NULL);
	}

	new_r = a->ydim;
	new_c = b->xdim;

	c = MakeArray2D(new_r,new_c);
	if(c == NULL)
	{
		return(NULL);
	}

	data = c->data;
	for(i=0; i < new_r; i++)
	{
		for(j=0; j < new_c; j++)
		{
			acc = 0.0;
			for(k=0; k < a->xdim; k++)
			{
				acc += (a->data[i*(a->xdim)+k] *
					b->data[k*(b->xdim)+j]);
			}
			data[i*new_c+j] = acc;
		}
	}

	return(c);
}

Array2D *AddArray2D(Array2D *a, Array2D *b)
{
	int i;
	int j;
	int cols;
	int rows;
	Array2D *Sum;

	if(a->ydim != b->ydim) {
		return(NULL);
	}

	if(a->xdim != b->xdim) {
		return(NULL);
	}

	rows = a->ydim;
	cols = a->xdim;

	Sum = MakeArray2D(rows,cols);

	if(Sum == NULL) {
		return(NULL);
	}

	for(i=0; i < rows; i++) {
		for(j=0; j < cols; j++) {
			Sum->data[i*cols+j] = 
				a->data[i*cols+j] +
				b->data[i*cols+j];
		}
	}

	return(Sum);
}
			
Array2D *SubtractArray2D(Array2D *a, Array2D *b)
{
	int i;
	int j;
	int cols;
	int rows;
	Array2D *Diff;

	if(a->ydim != b->ydim) {
		return(NULL);
	}

	if(a->xdim != b->xdim) {
		return(NULL);
	}

	rows = a->ydim;
	cols = a->xdim;

	Diff = MakeArray2D(rows,cols);

	if(Diff == NULL) {
		return(NULL);
	}

	for(i=0; i < rows; i++) {
		for(j=0; j < cols; j++) {
			Diff->data[i*cols+j] = 
				a->data[i*cols+j] -
				b->data[i*cols+j];
		}
	}

	return(Diff);
}

#ifdef STANDALONE

int main(int argc, char *argv[])
{
	int i;
	int j;
	Array2D *a;
	Array2D *b;
	Array2D *c;
	Array2D *d;
	Array2D *e;

	a = MakeArray2D(3,5);
	b = MakeArray2D(5,3);

	for(i=0; i < 3; i++)
	{
		for(j=0; j < 5; j++)
		{
			a->data[i*a->xdim+j] = 3.0;
			b->data[j*b->xdim+i] = 5.0;
		}
	}

	c = MultiplyArray2D(a,b);

	if(c == NULL)
	{
		printf("no c\n");
	}
	else
	{
		printf("a:\n");
		PrintArray2D(a);
		printf("b:\n");
		PrintArray2D(b);
		printf("c:\n");
		PrintArray2D(c);
	}

	for(i=0; i < c->ydim; i++)
	{
		for(j=0; j < c->xdim; j++)
		{
			c->data[i*c->xdim+j] += (3.0*drand48());
		}
	}

	d = InvertArray2D(c);
	if(d == NULL)
	{
		printf("c has no inverse\n");
		exit(1);
	}

	e = MultiplyArray2D(c,d);

	if(e == NULL)
	{
		printf("no e\n");
	}
	else
	{
		printf("c:\n");
		PrintArray2D(c);
		printf("d:\n");
		PrintArray2D(d);
		printf("e:\n");
		PrintArray2D(e);
	}

	FreeArray2D(a);
	FreeArray2D(b);
	FreeArray2D(c);
	FreeArray2D(d);
	FreeArray2D(e);

	return(0);
}

#endif

	

	



