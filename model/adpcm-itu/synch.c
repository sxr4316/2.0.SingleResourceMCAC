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

extern int br;

long
synch(I, sp, dlnx, dsx, aLaw)
long	I, sp, dlnx, dsx;
int aLaw;
{
	long	i, is, im, id, sd;

	if (br == 40)
	{
		is = I >> 4;
		im = is ? (I & 15) : (I + 16);

		for (i = 0; i <= QTBL_40_LEN ; i++)
		{
			if ((dsx == quanbr40_tbl[i].ds) && (dlnx >= quanbr40_tbl[i].dln_lo) && (dlnx <= quanbr40_tbl[i].dln_hi))
			{
				id = quanbr40_tbl[i].id;
				break;
			}
			if (i == QTBL_40_LEN)
			{
				printf("id not set in synch\n");
			}
		}
	}
	else if (br == 32)
	{
		is = I >> 3;
		im = is ? (I & 7) : (I + 8);

		for (i = 0; i <= QTBL_32_LEN ; i++)
		{
			if ((dsx == quanbr32_tbl[i].ds) && (dlnx >= quanbr32_tbl[i].dln_lo) && (dlnx <= quanbr32_tbl[i].dln_hi))
			{
				id = quanbr32_tbl[i].id;
				break;
			}
			if (i == QTBL_32_LEN)
			{
				printf("id not set in synch\n");
			}
		}
	}
	else if (br == 24)
	{
		is = I >> 2;
		im = is ? (I & 3) : (I + 4);

		for (i = 0; i <= QTBL_24_LEN ; i++)
		{
			if ((dsx == quanbr24_tbl[i].ds) && (dlnx >= quanbr24_tbl[i].dln_lo) && (dlnx <= quanbr24_tbl[i].dln_hi))
			{
				id = quanbr24_tbl[i].id;
				break;
			}
			if (i == QTBL_24_LEN)
			{
				printf("id not set in synch\n");
			}
		}
	}
	else if (br == 16)
	{
		is = I >> 1;
		im = is ? (I & 1) : (I + 2);

		for (i = 0; i <= QTBL_16_LEN ; i++)
		{
			if ((dsx == quanbr16_tbl[i].ds) && (dlnx >= quanbr16_tbl[i].dln_lo) && (dlnx <= quanbr16_tbl[i].dln_hi))
			{
				id = quanbr16_tbl[i].id;
				break;
			}
			if (i == QTBL_16_LEN)
			{
				printf("id not set in synch\n");
			}
		}
	}

	if (debug) {
		printf("synch aLaw: %06x\n", aLaw);
		printf("synch I:    %06x\n", I);
		printf("synch is:   %06x\n", is);
		printf("synch im:   %06d\n", im);
		printf("synch sp:   %06x\n", sp);
		printf("synch dlnx: %06d\n", dlnx);
		printf("synch dsx:  %06x\n", dsx);
		printf("synch id:   %06d\n", id);
	}

	if ( !aLaw ) {
		/* u-law pcm */
		if (id == im) 
			sd = sp;
		else
		{
			switch ((int)sp) {
			case 0x80:
				if (id > im) 
					sd = sp + 1;
				else 
					sd = sp;
				break;
			case 0xff:
				if (id > im) 
					sd = 0x7e;
				else 
					sd = sp - 1;
				break;
			case 0x00:
				if (id > im) 
					sd = sp;
				else 
					sd = sp + 1;
				break;
			case 0x7f:
				if (id > im) 
					sd = sp - 1;
				else 
					sd = 0xfe;
				break;
			default:
				if (sp & 0x80) {
					if (id > im) 
						sd = sp + 1;
					else 
						sd = sp - 1;
				} else
				 {
					if (id > im) 
						sd = sp - 1;
					else 
						sd = sp + 1;
				}
				break;
			}
		}
	} else {
		/* a-law pcm */
		sp ^= 0x55 ;
		if (id == im) {
			sd = sp;
		} else {
			switch (sp) {
			case 0xff:	/* + max */
				if (id > im) 
					sd = 0xFE;
				else 
					sd = sp;
				break;
			case 0x80:	/* + 0 */
				if (id > im) 
					sd = 0x00;
				else 
				//	sd = sp - 1;
					sd = 0x81;
				break;
			case 0x00:	/* - 0 */
				if (id > im)
					sd = 0x01;
				else
					sd = 0x80;
				break;
			case 0x7F:	/* - max */
				if (id > im) 
					sd = sp;
				else 
					sd =0x7E ;
				break;
			default:
				if (sp & 0x80) {
					if (id > im) 
						sd = sp - 1;
					else 
						sd = sp + 1;
				} else {
					if (id > im) 
						sd = sp + 1;
					else 
						sd = sp - 1;
				}
				break;
			}
		}
		sd ^= 0x55 ;
	}

	sd = sd & 0xFF;
	return(sd);
}

