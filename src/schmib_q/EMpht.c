/*   EMpht.c  */
/*Latest corrections made 9 March, 1998  */
#include<stdio.h>
#include<stdlib.h>
#include<math.h>

double *obs, *weight, *censur, *cweight, *lower, *upper, *intweight;
double  SumOfWeights, SumOfCensored, SumOfInt;
int  *partner, NoOfObs, NoOfCensored, NoOfInt; 

double *v_alloc(int n)
{
  double *v;
  v=(double *) calloc(n,sizeof(double));
  if(v==NULL) {
    fprintf(stderr,"could not allocate memory");
    exit(1);
  }
  return v;
}

int *int_v_alloc(int n)
{
  int *v;
  v=(int *) calloc(n,sizeof(int));
  if(v==NULL) {
    fprintf(stderr,"could not allocate memory");
    exit(1);
  }
  return v;
}

double **m_alloc(int n, int k)
{
  int i;
  double **mat;
  mat=(double **) calloc(n,sizeof(double *));
  if(mat == NULL) {
    fprintf(stderr,"could not allocate memory");
    exit(1);
  }
  for(i=0; i<n; i++) {
    mat[i]=(double *) calloc(k,sizeof(double));
    if(mat[i] == NULL) {
      fprintf(stderr,"could not allocate memory");
      exit(1);
    }
  }
  return mat;
}

int **int_m_alloc(int n, int k)
{
  int i, **mat;
  mat=(int **) calloc(n,sizeof(int *));
  if(mat == NULL) {
    fprintf(stderr,"could not allocate memory");
    exit(1);
  }
  for(i=0; i<n; i++) {
    mat[i]=(int *) calloc(k,sizeof(int));
    if(mat[i] == NULL) {
      fprintf(stderr,"could not allocate memory");
      exit(1);
    }
  }
  return mat;
}

double ***m3_alloc(int n, int k, int v)
{
  int i,j;
  double ***mat;
  mat=(double ***) calloc(n,sizeof(double **));
  if(mat == NULL) {
    fprintf(stderr,"could not allocate memory");
    exit(1);
  }
  for(i=0; i<n; i++) {
    mat[i]=(double **) calloc(k,sizeof(double *));
    if(mat[i] == NULL) {
      fprintf(stderr,"could not allocate memory");
      exit(1);
    }
    for(j=0; j<k; j++) {
      mat[i][j]=(double *) calloc(v,sizeof(double));
      if(mat[i][j] == NULL) {
	fprintf(stderr,"could not allocate memory");
	exit(1);
      }
    }
  }
  return mat;
}

void init_vector(double *vector, int NoOfElements)
{
  int i;
  for(i=0; i < NoOfElements; i++)
    vector[i] = 0.0;
}

void init_integervector(int *vector, int dim)
{
  int i;

  for (i=0; i<dim; i++)
    vector[i] = 0;
}


void init_matrix(double **matrix, int dim1, int dim2)
{
  int i, j;

  for (i=0; i<dim1; i++)
    for(j=0; j<dim2; j++)
      matrix[i][j] = 0.0;
}

void init_integermatrix(int **matrix, int dim1, int dim2)
{
  int i, j;

  for (i=0; i<dim1; i++)
    for(j=0; j<dim2; j++)
      matrix[i][j] = 0;
}

void init_3dimmatrix(double ***matrix, int dim1, int dim2, int dim3)
{
  int i, j, k;

  for (i=0; i<dim1; i++) 
    for(j=0; j<dim2; j++) 
      for(k=0; k<dim3; k++)
	matrix[i][j][k] = 0;
}

void free_matrix(double **matrix, int dim1)
{
  int i;

  for (i=0; i<dim1; i++)
    free(matrix[i]); 
  free(matrix);
}

void free_integermatrix(int **matrix, int dim1)
{
  int i;

  for (i=0; i<dim1; i++)
    free(matrix[i]);
  free(matrix);
}

void free_3dimmatrix(double ***matrix, int dim1, int dim2)
{
  int i;

  for (i=0; i<dim1; i++) 
    free_matrix(matrix[i],dim2);
  free(matrix);
}


void a_rungekutta(int p, double *avector, double **ka, double dt, double h, 
		  double **T)
{
  int i,j;
  double eps, h2, sum;

  i=dt/h;
  h2=dt/(i+1);
  init_matrix(ka, 4, p);

  for (eps=0; eps<=dt-h2/2; eps += h2) {
    for (i=0; i < p; i++) {
      sum=0;
      for (j=0; j < p; j++)
        sum += T[j][i]*avector[j];
      ka[0][i] = h2*sum;
    }
    for (i=0; i < p; i++) {
      sum=0;
      for (j=0; j < p; j++)
        sum += T[j][i]*(avector[j]+ka[0][j]/2);
      ka[1][i] = h2*sum;
    }
    for (i=0; i < p; i++) {
      sum=0;
      for (j=0; j < p; j++)
        sum += T[j][i]*(avector[j]+ka[1][j]/2);
      ka[2][i] = h2*sum;
    }
    for (i=0; i < p; i++) {
      sum=0;
      for (j=0; j < p; j++)
        sum += T[j][i]*(avector[j]+ka[2][j]);
      ka[3][i] = h2*sum;
    }

    for (i=0; i < p; i++) 
      avector[i] += (ka[0][i]+2*ka[1][i]+2*ka[2][i]+ka[3][i])/6;
  }
}

