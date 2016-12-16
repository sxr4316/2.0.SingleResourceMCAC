/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: g711llc_encoder_engine_ntt.h
 Contents: G.711-LLC encoder main control logic and encoder functions.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _G711LLC_ENCODER_H
#define _G711LLC_ENCODER_H

#include "g711llc_defines.h"
#include "../basicop/basicop.h"

struct g711llc_encoder
{
	/* Persistent variables for LTP pre-processing */
	Word16 norm_avg;
	Word16 noise_count; 
	Word16 frame_count;
	Word16 avg_thr;
	Word16 Tflag_buff;
};

/* Initializes the encoder persistent storage.
 * Equivalent to zero filling the structure. */
void g711llc_encoder_init(struct g711llc_encoder *);

/* Encode one frame on memory.
 *
 * Parameters:
 *   input:         pointer to uncompressed audio data (G.711 PCM)
 *   frame_length:  number of samples in the input buffer (0, 40, 80, 160, 240, or 320)
 *   output:        pointer to the output buffer
 *   output_size:   size of the output buffer (for error checking)
 *   mu_law:        0 for a-law, 1 for mu-law
 *   encoder:       pointer to the encoder persistent storage structure
 *
 * Returns:
 *   The number of elements written to the output buffer (negative on error).
 */
Word16 g711llc_encode_frame(const Word16 *input,
                            Word16 frame_length,
                            Word16 *output,
                            UWord16 output_size,
                            BOOL mu_law,
							struct g711llc_encoder *encoder);

#endif /* !_G711LLC_ENCODER_H */

