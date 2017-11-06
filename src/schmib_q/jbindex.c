/*
 * http://www.swox.com/gmp/manual
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "jval.h"
#include "jrb.h"
#include "input.h"

#include "norm.h"

char Cache_name[255];

#define RAND() (drand48())
#define SEED() (srand48())


int npk(mpf_t r, mpf_t n, mpf_t k);
int nck(mpf_t r, mpf_t n, mpf_t k);

#define JBCACHESIZE (5000000)

double JB_cache[JBCACHESIZE][5];

int JBApprox(int n, double q, double c)
{
	double coeff;
	double approx;
	int foo;

	coeff = InvNormal(1.0 - c,0,1.0);

	approx = q * (double)n + coeff * sqrt((double)n*q*(1.0-q));
	approx += 0.5;
	foo = (int)approx;
	return(foo);
}


void InitJBCache(char *fname, double q, double c)
{
	FILE *fd;
	int i;
	char line_buff[255];
	int index;
	int size;
	int err;
	int line_no = 0;

	fd = fopen(fname,"r");
	if(fd == NULL)
	{
		fprintf(stderr,
		"cannot open cache file %s\n",
			fname);
		fflush(stderr);
		return;
	}

	memset(line_buff,0,sizeof(line_buff));
	while(fgets(line_buff,sizeof(line_buff),fd) != NULL)
	{
		line_no++;
		err = sscanf(line_buff,"%d %d",&size,&index);
		if(err < 2)
		{
			fprintf(stderr,"InitJBFile: sccanf failed on line %d\n",
				line_no);
			fprintf(stderr,"line: %s\n",line_buff);
			fflush(stderr);
			continue;
		}
		if(size >= JBCACHESIZE)
			break;

		i = size % JBCACHESIZE;

		JB_cache[i][0] = q;
		JB_cache[i][1] = c;
		JB_cache[i][2] = (double)size;
		JB_cache[i][3] = (double)index;
		JB_cache[i][4] = 1.0;	/* ratio set to 1 */
	}

	fclose(fd);

	return;
}



/*
 * the cache uses the last ratio even though it would be different
 * since the data has changed
 */
int FindJBIndex(double q, double c, int size, int *index, double *ratio)
{
	int i;

	i = size % JBCACHESIZE;

	if((JB_cache[i][0] == q) &&
	   (JB_cache[i][1] == c) &&
	   (JB_cache[i][2] == (double)size))
	{
		*index = (int)JB_cache[i][3];
		*ratio = JB_cache[i][4];
		return(1);
	}

	return(0);
} 

void InsertJBIndex(double q, double c, int size, int index, double ratio)
{
	int i;

	i = size % JBCACHESIZE;

	JB_cache[i][0] = q;
	JB_cache[i][1] = c;
	JB_cache[i][2] = (double)size;
	JB_cache[i][3] = (double)index;
	JB_cache[i][4] = ratio;

	return;
}

int JBIndex(int size, double quantile, double confidence, double *ratio) 
{
  
  mpf_t q, c, sum, last_sum, n, j;
  mpf_t t1, t2, t3, t4, t5, t6, t7;

  double foo;
  unsigned long idx;
  int index;
  double value_diff;
  double frac_diff;

  if(FindJBIndex(quantile,confidence,size,&index,ratio))
  {
	  return(index);
  }

  if(size > 500)
  {
    idx = JBApprox(size,quantile,confidence);
    *ratio = 0.0;
    return(idx);
  }

  if(size > 200)
  {
  }

  foo = 1.0 - confidence;

  mpf_init_set_ui(n, size);
  mpf_init_set_d(q, quantile);
  mpf_init_set_d(c, foo);
  mpf_init_set_ui(sum, 0);
  mpf_init_set_ui(last_sum, 0);

  mpf_init(t1);
  mpf_init(t2);
  mpf_init(t3);
  mpf_init(t4);
  mpf_init(t5);
  mpf_init(t6);
  mpf_init(t7);

  /*
  printf("N: ");
  mpf_out_str(stdout, 10, 10, n);
  printf(" ");

  printf("Q: ");
  mpf_out_str(stdout, 10, 10, q);
  printf(" ");

  printf("1.0 - C: ");
  mpf_out_str(stdout, 10, 10, c);
  printf(" ");
   */

  for (mpf_init_set_ui(j, 0);  mpf_cmp(n, j); mpf_add_ui(j, j, 1)) {

    nck(t1, n, j);
    mpf_ui_sub(t2, 1, q);
    mpf_sub(t3, n, j);
    npk(t4, t2, t3);
    npk(t5, q, j);

    mpf_mul(t2, t1, t4);
    mpf_mul(t2, t2, t5);
    mpf_add(sum, sum, t2);

    if (mpf_cmp(sum, c) > 0) {
      break;
    }
    mpf_set(last_sum,sum);
        
  }

  /*
   * distance between two values straddling c
   */
  mpf_sub(t6,sum,last_sum);
  value_diff = mpf_get_d(t6);

  /*
   * distance from last value before c and c
   */
  mpf_sub(t7,c,last_sum);
  frac_diff = mpf_get_d(t7);

  /*
   * percentage of the distance between last value and where we want to be
   */
  if(value_diff > 0)
  	*ratio = frac_diff / value_diff; 
  else
	*ratio = 1.0;


  idx = mpf_get_ui(j);
  /*
  printf("idx=%ld;\n", idx);
   */

  mpf_clear(t1);
  mpf_clear(t2);
  mpf_clear(t3);
  mpf_clear(t4);
  mpf_clear(t5);
  mpf_clear(t6);
  mpf_clear(t7);
  mpf_clear(c);
  mpf_clear(q);
  mpf_clear(n);
  mpf_clear(sum);
  mpf_clear(last_sum);
  mpf_clear(j);

  InsertJBIndex(quantile,confidence,size,(int)idx,*ratio);
  return(idx);
}

int MinHistory(double q, double c)
{
	long double i;
	long double v;

	for(i=1.0; i < 10000.0; i++)
	{
		v = powl(q,i);
		if(v <= c)
		{
			return((int)i);
		}
	}

	return(10000);
} 



