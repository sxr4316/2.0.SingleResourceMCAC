#include <stdio.h>
#include <stdlib.h>
#include "encode.h"


extern int debug;

extern long yu;
extern long yl;
extern long dms;
extern long dml;
extern long ap;
extern long pk2;
extern long pk1;
extern long sr2;
extern long sr1;
extern long b1;
extern long b2;
extern long b3;
extern long b4;
extern long b5;
extern long b6;
extern long dq6;
extern long dq5;
extern long dq4;
extern long dq3;
extern long dq2;
extern long dq1;
extern long a2;
extern long a1;
extern long td;
extern long sez;
extern long se;
extern long sd;
extern long pcm;

long	inv_g711(), g711(), expand(), subta(), subtb();
long	quan(), adda(), antilog();
long	filtd(), functw();
long	filte(), upa2();
long	mix(), functf(), addb(), addc();
long	floata(), floatb(), fmult();
long	limc(), limb(), limd(), upa1(), calcbn();
long	upa1(), upb(), xor(), lima(), subtc();
long	filta(), filtb(), filtc();
long	compress(), synch();
long	adapt_pred(), inv_adapt_quan(), adapt_quan();
long	quan_scl_fact(), adapt_speed(), output_conv();
long	getSample();

extern struct tbl quan_tbl[];
extern long	recon_tbl[];
extern long	mult_tbl[];
extern int br;


long
upb(un, bn, dq)
long	un, bn, dq;
{
	long	ugbn, bns, ulbn, ubn, bnp, dqmag;
	dqmag = dq & 32767;

	if ((un == 0) && (dqmag != 0))
		ugbn = 128;
	else if (un && (dqmag != 0))
		ugbn = 65408 ;
	else
		ugbn = 0;	
	bns = bn >> 15;
	if (br < 40)
	{
		ulbn = bns ? (65536 - ((bn >> 8) + 65280)) & 65535 
	    	 : (65536 - (bn >> 8)) & 65535;
	}
	else
	{
		ulbn = bns ? (65536 - ((bn >> 9) + 65408)) & 65535 
	    	 : (65536 - (bn >> 9)) & 65535;
	}

	ubn = (ugbn + ulbn) & 65535;
	bnp = (bn + ubn) & 65535;
	return(bnp);
}

