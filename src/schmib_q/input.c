#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "input.h"

static double Seq_no = 0;


/*
 * in the mess that follows,
 * head -- the location of the oldest valid data 
 * tail -- the location of the next location in which valid data
 *         may be stored
 * curr -- (circular case only) location of the next readable
 *         valid data
 */

void
SetBias(char *icookie, double bias)
{
	struct data_set *cookie;
	
	cookie = (struct data_set *)icookie;
	cookie->bias = bias;
	return;
}


int
InitDataSet(int diff, double bias, char **cookie)
{
	struct data_set *lds;
	
	if(diff >= (BUFSIZE - 1))
	{
		fprintf(stderr,"InitDataSet: diff %d bigger than buffer %d\n",
				diff,BUFSIZE);
		fflush(stderr);
		return(0);
	}
	
	if(diff < 0)
	{
		fprintf(stderr,"InitDatSet: diff %d must be > 0\n",
				diff);
		fflush(stderr);
		return(0);
	}
	
	lds = (struct data_set *)malloc(sizeof(struct data_set));
	if(lds == NULL)
	{
		fprintf(stderr,
		"InitDataSet: couldn't malloc space for data_set on open\n");
		fflush(stderr);
		return(0);
	}
	
	
	lds->data = 
		(double *)malloc(DELEMENTS*BUFSIZE*sizeof(double));
	if(lds->data == NULL)
	{
		fprintf(stderr,
	"InitDataSet: can't malloc %d elements on init\n",BUFSIZE);
		fflush(stderr);
		free(lds);
		return(0);
	}

	lds->dbuf1 = (double *)malloc((diff+1)*sizeof(double));
	if(lds->dbuf1 == NULL)
	{
		fprintf(stderr,
			"InitDataSet: couldn't malloc space for dbuf1\n");
		fflush(stderr);
		free(lds->data);
		free(lds);
		return(0);
	}

	lds->dbuf2 = (double *)malloc((diff+1)*sizeof(double));
	if(lds->dbuf2 == NULL)
	{
		fprintf(stderr,
			"InitDataSet: couldn't malloc space for dbuf2\n");
		fflush(stderr);
		free(lds->data);
		free(lds->dbuf1);
		free(lds);
		return(0);
	}

	lds->data_size = BUFSIZE;

	
	lds->circular = 1;
	lds->head = 0;
	lds->wrap = 0;
	lds->last = 0;
	lds->size = -1;
	lds->diff = diff;
	lds->bias = bias;
	
	lds->curr = lds->head = lds->tail = 0;
	
	*cookie = (char *)lds;
	
	return(1);
}

void
SetDataSize(char *icookie, int size)
{
	struct data_set *cookie;
	int lsize;
	
	cookie = (struct data_set *)icookie;
	
	if(size > BUFSIZE)
	{
		lsize = BUFSIZE;
	}
	else
	{
		lsize = size;
	}
	
	cookie->data_size = lsize;
	if(cookie->last > lsize)
	{
		cookie->last = lsize;
	}
	
	return;
}

int
InitData(char *fname, int diff, double bias, int logflag, char **cookie)
{
	struct data_set *lds;
	FILE *fd;
	
	if(diff >= (BUFSIZE - 1))
	{
		fprintf(stderr,"InitData: diff %d bigger than buffer %d\n",
				diff,BUFSIZE);
		fflush(stderr);
		return(0);
	}
	
	if(diff < 0)
	{
		fprintf(stderr,"InitData: diff %d must be > 0\n",
				diff);
		fflush(stderr);
		return(0);
	}
	

	fd = fopen(fname,"r");
	if(fd == NULL)
	{
		fprintf(stderr,"InitData: could open %s for reading\n",
		                fname);
		fflush(stderr);
		return(0);
	}

	lds = (struct data_set *)malloc(sizeof(struct data_set));
	if(lds == NULL)
	{
		fprintf(stderr,
		"InitData: couldn't malloc space for data_set on open of %s\n",
		fname);
		fflush(stderr);
		fclose(fd);
		return(0);
	}
	
	
	lds->data = 
		(double *)malloc(DELEMENTS*BUFSIZE*sizeof(double));
	if(lds->data == NULL)
	{
		fprintf(stderr,
	"InitData: can't malloc %d elements for %s on init\n",
		BUFSIZE,fname);
		fflush(stderr);
		free(lds);
		fclose(fd);
		return(0);
	}

	lds->dbuf1 = (double *)malloc((diff+1)*sizeof(double));
	if(lds->dbuf1 == NULL)
	{
		fprintf(stderr,
			"InitData: couldn't malloc space for dbuf1\n");
		fflush(stderr);
		free(lds->data);
		free(lds);
		return(0);
	}

	lds->dbuf2 = (double *)malloc((diff+1)*sizeof(double));
	if(lds->dbuf2 == NULL)
	{
		fprintf(stderr,
			"InitData: couldn't malloc space for dbuf2\n");
		fflush(stderr);
		free(lds->data);
		free(lds->dbuf1);
		free(lds);
		return(0);
	}

	lds->data_size = BUFSIZE;

	
	strncpy(lds->name,fname,sizeof(lds->name));
	lds->circular = 0;
	lds->fd = fd;
	lds->head = 0;
	lds->wrap = 0;
	lds->last = 0;
	lds->size = -1;
	lds->diff = diff;
	lds->bias = bias;
	lds->logflag = logflag;
	
	lds->tail = lds->head = 0;
	
	*cookie = (char *)lds;
	
	return(1);
}

