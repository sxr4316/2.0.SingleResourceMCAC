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

long
trans(td1, yl1, dq)
long	td1, yl1, dq;
{
	long	tr, dqmag, ylint, ylfrac, thr1, thr2, dqthr;
	dqmag = dq & 32767;
	ylint = (yl1 >> 15) ;	
	ylfrac = (yl1 >> 10) & 31;
	thr1 = (32 + ylfrac) << ylint;

	thr2 = ylint > 9 ? 31 << 10 : thr1;
		

	dqthr = (thr2 + (thr2 >> 1)) >> 1;
	tr = (dqmag > dqthr) && (td1 == 1) ? 1 : 0;

	if (debug) {
		printf("trans td1:    %06x\n", td1);
		printf("trans yl1:    %06x\n", yl1);
		printf("trans dq:     %06x\n", dq);
		printf("trans dqmag:  %06x\n", dqmag);
		printf("trans ylint:  %06x\n", ylint);
		printf("trans ylfrac: %06x\n", ylfrac);
		printf("trans thr1:   %06x\n", thr1);
		printf("trans thr2:   %06x\n", thr2);
		printf("trans dqthr:  %06x\n", dqthr);
		printf("trans tr:     %06x\n\n", tr);
	}
	return(tr);
}

