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
upa2(pk01, pk11, pk21, a11, a21, sigpk1)
long	pk01, pk11, pk21, a11, a21, sigpk1;
{
	long	pks1, pks2, uga2a, a1s, fa1, fa, uga2s, uga2b, a2s, ula2, uga2, ua2, a2t;

	pks1 = pk01 ^ pk11;
	pks2 = pk01 ^ pk21;
	/* 114688 */
	uga2a = pks2 ? 114688 : 16384;
	a1s = (a11 >> 15) ;
	if (!a1s) 
		fa1 = (a11 >= 8192) ? 8191 << 2 : a11 << 2;
	else 
		fa1 = (a11 <= 57344) ? 24577 << 2 : (a11 << 2) & 131071;
	fa = pks1 ? fa1  : (131072 - fa1) & 131071;
	uga2b = (uga2a + fa) & 131071;
	uga2s = (uga2b >> 16) ;
	if ((uga2s == 0) && (sigpk1 == 0))
		uga2 = uga2b >> 7;
	else if ((uga2s == 1) && (sigpk1 == 0))
		uga2 = (uga2b >> 7) + 64512;
	else
		uga2 = 0;
	a2s = (a21 >> 15) ;
	ula2 = a2s ? (65536 - ((a21 >> 7) + 65024)) & 65535
	     : (65536 - (a21 >> 7)) & 65535;
	ua2 = (uga2 + ula2) & 65535;
	a2t = (a21 + ua2) & 65535;
	return(a2t);
}

