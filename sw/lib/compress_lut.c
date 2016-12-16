#include "../include/encode.h"
#include <stdio.h>

extern long inv_g711();

#define VARS_PER_LINE 16

#define BITS_ALAW 1
#define BITS_BR 2
#define BITS_SRI 16

long compress(long, long, long);

int main(void) {
	long max_compress = 0;
	long compress_out = 0;
	int aLaw_br_sri = 0;
	int aLaw = 0;
	int br_2bit = 0;
	int sri = 0;
	int totalCounter = 0;

	int max_aLaw, max_br, max_sri;
	max_aLaw = (1 << BITS_ALAW);
	max_br = (1 << BITS_BR);
	max_sri = (1 << BITS_SRI);

	for (aLaw=0; aLaw < max_aLaw; aLaw++) {
		for (br_2bit=0; br_2bit < max_br; br_2bit++) {
			for (sri=0; sri < max_sri; sri++) {
				aLaw_br_sri = (aLaw << BITS_SRI+BITS_BR) | (br_2bit << BITS_SRI) | (sri);
				compress_out = compress(aLaw, br_2bit, sri);
				max_compress = (compress_out > max_compress ? compress_out : max_compress);
				printf("%i,", compress_out);
				totalCounter++;
				if (totalCounter != 0 && totalCounter % VARS_PER_LINE == 0){
					printf("\n");
				}
			}
		}
	}
	printf("%i", totalCounter);
	printf("max value: %X", max_compress);
	return 0;
}

long
compress(long aLaw, long br, long sri)
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
		if (br == 0)
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

	sp = sp & 0xFF;
	return(sp);
}
