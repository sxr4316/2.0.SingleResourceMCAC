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
extern long     reconbr32_tbl[];
extern long     reconbr40_tbl[];
extern long     reconbr24_tbl[];
extern long     reconbr16_tbl[];
extern long	mult_tbl[];
extern int br;

reconst(I, dqln, dqs)
long	I, *dqln, *dqs;
{
    if(br==40)
    {
       *dqln = reconbr40_tbl[I];
       *dqs  = I >> 4;
    }
    else if (br==32)
    {
       *dqln = reconbr32_tbl[I];
       *dqs  = I >> 3;
    }
    else if(br==24)
    {
      *dqln = reconbr24_tbl[I];
      *dqs  = I >> 2;
    }
    else if(br==16)
    {
      *dqln = reconbr16_tbl[I];
      *dqs  = I >> 1;
    }
}
