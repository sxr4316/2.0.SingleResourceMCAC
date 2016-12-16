
/*
 *
 *  @(#) encode.h 1.4@(#)
 *
 */

#define T

#define FILE_LINE 32
#define FOREVER	1
#define R	1
#define NOTR	0
#define QTBL_LEN 18
#define RTBL_LEN 16
#define MTBL_LEN 8

#ifndef T
#define RETURN(y)	return(y)
#define RETURNR(y,z)	
#define NAME(y)		
#define PRINTF		if(tst)printf
#else
#define PRINTF		printf
#define RETURN(y)	if(tst)printf("y = %x\n",y);return(y)
#define RETURNR(y,z)	if(tst)printf("y = %x, z = %x\n",y,z)
#define NAME(y)		if(tst)printf("y:    ");
#endif
#define PRNT(x) if(tst)printf("x = %d\t",x)

#define MODE_DECODER 0
#define MODE_ENCODER 1

struct tbl{
	long	ds;
	long	dln_lo;
	long	dln_hi;
	long	i;
	long	id;
	};

