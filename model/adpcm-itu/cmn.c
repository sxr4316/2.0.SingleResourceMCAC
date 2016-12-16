/*
 *
 * common routines shared between encoder and decoder
 *
 */


/*
 *
 *	@(#) cmn.c 3.3@(#)
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
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

long	expand(), subta(), subtb();
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
getSample( fpin )
FILE *fpin;
{
	char	buff[3];
	long	val;

	buff[2] = (char)NULL;
	buff[0] = fgetc(fpin);
	if ( !isxdigit(buff[0]))
		buff[0] = fgetc(fpin);
	buff[1] = fgetc(fpin);
	if ( !isxdigit(buff[1]))
		buff[1] = fgetc(fpin);
	sscanf( buff, "%x", &val );

	return( val );
}

void
dumpState(i, aLaw, mode)
int i;
int	aLaw;
int	mode;
{

printf("\n=====================\n\n");
printf("cnt:   %6d\n", i);
if (mode)
	printf("mode:  %6s\n", "ENC");
else
	printf("mode:  %6s\n", "DEC");
if (aLaw)
	printf("Law:   %6s\n", "a-law");
else
	printf("Law:   %6s\n", "u-law");
printf("sez:   %06x\n", sez);
printf("se:    %06x\n", se);
printf("yu:    %06x\n", yu);
printf("yl:    %06x\n", yl);
printf("dms:   %06x\n", dms);
printf("dml:   %06x\n", dml);
printf("ap:    %06x\n", ap);
printf("pk2:   %06x\n", pk2);
printf("pk1:   %06x\n", pk1);
printf("sr2:   %06x\n", sr2);
printf("sr1:   %06x\n", sr1);
printf("b1:    %06x\n", b1);
printf("b2:    %06x\n", b2);
printf("b3:    %06x\n", b3);
printf("b4:    %06x\n", b4);
printf("b5:    %06x\n", b5);
printf("b6:    %06x\n", b6);
printf("dg6:   %06x\n", dq6);
printf("dg5:   %06x\n", dq5);
printf("dg4:   %06x\n", dq4);
printf("dg3:   %06x\n", dq3);
printf("dg2:   %06x\n", dq2);
printf("dg1:   %06x\n", dq1);
printf("a2:    %06x\n", a2);
printf("a1:    %06x\n", a1);
printf("td:    %06x\n", td);
printf("sd:    %06x\n", sd);
printf("pcm:   %06x\n", pcm);
printf("\n=====================\n\f\n");

}

void dumpPCM(long pcm, long i, long aLaw)
{
	long ss, is, imag, npcm, tc;

	ss = g711(pcm, aLaw);   /* expand 8 bits to 14/13 bit */
	is = aLaw ? ss >> 12 : ss >> 13 ;
	imag = aLaw ? (ss & 4095) : (ss & 8191) ;
	npcm = inv_g711(is, imag, aLaw);    /* compress 14/13 bit to 8 bits */
	tc = is ? (~imag + 1) : imag ;
	if (aLaw) {
		fprintf( stderr, "%8d i:%02x(^%02x)  do:%04x(%5d)  ro:%02x(^%02x)\n",
		i, pcm, pcm ^ 0x55, ss, tc, npcm, npcm ^ 0x55 );
	} else {
		fprintf( stderr, "%8d i:%02x  do:%04x(%5d)  ro:%02x\n", i, pcm, ss, tc, npcm );
	}
}