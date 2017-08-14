#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ARGS "f:"
char *Usage = "an-file -f filename\n";

int main(int argc, char **argv)
{
	int c;

	while((c = getopt(argc,argv,ARGS)) != EOF) {
		switch(c) {
			case 'f':
				strncpy(Fname,optarg,sizeof(Fname));
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

	exit(0);
}