void rungekutta(int p, double *avector, double *gvector, double *bvector, 
		double **cmatrix, double dt, double h, double **T, double *t, 
		double **ka, double **kg, double **kb, double ***kc)
{
  int i, j, k, m;
  double eps, h2, sum;

  i = dt/h;
  h2 = dt/(i+1);
  init_matrix(ka, 4, p);
  init_matrix(kb, 4, p);
  init_3dimmatrix(kc, 4, p, p);
  if (kg != NULL) 
    init_matrix(kg, 4, p);

  for (eps = 0; eps <= dt-h2/2; eps += h2) {
    for (i=0; i < p; i++) {
      sum=0;
      for (j=0; j < p; j++)
        sum += T[j][i]*avector[j];
      ka[0][i] = h2*sum;
    }
    for (i=0; i < p; i++) {
      sum=0;
      for (j=0; j < p; j++)
        sum += T[j][i]*(avector[j]+ka[0][j]/2);
      ka[1][i] = h2*sum;
    }
    for (i=0; i < p; i++) {
      sum=0;
      for (j=0; j < p; j++)
        sum += T[j][i]*(avector[j]+ka[1][j]/2);
      ka[2][i] = h2*sum;
    }
    for (i=0; i < p; i++) {
      sum=0;
      for (j=0; j < p; j++)
        sum += T[j][i]*(avector[j]+ka[2][j]);
      ka[3][i] = h2*sum;
    }
    
    if (gvector != NULL) {
      for (i=0; i < p; i++) 
	kg[0][i] = h2*avector[i];
      for (i=0; i < p; i++) 
	kg[1][i] = h2*(avector[i]+ka[0][i]/2);
      for (i=0; i < p; i++) 
	kg[2][i] = h2*(avector[i]+ka[1][i]/2);
      for (i=0; i < p; i++) 
	kg[3][i] = h2*(avector[i]+ka[2][i]);
      for (i=0; i < p; i++) 
      gvector[i] += (kg[0][i]+2*kg[1][i]+2*kg[2][i]+kg[3][i])/6;
    }
    
    for (i=0; i < p; i++) {
      sum=0;
      for (j=0; j < p; j++)
        sum += T[i][j]*bvector[j];
      kb[0][i] = h2*sum;
    }
    for (i=0; i < p; i++) {
      sum=0;
      for (j=0; j < p; j++)
        sum += T[i][j]*(bvector[j]+kb[0][j]/2);
      kb[1][i] = h2*sum;
    }
    for (i=0; i < p; i++) {
      sum=0;
      for (j=0; j < p; j++)
        sum += T[i][j]*(bvector[j]+kb[1][j]/2);
      kb[2][i] = h2*sum;
    }
    for (i=0; i < p; i++) {
      sum=0;
      for (j=0; j < p; j++)
        sum += T[i][j]*(bvector[j]+kb[2][j]);
      kb[3][i] = h2*sum;
    }
   
    for (m=0; m < p; m++)
      for (i=0; i < p; i++) {
        sum=t[m]*avector[i];
        for (j=0; j < p; j++)
          sum += T[m][j]*cmatrix[j][i];
        kc[0][m][i] = h2*sum;
      }
    for (m=0; m < p; m++)
      for (i=0; i < p; i++) {
        sum=t[m]*(avector[i]+ka[0][i]/2);
        for (j=0; j < p; j++)
          sum += T[m][j]*(cmatrix[j][i]+kc[0][j][i]/2);
        kc[1][m][i] = h2*sum;
      }
    for (m=0; m < p; m++)
      for (i=0; i < p; i++) {
        sum=t[m]*(avector[i]+ka[1][i]/2);
        for (j=0; j < p; j++)
          sum += T[m][j]*(cmatrix[j][i]+kc[1][j][i]/2);
        kc[2][m][i] = h2*sum;
      }
    for (m=0; m < p; m++)
      for (i=0; i < p; i++) {
        sum=t[m]*(avector[i]+ka[2][i]);
        for (j=0; j < p; j++)
          sum += T[m][j]*(cmatrix[j][i]+kc[2][j][i]);
        kc[3][m][i] = h2*sum;
      }
    
    for (i=0; i < p; i++) {
      avector[i] += (ka[0][i]+2*ka[1][i]+2*ka[2][i]+ka[3][i])/6;
      bvector[i] += (kb[0][i]+2*kb[1][i]+2*kb[2][i]+kb[3][i])/6;
      for (j=0; j < p; j++)
        cmatrix[i][j] +=(kc[0][i][j]+2*kc[1][i][j]+2*kc[2][i][j]+kc[3][i][j])/6;
    }
  }
} 


double density(double t, int type, double *par)
{
  if (t <= 0)
    return(0);
  else {
    switch(type) {
    case 1:
      if ((t >= par[0]) && (t <= par[1]))
	return(1/(par[1]-par[0]));
      else
	return(0);
      break;
    case 2:
      return(1/(sqrt(2*3.141593)*par[1]*exp((t-par[0])*(t-par[0])
					    /(2*par[1]*par[1]))));
      break;
    case 3:
      return(1/(sqrt(2*3.141593)*t*par[1]*exp((log(t)-par[0])*(log(t)-par[0])
					      /(2*par[1]*par[1]))));
      break;
    case 4:
      return(par[0]*pow(par[1],par[0])*pow(t,par[0]-1)/exp(pow((par[1]*t),
							       par[0])));
      break;
    case 5:
      return(par[0]/(sqrt(2*3.141593))*pow(t,-1.5)
	     *exp(par[1]*par[0]-0.5*(par[0]*par[0]/t+par[1]*par[1]*t)));
    case 7:
      /*this is where you should write your density f(t)*/
      break;
    default:
      return 0.0;
    }
  }
}

void ExportToMatlab_Phasetype(int dim, double h, double dt, double truncpoint,
			      double *pie, double **q, double *exit)
{
  int i,j;
  double t, deltat;
  double density, cumulative, intensity;
  double *avector, **ka;
  FILE *utfil2;
  
  utfil2 = fopen("inputdistr","w");
  deltat = truncpoint*1.1/400;
  i = deltat/h;
  h = deltat/(i+1);
  avector = v_alloc(dim);
  ka = m_alloc(4, dim);
    for (i=0; i < dim; i++) 
    avector[i] = pie[i];
  
  for (t=0; t<=truncpoint*1.3; t += deltat) {
    density = 0;
    for (j=0; j < dim; j++)
      density += avector[j]*exit[j];
    
    cumulative=1;
    for (j=0; j < dim; j++)
      cumulative -= avector[j];
    
    if (cumulative < 0.9999)
      intensity = density/(1-cumulative);
    else
      intensity = 0;
    fprintf(utfil2, "%e %e %e %e \n", t, cumulative, density, intensity);
    a_rungekutta(dim, avector, ka, deltat, h, q);
  }
  fclose(utfil2);
  free(avector);
  free(ka);
}


void ExportToEMPHTmain_Phasetype(int dim, double h, double dt,double truncpoint,
				 double *pie, double **q, double *exit)
{
  int i,j;
  double t, tcenterofmass, deltat;
  double density, density1, density2, density3;
  double *avector, **ka;
  FILE *utfil;
  
  utfil=fopen("sample","w");
  deltat=dt;
  i=deltat/h;
  h=deltat/(i+1);
  avector = v_alloc(dim);
  ka = m_alloc(4,dim);
  for (i=0; i < dim; i++) 
    avector[i]=pie[i];
  
  for (t=deltat; t<=truncpoint+deltat/2; t += deltat) {
    density1=0;
    for (j=0; j < dim; j++)
      density1 += avector[j]*exit[j];
    a_rungekutta(dim, avector, ka, deltat/2, h, q);
    density2=0;
    for (j=0; j < dim; j++)
      density2 += avector[j]*exit[j];
    a_rungekutta(dim, avector, ka, deltat/2, h, q);
    density3=0;
    for (j=0; j < dim; j++)
      density3 += avector[j]*exit[j];
    density=(density1+4*density2+density3)/6;
    tcenterofmass=(density1*t+4*density2*(t+deltat/2)+density3*(t+deltat))/
      (6*density);
    fprintf(utfil, "%e %e \n", tcenterofmass, density);
  }
  fprintf(utfil, "-1");
  fclose(utfil);
  free(avector);
  free(ka);
}

void ExportToMatlab(int k, double *parameter, double truncpoint)
{
  double t, deltat;
  double cumulative, intensity;
  FILE *utfil2;

  utfil2=fopen("inputdistr","w");
  cumulative=0;
  deltat=truncpoint/400;
  for (t=deltat; t<=truncpoint+deltat/2; t += deltat) {
    if (t>0)
      cumulative += (density(t-deltat,k,parameter)+
		     4*density(t-deltat/2,k,parameter)+
		     density(t,k,parameter))*deltat / 6;
    if (cumulative<0.9999)
      intensity=density(t,k,parameter)/(1-cumulative);
    else
      intensity=0;
    fprintf(utfil2, "%e %e %e %e \n", t, cumulative, density(t,k,parameter),
	    intensity);
  }
  fclose(utfil2);
}


