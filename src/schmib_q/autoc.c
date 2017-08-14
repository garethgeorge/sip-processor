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

#include "simple_input.h"

#define DEFAULT_LAGS 10

extern char *optarg;
int Lags;
int Diff;
int Model_pts;
int Period;


int
MeanVar(void *data_set,double *mean, double *var)
{
	double count;
	double temp_m;
	double temp_v;
	double ts;
	double val;
	int ierr;
	
	count = 0;
	
	Rewind(data_set);
	
	temp_m = 0.0;
	while(ReadEntry(data_set,&ts,&val) != 0)
	{
		temp_m += val;
		count += 1.0;
	}
	
	if(count == 0.0)
	{
		fprintf(stderr,"MeanVar: no entries found\n");
		fflush(stderr);
		return(0);
	}
	
	*mean = temp_m / count;
	
	Rewind(data_set);
	
	temp_v = 0.0;
	while(ReadEntry(data_set,&ts,&val) != 0)
	{
		temp_v += (val - *mean) * (val - *mean);
	}
	
	*var = temp_v / (count - 1.0);
	
	return(1);
}

/*
 * follows eqn at top of page 78 in Granger Newbold
 */
int
Rval(void *data_set, double mu, double var, int tau, double *rval)
{
	double *window;
	int ierr;
	int i;
	double ts;
	int index;
	double temp_r;
	double temp_v;
	double xval;
	int local_N;

	/*
	 * will hold last tau+1 value to x_(t - tau) can be accessed
	 */
	window = (double *)malloc(sizeof(double)*(tau+1));
	if(window == NULL)
	{
		fprintf(stderr,"Rval: couldn't malloc space for %d window\n",
				tau);
		fflush(stderr);
		return(0);
	}
	
	Rewind(data_set);
	/*
	 * get first tau values
	 */
	for(i=0; i < tau; i++)
	{
		ierr = ReadEntry(data_set,&ts,&(window[i]));
		if(ierr == 0)
		{
			fprintf(stderr,
			"Rval: couldn't get %d values initially\n", tau);
			free(window);
			return(0);
		}
	}	
	
	local_N = SizeOf(data_set);
	
	/*
	 * calculate the numerator as SUM[(x_t - mu)*(x_(t - tau) - mu)]
	 * for t=(tau..N)
	 */
	temp_r = 0.0;
	index = 0;
	for(i=tau; i < local_N; i++)
	{
		ierr = ReadEntry(data_set,&ts,&xval);
		if(ierr == 0)
		{
			fprintf(stderr,
		"Rval: ran out of data early, %d entries, %d expected\n",
			i,local_N);
			fflush(stderr);
			free(window);
			return(0);
		}
		if(tau > 0)
		{
			temp_r += ((xval - mu) * (window[index] - mu));
			window[index] = xval;
			index = (index + 1) % tau;
		}
		else
		{
			temp_r += ((xval - mu) * (xval - mu));
		}

	}
	
	/*
	 * calculate denominator as SUM[ (x_t - mu) ^ 2 ] from sample
	 * variance
	 */
	temp_v = var * (local_N - 1);
	
	*rval = temp_r / temp_v;
	
	free(window);
	return(1);
}

int
AutoCor(void *data_set,double mu, double var, int lags, double *rvals)
{
	int i;
	int err;
	
	for(i=0; i < lags; i++)
	{
		err = Rval(data_set,mu,var,i,&(rvals[i]));
		if(err == 0)
		{
			fprintf(stderr,"AutoCor: Rval failed at %d\n",
					i);
			fflush(stderr);
			return(0);
		}
	}
	
	return(1);
}
	


/*
 * follows page 82 of Granger-Newbold
 */
