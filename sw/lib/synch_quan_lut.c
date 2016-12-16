#include "../include/encode.h"
#include <stdio.h>

extern struct tbl quanbr32_tbl[];
extern struct tbl quanbr40_tbl[];
extern struct tbl quanbr24_tbl[];
extern struct tbl quanbr16_tbl[];

#define VARS_PER_LINE 16
#define BITS_DLNX 12
#define BITS_DSX 1
#define BITS_BR 2

long synch(long, long, long);

int main(void) {
	long synch_out = 0;
	int dlnx_dlnx_dsx_br = 0;
	int dlnx = 0;
	int dsx = 0;
	int br_2bit = 0;
	int totalCounter = 0;

	int max_dlnx, max_dsx, max_br_2bit;
	max_dlnx = (1 << BITS_DLNX);
	max_dsx = (1 << BITS_DSX);
	max_br_2bit = (1 << BITS_BR);

	for (dlnx=0; dlnx < max_dlnx; dlnx++) {
		for (dsx=0; dsx < max_dsx; dsx++) {
			for (br_2bit=0; br_2bit < max_br_2bit; br_2bit++) {
				dlnx_dlnx_dsx_br = (dlnx << BITS_BR+BITS_DSX) | (dsx << BITS_BR) | (br_2bit);
				synch_out = synch(dlnx, dsx, br_2bit);
				printf("%i,", synch_out);
				totalCounter++;
				if (totalCounter != 0 && totalCounter % VARS_PER_LINE == 0){
					printf("\n");
				}
			}
		}
	}
	printf("%i", totalCounter);
	return 0;
}

long
synch(dlnx, dsx, br)
long	dlnx, dsx, br;
{
	long	i, id;

	if (br == 0)
	{
		for (i = 0; i <= QTBL_40_LEN ; i++)
		{
			if ((dsx == quanbr40_tbl[i].ds) && (dlnx >= quanbr40_tbl[i].dln_lo) && (dlnx <= quanbr40_tbl[i].dln_hi))
			{
				id = quanbr40_tbl[i].id;
				break;
			}
		}
	}
	else if (br == 1)
	{
		for (i = 0; i <= QTBL_32_LEN ; i++)
		{
			if ((dsx == quanbr32_tbl[i].ds) && (dlnx >= quanbr32_tbl[i].dln_lo) && (dlnx <= quanbr32_tbl[i].dln_hi))
			{
				id = quanbr32_tbl[i].id;
				break;
			}
		}
	}
	else if (br == 2)
	{
		for (i = 0; i <= QTBL_24_LEN ; i++)
		{
			if ((dsx == quanbr24_tbl[i].ds) && (dlnx >= quanbr24_tbl[i].dln_lo) && (dlnx <= quanbr24_tbl[i].dln_hi))
			{
				id = quanbr24_tbl[i].id;
				break;
			}
		}
	}
	else if (br == 3)
	{
		for (i = 0; i <= QTBL_16_LEN ; i++)
		{
			if ((dsx == quanbr16_tbl[i].ds) && (dlnx >= quanbr16_tbl[i].dln_lo) && (dlnx <= quanbr16_tbl[i].dln_hi))
			{
				id = quanbr16_tbl[i].id;
				break;
			}
		}
	}

	return id;
}
