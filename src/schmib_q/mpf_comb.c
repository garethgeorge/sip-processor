/*
 * http://www.swox.com/gmp/manual
 */
#define PERF
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef CONDOR
#include <gmp.h>
#endif
#include <time.h>
#ifdef PERF
#include <sys/time.h>
#endif
#include <math.h>


#ifdef PERF
static double TotalTime;
static double TotalCount;
#endif

#ifndef CONDOR
#include "mpf_comb.h"

#define FACCACHESIZE (200000)

mpf_t FacCache[FACCACHESIZE];
int FacCacheValid[FACCACHESIZE];
static int Fac_initd = 0;

void InitFacCache()
{
	int i;

	if(Fac_initd == 1)
		return;

	for(i=0; i < FACCACHESIZE; i++)
	{
		
		mpf_init(FacCache[i]);
		FacCacheValid[i] = 0;
	}

	/*
	 * 0! is such a great number
	 */
	mpf_set_ui(FacCache[0],1);
	FacCacheValid[0] = 1;
	mpf_set_ui(FacCache[1],1);
	FacCacheValid[1] = 1;

	Fac_initd = 1;

	return;
}

int FindFacCache(int f, mpf_t v)
{
	if(f > FACCACHESIZE)
		return(0);

	if(FacCacheValid[f] == 0)
		return(0);

	InitFacCache();		/* in case it is the first time */

	/*
	 * make a copy so that we don't have problems with mpf_clear
	 */
	mpf_set(v,FacCache[f]);

	return(1);
}

void InsertFacCache(int f, mpf_t v)
{
	if(f > FACCACHESIZE)
		return;

	if(FacCacheValid[f] == 1)
		return;

	InitFacCache();		/* in case first time */

	mpf_set(FacCache[f],v);
	FacCacheValid[f] = 1;

	return;
}



int npk(mpf_t r, mpf_t n, mpf_t k) {
  mpf_t i;
  int j;
  int k_i;

  /*
  mpf_init_set_ui(i, 0);
   */
  k_i = mpf_get_ui(k);

  mpf_set_ui(r, 1);

/*
  for(j=0; mpf_cmp(k, i); mpf_add_ui(i, i, 1)) {
*/
  for(j=0; j < k_i; j++) {
    
    mpf_mul(r, r, n);
    
  }


  /*
  mpf_clear(i);
   */
  return(0);

}

void MPFac(mpf_t r, mpf_t n)
{
	int i;
	int n_i;
	int found;
	mpf_t i_f;
	mpf_t curr_f;
	int found_index;

	n_i = mpf_get_ui(n);
	mpf_set_ui(r,1);
	if(n_i == 0)
		return;
		

	mpf_init(i_f);
	mpf_init(curr_f);
	mpf_set(curr_f,n);

	found_index = 0;
	for(i=n_i; i > 0; i--)
	{
		/*
		 * if we find it, then since we counting down,
		 * r is finished
		 */
		found = FindFacCache(i,i_f);
		if(found == 1)
		{
			mpf_mul(r,r,i_f);
			found_index = i;
			break;
		}
		mpf_mul(r,r,curr_f);
		mpf_sub_ui(curr_f,curr_f,1);
	}

	/*
	 * here, we have computed n! insert it in the cache
	 *
	 * if we haven't just found it, insert all the information we can
	 * in the cache: n!, (n-1)!, (n-2)!, etc.
	 *
	 */
	if(n_i != found_index)
	{
		mpf_set(curr_f,r);
		for(i=n_i; i > found_index; i--)
		{
			InsertFacCache(i,curr_f);
			mpf_set_ui(i_f,i);
			mpf_div(curr_f,curr_f,i_f);
		}
	}

	mpf_clear(i_f);
	mpf_clear(curr_f);

	return;
}

int nck(mpf_t r, mpf_t n, mpf_t k)
{
	mpf_t nf;
	mpf_t kf;
	mpf_t nmk;
	mpf_t nmkf;
	mpf_t tmp;

	mpf_init(nf);
	mpf_init(kf);
	mpf_init(nmk);
	mpf_init(nmkf);
	mpf_init(tmp);

	MPFac(nf,n);
	MPFac(kf,k);

	mpf_sub(nmk,n,k);
	MPFac(nmkf,nmk);

	mpf_div(tmp,nf,kf);
	mpf_div(r,tmp,nmkf);

	mpf_clear(nf);
	mpf_clear(kf);
	mpf_clear(nmk);
	mpf_clear(nmkf);
	mpf_clear(tmp);

	return(0);
}
	

int nckold(mpf_t r, mpf_t n, mpf_t k) {
  mpf_t i, nf, rf, tmp, tmpa;

  mpf_init_set_ui(nf, 1);
  mpf_init_set_ui(rf, 1);
  mpf_init(tmp);
  mpf_init(tmpa);
  mpf_init_set_ui(i, 0);

  while( mpf_cmp(k, i)) {

    mpf_sub(tmpa, n, i);

    mpf_mul(nf, nf, tmpa);

    mpf_add_ui(i, i, 1);

  }

  mpf_add_ui(tmp, k, 1);

  for (mpf_set_ui(i, 1); mpf_cmp(tmp, i); mpf_add_ui(i, i, 1)) {
    mpf_mul(rf, rf, i);
  }

  mpf_div(r, nf, rf);

  mpf_clear(i);
  mpf_clear(nf);
  mpf_clear(rf);
  mpf_clear(tmp);
  mpf_clear(tmpa);
  
  return(0);
}
#endif
