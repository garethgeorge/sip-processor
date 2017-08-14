/*
 * http://www.swox.com/gmp/manual
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "jval.h"
#include "jrb.h"
#include "simple_input.h"

#include "norm.h"

#include "bmbp_q_sim.h"
#include "rare_event.h"
#include "bmbp_pred.h"
#include "jbindex.h"


double Quantile;
double Confidence;
int Size;


#define ARGS "s:c:q:"
char *Usage = "bmbp_index -s size\n\
\t-c confidence level\n\
\t-q quantile\n";


int main(int argc, char *argv[])
{
	int c;
	int jbindex;
	double ratio;

	while((c=getopt(argc,argv,ARGS)) != EOF) {
		switch(c) {
			case 's':
				Size = atoi(optarg);
				break;
			case 'q':
				Quantile = atof(optarg);
				break;
			case 'c':
				Confidence = atof(optarg);
				break;
			default:
				fprintf(stderr,"based command %c\n",(char)c);
				fprintf(stderr,"usage: %s",Usage);
				exit(1);
		}
	}

	if(Size < 0) {
		fprintf(stderr,"must enter size\n");
		fprintf(stderr,"usage: %s",Usage);
		exit(1);
	}

	if((Quantile < 0) || (Quantile > 1.0)) {
		fprintf(stderr,"bad quantile\n");
		exit(1);
	}

	if((Confidence < 0) || (Confidence > 1.0)) {
		fprintf(stderr,"bad confidence\n");
		exit(1);
	}

	jbindex = JBIndex(Size,Quantile,Confidence,&ratio);

	printf("%d\n",jbindex);

	exit(0);
}