void show_pi_T(int p, double *pi, double **T, double *t)
{
  int i, j;
  
  for (i=0; i < p; i++) {
    printf("\n%lf    ", pi[i]);
    for (j=0; j < p; j++)
      printf("%lf ", T[i][j]);
    printf("   %lf",t[i]);
  }
  printf("\n");
}


double set_steplength(int p, double **T)
{    
  int i;
  double h;

  h= -0.1/T[0][0];
  for (i=1; i < p; i++)
    if (h > -0.1/T[i][i])
      h = -0.1/T[i][i];
  return(h);
}


void input_density()
{
  int i, j, choice, choice1, stepchoice, dim;
  double t, deltat, prob, observation, parameter[2], *pie, **q, *exit;
  double truncpoint, maxprob, maxdeltat, dt, h;
  FILE *infil, *utfil;

  printf("\nType of density:\n");
  printf("      1. Rectangle\n");
  printf("      2. Normal\n");
  printf("      3. Lognormal\n");
  printf("      4. Weibull\n");
  printf("      5. Inverse Gauss\n");
  printf("      6. Phasetype\n");
  printf("      7. User specified\n");
  printf("Select(1-7): ");
  scanf("%d", &choice);
  switch(choice) {
  case 1:
    printf("A uniform (rectangle) distribution between a and b:\n");
    printf("a:");
    scanf("%lf", &parameter[0]);
    printf("b:");
    scanf("%lf", &parameter[1]);
    break;
  case 2:
    printf("A normal distribution with mean mu and standard deviation sigma:\n");
    printf("mu:");
    scanf("%lf", &parameter[0]);
    printf("sigma:");
    scanf("%lf", &parameter[1]);
    break;
  case 3:
    printf("A lognormal distribution with parameters alfa and beta:\n");
    printf("alfa:");
    scanf("%lf", &parameter[0]);
    printf("beta:");
    scanf("%lf", &parameter[1]);
    break;
  case 4:
    printf("A Weibull distribution with shape parameter beta and scale parameter lambda:\n");
    printf("beta:");
    scanf("%lf", &parameter[0]);
    printf("lambda:");
    scanf("%lf", &parameter[1]);
    break;
  case 5:
    printf("An inverse Gauss distribution with parameters c and xi:\n");
    printf("c:");
    scanf("%lf", &parameter[0]);
    printf("xi:");
    scanf("%lf", &parameter[1]);
    break;
  case 6:
    printf("How will the pi-vector (starting probabilities) and the T-matrix (jump rates)\nof the phasetype distribution be entered?\n");
    printf("      1. From file 'input-phases'\n");
    printf("      2. From keyboard\n");
    printf("Select 1 or 2: ");
    scanf("%d", &choice1);
    printf("Time at which density can be truncated:");
    scanf("%lf", &truncpoint);
    printf("Exact time interval corresponding to one point:");
    scanf("%lf", &dt);
    switch (choice1) {
    case 1:
      infil=fopen("input-phases", "r");
      fscanf(infil, "%d", &dim);
      pie = v_alloc(dim);
      q = m_alloc(dim,dim);
      for (i=0; i < dim; i++) {
	fscanf(infil, "%le", &pie[i]);
	for (j=0; j < dim; j++)
	  fscanf(infil, "%le", &q[i][j]);
      }
      fclose(infil);
      break;
    case 2:
      printf("Dimension p: ");
      scanf("%d", &dim);
      pie = v_alloc(dim);
      q = m_alloc(dim,dim);
      for (i=0; i < dim; i++) {
	printf("pi[%d]: ", i+1);
	scanf("%le", &pie[i]);
	for (j=0; j < dim; j++) {
	  printf("T[%d][%d]: ", i+1, j+1);
	  scanf("%le", &q[i][j]);
	}            
      }     
      break;
    }
    exit = v_alloc(dim);
    for (i=0; i < dim; i++) 
      for (j=0; j < dim; j++)
	exit[i] -= q[i][j];
    show_pi_T(dim, pie, q, exit);
    printf("\n");
    printf("\nChoose step-length for the Runge_Kutta procedure:");
    printf("\n1. Default value.");
    printf("\n2. Your own choice of value.");
    printf("\nSelect 1 or 2.\n");
    scanf("%d", &stepchoice);
    if (stepchoice == 2) {
      printf("Step-length = ");
      scanf("%le", &h);
    }
    else 
      h = set_steplength(dim, q);
    
    ExportToEMPHTmain_Phasetype(dim, h, dt, truncpoint, pie, q, exit);     
    ExportToMatlab_Phasetype(dim, h, dt, truncpoint, pie, q, exit);
    free(pie);
    free(exit);
    free(q);
    break;
  case 7:
    printf("You must do some programming, - see instructions\n");
    /* This is where the parameters are entered */
    break;
  }
  
  if (choice <= 5 || choice == 7) {
    utfil=fopen("sample","w");
    printf("Time at which density can be truncated:");
    scanf("%lf", &truncpoint);
    printf("Maximum acceptable probability in one point: ");
    scanf("%lf", &maxprob);
    printf("Maximum time interval corresponding to one point: ");
    scanf("%lf", &maxdeltat);
    t=0;
    while (t<truncpoint) {
      if (density(t,choice,parameter) < maxprob/maxdeltat)
        deltat = maxdeltat;
      else 
        deltat = maxprob / density(t,choice,parameter);
      prob = deltat / 6*(density(t,choice,parameter)+
		     4*density(t+deltat/2,choice,parameter)+
		     density(t+deltat,choice,parameter));
      while (prob > maxprob) {
        deltat = deltat*0.9;
        prob = deltat/6*(density(t,choice,parameter)+
		       4*density(t+deltat/2,choice,parameter)+
		       density(t+deltat,choice,parameter));
      }
      if (prob>0) {
        observation=(t*density(t,choice,parameter)+
		     4*(t+deltat/2)*density(t+deltat/2,choice,parameter)+
		     (t+deltat)*density(t+deltat,choice,parameter))/
		       (density(t,choice,parameter)+
			4*density(t+deltat/2,choice,parameter)+
			density(t+deltat,choice,parameter));
        fprintf(utfil, "%e %e \n", observation, prob);
      }
      t+=deltat;
    }
    fprintf(utfil, "-1");
    fclose(utfil);
    ExportToMatlab(choice, parameter, truncpoint);
  }
}


