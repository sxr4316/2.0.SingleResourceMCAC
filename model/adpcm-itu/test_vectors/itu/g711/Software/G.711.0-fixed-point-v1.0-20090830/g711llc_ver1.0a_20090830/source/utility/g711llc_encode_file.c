/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: g711llc_encode_file.c
 Contents: Wrapper function to implement G.711-LLC encoding of files.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include "wmops_timer.h"
#include "stack_profile.h"
#include "g711llc_encode_file.h"
#include "../g711llc/g711llc_encoder.h"

int g711llc_encode_file(FILE *input_stream, FILE *output_stream, BOOL mu_law, int frame_length, BOOL verbose)
{
	/* Buffers for holding the input and output data. */
	UWord8 input_buffer[320], output_buffer[321];

	Word16 input_buffer_basicop[320]; /* Buffer for holding the input data (for use with the basic operators). */
	/* NOTE: this reference implementation requires 3 extra bytes at the end of the output buffer,
	   because of the way output_bit_stream.c is implemented. This is merely an implementation artifact.
	   The G.711-LLC encoder always produces an output shorter or equal to 321 bytes. */
	Word16 output_buffer_basicop[321+3]; /* Buffer for holding the output data (for use with the basic operators). */

	unsigned long frame_number = 0; /* only used for verbose output */
	int error = 0;
	struct g711llc_encoder encoder;

	g711llc_encoder_init(&encoder);

	/* Check the parameters */
	if (input_stream == NULL)
		return -102;
	if (output_stream == NULL)
		return -103;

	setFrameRate(g711llc_default_hz, frame_length);

	wmops_timer_start();

	do {
		Word16 input_size = (Word16)fread(input_buffer, 1, frame_length, input_stream);
		if (ferror(input_stream) != 0) {
			error = -107; /* file read error */
			goto cleanup;
		}

		if (input_size > 0) {
			/* Convert the input buffer to Word16 for use with basic operators */
			Word16 i, output_size;
			for (i=0; i<input_size; ++i)
				input_buffer_basicop[i] = input_buffer[i];

			/* Also pad any partial frames (at the end of the input file) with zero */
			for (i=input_size; i<frame_length; ++i)
				input_buffer_basicop[i] = mu_law ? 0xff : 0x55;

			/* Encode the frame (memory -> memory) */
			output_size = (Word16)g711llc_encode_frame(input_buffer_basicop, frame_length, output_buffer_basicop, sizeof(output_buffer_basicop)/sizeof(output_buffer_basicop[0]), mu_law, &encoder);

			SPR_NEXT_FRAME();
			if (output_size <= 0) {
				error = -108; /* compressed frame had zero length or error */
				goto cleanup;
			}

			if (verbose) {
				printf("   %3lu: %3lu samples, write %3lu bytes\n", frame_number, (unsigned long)input_size, (unsigned long)output_size);
				++frame_number;
			}

			/* Write the .inf body entry */
			wmops_timer_next_frame(verbose);

			/* Convert the output buffer from Word16 to unsigned char for fwrite() */
			for (i=0; i<output_size; ++i)
				output_buffer[i] = (UWord8)output_buffer_basicop[i];

			/* Write the frame to the output stream */
			if (fwrite(output_buffer, output_size, 1, output_stream) != 1) {
				error = -110; /* file write error */
				goto cleanup;
			}
		}
	} while (!feof(input_stream));

cleanup:
	wmops_timer_end();
	return error;
}

