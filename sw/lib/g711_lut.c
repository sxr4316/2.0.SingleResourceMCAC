/* decode pcm to sign and mangnitude */
#include "../include/encode.h"
#include <stdio.h>

#define BITS_PCM 8
#define BITS_ALAW 1


long g711(long, int);

int main(void) {
	long g711_out = 0;
	int aLaw_pcm = 0;
	int pcm = 0;
	int aLaw = 0;
	int totalCounter = 0;

	int max_pcm, max_aLaw;
	max_pcm = (1 << BITS_PCM);
	max_aLaw = (1 << BITS_ALAW);

	for (aLaw=0; aLaw < max_aLaw; aLaw++) {
		for (pcm=0; pcm < max_pcm; pcm++) {
			aLaw_pcm = (aLaw << BITS_PCM) | (pcm);
			g711_out = g711(pcm, aLaw);
			printf("%i,", g711_out);
			printf("\n");
			totalCounter++;
		}
	}
	printf("%i", totalCounter);
	return 0;
}

long
g711(pcm, aLaw)
long	pcm;
int	aLaw;
{
	long	q, s, sgn, mag, bias;

	if ( !aLaw ) {
		/* process u-law pcm */
		pcm ^= 0xff;
		q = 0x0f & pcm;
		s = (0x70 & pcm) >> 4;
		sgn = 0x80 & pcm;
		bias = 33;

		mag = (((2 * q) + bias) << s) - bias;
		mag = (sgn ? mag | (1 << 13) : mag) & 16383;

	} else {
		/* process a-law pcm */
		pcm ^= 0x55;
		q = 0x0f & pcm;
		s = (0x70 & pcm) >> 4;
		sgn = 0x80 & pcm;

		bias = (s > 0) ? 0x00021 : 0x00001;

		mag = (2 * q) + bias;
		mag = (s > 1) ? mag << (s - 1) : mag;
		mag = (sgn ? mag : mag | (1 << 12)) & 8191;
	}
	return(mag);
}
