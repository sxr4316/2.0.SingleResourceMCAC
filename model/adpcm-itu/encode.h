
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

// Original Lengths
#define QTBL_LEN 18
#define RTBL_LEN 16
#define MTBL_LEN 8

// New lengths
#define QTBL_40_LEN 34
#define QTBL_32_LEN 18
#define QTBL_24_LEN 10
#define QTBL_16_LEN 6

#define RTBL_40_LEN 32
#define RTBL_32_LEN 16
#define RTBL_24_LEN 8
#define RTBL_16_LEN 4

#define MTBL_40_LEN 16
#define MTBL_32_LEN 8
#define MTBL_24_LEN 4
#define MTBL_16_LEN 2

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

