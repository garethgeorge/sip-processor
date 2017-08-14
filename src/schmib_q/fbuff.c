#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "fbuff.h"


#ifdef DEBUG

static int Msize = 0;

static void *Malloc(int size)
{
	Msize += size;
	return(malloc(size));
}

static void Free(void *p, int size)
{
	Msize -= size;
	return(free(p));
}

#else

#define Malloc malloc
#define Free(p,s) free(p)

#endif

fbuff
InitFBuff(int size)
{
	fbuff lb;

	lb = (fbuff)Malloc(FBUFF_SIZE);

	if(lb == NULL)
	{
		fprintf(stderr,"InitFBuff: couldn't get space for fbuff\n");
		fflush(stderr);
		return(NULL);
	}

	/*
	 * count from bottom to top
	 *
	 * head is the next available space
	 * tail is the last valid data item
	 */
	lb->size = size;
	lb->head = size - 1;
	lb->tail = lb->head;
	lb->empty = 1;

	lb->vals = (double *)Malloc((size+1)*sizeof(double));
	if(lb->vals == NULL)
	{
		fprintf(stderr,"InitFBuff: couldn't get space for vals\n");
		fflush(stderr);
		Free(lb,sizeof(*lb));
		return(NULL);
	}

#ifdef DEBUG

	printf("Initfbuff: %d bytes allocated\n",Msize);

#endif

	return(lb);

}

void
FreeFBuff(fbuff fb)
{
	Free(fb->vals,fb->size);
	Free(fb,sizeof(*fb));
	
	return;
}

void
UpdateFBuff(fbuff fb, double val)
{
	F_HEAD(fb) = val;
	fb->head = MODMINUS(fb->head,1,(fb->size));
	
	/*
	 * if we have moved the head over the tail, bump the tail around too
	 */
	if(!fb->empty && (fb->head == fb->tail))
	{
		fb->tail = MODMINUS(fb->tail,1,(fb->size));
	}

	fb->empty = 0;
	
	return;
}

void DeleteFBuffTail(fbuff fb)
{
	if(fb->empty)
	{
		return;
	}

	fb->tail = MODMINUS(fb->tail,1,(fb->size));

	if(fb->tail == fb->head)
	{
		fb->empty = 1;
	}

	return;
}

int ValidCount(fbuff fb)
{
	int count;

	if(fb->empty == 1)
	{
		return(0);
	}

	count = MODMINUS(fb->tail,fb->head,fb->size);

	return(count);
}