void
RemoveData(char *cookie)
{
	struct data_set *lds;
	
	lds = (struct data_set *)cookie;
	if(lds->circular == 0)
	{
		fclose(lds->fd);
	}
	free(lds->dbuf1);
	free(lds->dbuf2);
	free(lds->data);
	free(lds);
	
	return;
}

int
Rewind(char *icookie)
{
	struct data_set *cookie;
	int temp_i;
	
	cookie = (struct data_set *)icookie;
	
	/*
	 * circular flag defines buffer instead of file
	 */
	if(cookie->circular == 1)
	{
		temp_i = MODMINUS(cookie->tail,cookie->head,cookie->last+1);
		if(temp_i > cookie->diff)
		{
			cookie->curr = 
			   MODPLUS(cookie->head,
			   cookie->diff,cookie->last+1);
		}
		else
		{
			cookie->curr = cookie->head;
		}
		return(1);
	}

	/*
	 * if the entire data set is contained in the buffer
	 * then simply reset the head pointer to 0
	 */
	if(cookie->wrap == 0)
	{
		/*
		 * if we have already read some data, reset
		 */
		if((cookie->last != 0) && (cookie->last > cookie->diff))
		{
			cookie->head = cookie->diff;
			return(1);
		}
		
		/*
		 * otherwise, set the head up to point to the
		 * last piece of valid data (which could be zero)
		 */
		cookie->head = cookie->last;
		return(1);
	}
	
	/*
	 * otherwise, rewind the file and reset
	 */
	fclose(cookie->fd);
	cookie->fd = fopen(cookie->name,"r");
	if(cookie->fd == NULL)
	{
		fprintf(stderr,"Rewind: couldn't reopen file %s\n",
				cookie->name);
		fflush(stderr);
		return(0);
	}
	cookie->tail = 0;
	cookie->head = 0;
	cookie->wrap = 0;
	
	return(1);
}
	
int
WriteEntry(char *icookie, double ts, double value)
{
	struct data_set *cookie;
	int temp_i;
	
	cookie = (struct data_set *)icookie;
	
	if(cookie->circular == 0)
	{
		fprintf(stderr,"WriteEntry: can't write permanent file %s\n",
				cookie->name);
		fflush(stderr);
		return(0);
	}
	
	TS(cookie->data,cookie->tail) = ts;
	VAL(cookie->data,cookie->tail) = value;

	if(cookie->last < cookie->data_size)
	{
		cookie->last = cookie->last + 1;
	}
	
	cookie->tail = MODPLUS(cookie->tail,1,cookie->data_size);

	/*
	 * if the tail has come around to meet the head, bump
	 */
	if(cookie->head == cookie->tail)
	{
		cookie->head = MODPLUS(cookie->head,1,cookie->data_size);
	}
	if(cookie->tail == cookie->curr)
	{
		cookie->curr = MODPLUS(cookie->curr,1,cookie->data_size);
	}
	
	/*
	 * bump curr until it gets enough data to do the diff thing
	 */
	temp_i = MODMINUS(cookie->curr,cookie->head,cookie->last+1);
	if(temp_i < cookie->diff)
	{
		cookie->curr = MODPLUS(cookie->curr,1,cookie->last+1);
	}
	
	
	return(1);
}

