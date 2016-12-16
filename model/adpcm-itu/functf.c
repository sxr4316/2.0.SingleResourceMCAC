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
functf(I)
long	I;
{
	long	is, fi;
	long	im;
	
	if(br==40)
	{
		is = I >> 4;
		im = is ? ((31 - I) & 15) : (I & 15);

		switch ((int)im) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			fi = 0;
			break;
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			fi = 0x1;
			break;
		case 10:
			fi = 0x2;
			break;
		case 11:
			fi = 0x3;
			break;
		case 12:
			fi = 0x4;
			break;
		case 13:
			fi = 0x5;
			break;
		case 14:
		case 15:
			fi = 0x6;
			break;
		default:
			printf("default found funcf\n");
		}
	}
	else if(br==32)
	{
		is = I >> 3;
		im = is ? ((15 - I) & 7) : (I & 7);

		switch ((int)im) {
		case 0:
		case 1:
		case 2:
			fi = 0;
			break;
		case 3:
		case 4:
		case 5:
			fi = 0x1;
			break;
		case 6:
			fi = 0x3;
			break;
		case 7:
			fi = 0x7;
			break;
		default:
			printf("default found funcf\n");
		}
	}
	else if(br==24)
	{
		is = I >> 2;
		im = is ? ((7 - I) & 3) : (I & 3);

		switch ((int)im) {
		case 0:
			fi = 0;
			break;
		case 1:
			fi = 0x1;
			break;
		case 2:
			fi = 0x2;
			break;
		case 3:
			fi = 0x7;
			break;
		default:
			printf("default found funcf\n");
		}
	}
	else if(br==16)
	{
		is = I >> 1;
		im = is ? ((3 - I) & 1) : (I & 1);

		switch ((int)im) {
		case 0:
			fi = 0;
			break;
		case 1:
			fi = 0x7;
			break;
		default:
			printf("default found funcf\n");
		}
	}
	return(fi);
}
