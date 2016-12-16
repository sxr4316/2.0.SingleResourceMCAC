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
long	floata(), floatb();
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
fmult(x, y1, test)
long	x, y1;
int	test;
{
	long	xs, xmag, xexp, xmant;
	long	ys, yexp, ymant;
	long	wxs, wx, wxexp, wxmant, wxmag, i;

	xs = (x >> 15);
	xmag = xs ? (16384 - (x >> 2)) & 8191 : x >> 2;
	xexp = 0;
	for (i = 0; i <= 13; i += 1) {
		if (!(xmag >> i)) {
			xexp = i;
			break;
		}
		if (i == 13)
			printf("mag didn't get set in fmult\n");
	}
	xmant = xmag ? (xmag << 6) >> xexp : 1 << 5;

	ys = (y1 >> 10) ;
	yexp = (y1 >> 6) & 15;
	ymant = y1 & 63;
/*

    if (test)
    {
    printf("bs = %x  bexp = %x   bmant = %x\n",xs,xexp,xmant);
    printf("ds = %x  dexp = %x   dmant = %x\n",ys,yexp,ymant);
    }
*/

	wxs = ys ^ xs;
	wxexp = yexp + xexp;
	wxmant = ((ymant * xmant) + 48) >> 4;
	wxmag = wxexp > 26 ? ((wxmant << 7) << (wxexp - 26)) & 32767
	     : (wxmant << 7 ) >> ( 26 - wxexp);
	wx = wxs ? (65536 - wxmag) & 65535 : wxmag ;
/*
    if (test)
	printf("wb = %x\n",wx);
*/
	return(wx);
}


