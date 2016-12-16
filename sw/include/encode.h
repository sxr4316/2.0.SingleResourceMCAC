
/*
 *
 *  @(#) encode.h 1.4@(#)
 *
 */

#ifndef ENCODE_H
#define ENCODE_H
 
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

#define PRINT_VEC_LINE(Y,Z) printf("@%04X\t%06X\n", Y, Z)

struct tbl{
	long	ds;
	long	dln_lo;
	long	dln_hi;
	long	i;
	long	id;
	};

#ifdef CO_P
struct channel {
	long 
	//encoder
	a2p,aLaw,al,ap,app,apr,ax,
	br, br_2bit,
	d,dl,dln,dml,dmlp,dms,dmsp,dq,dql,dqln,dqs,ds,
	fi,
	I,
	pcm,
	se,sl,ss,
	td,tdp,tdr,tr,
	wi,
	y,yl,ylp,yu,yup,yut,
	//decoder
	sr, sp, sg, sd, slx, dx, dsx, dlx, dlnx, len;
};
#else
struct channel {
	long 
	//encoder
	a1,a1p,a1r,a1t,a2,a2p,a2r,a2t,aLaw,al,ap,app,apr,ax,
	b1,b1p,b1r,b2,b2p,b2r,b3,b3p,b3r,b4,b4p,b4r,b5,b5p,b5r,b6,b6p,b6r,br,br_2bit,
	d,dl,dln,dml,dmlp,dms,dmsp,dq,dq0,dq1,dq2,dq3,dq4,dq5,dq6,dql,dqln,dqs,ds,
	fi,
	I,
	pcm,pk0,pk1,pk2,
	se,sez,sigpk,sl,smdq,sr,sr0,sr1,sr2,ss,
	td,tdp,tdr,tr,
	u1,u2,u3,u4,u5,u6,
	wa1,wa2,wb1,wb2,wb3,wb4,wb5,wb6,wi,
	y,yl,ylp,yu,yup,yut,
	//decoder
	sp, sg, sd, slx, dx, dsx, dlx, dlnx, len;
};
#endif
#endif
// ENCODE_H