void input_sample(int NoOfInput[3])
{
  int i, choice;
  double Obs, Weight;
  FILE *utfil, *infil;
  
  
  printf("\nType of sample, and way of entering:\n");
  printf("      1. Unweighted, from keyboard\n");
  printf("      2. Weighted, from keyboard\n");
  printf("      3. Unweighted, from file 'unweighted'\n");
  printf("      4. Weighted, from file 'sample'\n");
  printf("Select (1-4): ");
  scanf("%d", &choice);
  i=1;     
  switch(choice) {
  case 1:
    Weight=1;
    utfil=fopen("sample","w");
    printf("Enter failure times, and quit with -1\n");
    printf("Time 1:");
    scanf("%le", &Obs);
    while (Obs >= 0) {
      fprintf(utfil, "%e %e \n", Obs, Weight);
      NoOfInput[1]++;
      printf("Time %d:", i+1);
      scanf("%le", &Obs);
      i++;
    }
    fprintf(utfil, "-1");
    fclose(utfil);
    break;
  case 2:
    utfil=fopen("sample","w");     
    printf("Enter failure times and number of cases. Quit with time = -1.");
    printf("\nTime 1:");
    scanf("%le", &Obs);
    while (Obs >= 0) {
      printf("Number of cases:");
      scanf("%le", &Weight);
      fprintf(utfil, "%e %e \n", Obs, Weight);
      NoOfInput[1]++;
      printf("\nTime %d:", i+1);
      scanf("%le", &Obs);
      i++;
    }
    fprintf(utfil, "-1");
    fclose(utfil);
    break;
  case 3:
    Weight=1;
    infil=fopen("unweighted", "r");
    utfil=fopen("sample", "w");
    fscanf(infil, "%le", &Obs);
    while (Obs >= 0) {
      NoOfInput[1]++;
      fprintf(utfil, "%e %e \n", Obs, Weight);
      fscanf(infil, "%le", &Obs);
    }
    fprintf(utfil, "-1");
    fclose(infil);
    fclose(utfil);
    break;
  case 4:
    infil=fopen("sample", "r");
    fscanf(infil, "%le", &Obs);
    while (Obs >= 0) {
      NoOfInput[1]++;
      fscanf(infil, "%le %le", &Weight, &Obs);
    }
    fclose(infil);
    break;
  default:
    printf("Wrong number\n");
    break;             
  }
}


void input_Csample(int NoOfInput[3])
{
  int i, choice, indicator;
  double Obs, Weight, Low, Upp;
  FILE *utfil, *infil;
  
  printf("\nThe sample is entered:\n");
  printf("      1. Unweighted, from keyboard\n"); 
  printf("      2. Weighted, from keyboard\n");
  printf("      3. Unweighted, from file 'unweighted'\n");
  printf("      4. Weighted, from file 'sample'\n");
  printf("Select (1-4): ");
  scanf("%d", &choice);
  i=1;    
  switch(choice) {
  case 1:
    Weight = 1;
    utfil=fopen("sample","w");
    printf("\nThe indicator is: = 0 for right censored observation\n");
    printf("                  = 1 for uncensored observation.\n");
    printf("                  = 2 for interval censored observation.\n");
    printf("Quit with indicator = -1\n");
    printf("\nIndicator 1:");
    scanf("%d",&indicator);
    while ( indicator != -1) {
      if (indicator == 2) {
	printf("Interval-observation %d:", i);
	scanf("%le %le", &Low, &Upp);
        fprintf(utfil, "%d %e %e %e \n", indicator, Low, Upp, Weight);
	NoOfInput[2] ++;
      }
      else {
	printf("Observation %d:", i);
	scanf("%le", &Obs);
	fprintf(utfil, "%d %e %e \n", indicator, Obs, Weight);
	if (indicator == 0)
	  NoOfInput[0] ++;
	if (indicator == 1)
	  NoOfInput[1] ++;
      }
      printf("\nIndicator %d:", i+1);
      scanf("%d", &indicator);
      i++;
    }
    fprintf(utfil, "-1");
    fclose(utfil);
    break;     
  case 2:
    utfil=fopen("sample","w");
    printf("\nThe indicator is:   =0 for right censored observation\n");
    printf("                    =1 for uncensored observation \n");
    printf("                    =2 for interval censored observation \n");
    printf("Quit with indicator = -1\n");
    printf("\nIndicator 1:");
    scanf("%d",&indicator);
    while ( indicator != -1) {
      if (indicator == 2) {
	printf("Interval-observation %d:", i);
	scanf("%le %le", &Low, &Upp);
	printf("Number of cases %d:", i);
	scanf("%le", &Weight);
        fprintf(utfil, "%d %e %e %e \n", indicator, Low, Upp, Weight);
	NoOfInput[2] ++;
      }
      else {
	printf("Observation %d:", i);
	scanf("%le", &Obs);
	printf("Number of cases %d:", i);
	scanf("%le", &Weight);
	fprintf(utfil, "%d %e %e \n", indicator, Obs, Weight);
	if (indicator == 0)
	  NoOfInput[0] ++;
	if (indicator == 1)
	  NoOfInput[1] ++;
      }
      printf("\nIndicator %d:", i+1);
      scanf("%d", &indicator);
      i++;
    }
    fprintf(utfil, "-1");
    fclose(utfil);
    break;
  case 3:
    Weight=1;
    infil=fopen("unweighted", "r");
    utfil=fopen("sample", "w");
    fscanf(infil, "%d", &indicator);
    while (indicator != -1) {
      if (indicator == 2) {
	fscanf(infil, "%le %le", &Low, &Upp); 
	fprintf(utfil, "%d %e %e %e \n", indicator, Low, Upp, Weight);
	NoOfInput[2] ++;
      }
      else {
	fscanf(infil, "%le", &Obs);
	fprintf(utfil, "%d %e %e \n", indicator, Obs, Weight);
	if (indicator == 0)
	  NoOfInput[0] ++;
	if (indicator == 1)
	  NoOfInput[1] ++;
      }
      fscanf(infil, "%d", &indicator);
    }
    fprintf(utfil, "-1");
    fclose(infil);
    fclose(utfil);
    break;
  case 4:
    infil=fopen("sample", "r");
    fscanf(infil, "%d", &indicator);
    while (indicator != -1) {
      if (indicator == 2) {
	fscanf(infil, "%le %le %le", &Low, &Upp, &Weight); 
	NoOfInput[2] ++;
      }
      else {
	fscanf(infil, "%le %le", &Obs, &Weight);
	if (indicator == 0)
	  NoOfInput[0] ++;
	if (indicator == 1)
	  NoOfInput[1] ++;
      }
      fscanf(infil, "%d", &indicator);
    }
    fclose(infil);
    break;
  default:
    printf("Wrong number\n");
    break; 
  }
}

void assign_vectors(int No)
{
  int i;
  FILE *infil;
  
  infil=fopen("sample", "r");
   for(i=0; i < No; i++) {
    fscanf(infil, "%le %le", &obs[i], &weight[i]);
    SumOfWeights += weight[i];
  }
  fclose(infil);
}

void assign_Cvectors()
{
  int i, j, k, indicator;
  double Obs, Weight, Low, Upp;
  FILE *infil;

  i=0; j=0; k=0;
  infil=fopen("sample", "r");
  fscanf(infil, "%d", &indicator);
  while (indicator != -1) {
    if (indicator == 2) {
      fscanf(infil, "%le %le %le", &Low, &Upp, &Weight);
      lower[i]=Low;
      upper[i]=Upp;
      intweight[i]=Weight;
      SumOfInt += intweight[i];
      i++;
    }
    else {
      fscanf(infil, "%le %le", &Obs, &Weight);
      if(indicator == 0) {
	censur[j] = Obs;
	cweight[j] = Weight;
	SumOfCensored += cweight[j];
	j++;
      }
      if(indicator == 1) {
	obs[k] = Obs;
	weight[k] = Weight;
	SumOfWeights += weight[k];
	k++;
      }
    }
    fscanf(infil, "%d", &indicator);
  }
  fclose(infil);
}

