#include "../include/encode.h"
#include <stdio.h>

#define BITS_D 	16
#define VARS_PER_LINE 32

long llog(int);

int main(void) {
	long llog_out = 0;
	int d = 0;
	int totalCounter = 0;

	int max_d;
	max_d = (1 << BITS_D);

	for (d=0; d < max_d; d++) {
		llog_out = llog(d);
		printf("%i,", llog_out);
		if (!(totalCounter % VARS_PER_LINE)) {
			printf("\n");
		}
		
		totalCounter++;
	}
	printf("%i", totalCounter);

	return 0;
}

long
llog(d)
int	d;
{
	long	i;
	long	dqm, exp, mant, ds;

	ds = (d >> 15) ;
	dqm = ds ? (65536 - d) & 32767 : d;

	for (i = 0 ; i <= 14; i++) {
		if ((dqm >> i) <= 1) {
			exp = i;
			break;
		}
	}

	return exp;
}