int
ParCor(void *data_set, int K, double *avals)
{
	int err;
	int i;
	int j;
	int k;
	double mu;
	double var;
	double temp_d1;
	double temp_d2;
	double *rvals;
	
	rvals = (double *)malloc((K+1)*sizeof(double));
	
	if(rvals == NULL)
	{
		fprintf(stderr,
			"ParCor: couldn't malloc space for r vals\n");
		fflush(stderr);
		return(0);
	}
	
	Rewind(data_set);

	
	/*
	 * first, calculate the first K autocorrelations (r values)
	 */
	err = MeanVar(data_set,&mu,&var);
	if(err == 0)
	{
		fprintf(stderr,
		"ParCor: couldn't get mean and variance\n");
		fflush(stderr);
		free(rvals);
		return(0);
	}
	for(i=1; i <= K; i++)
	{
		err = Rval(data_set,
			   mu,
			   var,
			   i,
			   &(rvals[i]));
		if(err == 0)
		{
			fprintf(stderr,
			"ParCor: couldn't get %dth r value\n",
			i);
			fflush(stderr);
			free(rvals);
			return(0);
		}
	}
	
	/*
	 * zeroth element not used --> a[1,1] = r[1]
	 */
	avals[1*(K+1)+1] = rvals[1];
	/*
	 * calc a[2,2] through a[K,K]
	 */
	for(k = 2; k <= K; k++)
	{
		temp_d2 = 0;
		for(j=1; j <= k-1; j++)
		{
			temp_d2 = temp_d2 +
				avals[(k-1)*(K+1)+j] * rvals[k - j];
		}
		temp_d2 = rvals[k] - temp_d2;
		
		temp_d1 = 0;
		for(j=1; j <= k-1; j++)
		{
			temp_d1 = temp_d1 +
				avals[(k-1)*(K+1)+j] * rvals[j];
		}
		temp_d1 = 1.0 - temp_d1;
		
		avals[k*(K+1)+k] = temp_d2 / temp_d1;
		
		for(j=1; j <= k-1; j++)
		{
			avals[k*(K+1)+j] = avals[(k-1)*(K+1)+j] -
					avals[k*(K+1)+k] * 
					    avals[(k-1)*(K+1)+(k-j)];
		}
	}
	
	free(rvals);
	return(1);
}

int
ErrVar(void *data_set, int K, double *sigmas)
{
	int t;
	int n;
	int k;
	int err;
	double r1;
	double mu;
	double var;
	double temp_d;
	double val;
	double ts;
	double *parcor;
	double a;
	
	parcor = (double *)malloc((K+1)*(K+1)*sizeof(double));
	if(parcor == NULL)
	{
		fprintf(stderr,"ErrVar: couldn't malloc space for parcor\n");
		fflush(stderr);
		return(0);
	}
	
	err = MeanVar(data_set, &mu, &var);
	
	if(err == 0)
	{
		fprintf(stderr,"ErrVar: couldn't get mean and variance\n");
		fflush(stderr);
		free(parcor);
		return(0);
	}
	
	err = Rval(data_set,mu,var,1,&r1);
	
	if(err == 0)
	{
		fprintf(stderr,"ErrVar: couldn't get r[1]\n");
		fflush(stderr);
		free(parcor);
		return(0);
	}
	
	n = SizeOf(data_set);
	
	Rewind(data_set);
	temp_d = 0.0;
	for(t=0; t < n; t++)
	{
		err = ReadEntry(data_set,&ts,&val);
		if(err == 0)
		{
			fprintf(stderr,"ErrVar: couldn't read entry %d\n",
					t+1);
			fflush(stderr);
			free(parcor);
			return(0);
		}
		
		temp_d = temp_d + val * val / (double)n;
		
	}
	
	err = ParCor(data_set,K,parcor);
	if(err == 0)
	{
		fprintf(stderr,"ErrVar: couldn't get parcors\n");
		fflush(stderr);
		free(parcor);
		return(0);
	}

	sigmas[1] = (1.0 - r1*r1) * temp_d;
	
	for(k=2; k <= K; k++)
	{
		a = parcor[k*(K+1)+k];
		sigmas[k] = (1.0 - a*a) * sigmas[k-1];
	}	
	
	free(parcor);
	
	return(1);
}