int
ReadValue(char *icookie,double *value)
{
	char line_buff[255];
	char *err;
	char *c;
	struct data_set *cookie;
	int i;
	int temp_i;
	double *temp_b;
	double *b1;
	double *b2;
	int d;
	double temp_d;
	
	cookie = (struct data_set *)icookie;
	
	/*
	 * do circular stuff separately
	 */
	if(cookie->circular == 1)
	{
		if(cookie->curr == cookie->tail)
		{
			return(0);
		}
		
		/*
		 * now check to see if there is enough data based on the
		 * diff value
		 */
		temp_i = MODMINUS(cookie->curr,cookie->head,cookie->last+1);
		if(temp_i < cookie->diff)
		{
			fprintf(stderr,
				"ReadValue: short read of buffer\n");
			fflush(stderr);
			return(0);
		}
		
		b1 = cookie->dbuf1;
		b2 = cookie->dbuf2;
		for(i=0; i < cookie->diff+1; i++)
		{
			temp_i = 
			MODMINUS(cookie->curr,i,cookie->last+1);
			/*
			 * only for cirsular case do we apply the bias
			 * when the data comes out of the data structure
			 */
			b1[i] = VAL(cookie->data,temp_i) - cookie->bias;
		}
		for(d=cookie->diff; d > 0; d--)
		{
			for(i=0; i < d; i++)
			{
				b2[i] = b1[i] - b1[i+1];
			}
			temp_b = b1;
			b1 = b2;
			b2 = temp_b;
		}

		*value = b1[0];
		cookie->curr = MODPLUS(cookie->curr,1,cookie->last+1);
		
		return(1);
	}

	/*
	 * FILE version begins here
	 */
		
	/*
	 * if head != tail, then data set contains unread data
	 */
	if(cookie->head != cookie->tail)
	{

		b1 = cookie->dbuf1;
		b2 = cookie->dbuf2;
		for(i=0; i < cookie->diff+1; i++)
		{
			temp_i = cookie->head - i;
			b1[i] = VAL(cookie->data,temp_i);
		}
		for(d=cookie->diff; d > 0; d--)
		{
			for(i=0; i < d; i++)
			{
				b2[i] = b1[i] - b1[i+1];
			}
			temp_b = b1;
			b1 = b2;
			b2 = temp_b;
		}

		*value = b1[0];
		cookie->head = cookie->head + 1;
		return(1);
	}
	

	if(feof(cookie->fd))
		return(0);
	
	/*
	 * otherwise, just reset head and tail to beginning of buffer and 
	 * read some more
	 */
	cookie->head = 0;

	/*
	 * if we have wrapped, we need the last diff values from the end of
	 * this buffer to go up front in the next
	 */ 
	if(cookie->wrap == 1)
	{
		for(i=cookie->diff; i > 0; i--)
		{
			TS(cookie->data,cookie->head) =
				TS(cookie->data,cookie->tail - i);
			VAL(cookie->data,cookie->head) =
				VAL(cookie->data,cookie->tail - i);
			cookie->head = cookie->head + 1;
		}
		/*
		 * here, there is no valid data so head and
		 * tail must point to the same location
		 */
		cookie->head = cookie->diff;
		cookie->tail = cookie->head;
	}

	while(cookie->tail < cookie->data_size)
	{
		err = fgets(line_buff,254,cookie->fd);

		if(err == NULL)
		{
			/*
			 * if there is valid data, return it
			 */
			if(cookie->head != cookie->tail)
			{
				b1 = cookie->dbuf1;
				b2 = cookie->dbuf2;
				for(i=0; i < cookie->diff+1; i++)
				{
					temp_i = cookie->head - i; 
					b1[i] = VAL(cookie->data,temp_i);
				}
				for(d=cookie->diff; d > 0; d--)
				{
					for(i=0; i < d; i++)
					{
						b2[i] = b1[i] - b1[i+1];
					}
					temp_b = b1;
					b1 = b2;
					b2 = temp_b;
				}

				*value = b1[0];
				cookie->head = cookie->head + 1;
				/*
				 * if we are bailing out on EOF before
				 * reading a complete BUFSIZE, then remember
				 * the index of the last data value
				 */
				if(cookie->wrap == 0)
				{
					cookie->last = cookie->tail;
				}
				return(1);
			}
			else
			{
				return(0);
			}
		}

		TS(cookie->data,cookie->tail) = DUMMY_TS;

		c = line_buff;

		while(!isdigit(*c) && (*c != '-'))
			c++;

		if(cookie->logflag == 1)
		{
			temp_d = strtod(c,&c);
			if(temp_d == 0.0)
			{
				fprintf(stderr,
		"ReadEntry: 0 value on log input, ts: %10.0f, skipping\n",
				TS(cookie->data,cookie->tail));
				fflush(stderr);
				continue;
			}
			VAL(cookie->data,cookie->tail) = 
				log(temp_d) - cookie->bias;
		}
		else
		{
			VAL(cookie->data,cookie->tail) = 
				strtod(c,&c) - cookie->bias;
		}
		
		cookie->tail = cookie->tail + 1;

		/*
		 * bump the head up until there is enough to do the
		 * diff thing
		 */
		if(cookie->head < cookie->diff)
		{
			cookie->head = cookie->head + 1;
		}
		
	}
	
	cookie->wrap = 1;
	if(cookie->head != cookie->tail)
	{
		b1 = cookie->dbuf1;
		b2 = cookie->dbuf2;
		for(i=0; i < cookie->diff+1; i++)
		{
			temp_i =  cookie->head - i;
			b1[i] = VAL(cookie->data,temp_i);
		}
		for(d=cookie->diff; d > 0; d--)
		{
			for(i=0; i < d; i++)
			{
				b2[i] = b1[i] - b1[i+1];
			}
			temp_b = b1;
			b1 = b2;
			b2 = temp_b;
		}

		*value = b1[0];
		cookie->head = cookie->head + 1;
		return(1);
	}
	else /* paranoid */
	{
		cookie->head = cookie->tail = 0;
		return(0);
	}

}	

