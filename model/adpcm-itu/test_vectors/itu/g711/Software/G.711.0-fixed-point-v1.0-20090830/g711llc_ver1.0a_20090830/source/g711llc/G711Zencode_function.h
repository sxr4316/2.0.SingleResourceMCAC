/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: G711Zencode_function.h
 Contents: G.711-LLC Min Max level encoder functions.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _G711ZDECODE_FUNCTION_H
#define _G711ZDECODE_FUNCTION_H

#include "g711llc_defines.h"
#include "../basicop/basicop.h"

void G711Zcompress_analyze_low(BOOL mu_or_a,
                               const Word16 *input_frame,
                               Word16 *converted_frame,
                               Word16 input_frame_size,
                               Word16 *y_min,
                               Word16 *y_max,
                               Word16 *range);

Word16 G711Zcompress_encode(Word16 *input_frame,
                            Word16 input_frame_size,
                            Word16 *output_frame,
                            Word16 y_min,
                            Word16 y_max,
                            Word16 num_bits);

Word16 G711Zcompress_encode_low(Word16 *input_frame,
                                Word16 input_frame_size,
                                Word16 *output_frame,
                                Word16 y_min,
                                Word16 y_max,
                                Word16 num_bits);

#endif /* !_G711ZDECODE_FUNCTION_H */

