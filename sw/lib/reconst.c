#include "../include/encode.h"

extern long     reconbr32_tbl[];
extern long     reconbr40_tbl[];
extern long     reconbr24_tbl[];
extern long     reconbr16_tbl[];

void
reconst(I, br, dqln, dqs)
long	I, br, *dqln, *dqs;
{
    if(br==40)
    {
       *dqln = reconbr40_tbl[I];
       *dqs  = I >> 4;
    }
    else if (br==32)
    {
       *dqln = reconbr32_tbl[I];
       *dqs  = I >> 3;
    }
    else if(br==24)
    {
      *dqln = reconbr24_tbl[I];
      *dqs  = I >> 2;
    }
    else if(br==16)
    {
      *dqln = reconbr16_tbl[I];
      *dqs  = I >> 1;
    }
}
