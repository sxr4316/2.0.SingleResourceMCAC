/**************************************************************************

                (C) Copyright 1990, Motorola Inc., U.S.A.

Note:  Reproduction and use for the development of North American digital
       cellular standards or development of digital speech coding
       standards within the International Telecommunications Union -
       Telecommunications Standardization Sector is authorized by Motorola 
       Inc.  No other use is intended or authorized.

       The availability of this material does not provide any license
       by implication, estoppel, or otherwise under any patent rights
       of Motorola Inc. or others covering any use of the contents
       herein.

       Any copies or derivative works must incude this and all other
       proprietary notices.


Systems Research Laboratories
Chicago Corporate Research and Development Center
Motorola Inc.

**************************************************************************/
/*-------------------------------------------------------------*/
/**/
/*	b_con.c -- Constructs bit array.*/
/**/
/*-------------------------------------------------------------*/
/**/
/*	Written by: Matt Hartman*/
/**/
/*-------------------------------------------------------------*/
/*	inclusions*/
/**/
#include "vparams.h"

void            B_CON(codeWord, numBits, bitArray)
  int             codeWord;
  int             numBits;
  FTYPE          *bitArray;
{
  int             mask;
  FTYPE          *endPtr;

  mask = 0x1;
  for (endPtr = bitArray + numBits; bitArray < endPtr; bitArray++)
  {
    if (codeWord & mask)
      *bitArray = 1.0;
    else
      *bitArray = -1.0;
    mask <<= 1;
  }
}
