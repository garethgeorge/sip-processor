#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "mioarray.h"


int main(int argc, char *argv[])
{
	int i;
	int j;
	Array2D *a;
	Array2D *b;
	Array2D *c;
	Array2D *ct;
	Array1D *ev;
	Array2D *temp;
	Array2D *temp1;

	a = MakeArray2D(3,3);

	a->data[0*3+0] = 2382.78;
	a->data[0*3+1] = 2611.84;
	a->data[0*3+2] = 2136.20;
	a->data[1*3+0] = 2611.84;
	a->data[1*3+1] = 3106.47;
	a->data[1*3+2] = 2553.9;
	a->data[2*3+0] = 2136.2;
	a->data[2*3+1] = 2553.9;
	a->data[2*3+2] = 2650.71;

	b = EigenVectorArray2D(a);
	if(b == NULL) {
		fprintf(stderr,"regress 1 failed\n");
		exit(1);
	}

	/*
	 * answer should be
	 * c(0,*):[0.5417]
	 *        [0.6295]
	 *        [0.5570]
	 */
	c = NormalizeColsArray2D(b);


	printf("a: ");
	PrintArray2D(a);
	printf("b: ");
	PrintArray2D(b);
	printf("c: ");
	PrintArray2D(c);
	printf("\n");

	ct = TransposeArray2D(c);

	temp = MultiplyArray2D(ct,a);
	temp1 = MultiplyArray2D(temp,c);

	printf("ct*a*c:");
	PrintArray2D(temp1);

	ev = EigenValueArray2D(a);
	printf("ev: ");
	PrintArray1D(ev);


	FreeArray2D(a);
	FreeArray2D(b);
	FreeArray2D(c);
	FreeArray2D(temp);
	FreeArray2D(temp1);
	FreeArray1D(ev);

	return(0);
}

	

	



