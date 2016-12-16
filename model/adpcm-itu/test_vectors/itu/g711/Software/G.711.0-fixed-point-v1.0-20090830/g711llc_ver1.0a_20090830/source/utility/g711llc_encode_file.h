/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: g711llc_encode_file.h
 Contents: Wrapper function to implement G.711-LLC encoding of files.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _G711LLC_ENCODE_FILE_H
#define _G711LLC_ENCODE_FILE_H

#include <stdio.h>
#include "../g711llc/g711llc_defines.h"

/* Encode a file containing uncompressed G.711 PCM audio to a G.711-LLC file.
 *
 * Parameters:
 *   input_stream:      input file
 *   output_stream:     output file
 *   mu_law:            0 for a-law, 1 for mu-law
 *   frame_length:      frame length (40, 80, 160, 240, or 320)
 *   verbose:           verbosity level (0 or 1)
 * Returns:
 *   0              on success
 *   negative value on failure
 */
int g711llc_encode_file(FILE *input_stream, FILE *output_stream, BOOL mu_law, int frame_length, BOOL verbose);

#endif /* !_G711LLC_ENCODE_FILE_H */

