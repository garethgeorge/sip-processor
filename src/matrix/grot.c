
#ifdef STANDALONE

int main(int argc, char *argv[])
{
	int i;
	int j;
	Array2D *a;
	Array2D *b;
	Array2D *c;
	Array2D *d;
	Array2D *e;

	a = MakeArray2D(5,3);
	b = MakeArray2D(3,5);

	for(i=0; i < 3; i++)
	{
		for(j=0; j < 5; j++)
		{
			a->data[i*a->xdim+j] = 3.0;
			b->data[j*b->xdim+i] = 5.0;
		}
	}

	c = MultiplyArray2D(a,b);

	if(c == NULL)
	{
		printf("no c\n");
	}
	else
	{
		printf("a:\n");
		PrintArray2D(a);
		printf("b:\n");
		PrintArray2D(b);
		printf("c:\n");
		PrintArray2D(c);
	}

	for(i=0; i < c->ydim; i++)
	{
		for(j=0; j < c->xdim; j++)
		{
			c->data[i*c->xdim+j] += (3.0*drand48());
		}
	}

	d = InvertArray2D(c);
	if(d == NULL)
	{
		printf("c has no inverse\n");
		exit(1);
	}

	e = MultiplyArray2D(c,d);

	if(e == NULL)
	{
		printf("no e\n");
	}
	else
	{
		printf("c:\n");
		PrintArray2D(c);
		printf("d:\n");
		PrintArray2D(d);
		printf("e:\n");
		PrintArray2D(e);
	}

	FreeArray2D(a);
	FreeArray2D(b);
	FreeArray2D(c);
	FreeArray2D(d);
	FreeArray2D(e);

	return(0);
}

#endif

	

	



