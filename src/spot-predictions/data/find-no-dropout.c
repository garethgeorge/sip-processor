#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "textlist.h"
#include "convert_time.h"
#include "mymalloc.h"
#include "mio.h"
#include "an-file.h"


#define ARGS "f:a:t:I:M:T:"
char *Usage = "find-no-dropout -f filename\n\
\t -a availability-zone\n\
\t -I interval\n\
\t -M min-run-count\n\
\t -t type\n\
\t -T outlier threshold\n";

char Fname[4096];
char Avail_zone[4096];
char Type[4096];
int Interval;
int Min_run;
double Outlier;

int main(int argc, char **argv)
{
	int c;
	MIO *db;
	int err;
	MIONDX *miondx;
	MIONDX *subndx;
	int runcount;
	int Interval;
	RB *rb;
	RB *nrb;
	RB *start_rb;
	RB *last_rb;
	double ts;
	double last_ts;
	int fields;
	unsigned long i;
	char **array;
	int run;
	int last_is_outlier;
	

	Interval = 350;
	Min_run = 20;
	Outlier = 999999999;
	while((c = getopt(argc,argv,ARGS)) != EOF) {
		switch(c) {
			case 'f':
				strncpy(Fname,optarg,sizeof(Fname));
				break;
			case 'a':
				strncpy(Avail_zone,optarg,sizeof(Avail_zone));
				break;
			case 'I':
				Interval = atoi(optarg);
				break;
			case 'M':
				Min_run = atoi(optarg);
				break;
			case 't':
				strncpy(Type,optarg,sizeof(Type));
				break;
			case 'T':
				Outlier = atof(optarg);
				break;
			default:
				fprintf(stderr,
				  "unrecognized command %c\n",
					(char)c);
				fprintf(stderr,
					"%s",Usage);
				exit(1);
		}
	}

	if(Fname[0] == 0) {
		fprintf(stderr,"must specify filename\n");
		fprintf(stderr,"%s",Usage);
		exit(1);
	}

	if(Type[0] == 0) {
		fprintf(stderr,"must specify type\n");
		fprintf(stderr,"%s",Usage);
		exit(1);
	}

	if(Avail_zone[0] == 0) {
		fprintf(stderr,"must specify availability zone\n");
		fprintf(stderr,"%s",Usage);
		exit(1);
	}

	db = LoadRawData(Fname,"1,2,4,5");
	err = ConvertTimeStamp(db,3);
//	PrintMIODB(db);

	miondx = MIONDXInit(db,3,Type);
	if(miondx == NULL) {
		fprintf(stderr,"couldn't create %s index\n",Type);
		MIOClose(db);
		exit(1);
	}
	subndx = MIONDXSubNdx(miondx,0,Avail_zone);
	if(subndx == NULL) {
		fprintf(stderr,"couldn't create %s subindex\n",Avail_zone);
		MIONDXFree(miondx);
		MIOClose(db);
		exit(1);
	}
	MIONDXFree(miondx);


	db = subndx->db;
	array = (char **)MIOAddr(db);
	fields = db->fields;

	runcount = 0;
	run = 0;
	last_ts = 0;
	start_rb = NULL;
	last_is_outlier = 0;
	RB_FORWARD(subndx->index,rb) {
		ts = K_D(rb->key);
		if(last_ts == 0) {
			last_ts = ts;
			continue;
		}
		if((ts - last_ts) < (double)Interval) {
			if(run == 0) {
				start_rb = rb;
			}
			run++;
		} else {
			if(run > Min_run) {
				nrb = start_rb;
				while((nrb != NULL) && (nrb != rb)) {
					i = nrb->value.l;
					if(atof(array[i*fields+2]) > Outlier) {
						if(last_is_outlier == 0) {
							printf("OUTLIER  ");
							printf("%10.0f ",
								K_D(nrb->key));
							printf("%s ",
							   array[i*fields+2]);
							printf("%s %s\n", 
							   Avail_zone, Type);
							last_is_outlier = 1;
						} 
					} else {
						last_is_outlier = 0;
					}
					printf("RUN%5.5d ",runcount);
					printf("%10.0f ",K_D(nrb->key));
					printf("%s ",array[i*fields+2]);
					printf("%s %s\n",
						Avail_zone, Type);
					nrb = nrb->next;
				}
				runcount++;
			}
			run = 0;
			start_rb = NULL;
		}
		last_ts = ts;
	}

//	MIONDXPrint(subndx);
	MIONDXFree(subndx);

	MIOClose(db);

	exit(0);
}

