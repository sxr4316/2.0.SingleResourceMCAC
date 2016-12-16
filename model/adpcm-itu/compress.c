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
compress(sri, aLaw)
long	sri;
int	aLaw;
{
	long	is, im, imag, sp;

	is = (sri >> 15);
	im = is ? ((65536 - sri) & 32767) : (sri);

	if ( !aLaw )
	{
		/* process u-pcm */
		imag = im;
	}
	else
	{
		/* process a-pcm */
		if (br == 40)
		{
			im = ( sri == 0x8000) ? 2 : im;
		}
		else
		{
			im=(sri > 65535) ? 2 : im;
		}

		imag = is ? ((im + 1) >> 1) : (im >> 1);
		if(is)
			--imag;
		if (imag > 4095)
			imag = 4095;
	}

	sp = inv_g711(is, imag, aLaw);

	if (debug) {
		printf("compress aLaw: %06x\n", aLaw);
		printf("compress is:   %06x\n", is);
		printf("compress imag: %06x\n", imag);
		printf("compress sp:   %06x\n\n", sp);
	}

	sp = sp & 0xFF;
	return(sp);
}

