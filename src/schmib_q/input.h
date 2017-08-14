#define BUFSIZE (50000)
#define DELEMENTS (2)

#define MODMINUS(a,b,m) ((a) - (b) + (((a) >= (b)) ? 0 : ((b) <= (m)) ? (m) : ((m) * (1 + (int)(b) / (int)(m)))))
#define MODPLUS(a,b,m)  ((a) + (b) - ((((m) - (b)) > (a)) ? 0 : ((b) <= (m)) ? (m) : ((m) * (1 + (int)(b) / (int)(m)))))

struct data_set
{
	int circular;
        char name[255];
        FILE *fd;
        int head;
        int tail;
	int curr;
	int last;
        int wrap;
	int size;
	int diff;
	int logflag;
	double bias;
	double *dbuf1;
	double *dbuf2;
        double *data;
	int data_size;
};     

#define TS_i (0)
#define VAL_i (1)

#define TS(data,i) ((data)[(i)*DELEMENTS + TS_i])
#define VAL(data,i) ((data)[(i)*DELEMENTS + VAL_i])

#define DUMMY_TS (9999.99)

extern int InitData(char *fname, int diff, double bias, int logflag, 
		char **cookie);
extern int InitDataSet(int diff, double bias, char **cookie);
extern void SetBias(char *icookie, double bias);
extern void SetDataSize(char *icookie, int size);
extern int CopyDataSet(char *src, char *dst, int elements);
extern void RemoveData(char *cookie);
extern int Rewind(char *icookie);
extern int Seek(char *icookie, int element);
extern int ReadEntry(char *icookie, double *ts, double *value);
extern int ReadValue(char *icookie, double *value);
extern int WriteEntry(char *icookie, double ts, double value);
extern int SizeOf(char *cookie);
extern char *CopyData(char *cookie);
