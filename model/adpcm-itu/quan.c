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

extern struct tbl quanbr32_tbl[];
extern struct tbl quanbr40_tbl[];
extern struct tbl quanbr24_tbl[];
extern struct tbl quanbr16_tbl[];
extern long	recon_tbl[];
extern long	mult_tbl[];
extern int br;

long
quan(dln, ds)
long dln, ds;
{
	int	i;
    long I;

	if (br==40)
	{
		for (i = 0; i <= QTBL_40_LEN; i++)
		{
			I = quanbr40_tbl[i].i;
			if ((dln >= quanbr40_tbl[i].dln_lo) && (dln <= quanbr40_tbl[i].dln_hi) && (ds == quanbr40_tbl[i].ds))
			{
				return(I);
			}
		}
		printf("no match in quan\n");
		return(I);
	}
	else if(br==32)
	{
		for (i = 0; i <= QTBL_32_LEN; i++)
		{
			I = quanbr32_tbl[i].i;
			if ((dln >= quanbr32_tbl[i].dln_lo) && (dln <= quanbr32_tbl[i].dln_hi) && (ds == quanbr32_tbl[i].ds))
			{
				return(I);
			}
		}
		printf("no match in quan\n");
		return(I);
	}
	else if (br==24)
	{
	  for (i = 0; i <= QTBL_24_LEN; i++)
	  {
			I = quanbr24_tbl[i].i;
			if ((dln >= quanbr24_tbl[i].dln_lo) && (dln <= quanbr24_tbl[i].dln_hi) && (ds == quanbr24_tbl[i].ds))
			{
				return(I);
			}
		}
		printf("no match in quan\n");
		return(I);
	}
	else if (br==16)
	{
	  for (i = 0; i <= QTBL_16_LEN; i++)
	  {
			I = quanbr16_tbl[i].i;
			if ((dln >= quanbr16_tbl[i].dln_lo) && (dln <= quanbr16_tbl[i].dln_hi) && (ds == quanbr16_tbl[i].ds))
			{
				return(I);
			}
		}
		printf("no match in quan\n");
		return(I);
	}
	else
	{
		printf("Invalid bitrate: %i!\n", br);
		exit(0);
		return 0;
	}
}