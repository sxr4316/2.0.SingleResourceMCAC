#include "../include/encode.h"
#include <stdio.h>

#define BITS_SS   14
#define BITS_ALAW 1


long expand(long, int);

int main(void) {
	long expand_out = 0;
	int aLaw_ss = 0;
	int ss = 0;
	int aLaw = 0;
	int totalCounter = 0;

	int max_ss, max_aLaw;
	max_ss = (1 << BITS_SS);
	max_aLaw = (1 << BITS_ALAW);

	for (aLaw=0; aLaw < max_aLaw; aLaw++) {
		for (ss=0; ss < max_ss; ss++) {
			aLaw_ss = (aLaw << BITS_SS) | (ss);
			expand_out = expand(ss, aLaw);
			printf("%i,", expand_out);
			printf("\n");
			totalCounter++;
		}
	}
	printf("%i", totalCounter);
	return 0;
}

long
expand(ss, aLaw)
long	ss;
int	aLaw;
{
	long	sss, ssm, ssq, sl;

	if ( !aLaw ) {
		/* process u-law */
		sss = ss >> 13;
		ssq = ss & 8191;
	} else {
		/* process a-law */
		sss = ss >> 12;
		ssm = ss & 4095;
		ssq = ssm << 1;
	}
	sl = sss ? (16384 - ssq) & 16383 : ssq;
	return(sl);
}