int
AIC(void *data_set, int K, double *aic)
{
	int err;
	double *err_var;
	int n;
	
	err_var = (double *)malloc((K+1)*sizeof(double));
	if(err_var == NULL)
	{
		fprintf(stderr,"AIC: couldn't malloc space for sigmas\n");
		fflush(stderr);
		return(0);
	}
	
	err = ErrVar(data_set,K,err_var);
	if(err == 0)
	{
		fprintf(stderr,"AIC: ErrVar failed\n");
		fflush(stderr);
		free(err_var);
		return(0);
	}

	n = SizeOf(data_set);
	*aic = log(err_var[K]) + (2.0*(double)K/(double)n);
	
	free(err_var);
	
	return(1);
}



	
#ifdef AUTOC

char *PRED_ARGS = "d:l:f:m:P:";

int
main(argc,argv)
int argc;
char *argv[];
{

	char fname[255];
	int c;
	int ierr;
	double mu;
	double sigma;
	char *data_set;
	void *input_data;
	double rval;
	int tau;
	double *avals;
	double bias;

	if(argc < 2)
	{
		fprintf(stderr,"usage: autoc -f filename [-l lags]\n");
		fflush(stderr);
		exit(1);
	}

	fname[0] = 0;

	Lags = DEFAULT_LAGS;
	Diff = 0;
	Model_pts = 0;
	Period = 0;
	while((c = getopt(argc,argv,PRED_ARGS)) != EOF)
	{
		switch(c)
		{
			case 'f':
				strcpy(fname,optarg);
				break;
			case 'l':
				Lags = atoi(optarg);
				break;
			case 'd':
				Diff = atoi(optarg);
				break;
			case 'm':
				Model_pts = atoi(optarg);
				break;
			case 'P':
				Period = atoi(optarg);
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
		fprintf(stderr,"usage: autoc -f fname [-l lags]\n");
		fflush(stderr);
		exit(1);
	}
	
	if(Model_pts != 0)
	{
		ierr = InitDataSet(&input_data,2);
		if(ierr == 0)
		{
		fprintf(stderr,
		"autoc: couldn't init data for file %s with pts %d\n",
					fname,Model_pts);
			fflush(stderr);
			exit(1);
		}
		ierr = LoadDataSet(fname,input_data);
		if(ierr == 0)
		{
		fprintf(stderr,
		"autoc: couldn't load data for file %s with pts %d\n",
					fname,Model_pts);
			fflush(stderr);
			exit(1);
		}

		ierr = InitDataSet(&data_set,2);
		if(ierr == 0)
		{
		fprintf(stderr,
		"autoc: couldn't init model data for file %s with pts %d\n",
					fname,Model_pts);
			fflush(stderr);
			exit(1);
		}
		ierr = CopyDataSet(input_data,data_set,Model_pts);
		if(ierr == 0)
		{
			fprintf(stderr,
				"autoc: couldn't copy data\n");
			fflush(stderr);
			exit(1);
		}

	}
	else
	{
		ierr = InitData(&data_set,0);
		if(ierr == 0)
		{
			fprintf(stderr,"autoc: couldn't init data for file %s\n",
					fname);
			fflush(stderr);
			exit(1);
		}
		ierr = LoadDataSet(fname,data_set);
		if(ierr == 0)
		{
			fprintf(stderr,"autoc: couldn't load data for file %s\n",
					fname);
			fflush(stderr);
			exit(1);
		}
	}

	ierr = MeanVar(data_set,&mu,&sigma);
	
	if(ierr == 0)
	{
		fprintf(stderr,
		"autoc: couldn't calc mean and variance from file %s\n",
		fname);
		fflush(stderr);
	}
	
/*
	fprintf(stdout,
	"diff: %d, per: %d, mu: %f, sigma: %f, significance level: %f\n",Diff,
		Period,mu,sqrt(sigma),2.0/sqrt((double)SizeOf(data_set)));
	fflush(stdout);
*/
	
	for(tau=0; tau <= Lags; tau++)
	{
		ierr = Rval(data_set,mu,sigma,tau,&rval);
		if(ierr == 0)
		{
			fprintf(stderr,"autoc: rval failed for tau: %d\n",
					tau);
			fflush(stderr);
			FreeDataSet(data_set);
			exit(1);
		}
		fprintf(stdout,"%3.4f\n",rval);
		fflush(stdout);
	}
	
	fprintf(stdout,"\n");
	
	FreeDataSet(data_set);
	exit(0);
	
}

#endif