int sort_observations(int size, double *vec1, double *vec2)
{
  int i,j, tempweight, newsize;
  double tempobs;

  newsize=size;
  for (i=0; i < size-1; i++)
    for (j=i+1; j < size; j++)
      if (vec1[i] > vec1[j]) {
        tempobs = vec1[i];
        vec1[i] = vec1[j];
        vec1[j] = tempobs;
        tempweight = vec2[i];
        vec2[i] = vec2[j];
        vec2[j] = tempweight;
      }
  for (i=size-2; i >= 0; i--)
    if (vec1[i] == vec1[i+1]) {
      vec2[i] += vec2[i+1];
      newsize--;
      for(j=i+1; j < newsize; j++) {
        vec1[j] = vec1[j+1];
        vec2[j] = vec2[j+1];
      }
    }
  return (newsize);
}

int sort_interval(int NoOfPairs)
{
  double temp;
  int i, j, v, newsize;

  for (i=0; i < NoOfPairs-1; i++)
    for (j=i+1; j < NoOfPairs; j++)
      if (lower[i] > lower[j]) {
	temp = lower[i];
	lower[i] = lower[j];
	lower[j] = temp;
	temp = upper[i];
	upper[i] = upper[j];
	upper[j] = temp;
	temp = intweight[i];
	intweight[i] = intweight[j];
	intweight[j] = temp;
      }

  newsize=NoOfPairs;
  for (i=NoOfPairs-2; i>=0; i--)
    if (lower[i]==lower[i+1] && upper[i]==upper[i+1]) {
      intweight[i] += intweight[i+1];
      newsize --;
      for (j=i+1; j<newsize; j++) {
	lower[j]=lower[j+1];
	upper[j]=upper[j+1];
	intweight[j]=intweight[j+1];
      }
    }

  partner[0]=0; v=0;
  for (i=1; i < newsize; i++) {
    if (lower[i]>lower[i-1]) 
      v++;      
    partner[i]=v;
  }

  for (i=0; i < newsize-1; i++)
    for (j=i+1; j < newsize; j++)
      if (upper[i] > upper[j]) {
	temp = upper[i];
	upper[i] = upper[j];
	upper[j] = temp;
	temp = partner[i];
	partner[i] = partner[j];
	partner[j] = temp;
	temp = intweight[i];
	intweight[i] = intweight[j];
	intweight[j] = temp;
      }
  return(newsize);
}

int count_input()
{
  int i;
  double obs, weight;
  FILE *infil;

  infil=fopen("sample", "r");
  fscanf(infil, "%le", &obs);
  for (i=0; obs != -1; i++) 
    fscanf(infil, "%le %le", &weight, &obs);
  fclose(infil);
  return(i);
}

void Export_Sample()
{
  int i;
  FILE *utfil2;
  double CumWeight;

  utfil2 = fopen("inputdistr","w");
  CumWeight = 0.0;
  
  fprintf(utfil2, "%e %e %e %e\n", obs[0], 0.0, 0.0, 0.0);
  for (i=0; i < NoOfObs-1; i++) {
    CumWeight += weight[i];
    fprintf(utfil2, "%e %e %e %e \n",obs[i], CumWeight/SumOfWeights, 0.0,0.0);
    fprintf(utfil2, "%e %e %e %e \n",obs[i+1], CumWeight/SumOfWeights,0.0,0.0);
  }
  fprintf(utfil2, "%e %e %e %e \n", obs[NoOfObs-1], 1.0, 0.0, 0.0);
  fclose(utfil2);
}

void Export_CensoredSample(int NoOfInput[3])
{
  int i, j, risk;
  double kaplan;
  FILE *utfil2;
 
  utfil2 = fopen("inputdistr","w");
  fprintf(utfil2, "%e %e %e %e\n", 0.0, 0.0, 0.0, 0.0);
  if (NoOfInput[0]>0 && NoOfInput[2]==0) {
    kaplan=1.0;
    risk = SumOfWeights + SumOfCensored;
    for (i=0, j=0; i < NoOfObs-1; i++) {
      while (censur[j] < obs[i] && NoOfCensored > j) {
	risk -= cweight[j];
	j++;
      } 
      kaplan = kaplan*(1.0-weight[i]/risk);
      fprintf(utfil2, "%e %e %e %e \n", obs[i], 1-kaplan, 0.0,  0.0);
      fprintf(utfil2, "%e %e %e %e \n", obs[i+1], 1-kaplan, 0.0, 0.0);
      risk -=  weight[i];
    }
    while (censur[j] < obs[NoOfObs-1] && NoOfCensored > j) {
      risk -= cweight[j];
      j++;
    }
    kaplan = kaplan*(1.0-weight[i]/risk);
    fprintf(utfil2, "%e %e %e %e \n", obs[NoOfObs-1], 1-kaplan, 0.0, 0.0);
    fprintf(utfil2, "%e %e %e %e \n", obs[NoOfObs-1]+1, 1-kaplan, 0.0, 0.0);
  }   
  fclose(utfil2);
}

int AskForInput(int NoOfInput[3])
{
  int input, sampletype;

  printf("\nType of input:\n");
  printf("      1. Sample\n");
  printf("      2. Density\n");
  printf("Select 1 or 2: ");
  scanf("%d", &input);
  switch(input) {
  case 1:
    printf("\nThe sample contains:\n");
    printf("   1. no censored observations \n");
    printf("   2. some right- and/or interval- censored observations\n");
    printf("Select 1 or 2: ");    
    scanf("%d", &sampletype);
    switch(sampletype) {
    case 1:
      input_sample(NoOfInput);
      obs = v_alloc(NoOfInput[1]);
      weight = v_alloc(NoOfInput[1]);
      assign_vectors(NoOfInput[1]);
      NoOfObs = sort_observations(NoOfInput[1], obs, weight);
      Export_Sample();
      printf("Total number of observations = %d\n", (int) SumOfWeights);   
      break;
    case 2:    
      input_Csample(NoOfInput);
      if (NoOfInput[0] > 0) {
	censur = v_alloc(NoOfInput[0]);
	cweight = v_alloc(NoOfInput[0]);
      }
      if (NoOfInput[1] > 0) {
	obs = v_alloc(NoOfInput[1]);
	weight = v_alloc(NoOfInput[1]);
      }
      if (NoOfInput[2] > 0) {
	lower = v_alloc(NoOfInput[2]);
	upper = v_alloc(NoOfInput[2]);
	intweight = v_alloc(NoOfInput[2]);
	partner = int_v_alloc(NoOfInput[2]);
      }
      assign_Cvectors(NoOfInput);
      if (NoOfInput[0] > 0)       
	NoOfCensored = sort_observations(NoOfInput[0], censur, cweight);
      if (NoOfInput[1] > 0) 
	NoOfObs = sort_observations(NoOfInput[1], obs, weight);
      if (NoOfInput[2] > 0) 
	NoOfInt = sort_interval(NoOfInput[2]);
      
      Export_CensoredSample(NoOfInput);
      printf("Total number of observations:\n"); 
      printf("     non-censored = %d\n", (int)(SumOfWeights));
      printf("   right-censored = %d\n", (int)(SumOfCensored));  
      printf("interval-censored = %d\n", (int)(SumOfInt)); 
      break;
    }
    break; 
  case 2:
    input_density();
    NoOfObs = count_input();
    obs = v_alloc(NoOfObs);
    weight = v_alloc(NoOfObs);
    assign_vectors(NoOfObs);
    break;
  }
  return(sampletype);
}


