#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "mio.h"
#include "meanvar.h"

char Fname[4096];
int Experiments;

char *PRED_ARGS = "f:c:";
char *Usage = "verify-predictions -f filename\n\
\t-c count\n";

int main(int argc,char **argv)
{
	int c;
	MIO *data;
	MIO *d_mio;
	unsigned long int size;
	int diff;
	unsigned long int i;
	int f;
	int cols;
	double *darray;

	while((c = getopt(argc,argv,PRED_ARGS)) != EOF)
	{
		switch(c)
		{
			case 'f':
				strcpy(Fname,optarg);
				break;
			case 'c':
				Experiments = atoi(optarg);
				break;
			default:
				fprintf(stderr,"unrecognized argument %c\n",
						c);
				fflush(stderr);
				break;
		}
	}

	if(Fname[0] == 0)
	{
		fprintf(stderr,"must specify file name\n");
		fprintf(stderr,"%s",Usage);
		exit(1);
	}

	size = MIOSize(Fname);
	data = MIOOpenText(Fname,"r",size);
	if(data == NULL) {
		fprintf(stderr, 
	"verify-predictions: couldn't init data for file %s\n",
				Fname);
		fflush(stderr);
		exit(1);
	}
	d_mio = MIODoubleFromText(data,NULL);
	if(d_mio == NULL) {
		MIOClose(data);
		exit(1);
	}
	MIOClose(data);

	darray = MIOAddr(d_mio);
	f = d_mio->fields-1;
	cols = d_mio->fields;
		

	MIOClose(d_mio);
	exit(0);
	
}

#endif
