#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "textlist.h"
#include "convert_time.h"
#include "mymalloc.h"
#include "mio.h"
#include "an-file.h"


MIO *LoadRawData(char *fname,char *field_list)
{
	int *fields;
	TXL *fl;
	TXL *rxl;
	DlistNode *dxl;
	int field_count;
	int i;
	int j;
	int k;
	int m;
	MIO *raw_mio;
	MIO *db_mio;
	char *data;
	char **db;
	unsigned long raw_size;
	unsigned long recs;
	int curr_f;
	int fcount;
	char line[4096];
	char *str;

	fl = ParseLine(field_list,", ");
	if(fl == NULL) {
		return(NULL);
	}

	field_count = 0;
	DLIST_FORWARD(fl->list,dxl) {
		field_count++;
	}

	if(field_count == 0) {
		DestroyTXL(fl);
		return(NULL);
	}

	fields = (int *)Malloc(field_count*sizeof(int));
	if(fields == NULL) {
		DestroyTXL(fl);
		return(NULL);
	}

	i=0;
	DLIST_FORWARD(fl->list,dxl) {
		fields[i] = atoi(dxl->value.s);
		i++;
	}
	DestroyTXL(fl);

	raw_size = MIOSize(fname);
	if(raw_size <= 0) {
		DestroyTXL(fl);
		free(fields);
		return(NULL);
	}

	raw_mio = MIOOpenText(fname,"r",raw_size);
	if(raw_mio == NULL) {
		DestroyTXL(fl);
		free(fields);
		return(NULL);
	}

	recs = MIOTextRecords(raw_mio);

	db_mio = MIOMalloc(recs*field_count*sizeof(char *));
	if(db_mio == NULL) {
		MIOClose(raw_mio);
		free(fields);
		return(NULL);
	}

	j=0;
	data = (char *)MIOAddr(raw_mio);
	db = MIOAddr(db_mio);
	for(i=0; i < recs; i++) {
		memset(line,0,sizeof(line));
		k = 0;
		while((k < sizeof(line)) && 
				(data[j] != 0) && (data[j] != '\n')) {
			line[k] = data[j];
			k++;
			j++;
		}
		rxl = ParseLine(line,"\n\t");
		if(rxl == NULL) {
			j++;
			continue;
		}
		fcount = 0;
		curr_f = 0;
		DLIST_FORWARD(rxl->list,dxl) {
			for(m=0; m < field_count; m++) {
				if(fields[m] == fcount) {
					str = Malloc(strlen(dxl->value.s)+1);
					if(str == NULL) {
						continue;
					}
					strcpy(str,dxl->value.s);
					db[i*field_count+curr_f] = str;
					curr_f++;
				}
			}
			fcount++;
		}
		while((data[j] != 0) && (data[j] == '\n')) {
			j++;
		}
		DestroyTXL(rxl);
	}

	free(fields);
	MIOClose(raw_mio);
	db_mio->recs = recs;
	db_mio->fields = field_count;
	return(db_mio);
}

#define TIMESIZE (11)
int ConvertTimeStamp(MIO *db, int time_field)
{
	unsigned long i;
	int j;
	char ts_buf[4096];
	char *new_ts;
	char *ts;
	char **data;
	int err;
	double dtime;

	data = (char **)MIOAddr(db);

	err = 1;
	for(i=0; i < db->recs; i++) {
		ts=data[i*db->fields+time_field];
		j = 0;
		/*
		 * punch the T witha  space
		 */
		while((ts[j] != 0) && (ts[j] != 'T')) {
			j++;
		}
		if(ts[j] == 0) {
			err = -1;
			continue;
		}
		ts[j] = ' ';
		/*
		 * drop the Z
		 */
		while((ts[j] != 0) && (ts[j] != 'Z')) {
			j++;
		}
		if(ts[j] == 0) {
			err = -1;
			continue;
		}
		ts[j] = ' ';
		dtime = ConvertTimeStringNew(ts);
		new_ts = Malloc(TIMESIZE);
		if(new_ts == NULL) {
			err = -1;
			continue;
		}
		sprintf(new_ts,"%10.0f",dtime);
		/*
		 * replace
		 */
		data[i*db->fields+time_field] = new_ts;
		Free(ts);
	}

	return(err);
}


void PrintMIODB(MIO *db)
{
	unsigned long i;
	int j;
	char **data;

	data = (char **)MIOAddr(db);
	for(i=0; i < db->recs; i++) {
		for(j=0; j < db->fields; j++) {
			printf("%s ",data[i*db->fields+j]);
		}
		printf("\n");
	}

	return;
}


