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
upa1(pk01, pk11, a11, sigpk1)
long	pk01, pk11, a11, sigpk1;
{
	long	pks, uga1, a1s, ula1, ua1, a1t;

	pks = pk01 ^ pk11;
	if ((pks == 0) && (sigpk1 == 0))
		uga1 = 192;
	else if ((pks == 1) && (sigpk1 == 0))
		uga1 = 65344;
	else
		uga1 = 0;
	a1s = (a11 >> 15) ;
	ula1 = a1s ? (65536 - (( a11 >> 8) + 65280)) & 65535
	     : (65536 - (a11 >> 8)) & 65535;
	ua1 = (uga1 + ula1) & 65535;
	a1t = ( a11 + ua1) & 65535;
	return(a1t);
}

