#ifndef AN_FILE_H
#define AN_FILE_H

#include "redblack.h"
#include "mio.h"

struct index_db_stc
{
	MIO *db;
	RB *index;
	int key_field;
	char *select;
};

typedef struct index_db_stc MIONDX;

MIO *LoadRawData(char *fname,char *field_list);
int ConvertTimeStamp(MIO *db, int time_field);
void PrintMIODB(MIO *db);
MIONDX *MIONDXInit(MIO *db, int key_field, char *select);
MIONDX *MIONDXSubNdx(MIONDX *ndx, int field, char *select);
void MIONDXFree(MIONDX *miondx);
void MIONDXPrint(MIONDX *miondx);


#endif