double myrandom()
{
  double r;

  r=rand();
  r /= ((double) RAND_MAX);
  r *= 0.9;
  r += 0.1;
  return(r);
}

void myrandomphase(int p, double *pi, double **T, double *exitvector, 
		 int *pilegal, int **Tlegal)
{
  int i, j;
  double r, sum, scalefactor;

  if ( (NoOfObs>NoOfCensored) || (NoOfObs>NoOfInt) )
    scalefactor=obs[(NoOfObs-1)/2];
  else if (NoOfInt > NoOfCensored)
    scalefactor=upper[NoOfInt/2];
  else
    scalefactor=censur[NoOfCensored/2];

  sum=0;
  for (i=0; i < p; i++)
    if (pilegal[i] == 1) {
      pi[i]=myrandom();
      sum += pi[i];
    }
  for (i=0; i < p; i++)
    pi[i]=pi[i]/sum;
  for (i=0; i < p; i++)
    for (j=0; j < p; j++)
      if ((i != j) && (Tlegal[i][j] == 1)) {
        T[i][j]=myrandom();
        T[i][i] -= T[i][j];
      }
  for (i=0; i < p; i++)
    if (Tlegal[i][i] == 1) {
      r=myrandom();
      T[i][i] -= r;
      exitvector[i]=r;
    }
  for (i=0; i < p; i++) {
    exitvector[i] = exitvector[i] * p/ scalefactor;
    for (j=0; j < p; j++)
      T[i][j] = T[i][j] * p / scalefactor;
  }
}
     
void selectstructure(int p, double *pi, double **T, double *t, int *pilegal,
		     int **Tlegal)
{
  int i, j, structure;
  FILE *infil;

  printf("Select distribution type: \n");
  printf("      1. General phase-type \n");
  printf("      2. Hyperexponential \n");
  printf("      3. Sum of exponentials \n");
  printf("      4. Coxian \n");
  printf("      5. Coxian general \n");
  printf("      6. User specified structure (from file 'distrtype') \n");
  printf("      7. User specified starting values (from file 'phases') \n");
  printf("      8. User specified starting values (from keyboard) \n");
  printf("Select(1-8): ");
  scanf("%d", &structure);
  switch(structure) {
  case 1:
    for (i=0; i < p; i++) {
      pilegal[i]=1;
      for (j=0; j < p; j++)
	Tlegal[i][j]=1;
    }
    break;
  case 2:
    for (i=0; i < p; i++) {  
      pilegal[i]=1;
      Tlegal[i][i]=1;
    }
    break;
  case 3:
    pilegal[0]=1;
    for (i=0; i < p-1; i++) 
      Tlegal[i][i+1]=1;
    Tlegal[p-1][p-1]=1;
    break;
  case 4:
    pilegal[0]=1;
    for (i=0; i < p-1; i++) { 
      Tlegal[i][i]=1;
      Tlegal[i][i+1]=1;
    }
    Tlegal[p-1][p-1]=1;  
    break;
  case 5:   
    for (i=0; i < p-1; i++) { 
      pilegal[i]=1;
      Tlegal[i][i]=1;
      Tlegal[i][i+1]=1;
    }
    Tlegal[p-1][p-1]=1;
    pilegal[p-1]=1;
    break;
  case 6:
    infil=fopen("distrtype", "r");
    for (i=0; i < p; i++) {
      fscanf(infil,"%d", &pilegal[i]);
      for (j=0; j < p; j++)
	fscanf(infil,"%d", &Tlegal[i][j]);
    }
    printf("Number of phases:%d\n",p);
    fclose(infil);
    break;
  case 7:
    infil=fopen("phases", "r");
    for (i=0; i < p; i++) {
      fscanf(infil, "%le", &pi[i]);
      for (j=0; j < p; j++)
	fscanf(infil, "%le", &T[i][j]);
    }
    fclose(infil);
    break;
  case 8:
    for (i=0; i < p; i++) {
      printf("pi[%d]: ", i+1);
      scanf("%le", &pi[i]);
      for (j=0; j < p; j++) {
	printf("T[%d][%d]: ", i+1, j+1);
	scanf("%le", &T[i][j]);
      }         
    }   
   break;
  }
  
  if (structure < 7) {
    printf("\n Phase-type structure:");
    for (i=0; i < p; i++) {
      printf("\n%d     ", pilegal[i]);
      for (j=0; j < p; j++)
        printf("%d ", Tlegal[i][j]);
    }
    printf("\nMyrandom initiation - enter an integer (at myrandom):");
    scanf("%d",&i);
    srand(i);
    printf("\n");
    myrandomphase(p, pi, T, t, pilegal, Tlegal);
  }
  else 
    for (i=0; i < p; i++) 
      for (j=0; j < p; j++) 
	t[i] -= T[i][j];
}

