/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: parcor.h
 Contents: Functions related to the PARCOR coefficients.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _PARCOR_H
#define _PARCOR_H

#include "g711llc_defines.h"
#include "../basicop/basicop.h"

struct output_bit_stream;
struct input_bit_stream;

/* Convert the PARCOR coefficients to direct LPC coefficients.
 * Parameters:
 *   rc: (input ) Q15 : rc[M]   Reflection coefficients.
 *   m:  (input ) prediction order
 *   A:  (output) Q12 : A[M]    LPC coefficients  (m = 10)
 */
extern void par2cof(const Word16 *rc, Word16 m, Word16 *A);

/* Get the list of supported prediction orders for some maximal order.
 * The returned vector has exactly 4 elements. Elements larger than max_order
 * should not be used.
 */
extern const Word8 *get_box(Word16 max_order);

struct encoder_parcor_functions {
	void (*quantize_parcor)(const Word16 *i_par, Word16 lpc_order, Word16 *asi, Word16 flen_ind);
	void (*reconstruct_parcor)(const Word16 *asi, Word16 lpc_order, Word16 *i_par, Word16 flen_ind);
	void (*encode_parcor)(const Word16 *asi, Word16 lpc_order, struct output_bit_stream *out, Word16 flen_ind);
	UWord16 (*count_parcor_bits)(const Word16 *asi, Word16 lpc_order, Word16 flen_ind);
};

struct decoder_parcor_functions {
	void (*reconstruct_parcor)(const Word16 *asi, Word16 lpc_order, Word16 *i_par, Word16 flen_ind);
	void (*decode_parcor)(struct input_bit_stream *in, Word16 lpc_order, Word16 *asi, Word16 flen_ind);
};

extern const struct encoder_parcor_functions encoder_parcor_functions_largeframe;
extern const struct encoder_parcor_functions encoder_parcor_functions_smallframe;

extern const struct decoder_parcor_functions decoder_parcor_functions_largeframe;
extern const struct decoder_parcor_functions decoder_parcor_functions_smallframe;

#endif /* !_PARCOR_H */

