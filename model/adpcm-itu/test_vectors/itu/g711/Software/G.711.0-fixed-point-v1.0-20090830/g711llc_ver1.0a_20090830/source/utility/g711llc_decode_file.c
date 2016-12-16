/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: g711llc_decode_file.c
 Contents: Wrapper function to implement G.711-LLC decoding of files.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#include <assert.h>
#include "wmops_timer.h"
#include "stack_profile.h"
#include "g711llc_decode_file.h"
#include "../g711llc/g711llc_decoder.h"

int g711llc_decode_file(FILE *input_stream, FILE *output_stream, BOOL mu_law, BOOL verbose, int *last_frame_length)
{
	UWord8 input_buffer[321], output_buffer[320];

	/* NOTE: this reference implementation requires 3 extra bytes at the end of the input buffer,
	   because of the way input_bit_stream.c is implemented. This is merely an implementation artifact.
	   The G.711-LLC encoder always produces an output shorter or equal to 321 bytes. */
	Word16 input_buffer_basicops[321+3]; /* Buffer for holding the input data (for use with the basic operators). */
	Word16 output_buffer_basicops[320]; /* Buffer for holding the output data (for use with the basic operators). */

	unsigned long frame_number; /* only used for verbose output */

	wmops_timer_start();

	frame_number = 0;
	do {
		size_t input_size;
		UWord16 bytes_processed, i;
		Word16 output_size;

		input_size = fread(input_buffer, 1, sizeof(input_buffer), input_stream);
		if (ferror(input_stream) != 0)
			return -107; /* file read error */

		if (input_size == 0)
			break;

		/* Convert the input buffer to Word16 for use with basic operators */
		for (i=0; i<input_size; ++i)
			input_buffer_basicops[i] = input_buffer[i];

		/* Decode the frame (memory -> memory) */
		bytes_processed = (UWord16)input_size;

		output_size = g711llc_decode_frame(input_buffer_basicops, &bytes_processed, output_buffer_basicops, sizeof(output_buffer_basicops)/sizeof(output_buffer_basicops[0]), mu_law ? TRUE : FALSE);

		SPR_NEXT_FRAME();
		if (output_size < 0)
			return -109; /* uncompressed frame had zero length or error */

		if (last_frame_length != NULL)
			*last_frame_length = output_size;

		/* Convert back the output buffer from Word16 to unsigned char */
		for (i=0; i<output_size; ++i)
			output_buffer[i] = (UWord8)output_buffer_basicops[i];

		assert(bytes_processed <= input_size);

		wmops_timer_next_frame(verbose);

		if (verbose) {
			printf("   %3lu: read %3lu bytes, decoded to %3lu bytes\n", frame_number, (unsigned long)bytes_processed, (unsigned long)output_size);
			++frame_number;
		}

		/* Adjust the file pointer in the input stream if needed */
		if (input_size != bytes_processed) {
			/* Seek backwards */
			fseek(input_stream, (int)bytes_processed - (int)input_size, SEEK_CUR);
		}

		/* Write the frame to the output stream */
		if (output_size > 0) {
			if (fwrite(output_buffer, output_size, 1, output_stream) != 1)
				return -111;
		}
	} while (!feof(input_stream));

	wmops_timer_end();
	return 0;
}



