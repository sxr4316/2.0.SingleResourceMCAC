/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: multirun.h
 Contents: G.711-LLC Value-location coder functions.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _MULTIRUN_H
#define _MULTIRUN_H

#include "g711llc_defines.h"
#include "../basicop/basicop.h"

struct input_bit_stream;

/*****************************************************************
  multirun coder parameters
  ***************************************************************/
#define MR_DATALEVELS 256
#define MR_MAXLEVELS 8
#define MR_MAXLEVELBITS 3
#define MR_MAXZERO 2
#define MR_MINBASE 2
#define MR_MAXBASE 128
#define MR_MAXFRAMESIZE 320
#define MR_MAXEXPONENT 4
#define MR_EXPONENTBITS 3
#define MR_DIRECTBITS 2

#define MR_ENCODELEVELS 4
#define MR_MAXMINDIFF ( MR_ENCODELEVELS - 1 )
#define MR_SCBITS 2

/*----------------------------------------------------------------
  Multirun hist structure
  ----------------------------------------------------------------*/
typedef struct mr_hist
{
	Word16 count[MR_MAXLEVELS];
	Word16 minlevel;
	Word16 maxlevel;
	Word16 maxcountidx;
	Word16 levels;
} mr_hist_t;


/******************************************************************
  multirun_analyze

  Determine if multi-run can be encoded successfully and meets 
  criteria for choice of multirun run-length encoding.  This function
  returns -1 if multirun should not be used, otherwise it returns 0.
  As a side effect it calculates the "linearized" byte data and the
  histogram of the data needed for encoding.
****************************************************************/
Word16 multirun_analyze(Word16 framelength_index, Word16 input_size, const Word16 *lindata, Word16 y_max, Word16 y_min, mr_hist_t *hist);


/******************************************************************
 multirun_encode 
 
 Encode a set of samples with the multirun coding algorithm.  The 
 algorithm may detect that the maximum number of levels has been
 exceeded and so multirun encoding can not be used, in which case the 
 function will return -1.  Otherwise it returns length of bitstream
 in bytes.

 ******************************************************************/
Word16 multirun_encode(const Word16 *input_frame, 
                       Word16 input_frame_size, 
                       mr_hist_t *hist, 
                       Word16 *output,
                       Word16 output_size);


/*******************************************************************
 multirun_decode

 Decode a bitstream that was encoded with multirun_encode.  

*******************************************************************/
Word16 multirun_decode(Word16 mu,
                       Word16 num_samples,
                       struct input_bit_stream *bitstream, 
                       Word16 *output_frame);

#endif /* !_MULTIRUN_H */
