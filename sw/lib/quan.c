#include "../include/encode.h"

extern struct tbl quanbr32_tbl[];
extern struct tbl quanbr40_tbl[];
extern struct tbl quanbr24_tbl[];
extern struct tbl quanbr16_tbl[];

long
quan(dln, ds, br)
long dln, ds, br;
{
	int	i;
    long I;

	if (br==40)
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
	else if(br==32)
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
	else if (br==24)
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
	else if (br==16)
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