int GetNextValue(char *s, double *v1, double *v2)
{
	char *c;
	int count = 0;

	c = s;

	while(!isdigit(*c) && (*c != '-') && (*c != '\n'))
		c++;

	if(*c == '\n')
		return(0);
	
	*v1 = strtod(c,&c);
	count = 1;

	while(!isdigit(*c) && (*c != '-') && (*c != '\n'))
		c++;

	if(*c == '\n')
		return(count);

	*v2 = strtod(c,&c);
	count++;

	return(count);
}


int
ReadEntry(char *icookie,double *ts,double *value)
{
	char line_buff[255];
	char *err;
	struct data_set *cookie;
	int i;
	int temp_i;
	double *temp_b;
	double *b1;
	double *b2;
	int d;
	double temp_d;
	double lts;
	double lvalue;
	int fcount;
	
	cookie = (struct data_set *)icookie;
	
	/*
	 * do circular stuff separately
	 */
	if(cookie->circular == 1)
	{
		if(cookie->curr == cookie->tail)
		{
			return(0);
		}
		
		/*
		 * now check to see if there is enough data based on the
		 * diff value
		 */
		temp_i = MODMINUS(cookie->curr,cookie->head,cookie->last+1);
		if(temp_i < cookie->diff)
		{
			fprintf(stderr,
				"ReadEntry: short read of buffer\n");
			fflush(stderr);
			return(0);
		}
		
		*ts = TS(cookie->data,cookie->curr);
		b1 = cookie->dbuf1;
		b2 = cookie->dbuf2;
		for(i=0; i < cookie->diff+1; i++)
		{
			temp_i = 
			MODMINUS(cookie->curr,i,cookie->last+1);
			/*
			 * only for cirsular case do we apply the bias
			 * when the data comes out of the data structure
			 */
			b1[i] = VAL(cookie->data,temp_i) - cookie->bias;
		}
		for(d=cookie->diff; d > 0; d--)
		{
			for(i=0; i < d; i++)
			{
				b2[i] = b1[i] - b1[i+1];
			}
			temp_b = b1;
			b1 = b2;
			b2 = temp_b;
		}

		*value = b1[0];
		cookie->curr = MODPLUS(cookie->curr,1,cookie->last+1);
		
		return(1);
	}

	/*
	 * FILE version begins here
	 */
		
	/*
	 * if head != tail, then data set contains unread data
	 */
	if(cookie->head != cookie->tail)
	{
		*ts = TS(cookie->data,cookie->head);

		b1 = cookie->dbuf1;
		b2 = cookie->dbuf2;
		for(i=0; i < cookie->diff+1; i++)
		{
			temp_i = cookie->head - i;
			b1[i] = VAL(cookie->data,temp_i);
		}
		for(d=cookie->diff; d > 0; d--)
		{
			for(i=0; i < d; i++)
			{
				b2[i] = b1[i] - b1[i+1];
			}
			temp_b = b1;
			b1 = b2;
			b2 = temp_b;
		}

		*value = b1[0];
		cookie->head = cookie->head + 1;
		return(1);
	}
	

	if(feof(cookie->fd))
		return(0);
	
	/*
	 * otherwise, just reset head and tail to beginning of buffer and 
	 * read some more
	 */
	cookie->head = 0;

	/*
	 * if we have wrapped, we need the last diff values from the end of
	 * this buffer to go up front in the next
	 */ 
	if(cookie->wrap == 1)
	{
		for(i=cookie->diff; i > 0; i--)
		{
			TS(cookie->data,cookie->head) =
				TS(cookie->data,cookie->tail - i);
			VAL(cookie->data,cookie->head) =
				VAL(cookie->data,cookie->tail - i);
			cookie->head = cookie->head + 1;
		}
		/*
		 * here, there is no valid data so head and
		 * tail must point to the same location
		 */
		cookie->head = cookie->diff;
		cookie->tail = cookie->head;
	}

	while(cookie->tail < cookie->data_size)
	{
		err = fgets(line_buff,254,cookie->fd);

		if(err == NULL)
		{
			/*
			 * if there is valid data, return it
			 */
			if(cookie->head != cookie->tail)
			{
				*ts = TS(cookie->data,cookie->head);
				b1 = cookie->dbuf1;
				b2 = cookie->dbuf2;
				for(i=0; i < cookie->diff+1; i++)
				{
					temp_i = cookie->head - i; 
					b1[i] = VAL(cookie->data,temp_i);
				}
				for(d=cookie->diff; d > 0; d--)
				{
					for(i=0; i < d; i++)
					{
						b2[i] = b1[i] - b1[i+1];
					}
					temp_b = b1;
					b1 = b2;
					b2 = temp_b;
				}

				*value = b1[0];
				cookie->head = cookie->head + 1;
				/*
				 * if we are bailing out on EOF before
				 * reading a complete BUFSIZE, then remember
				 * the index of the last data value
				 */
				if(cookie->wrap == 0)
				{
					cookie->last = cookie->tail;
				}
				return(1);
			}
			else
			{
				return(0);
			}
		}

		fcount = GetNextValue(line_buff, &lts, &lvalue);
		if(fcount == 0)
		{
			continue;
		}
		else if(fcount == 1)
		{
			lvalue = lts;
			lts = Seq_no;
			Seq_no++;
		}

		TS(cookie->data,cookie->tail) = lts;

		if(cookie->logflag == 1)
		{
			temp_d = lvalue;
			if(temp_d == 0.0)
			{
				fprintf(stderr,
		"ReadEntry: 0 value on log input, ts: %10.0f, skipping\n",
				TS(cookie->data,cookie->tail));
				fflush(stderr);
				continue;
			}
			VAL(cookie->data,cookie->tail) = 
				log(temp_d) - cookie->bias;
		}
		else
		{
			VAL(cookie->data,cookie->tail) = 
				lvalue - cookie->bias;
		}
		
		cookie->tail = cookie->tail + 1;

		/*
		 * bump the head up until there is enough to do the
		 * diff thing
		 */
		if(cookie->head < cookie->diff)
		{
			cookie->head = cookie->head + 1;
		}
		
	}
	
	cookie->wrap = 1;
	if(cookie->head != cookie->tail)
	{
		*ts = TS(cookie->data,cookie->head);
		b1 = cookie->dbuf1;
		b2 = cookie->dbuf2;
		for(i=0; i < cookie->diff+1; i++)
		{
			temp_i =  cookie->head - i;
			b1[i] = VAL(cookie->data,temp_i);
		}
		for(d=cookie->diff; d > 0; d--)
		{
			for(i=0; i < d; i++)
			{
				b2[i] = b1[i] - b1[i+1];
			}
			temp_b = b1;
			b1 = b2;
			b2 = temp_b;
		}

		*value = b1[0];
		cookie->head = cookie->head + 1;
		return(1);
	}
	else /* paranoid */
	{
		cookie->head = cookie->tail = 0;
		return(0);
	}

}

