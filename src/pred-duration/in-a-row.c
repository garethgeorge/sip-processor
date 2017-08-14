/****************************************************************/
/*                                                              */
/*      Global include files                                    */
/*                                                              */
/****************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include "mio.h"



char *ARGS = "f:LT:";
int UseLow;
double Threshold;

char *Usage = "in-a-row -f filename\n\
\t-L <use low>\n\
\t-T threshold\n";

int main(int argc,char **argv)
{

	char fname[255];
	int c;
	int ierr;
	MIO *data;
	MIO *d_mio;
	unsigned long size;
	double now;
	unsigned long i;
	int f;
	double *series;
	double value;
	int found;
	double duration;
	unsigned long recs;
	int in_a_row;
	int data_f;

	UseLow = 0;
	Threshold = 0;
	while((c = getopt(argc,argv,ARGS)) != EOF)
	{
		switch(c)
		{
			case 'f':
				strcpy(fname,optarg);
				break;
			case 'L':
				UseLow = 1;
				break;
			case 'T':
				Threshold = atof(optarg);
				break;
			default:
				fprintf(stderr,"unrecognized argument %c\n",
						c);
				fprintf(stderr,"%s",Usage);
				fflush(stderr);
				break;
		}
	}

	if(fname[0] == 0)
	{
		fprintf(stderr,"usage: autoc -f fname [-l lags]\n");
		fflush(stderr);
		exit(1);
	}
	
	size = MIOSize(fname);
	data = MIOOpenText(fname,"r",size);
	if(data == NULL) {
		fprintf(stderr,"autoc: couldn't init data for file %s\n",
				fname);
		fflush(stderr);
		exit(1);
	}

	f = MIOTextFields(data);
	if(f <= 0) {
		fprintf(stderr,
		"file %s must have at least one column of data\n",
			fname);
		MIOClose(data);
		exit(1);
	}
	recs = MIOTextRecords(data);
	data_f = f - 1;
	d_mio = MIODoubleFromText(data,NULL);
	if(d_mio == NULL) {
		MIOClose(data);
		exit(1);
	}
	MIOClose(data);

	series = (double *)MIOAddr(d_mio);

	in_a_row = 0;
	for(i=0; i < recs; i++) {
		value = series[i*f+data_f];
		if(UseLow == 0) {
			if(value > Threshold) {
				in_a_row++;
			} else {
				printf("%d\n",in_a_row);
				in_a_row = 0;
			}
		} else {
			if(value < Threshold) {
				in_a_row++;
			} else {
				printf("%d\n",in_a_row);
				in_a_row = 0;
			}
		}
	}

	MIOClose(d_mio);

	return(0);
}
				
			 
