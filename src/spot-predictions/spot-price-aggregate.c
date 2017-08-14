#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mio.h"
#include "mymalloc.h"
#include "textlist.h"

#define FSEP ","

#define ARGS "f:i:"
char *Usage = "spot-price-aggregate -f file-list (comma separated)\n\
\t-I interval\n";

char **Files;
int Interval;

int main(int argc, char **argv)
{
	int c;
	char *file_list;
	TXL *tl;
	int i;
	int j;
	int k;
	double now;
	int min_k;
	double min_p;
	DlistNode *dn;
	char *fn;
	int fcount;
	MIO **mios;
	int size;
	double **arrays;
	int *ndx;
	int done;
	int cols;
	double *data;
	int last_k;
	double last_p;
	double last_ts;
	int next;
	int good;

	file_list = NULL;
	while((c = getopt(argc,argv,ARGS)) != EOF) {
		switch(c) {
			case 'f':
				file_list = (char *)Malloc(strlen(optarg)+1);
				if(file_list == NULL) {
					exit(1);
				}
				strncpy(file_list,optarg,strlen(optarg));
				break;
			case 'i':
				Interval = atoi(optarg);
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

	if(file_list == NULL) {
		fprintf(stderr,"must specify file list\n");
		fprintf(stderr,"%s",Usage);
		exit(1);
	}

	tl = ParseLine(file_list,FSEP);
	if(tl == NULL) {
		fprintf(stderr,"couldn't parse file list %s\n",file_list);
		fprintf(stderr,"%s",Usage);
		exit(1);
	}

	Files = (char **)Malloc(tl->count*sizeof(char *));
	if(Files == NULL) {
		exit(1);
	}

	i=0;
	DLIST_FORWARD(tl->list,dn) {
		fn = dn->value.s;
		Files[i] = (char *)Malloc(strlen(fn)+1);
		if(Files[i] == NULL) {
			exit(1);
		}
		strncpy(Files[i],fn,strlen(fn));
		i++;
	}
	fcount = i;
	DestroyTXL(tl);

	mios = (MIO **)Malloc(fcount*sizeof(MIO *));
	if(mios == NULL) {
		exit(1);
	}
	good = 0;
	for(i=0; i < fcount; i++) {
		size = MIOSize(Files[good]);
		mios[good] = MIOOpenDouble(Files[i],"r",size);
		if(mios[good] == NULL) {
			fprintf(stderr,
				"couldn't open %s for data\n",Files[i]);
		} else {
			good++;
		}
	}

	fcount = good;

	arrays = (double **)Malloc(fcount*sizeof(double *));
	if(arrays == NULL) {
		exit(1);
	}
	ndx = (int *)Malloc(fcount*sizeof(int));
	if(ndx == NULL) {
		exit(1);
	}


	for(i=0; i < fcount; i++) {
		arrays[i] = MIOAddr(mios[i]);
		ndx[i] = 0;
	}

	min_p = 9999999999999;
	min_k = -1;
	now = 99999999999;
	last_ts = 0;

	for(k=0; k < fcount; k++) {
		data = arrays[k];
		cols = mios[k]->fields;
		i = ndx[k];
		/*
		 * ts in first column (sort from lowest to highest)
		 * price in second column
		 *
		 * find eariest to start
		 */
		if(data[i*cols+0] < now) {
			min_k = k;
			min_p = data[i*cols+1];
			now = data[i*cols+0];
		} else if((data[i*cols+0] == now) &&
			  (data[i*cols+1] < min_p)) {
				min_k = k;
				min_p = data[i*cols+1];
		}
	}
	next = min_k;

		

	while(1) {
		done = 1;
		for(i=0; i < fcount; i++) {
			if(ndx[i] < mios[i]->recs) {
				done = 0;
				break;
			}
		}
		if(done == 1) {
			break;
		}

		/*
		 * print the current min price
		 */
		printf("%10.0f %f\n",now,min_p);
		last_p = min_p;
		last_k = min_k;
		ndx[next]++;


		/*
		 * move time ahead and see if a lower price is available at
		 * next time stamp;
		 */
		last_ts = now;
		now = 999999999999;
		min_p = 99999999999;
		for(k=0; k < fcount; k++) {
			data = arrays[k];
			cols = mios[k]->fields;
			i = ndx[k];
			/*
			 * skip any we have finished
			 */
			if(i >= mios[k]->recs) {
				continue;
			}

			/*
			 * ts in first column
			 * price in second column
			 *
			 * find eariest to start
			 */
			if(data[i*cols+0] < now) {
				now = data[i*cols+0];
				/*
				 * index to advance next
				 */
				next = k;
				/*
				 * we know that "now" is the earliest we've
				 * seen so far
				 *
				 * is this the last min?  if so, the price is
				 * no longer valid so take this one
				 * 
				 * if we already chose last_p because of
				 * another AZ, then we take this one because
				 * last_p is no longer valid
				 */
				if(k == last_k) {
					if((min_p == last_p) || 
						(data[i*cols+1] <= min_p)) {
						min_k = k;
						min_p = data[i*cols+1];
					}
				} else {
					/*
					 * take this price if it is lower
					 * than last quoted
					 */
					if(data[i*cols+1] <= last_p) {
						min_k = k;
						min_p = data[i*cols+1];
					} else {
						min_p = last_p;
					}
				}
			} else if((data[i*cols+0] == now) &&
				  (data[i*cols+1] < min_p)) {
					min_k = k;
					min_p = data[i*cols+1];
			}
		}
		if(now == 999999999999) {
			break;
		}
		if((Interval != 0) && ((now - last_ts) > (double)Interval)) {
			last_ts = last_ts + Interval;
			while(last_ts < now) {
				printf("%10.0f %f\n",last_ts,last_p);
				last_ts = last_ts + Interval;
			}
		}
	}

	for(k=0; k < fcount; k++) {
		MIOClose(mios[k]);
	}

		exit(0);
		 
}


