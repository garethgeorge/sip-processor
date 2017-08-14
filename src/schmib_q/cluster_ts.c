#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "jval.h"
#include "jrb.h"
#include "dllist.h"
#include "simple_input.h"
#include "norm.h"
#include "schmib-means.h"

#define MAX_B (3)

extern int Max_bin;

int Use2;
char Fname[255];
int Means;
int Find_outlier;
int Verbose;
int UseNormal;
int UseUniform;
extern int Keep_going;
int data_count;

int DoEdgeMerge(Bin**, int, int);
int DoCrossMerge(Bin**, int, int);
int DoSingletonMerge(Bin**, int);
     
char Line_buf[1024*1024];

char *ARGS = "f:vKNUB:";
char *Usage = "bic_means -f filename\n\
\t-B max bins\n\
\t-K <keep going>\n\
\t-N <use log normal>\n\
\t-v <verbose>\n";

#define DATA_FIELDS (4)
#define EXP_FIELD (3)
#define RESP_FIELD (1)

int main(int argc, char *argv[])
{
	int c;
	void *data;
	void *raw_data;
	int err;
	int means;
	Bin **bins;
	double bic;
	int i;
	int src;
	int dst;
	double w;
	double x;
	double y;
	double z;
	double *states;
	double min;
	double max;
	double N;
	double row_total;
	int scount;
	
	Verbose = 0;
	Keep_going = 0;
	data_count = 0;
	UseNormal = 0;
	UseUniform = 0;
	while((c = getopt(argc,argv,ARGS)) != EOF)
	{
		switch(c)
		{
			case 'f':
				strncpy(Fname,optarg,sizeof(Fname));
				break;
			case 'B':
				Max_bin = atoi(optarg);
				break;
			case 'v':
				Verbose = 1;
				break;
			case 'K':
				Keep_going = 1;
				break;
			case 'N':
				UseNormal = 1;
				break;
			case 'U':
				UseUniform = 1;
				break;
			default:
				fprintf(stderr,"unrecognized arg %c\n",c);
				fprintf(stderr,"%s",Usage);
				fflush(stderr);
				exit(1);
		}
	}

	if(Fname[0] == 0)
	{
		fprintf(stderr,"must specify filename\n");
		fprintf(stderr,"%s",Usage);
		fflush(stderr);
		exit(1);
	}

	err = InitDataSet(&raw_data,DATA_FIELDS);
	if(err == 0)
	{
		fprintf(stderr,"couldn't open file %s\n",
				Fname);
		fflush(stderr);
		exit(1);
	}

	err = LoadDataSet(Fname,raw_data);
	if(err == 0)
	{
		fprintf(stderr,"couldn't read data from file %s\n",
				Fname);
		fflush(stderr);
		exit(1);
	}

	/*
	 * now purify the data
	 */
	err = InitDataSet(&data,DATA_FIELDS);
	if(err == 0)
	{
		exit(1);
	}

	while(ReadEntry4(raw_data,&w,&x,&y,&z))
	{
		if((x <= 0) || (z <= 0))
		{
			continue;
		}
		WriteEntry4(data,w,x,y,z);
	}

	Rewind(data);

	bins = SchmibMeans(data,
			   DATA_FIELDS,
			   EXP_FIELD,
			   RESP_FIELD,
			   &means);
	if(UseNormal)
	{
		bic = NBIC(bins,(double)means,RESP_FIELD,(double)SizeOf(data));
	}
	else if(UseUniform)
	{
		bic = UBIC(bins,(double)means,RESP_FIELD,(double)SizeOf(data));
	}
	else
	{
		bic = OrderBIC(bins,(double)means,RESP_FIELD,(double)SizeOf(data));
	}
	fprintf(stdout,"BIC: %f, %d bins\n",bic,means);
	for(i=0; i < means; i++)
	  {
	    PrintBin(stdout,bins[i],
			EXP_FIELD,
			RESP_FIELD,
			i);
	  }

	scount = means + 1;

	states = (double *)malloc(scount*scount*sizeof(double));
	if(states == NULL)
	{
		exit(1);
	}

	memset(states,0,scount*scount*sizeof(double));

	Rewind(raw_data);
	src = 0;
	dst = 0;
	while(ReadEntry4(raw_data,&w,&x,&y,&z))
	{
		/*
		 * treat zeros as their own state
		 */
		if(y == 0)
		{
			dst = 0;
		}
		else
		{
			for(i=0; i < means; i++)
			{
				BinMinMax(bins[i],RESP_FIELD,&min,&max);
				/*
				 * y is RESP_FIELD
				 */
				if((y >= min) && (y <= max))
				{
					break;
				}
			}
			if(i == means)
			{
				i = means - 1;
			}
			dst = i+1;
		}
		states[src*scount + dst]++;
		src = dst;
	}

	N = SizeOf(raw_data);

	for(src=0; src < scount; src++)
	{
		row_total = 0;
		for(dst=0; dst < scount; dst++)
		{
			row_total += states[src*scount+dst];
		}
		for(dst=0; dst < scount; dst++)
		{
			printf("%f ",states[src*scount+dst]/row_total);
		}
		printf("\n");
	}

	BinMinMax(bins[0],RESP_FIELD,&min,&max);
	Rewind(raw_data);
	while(ReadEntry4(raw_data,&w,&x,&y,&z))
	{
		/*
		 * treat zeros as their own state
		 */
		if(y == 0)
		{
			continue;
		}
		else
		{
			if((y < min) || (y > max))
			{
				continue;
			}
		}
		printf("%f\n",y);
	}	
	
	return(0);
}