MIONDX *MIONDXInit(MIO *db, int key_field, char *select)
{
	RB *index;
	unsigned long i;
	int j;
	double dkey;
	char **data;
	MIONDX *miondx;
	int found;
	
	index = RBInitD();
	if(index == NULL) {
		return(NULL);
	} 

	miondx = (MIONDX *)Malloc(sizeof(MIONDX));
	if(miondx == NULL) {
		RBDestroyD(index);
		return(NULL);
	}
	miondx->select = (char *)Malloc(strlen(select)+1);
	if(miondx->select == NULL) {
		RBDestroyD(index);
		Free(miondx);
		return(NULL);
	}

	strcpy(miondx->select,select);
	miondx->key_field = key_field;
	miondx->db = db;

	data = MIOAddr(db);
	for(i=0; i < db->recs; i++) {
		found = 1;
		if(select != NULL) {
			found = 0;
			for(j=0; j < db->fields; j++) {
				if(strcmp(data[i*db->fields+j],select) == 0) {
					found = 1;
					break;
				}
			}
		}
		if(found == 1) {
			dkey = atof(data[i*db->fields+key_field]);
			RBInsertD(index,dkey,(Hval)i);
		}
	}

	miondx->index = index;
		
	return(miondx);
}

MIONDX *MIONDXSubNdx(MIONDX *ndx, int field, char *select)
{
	RB *rb;
	int fields;
	char **array;
	unsigned long i;
	char *target;
	MIONDX *newndx;
	int found;
	MIO *db;

	newndx = (MIONDX *)Malloc(sizeof(MIONDX));
	if(newndx == NULL) {
		return(NULL);
	}

	db = ndx->db;

	fields = db->fields;
	array = (char **)MIOAddr(db);

	newndx->index = RBInitD();
	if(newndx->index == NULL) {
		MIONDXFree(newndx);
		return(NULL);
	}
	found = 0;
	RB_FORWARD(ndx->index,rb) {
		i = rb->value.l;
		target = array[i*fields+field];
		if(strcmp(target,select) == 0) {
			found = 1;
			RBInsertD(newndx->index,K_D(rb->key),(Hval)i);
		}
	}

	if(found == 0) {
		MIONDXFree(newndx);
		return(NULL);
	}

	newndx->select = (char *)Malloc(strlen(select)+1);
	if(newndx->select == NULL) {
		MIONDXFree(newndx);
		return(NULL);
	}
	strcpy(newndx->select,select);

	newndx->key_field = field;
	newndx->db = db;

	return(newndx);
}
		

void MIONDXFree(MIONDX *miondx)
{
	if(miondx->index != NULL) {
		RBDestroyD(miondx->index);
	}
	if(miondx->select != NULL) {
		Free(miondx->select);
	}
	Free(miondx);
}

void MIONDXPrint(MIONDX *miondx)
{
	RB *rb;
	unsigned long i;
	int j;
	char **data;
	int kf;
	int fields;

	data = MIOAddr(miondx->db);
	kf = miondx->key_field;
	fields=miondx->db->fields;
	RB_FORWARD(miondx->index,rb) {
		i = rb->value.l;
		printf("%s ",data[i*fields+kf]);
		for(j=0; j<fields; j++) {
			if(j == kf) {
				continue;
			}
			printf("%s ",data[i*fields+j]);
		}
		printf("\n");
	}

	return;
}
		
		

	

#ifdef STANDALONE

#define ARGS "f:"
char *Usage = "an-file -f filename\n";

char Fname[4096];

int main(int argc, char **argv)
{
	int c;
	MIO *db;
	int err;
	MIONDX *miondx;
	MIONDX *subndx;

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

	db = LoadRawData(Fname,"1,2,4,5");
	err = ConvertTimeStamp(db,3);
//	PrintMIODB(db);

	miondx = MIONDXInit(db,3,"c1.medium");
	if(miondx == NULL) {
		fprintf(stderr,"couldn't create m1.large index\n");
		MIOClose(db);
		exit(1);
	}

	subndx = MIONDXSubNdx(miondx,0,"us-west-2a");
	if(subndx == NULL) {
		fprintf(stderr,"no subndx for us-west-2a\n");
		exit(1);
	}

	MIONDXPrint(subndx);
	MIONDXFree(miondx);
	MIONDXFree(subndx);

	MIOClose(db);

	exit(0);
}

#endif
