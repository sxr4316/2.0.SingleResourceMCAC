/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: g711llc_decode_file.h
 Contents: Wrapper function to implement G.711-LLC decoding of files.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _G711LLC_DECODE_FILE_H
#define _G711LLC_DECODE_FILE_H

#include <stdio.h>
#include "../g711llc/g711llc_defines.h"

/* Decode a file containing G.711-LLC compressed frames to a file.
 *
 * Parameters:
 *   input_stream:      input file
 *   output_stream:     output file
 *   mu_law:            0 for a-law, 1 for mu-law
 *   verbose:           verbosity level (0 or 1)
 *   last_frame_length: if not NULL, the length of the last decoded frame
 *                      (in samples) will be stored into the pointed integer
 * Returns:
 *   0              on success
 *   negative value on failure
 */
int g711llc_decode_file(FILE *input_stream, FILE *output_stream, BOOL mu_law, BOOL verbose, int *last_frame_length);

#endif /* !_G711LLC_DECODE_FILE_H */
