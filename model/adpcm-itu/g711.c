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

extern struct tbl quan_tbl[];
extern long	recon_tbl[];
extern long	mult_tbl[];


/* decode pcm to sign and mangnitude */
long
g711(pcm, aLaw)
long	pcm;
int	aLaw;
{
	long	q, s, sgn, mag, bias;

	if ( !aLaw ) {
		/* process u-law pcm */
		if (debug) {
			printf("g711 aLaw: %06x\n", aLaw);
			printf("g711 pcm:  %06x\n", pcm);
		}
		pcm ^= 0xff;
		q = 0x0f & pcm;
		s = (0x70 & pcm) >> 4;
		sgn = 0x80 & pcm;
		bias = 33;

		mag = (((2 * q) + bias) << s) - bias;

		if (debug) {
			printf("g711 pcm:  %06x\n", pcm);
			printf("g711 q:    %06x\n", q);
			printf("g711 s:    %06x\n", s);
			printf("g711 sgn:  %06x\n", sgn);
			printf("g711 bias: %06x\n", bias);
			printf("g711 mag:  %06x(%6d)\n", mag, sgn ? ~mag + 1 : mag);
		}

		mag = (sgn ? mag | (1 << 13) : mag) & 16383;

		if (debug) {
			printf("g711 mag:  %06x\n\n", mag);
		}
	} else {
		/* process a-law pcm */
		if (debug) {
			printf("g711 aLaw: %06x\n", aLaw);
			printf("g711 pcm:  %06x\n", pcm);
		}
		pcm ^= 0x55;
		q = 0x0f & pcm;
		s = (0x70 & pcm) >> 4;
		sgn = 0x80 & pcm;

		bias = (s > 0) ? 0x00021 : 0x00001;

		mag = (2 * q) + bias;
		mag = (s > 1) ? mag << (s - 1) : mag;

		if (debug) {
			printf("g711 pcm:  %06x\n", pcm);
			printf("g711 q:    %06x\n", q);
			printf("g711 s:    %06x\n", s);
			printf("g711 sgn:  %06x\n", sgn);
			printf("g711 bias: %06x\n", bias);
			printf("g711 mag:  %06x(%6d)\n", mag, sgn ? mag : ~mag + 1);
		}

		mag = (sgn ? mag : mag | (1 << 12)) & 8191;

		if (debug) {
			printf("g711 mag:  %06x\n\n", mag);
		}
	}
	return(mag);
}

