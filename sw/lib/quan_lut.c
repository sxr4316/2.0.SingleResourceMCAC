#include "../include/encode.h"
#include <stdio.h>

#define printh(N) printf("%X\n",N) 
extern struct tbl quanbr32_tbl[];
extern struct tbl quanbr40_tbl[];
extern struct tbl quanbr24_tbl[];
extern struct tbl quanbr16_tbl[];

#define BITS_DS 1
#define BITS_BR 2
#define BITS_DLN 12

long quan(long, long, long);

int main(void) {
	long quan_out = 0;
	int dln_ds_br = 0;
	int dln = 0;
	int ds = 0;
	int br_2bit = 0;
	int totalCounter = 0;

	int max_dln, max_ds, max_br;
	max_dln = (1 << BITS_DLN);
	max_ds = (1 << BITS_DS);
	max_br = (1 << BITS_BR);

	for (dln=0; dln < max_dln; dln++) {
		for (ds=0; ds < max_ds; ds++) {
			for (br_2bit=0; br_2bit < max_br; br_2bit++) {
				dln_ds_br = (dln << BITS_BR+BITS_DS) | (ds << BITS_BR) | (br_2bit);
				quan_out = quan(dln, ds, br_2bit);
				printf("%i,", quan_out);
				totalCounter++;
			}
		}
		printf("\n");
	}
	printf("%i", totalCounter);
	return 0;
}

long
quan(dln, ds, br)
long dln, ds, br;
{
	int	i;
    long I;

	if (br==0)
	{
		for (i = 0; i <= QTBL_40_LEN; i++)
		{
			I = quanbr40_tbl[i].i;
			if ((dln >= quanbr40_tbl[i].dln_lo) && (dln <= quanbr40_tbl[i].dln_hi) && (ds == quanbr40_tbl[i].ds))
			{
				return(I);
			}
		}
		return(I);
	}
	else if(br==1)
	{
		for (i = 0; i <= QTBL_32_LEN; i++)
		{
			I = quanbr32_tbl[i].i;
			if ((dln >= quanbr32_tbl[i].dln_lo) && (dln <= quanbr32_tbl[i].dln_hi) && (ds == quanbr32_tbl[i].ds))
			{
				return(I);
			}
		}
		return(I);
	}
	else if (br==2)
	{
	  for (i = 0; i <= QTBL_24_LEN; i++)
	  {
			I = quanbr24_tbl[i].i;
			if ((dln >= quanbr24_tbl[i].dln_lo) && (dln <= quanbr24_tbl[i].dln_hi) && (ds == quanbr24_tbl[i].ds))
			{
				return(I);
			}
		}
		return(I);
	}
	else if (br==3)
	{
	  for (i = 0; i <= QTBL_16_LEN; i++)
	  {
			I = quanbr16_tbl[i].i;
			if ((dln >= quanbr16_tbl[i].dln_lo) && (dln <= quanbr16_tbl[i].dln_hi) && (ds == quanbr16_tbl[i].ds))
			{
				return(I);
			}
		}
		return(I);
	}
	else
	{
		return 0;
	}
}