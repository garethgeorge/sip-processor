#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "mio.h"

char Fname[4096];
double Duration;

char *PRED_ARGS = "f:d:";
char *Usage = "split-data -f predfile\n\
\t-d duration threshold\n";

int main(int argc,char **argv)
{
	int c;
	MIO *data;
	MIO *p_mio;
	unsigned long int psize;
	int i;
	int j;
	int pcols;
	int precs;
	double *parray;
	int tag;
	double ts;
	double last_ts;

	Duration = 0;
	while((c = getopt(argc,argv,PRED_ARGS)) != EOF)
	{
		switch(c)
		{
			case 'f':
				strcpy(Fname,optarg);
				break;
			case 'd':
				Duration = atof(optarg);
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
		fprintf(stderr,"must specify prediction file name\n");
		fprintf(stderr,"%s",Usage);
		exit(1);
	}

	psize = MIOSize(Fname);
	data = MIOOpenText(Fname,"r",psize);
	if(data == NULL) {
		fprintf(stderr, 
	"verify-predictions: couldn't init data for file %s\n",
				Fname);
		fflush(stderr);
		exit(1);
	}
	p_mio = MIODoubleFromText(data,NULL);
	if(p_mio == NULL) {
		MIOClose(data);
		exit(1);
	}
	MIOClose(data);

	parray = MIOAddr(p_mio);
	pcols = p_mio->fields;
	precs = p_mio->recs;

	/*
	 * first col must be ts
	 */

	tag = 0;
	for(i=0; i < precs i++) {
		if(i == 0) {
			last_ts = parray[i*pcols+0];
			continue;
		}
		ts = parray[i*pcols+0]
		if((ts - last_ts) > Duration) {
			tag++;
		}
		printf("%10d | ",tag);
		for(j=0; j < pcols; j++) {
			printf("%f ",parray[i*pcols+j]);
		}
		printf("\n");
	}
	}

	MIOClose(p_mio);
	exit(0);
	
}

