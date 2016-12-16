/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: g711llc_decoder.h
 Contents: G.711-LLC frame decoder function.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _G711LLC_DECODER_H
#define _G711LLC_DECODER_H

#include "g711llc_defines.h"
#include "../basicop/basicop.h"

/* Decode one G.711-LLC compressed frame on memory.
 *
 * Parameters:
 *   input:       pointer to the compressed bit-stream holding
 *                at least one frame worth of data
 *   input_size:  number of samples contained in the input
 *                buffer. This is set the the number of elements
 *                processed from the input buffer on return.
 *   output:      pointer to the output buffer where the
 *                uncompressed audio samples are put
 *   output_size: size of the output buffer (for error checking)
 *
 * Returns:
 *   The number of samples decoded and put to the output buffer.
 *   Also sets the input_size parameter to the number of elements
 *   processed from the input buffer. Returns negative value on error.
 */
Word16 g711llc_decode_frame(const Word16 *input,
                            UWord16 *input_size,
                            Word16 *output,
                            UWord16 output_size,
                            BOOL mu_law);

#endif /* !_G711LLC_DECODER_H */