void EMstep(int p, double h, double *pi, double **T, double *t, double 
            *gvector, double *avector, double *bvector, double **cmatrix, 
            double *Bmean, double *Zmean, double **Nmean, double **kg, 
            double **ka, double **kb, double ***kc, double *ett, double 
            **g_left, double **a_left, double **b_left, double ***c_left)
{
  int i, j, k, n, v, m, mate;
  double pitimesb, dt, adiffsum, prev;

  init_vector(Bmean, p);
  init_vector(Zmean, p);
  init_matrix(Nmean, p, p+1);

  if (NoOfObs > 0) {
    for (i=0; i < p; i++) {
      avector[i]=pi[i];
      bvector[i]=t[i];
    }
    init_matrix(cmatrix, p, p);  
    dt = obs[0];
    for (k=0; k < NoOfObs; k++) {
      if (dt > 0)
	rungekutta(p, avector, NULL, bvector, cmatrix, dt, h, T, t, ka, NULL,
		   kb, kc);
      pitimesb=0;
      for (i=0; i < p; i++)
	pitimesb += pi[i]*bvector[i];
      for (i=0; i < p; i++) {
	Bmean[i] += pi[i]*bvector[i]*weight[k]/pitimesb;
	Nmean[i][p] += avector[i]*t[i]*weight[k]/pitimesb;
	Zmean[i] += cmatrix[i][i]*weight[k]/pitimesb;
	for (j=0; j < p; j++) 
	  Nmean[i][j] += T[i][j]*cmatrix[j][i]*weight[k]/pitimesb;
      }
      dt=obs[k+1]-obs[k];
    }
  }

  if (NoOfCensored > 0) {
    for (i=0; i < p; i++) {
      avector[i]=pi[i];
      bvector[i]=1;
    }
    init_matrix(cmatrix, p, p);   
    dt = censur[0];
    for (k=0; k < NoOfCensored; k++) {
      if (dt > 0)
	rungekutta(p, avector, NULL, bvector, cmatrix, dt, h, T, ett, ka, NULL, 
		   kb, kc);
      pitimesb = 0;
      for (i=0; i < p; i++) 
	pitimesb += pi[i]*bvector[i];
      for (i=0; i < p; i++) {
	Bmean[i] += pi[i]*bvector[i]*cweight[k]/pitimesb;
	Zmean[i] += cmatrix[i][i]*cweight[k]/pitimesb;
	for(j=0; j < p; j++)
	  Nmean[i][j] += T[i][j]*cmatrix[j][i]*cweight[k]/pitimesb;
      }
      dt = censur[k+1]-censur[k];
    }
  }

  if (NoOfInt > 0) {
    for (i=0; i < p; i++) {
      avector[i]=pi[i];
      bvector[i]=1;
    }
    init_vector(gvector, p);
    init_matrix(cmatrix, p, p);
    dt = lower[0];
    if (dt > 0)
      rungekutta(p, avector, gvector, bvector, cmatrix, dt, h, T, ett, ka, kg, 
		 kb, kc);
    for(i=0; i < p; i++) {
      a_left[0][i] = avector[i];
      g_left[0][i] = gvector[i];
      b_left[0][i] = bvector[i];
      for(j=0; j < p; j++)
	c_left[0][i][j] = cmatrix[i][j];
    }
    m=1;
    prev=lower[0];
    v=1;
    for (k=0; k < NoOfInt; k++) {
      
      while ((v < NoOfInt) && (lower[v] < upper[k])) {
	dt = lower[v] - prev;
	if (dt > 0)
	  rungekutta(p, avector, gvector, bvector, cmatrix, dt, h, T, ett, ka, 
		     kg, kb, kc);
	if (lower[v] > lower[v-1]) {
	  for(i=0; i < p; i++) {
	    a_left[m][i] = avector[i];
	    g_left[m][i] = gvector[i];
	    b_left[m][i] = bvector[i];
	    for(j=0; j < p; j++)
	      c_left[m][i][j] = cmatrix[i][j];
	  }
	  m++;
	}
	prev=lower[v];
	v++;
      }
      dt = upper[k] - prev;
      if (dt > 0)
	rungekutta(p, avector, gvector, bvector, cmatrix, dt, h, T, ett, ka, kg, kb, kc);
      mate=partner[k];
            
      adiffsum = 0;
      for (i=0; i < p; i++)
	adiffsum += a_left[mate][i] - avector[i];
      for (i=0; i < p; i++) {
	Bmean[i] += pi[i]*(b_left[mate][i]-bvector[i])*intweight[k] /
	  adiffsum;
	Nmean[i][p] += t[i]*(gvector[i]-g_left[mate][i])*intweight[k]/
	  adiffsum;
	Zmean[i] += (gvector[i] - g_left[mate][i] + c_left[mate][i][i] - 
		     cmatrix[i][i])*intweight[k] / adiffsum;
	for (j=0; j < p; j++) 
	  Nmean[i][j] += T[i][j]*(gvector[i] - g_left[mate][i] + 
				  c_left[mate][j][i] - cmatrix[j][i])*
				    intweight[k] / adiffsum;
      }

      prev=upper[k];
      
    }
  }

  for (i=0; i < p; i++) {
    pi[i] = Bmean[i] / (SumOfCensored + SumOfWeights + SumOfInt);
    if (pi[i] < 0)
      pi[i] = 0;
    t[i] = Nmean[i][p] / Zmean[i];
    if (t[i] < 0)
      t[i] = 0;
    T[i][i] = -t[i];
    for (j=0; j < p; j++) 
      if (i!=j) {
	T[i][j] = Nmean[i][j] / Zmean[i];
	if (T[i][j] < 0)
	  T[i][j] = 0;
	T[i][i] -= T[i][j];
      } 
  } 
}


void compute_loglikelihood(double h, int p, double *pi, double **T, double *t, 
			   int stepindicator, double *avector, double **ka, 
			   double **a_left)
{
  int i, j, k, n, m, v, mate;
  double loglikelihood, asum, atimesexit, dt, adiffsum, prev;
  
  if (stepindicator == 1)
    h = set_steplength(p, T);
  loglikelihood = 0;
  
  if (NoOfObs > 0) {
    for (i=0; i < p; i++)
      avector[i] = pi[i];
    dt = obs[0];
    for (i=0; i < NoOfObs; i++) {
      atimesexit = 0.0;
      if (dt > 0)
	a_rungekutta(p, avector, ka, dt, h, T);
      for (j=0; j < p; j++)
	atimesexit += avector[j] * t[j];
      loglikelihood += weight[i] * log(atimesexit);  
      dt = obs[i+1]-obs[i];
    }
  }

  if (NoOfCensored > 0) {
    for (i=0; i < p; i++)
      avector[i] = pi[i];
    dt = censur[0];
    for (i=0; i< NoOfCensored; i++) {
      asum  = 0.0;
      if (dt > 0)
	a_rungekutta(p, avector, ka, dt, h, T);
      for (j=0; j < p; j++)
	asum += avector[j];
      loglikelihood += cweight[i] * log(asum);
      dt = censur[i+1]-censur[i];
    }
  }

  if (NoOfInt > 0) {
    for (i=0; i < p; i++) 
      avector[i]=pi[i];
    dt = lower[0];
    if (dt > 0)
      a_rungekutta(p, avector, ka, dt, h, T);
    for(i=0; i < p; i++) 
      a_left[0][i] = avector[i];
    m=1;
    prev=lower[0];
    v=1;
    for (k=0; k < NoOfInt; k++) {
      while ((v < NoOfInt) && (lower[v] < upper[k])) {
	dt = lower[v] - prev;
	if (dt > 0)
	  a_rungekutta(p, avector, ka, dt, h, T);
	if (lower[v] > lower[v-1]) {
	  for(i=0; i < p; i++) 
	    a_left[m][i] = avector[i];
	  m++;
	}
	prev=lower[v];
        v++;
      }
 
      dt = upper[k] - prev;
      if (dt > 0)
	a_rungekutta(p, avector, ka, dt, h, T);
      mate=partner[k];
            
      adiffsum = 0;
      for (i=0; i < p; i++)
	adiffsum += a_left[mate][i] - avector[i];
      loglikelihood += intweight[k] * log(adiffsum);

      prev=upper[k];
    }
  }
  printf("Log-likelihood = %lf \n", loglikelihood);
}


void SavePhases(int p, double *pi, double **T)
{ 
  int i,j;
  FILE *utfil;

  utfil=fopen("phases", "w");
  for (i=0; i < p; i++) { 
    fprintf(utfil, "\n%e   ", pi[i]);
    for (j=0; j < p; j++)
      fprintf(utfil, "%e ", T[i][j]);
  }
  fclose(utfil);
}

