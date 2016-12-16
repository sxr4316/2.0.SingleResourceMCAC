#include "../include/encode.h"
#include "../include/synch_quan_lut.h"

extern struct tbl quanbr32_tbl[];
extern struct tbl quanbr40_tbl[];
extern struct tbl quanbr24_tbl[];
extern struct tbl quanbr16_tbl[];

long
synch(I, sp, dlnx, dsx, aLaw, br)
long	I, sp, dlnx, dsx, br;
int aLaw;
{
	long	i, is, im, id, sd;

	long dlnx_dsx_br;
	long br_2bit;
	br_2bit = (br == 40 ? 00 : (br == 32 ? 01 : (br == 24 ? 02 : 03)));
	dlnx_dsx_br = (dlnx << BITS_BR+BITS_DSX) | (dsx << BITS_BR) | (br_2bit);
	id = synch_quan_lut[dlnx_dsx_br];

	switch(br_2bit) {
		case 0:
			is = I >> 4;
			im = is ? (I & 15) : (I + 16);
			break;
		case 1:
			is = I >> 3;
			im = is ? (I & 7) : (I + 8);
			break;
		case 2:
			is = I >> 2;
			im = is ? (I & 3) : (I + 4);
			break;
		case 3:
			is = I >> 1;
			im = is ? (I & 1) : (I + 2);
			break;
	}


/*
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
		}
	}*/


	
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
