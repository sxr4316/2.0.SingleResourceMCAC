#include "../include/encode.h"

extern long multbr40_tbl[];
extern long multbr32_tbl[];
extern long multbr24_tbl[];
extern long multbr16_tbl[];

long
functw(	I, br )
long	I, br;
{
	long is, im, wi;

	if(br==40)
	{
		is = I >> 4;
		im = is ? (31 - I) & 15 : I & 15;
		wi = multbr40_tbl[im];
		return(wi);
	}
	else if(br==32)
	{
		is = I >> 3;
		im = is ? (15 - I) & 7 : I & 7;
		wi = multbr32_tbl[im];
		return(wi);
	}
	else if(br==24)
	{
		is = I >> 2;
		im = is ? (7 - I) & 3 : I & 3;
		wi = multbr24_tbl[im];
		return(wi);
	}
	else if(br==16)
	{
		is = I >> 1;
		im = is ? (3 - I) & 1 : I & 1;
		wi = multbr16_tbl[im];
		return(wi);
	}
}

