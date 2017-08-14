#if !defined(FBUFF_H)
#define FBUFF_H

#define MODMINUS(a,b,m) ((a) - (b) + (((a) >= (b)) ? 0 : ((b) <= (m)) ? (m) : ((m) * (1 + (int)(b) / (int)(m)))))
#define MODPLUS(a,b,m)  ((a) + (b) - ((((m) - (b)) > (a)) ? 0 : ((b) <= (m)) ? (m) : ((m) * (1 + (int)(b) / (int)(m)))))

struct fbuff_stc
{
	int head;
	int tail;
	int size;
	double *vals;
	int empty;
};

typedef struct fbuff_stc *fbuff;

#define FBUFF_SIZE (sizeof(struct fbuff_stc))

#define F_HEAD(fb) ((fb)->vals[(fb)->head])
#define F_SIZE(fb) ((fb)->size)
#define F_FIRST(fb) MODPLUS(((fb)->head),1,(fb)->size)
#define F_LAST(fb) ((fb)->tail)
#define F_VAL(fb,i) ((fb)->vals[(i)])
#define F_COUNT(fb) ValidCount(fb)

#define IS_EMPTY(fb) (fb->empty) 

extern fbuff InitFBuff(int size);
extern void FreeFBuff(fbuff fb);
extern void UpdateFBuff(fbuff fb, double val);
extern void DeleteFBuffTail(fbuff fb);
extern int ValidCount(fbuff fb);

#endif
