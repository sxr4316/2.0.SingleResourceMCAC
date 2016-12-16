/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: G711Zdecode_function.h
 Contents: G.711-LLC Min Max level decoder function.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _G711ZDECODE_FUNCTION_H
#define _G711ZDECODE_FUNCTION_H

#include "g711llc_defines.h"
#include "../basicop/basicop.h"

Word16 G711Zdecompress(BOOL mu_or_a,
                       const Word16 *g711z_input_frame,
                       Word16 *g711z_input_frame_length,
                       Word16 *g711z_output_frame,
                       Word16 output_frame_length);

#endif /* !_G711ZDECODE_FUNCTION_H */