int
SizeOf(char *icookie)
{
	struct data_set *cookie;
	FILE *fd;
	int count;
	char line_buff[255];
	char *err;
	int temp_i;
	
	cookie = (struct data_set *)icookie;
	
	if(cookie->circular == 1)
	{
		temp_i = MODMINUS(cookie->tail,cookie->head,cookie->last+1);
		return(temp_i - cookie->diff);
	}
	
	if(cookie->size >= 0)
	{
		return(cookie->size);
	}
	
	fd = fopen(cookie->name,"r");
	if(fd == NULL)
	{
		fprintf(stderr,"SizeOf: couldn't open %s\n", cookie->name);
		fflush(stderr);
		cookie->size = -1;
		return(0);
	}
	
	count = 0;
	while(!feof(fd))
	{
		err = fgets(line_buff,sizeof(line_buff),fd);
		if(err == NULL)
		{
			break;
		}
		if(line_buff[0] == '\n')
			break;
		count++;
	}
	
	fclose(fd);
	cookie->size = count - cookie->diff;
	return(cookie->size);
}

int
CopyDataSet(char *src, char *dst, int elements)
{
	struct data_set *s_ds;
	struct data_set *d_ds;
	int ierr;
	double ts;
	double value;
	int count;
	
	s_ds = (struct data_set *)src;
	d_ds = (struct data_set *)dst;
	
	count = 0;
	while((count < elements) && (ReadEntry((char *)s_ds,&ts,&value) != 0))
	{
		ierr = WriteEntry((char *)d_ds,ts,value);
		if(ierr == 0)
		{
			fprintf(stderr,
			 "CopyDataSet: couldn't write %d element\n",
			      count+1);
			fflush(stderr);
			return(0);
		}
		count++;
	}
	
	return(1);
}

