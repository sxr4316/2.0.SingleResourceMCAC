/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: mapping.h
 Contents: G.711 PCM and Linear PCM subtraction functions.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _MAPPING_H
#define _MAPPING_H

#include "../basicop/basicop.h"

struct encoder_map_functions {
	Word16 (*sub_values)(Word16 a, Word16 b);
	Word16 (*map_values)(Word16 a);
};

struct decoder_map_functions {
	Word16 (*add_values)(Word16 a, Word16 b);
	Word16 (*unmap_values)(Word16 a);
};

/* Encoder functions */
extern const struct encoder_map_functions enc_map_functions_ulaw[4];
extern const struct encoder_map_functions enc_map_functions_alaw;

/* Decoder functions */
extern const struct decoder_map_functions dec_map_functions_ulaw[4];
extern const struct decoder_map_functions dec_map_functions_alaw;

#endif /* !_MAPPING_H */