void EMiterate(int NoOfEMsteps, int p, double *pi, double **T, double *t,
	       double *gvector, double *avector, double *bvector, 
               double **cmatrix, double *Bmean, double *Zmean, double **Nmean, 
	       double **kg, double **ka, double **kb, double ***kc, 
               double *ett, double **g_left, double **a_left, 
               double **b_left, double ***c_left)
{
  int k, stepindicator, stepchoice; 
  double RKstep;

  printf("Choose step-length for the Runge_Kutta procedure:");
  printf("\n1. Default value");
  printf("\n2. Your own choice of value");
  printf("\nSelect 1 or 2 :");
  scanf("%d", &stepchoice);
  if (stepchoice == 2) {
    printf("Step-length = ");
    scanf("%le", &RKstep);
    stepindicator = 0;
  }
  else
    stepindicator = 1;
   
  for (k=1; k <= NoOfEMsteps; k++) {
    if (stepindicator == 1)
      RKstep = set_steplength(p, T);
    if (NoOfInt == 0) {
      EMstep(p, RKstep, pi, T, t, NULL, avector, bvector, cmatrix, Bmean, 
             Zmean, Nmean, NULL, ka, kb, kc, ett, NULL, NULL, NULL, NULL);
    }
    else { 
      EMstep(p, RKstep, pi, T, t, gvector, avector, bvector, cmatrix, Bmean,
	     Zmean, Nmean, kg, ka, kb, kc, ett, g_left, a_left, b_left, 
             c_left);
    }
    
    if ((k < 6) || ( (k % 25)==0 || k==NoOfEMsteps )) {
      printf("\nEM(%d)",k);
      show_pi_T(p, pi, T, t);
      SavePhases(p, pi, T);
      if (NoOfInt == 0)
	compute_loglikelihood(RKstep, p, pi, T, t, stepindicator, avector, ka,
			      NULL);
      else
	compute_loglikelihood(RKstep, p, pi, T, t, stepindicator, avector, ka,
			      a_left);
    }
  }
}


int search_partner(void)
{
  int i, max;

  max = partner[0]; 
  for (i=1;  i<NoOfInt; i++) 
    if (partner[i]>max)
      max=partner[i];
  
  return(max+1);
}   
 

main() 
{
  int i, j, sampletype, fitting, p,  NoOfEMsteps, choice, newp; 
  int *pilegal, **Tlegal, NoOfInput[3]={0, 0, 0}, NoToSave; 
  double  *pi, **T, *t, *avector, *gvector, *bvector, **cmatrix; 
  double *Bmean, *Zmean, **Nmean, **ka, **kg, **kb, ***kc; 
  double **a_left, **g_left, **b_left, ***c_left, *ett; 
  
  sampletype=0;
  fitting=1;
  NoOfObs=0; NoOfCensored=0; NoOfInt=0;
  SumOfWeights=0; SumOfCensored=0; SumOfInt=0;
  
  sampletype = AskForInput(NoOfInput);
  printf("\nNumber of phases of the PH-distribution to be fitted, (p): ");
  scanf("%d", &p); 
 
  pi=v_alloc(p); T=m_alloc(p, p); t=v_alloc(p);
  pilegal=int_v_alloc(p); Tlegal=int_m_alloc(p, p);
  avector=v_alloc(p);  bvector=v_alloc(p); cmatrix=m_alloc(p, p);  
  Bmean=v_alloc(p); Zmean=v_alloc(p); Nmean=m_alloc(p, p+1); 
  ka=m_alloc(4, p);  kb=m_alloc(4, p); kc=m3_alloc(4, p, p); 
  if (sampletype==2) {
    ett = v_alloc(p);
    for(i=0; i < p; i++)
      ett[i] = 1;
  }
  if(NoOfInt>0) {
    NoToSave = search_partner();
    gvector=v_alloc(p); kg=m_alloc(4,p);
    a_left=m_alloc(NoToSave,p); g_left=m_alloc(NoToSave,p);
    b_left=m_alloc(NoToSave,p); c_left=m3_alloc(NoToSave, p, p);
  }

  selectstructure(p, pi, T, t, pilegal, Tlegal);
  show_pi_T(p, pi, T, t);

  while (fitting == 1) {    
    printf("\nNumber of EM-iterations: ");
    scanf("%d", &NoOfEMsteps);
    EMiterate(NoOfEMsteps, p, pi, T, t, gvector, avector, bvector, cmatrix, 
      Bmean, Zmean, Nmean, kg, ka, kb, kc, ett, g_left, a_left, b_left,c_left);
    SavePhases(p, pi, T);
    
    printf("\nDo you want to:\n");
    printf("      1. do more iterations \n");
    printf("      2. start a new fit \n");
    printf("      3. quit \n");
    printf("Select 1-3: ");
    scanf("%d", &choice);
    switch(choice) {
    case 1:
      break;
    case 2:
      free(pi); free(t); free(pilegal);
      free_matrix(T, p);  free_integermatrix(Tlegal,p); 
      free(avector); free(bvector); free_matrix(cmatrix,p);
      free(Bmean); free(Zmean); free_matrix(Nmean, p);    
      free_matrix(ka, 4);  free_matrix(kb, 4); free_3dimmatrix(kc, 4, p); 
      if(NoOfInt>0) {
	free(gvector); free_matrix(kg, 4); 
	free_matrix(a_left, NoToSave); free_matrix(g_left, NoToSave);
	free_matrix(b_left, NoToSave); free_3dimmatrix(c_left, NoToSave, p); 
      }
      if(sampletype == 2)
	free(ett);    
      
      printf("Number of phases of the PH-distribution to be fitted, (p): ");
      scanf("%d", &newp);
      p = newp;
      pi=v_alloc(p); T=m_alloc(p, p);  t=v_alloc(p);
      pilegal=int_v_alloc(p); Tlegal=int_m_alloc(p, p);
      avector=v_alloc(p);  bvector=v_alloc(p); cmatrix=m_alloc(p, p);  
      Bmean=v_alloc(p); Zmean=v_alloc(p); Nmean=m_alloc(p, p+1); 
      ka=m_alloc(4, p);  kb=m_alloc(4, p); kc=m3_alloc(4, p, p); 
      if (sampletype==2) {
	ett = v_alloc(p);
	for(i=0; i < p; i++)
	  ett[i] = 1;
      }
      if(NoOfInt>0) {
	gvector=v_alloc(p); kg=m_alloc(4,p);
	a_left=m_alloc(NoToSave,p); g_left=m_alloc(NoToSave,p);
	b_left=m_alloc(NoToSave,p); c_left=m3_alloc(NoToSave, p, p);
      }
      selectstructure(p, pi, T, t, pilegal, Tlegal);
      show_pi_T(p, pi, T, t);
      break;
    case 3:
      fitting = 0;
      break;
    }
  }
  
  free(pi); free_matrix(T, p);  free(t); free(pilegal);
  free_integermatrix(Tlegal,p); 
  free(avector); free(bvector); free_matrix(cmatrix,p); 
  free_matrix(ka, 4);  free_matrix(kb, 4);  
  free_3dimmatrix(kc, 4, p);  free(Bmean); free(Zmean);
  free_matrix(Nmean, p);  
  if(NoOfInt>0) {
    free(upper); free(partner); free(gvector); free_matrix(kg, 4); 
    free_matrix(a_left, NoToSave);
    free_matrix(g_left, NoToSave);
    free_matrix(b_left, NoToSave);
    free_3dimmatrix(c_left, NoToSave, p); 
  }
  if(sampletype == 2)
    free(ett);      

}