int
Seek(char *icookie, int element)
{
	struct data_set *ds;
	int ierr;
	int i;
	double ts;
	double value;
	
	ds = (struct data_set *)icookie;
	
	ierr = Rewind((char *)ds);
	
	if(ierr == 0)
	{
		fprintf(stderr,"Seek: couldn't rewind data set\n");
		fflush(stderr);
		return(0);
	}
	
	for(i=0; i < element; i++)
	{
		ierr = ReadEntry((char *)ds,&ts,&value);
		if(ierr == 0)
		{
			fprintf(stderr,"Seek: read failed\n");
			fflush(stderr);
			return(0);
		}
	}

	return(1);
}

#ifdef TEST

extern char *optarg;

char *PRED_ARGS = "f:";


int
main(argc,argv)
int argc;
char *argv[];
{

	double val;
	double ts;
	char *data_set;
	int size;
	int c;
	int curr;
	int ierr;
	char fname[255];

	if(argc < 2)
	{
		fprintf(stderr,"usage: testinput -f filename\n");
		fflush(stderr);
		exit(1);
	}

	fname[0] = 0;

	while((c = getopt(argc,argv,PRED_ARGS)) != EOF)
	{
		switch(c)
		{
			case 'f':
				strcpy(fname,optarg);
				break;
			default:
				fprintf(stderr,"unrecognized argument %c\n",
						c);
				fflush(stderr);
				break;
		}
	}

	if(fname[0] == 0)
	{
		fprintf(stderr,"usage: testinput -f fname\n");
		fflush(stderr);
		exit(1);
	}
	
	ierr = InitData(fname,2,0,0,&data_set);
	
	if(ierr == 0)
	{
		fprintf(stderr,"testinput error: InitData failed\n");
		exit(1);
	}
	
	
	/*
	 * read half a buffer's worth
	 */
	curr = 0;
	while(curr < (BUFSIZE / 2))
	{
		ierr = ReadEntry(data_set,&ts,&val);
		if(ierr == 0)
		{
			size = SizeOf(data_set);
			if(size == 0)
			{
				fprintf(stderr,
		"testinput error: SizeOf failed at initial read %d\n",curr);
				fflush(stderr);
				exit(1);
			}
			/*
			 * if we reached EOF before 1/2 a BUFSIZE, and
			 * everything is cool, bail out
			 */
			if(size != curr)
			{
				fprintf(stderr,
				"testinput error: %d initial read failed\n",
					curr);
				fflush(stderr);
				exit(1);
			}
			else
			{
				break;
			}
		}
		curr++;
	}
	

	/*
	 * get size
	 */
	size = SizeOf(data_set);
	
	if(size == 0)
	{
		fprintf(stderr,"testinput error: SizeOf failed\n");
		fflush(stderr);
		exit(1);
	}
	
	/*
	 * if we have reached the end of the file, Rewind it here
	 */
	if(curr == size)
	{
		ierr = Rewind(data_set);
		if(ierr == 0)
		{
			fprintf(stderr,
	"testinput error: couldn't rewind after reading initial EOF\n");
			fflush(stderr);
			exit(1);
		}
		curr = 0;
	}

	/*
	 * read another half a buffer's worth
	 */
	while(curr < BUFSIZE)
	{
		ierr = ReadEntry(data_set,&ts,&val);
		if(ierr == 0)
		{
			if(curr != size)
			{
				fprintf(stderr,
				"testinput error: %d,%d second read failed\n",
				curr, size);
				fflush(stderr);
				exit(1);
			}
			else
			{
				break;
			}
		}
		curr++;
	}
	
	/*
	 * rewind and read half the data set
	 */
	ierr = Rewind(data_set);
	if(ierr == 0)
	{
		fprintf(stderr,"testinput error: couldn't rewind\n");
		fflush(stderr);
		exit(1);
	}
	curr = 0;
	while(curr < (size / 2))
	{
		ierr = ReadEntry(data_set,&ts,&val);
		if(ierr == 0)
		{
			fprintf(stderr,
			"testinput error: %d third read failed\n",
			curr);
			fflush(stderr);
			exit(1);
		}
		curr++;
	}
	
	
	/*
	 * rewind and print out entire contents
	 */
	ierr = Rewind(data_set);
	if(ierr == 0)
	{
		fprintf(stderr,"testinput error: couldn't rewind again\n");
		fflush(stderr);
		exit(1);
	}
	while(ReadEntry(data_set,&ts,&val) != 0)
	{
		fprintf(stdout,"%10.0f %3.4f\n",ts,val);
	}
	
	fflush(stdout);
	
	RemoveData(data_set);
	
	exit(0);
}

