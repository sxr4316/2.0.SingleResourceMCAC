/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: fract_bits.h
 Contents: Encoding of number of levels by blocked fractional-bit
           representation.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _FRACT_BITS_H
#define _FRACT_BITS_H

#include "g711llc_defines.h"
#include "../basicop/basicop.h"

struct input_bit_stream;

/* DO NOT REORDER! */
typedef enum _fract_frame_types
{
	fb_2lev128anc40 = 2,
	fb_4lev126anc40,
	fb_3lev126anc40z127,
	fb_4lev126anc40z127,
	fb_3lev126anc40,
	fb_3lev127anc40,
	fb_5lev125anc40z127,
	fb_5lev126anc40,
	fb_5lev126anc40z127,
	fb_6lev125anc40,
	fb_6lev125anc40z127,
	fb_7lev124anc40z127,

	fb_2lev128anc80,
	fb_4lev126anc80,
	fb_3lev126anc80z127,
	fb_4lev126anc80z127,
	fb_3lev126anc80,
	fb_3lev127anc80,

	fb_2lev128anc160,
	fb_4lev126anc160,
	fb_3lev126anc160z127,
	fb_4lev126anc160z127,

	fb_2lev128anc240,
	fb_4lev126anc240,
	fb_3lev126anc240z127,
	fb_4lev126anc240z127,

	fb_2lev128anc320,
	fb_4lev126anc320,
	fb_3lev126anc320z127,
	fb_4lev126anc320z127

} fract_frame_t;


#define FB_MIN_TYPE_HEADER fb_2lev128anc40
#define FB_MAX_TYPE_HEADER fb_4lev126anc320z127

/**********************************************
fract_bits encoding functions
**********************************************/
fract_frame_t fract_bits_analyze(Word16 framelength_index,
                                 Word16 input_size, 
                                 const Word16 *input, 
                                 Word16 y_max,
                                 Word16 y_min,
                                 Word16 range,
                                 Word16 *estimated_size, 
                                 Word16 *exists_minus1);

Word16 fract_bits_encode(const Word16 *input_frame, 
                         Word16 input_frame_size, 
                         Word16 *output_frame, 
                         Word16 output_frame_size,
                         Word16 y_max,
                         Word16 y_min,
                         Word16 exists_minus1);


/**********************************************
fract_bits decoding function
**********************************************/
Word16 fract_bits_decode(Word16 mu_or_a,
                         struct input_bit_stream *bitstream, 
                         Word16 *output_frame, 
                         Word16 output_frame_size,
                         fract_frame_t fb_type);


#endif /* !_FRACT_BITS_H */