char *CopyData(char *cookie)
{
	struct data_set *ids;	/* input ds */
	struct data_set *ods;	/* output ds */

	ids = (struct data_set *)cookie;

	ods = (struct data_set *)malloc(sizeof(struct data_set));

	if(ods == NULL)
	{
		fprintf(stderr,"CopyData: no space\n");
		fflush(stderr);
		return(NULL);
	}

	/*
	 * try to reopen file so that copy is completely
	 * separate
	 */
	ods->fd = fopen(fname,"r");
	if(ods->fd == NULL)
	{
		fprintf(stderr,"CopyData: could open %s for reading\n",
		                ods->fname);
		fflush(stderr);
		free(ods)
		return(0);
	}

	memcpy((void *)ods,(void *)ids,sizeof(struct data_set));

	ods->data = (double *)malloc(DELEMENTS*BUFSIZE*sizeof(double));
	if(ods->data == NULL)
	{
		fprintf(stderr,"CopyData: no space for buffer\n");
		fflush(stderr);
		fclose(ods->fd);
		free(ods);
		return(NULL);
	}

	memcpy((void *)ods->data,(void *)ids->data,
			DELEMENTS*BUFSIZE*sizeof(double));

	ods->dbuf1 = (double *)malloc((ids->diff+1)*sizeof(double));
	if(ods->dbuf1 == NULL)
	{
		fprintf(stderr,
			"CopyData: couldn't malloc space for dbuf1\n");
		fflush(stderr);
		free(ods->data);
		fclose(ods->fd);
		free(ods);
		return(0);
	}

	memcpy((void *)ods->dbuf1,(void *)ids->dbuf1,
			(ids->diff+1)*sizeof(double));

	ods->dbuf2 = (double *)malloc((ids->diff+1)*sizeof(double));
	if(ods->dbuf2 == NULL)
	{
		fprintf(stderr,
			"CopyData: couldn't malloc space for dbuf2\n");
		fflush(stderr);
		free(ods->data);
		free(ods->dbuf1);
		fclose(ods->fd);
		free(ods);
		return(0);
	}
	memcpy((void *)ods->dbuf2,(void *)ids->dbuf2,
			(ids->diff+1)*sizeof(double));

	return((char *)ods);
}


#endif
