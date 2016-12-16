/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: input_bit_stream.h
 Contents: Bit stream reader and entropy decoder functions.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _INPUT_BIT_STREAM_H
#define _INPUT_BIT_STREAM_H

#include "g711llc_defines.h"
#include "../basicop/basicop.h"

struct input_bit_stream
{
	const Word16 *buffer;
	Word16 index;
	Word16 buffer_size;
	Word16 start_bit_offset;
	Word16 bits_in_buffer;
};

void input_bit_stream_open(const Word16 *buffer, UWord16 buffer_size, UWord16 bit_offset, struct input_bit_stream *);

/* Read the specified number of bits from the input stream and returns a bit-vector */
UWord16 get_bits(Word16 length, struct input_bit_stream *);
/* Read the specified number of bits from the input stream and returns a bit-vector (length <= 8) */
UWord16 get_bits_le8(Word16 length, struct input_bit_stream *);
/* Read one bit from the input stream. */
UWord16 get_bit(struct input_bit_stream *);

/* Reads the 3 bits from the input stream without affecting the current read position and returns a bit-vector */
UWord16 peek_bits_3(struct input_bit_stream *);
/* Reads maximal_code_length bits from the input stream without affecting the current read position and returns a bit-vector */
UWord16 peek_bits_pc_max_codelen_largeframe(struct input_bit_stream *);

/* Skip the specified number of bits from the input stream */
void skip_bits(Word16 length, struct input_bit_stream *);

/* Decode one Golomb-Rice encoded symbol from the input stream. "s" is
 * the Golomb-Rice parameter. */
Word16 rice_decode(Word16 s, struct input_bit_stream *);
/* Decode a block of Golomb-Rice encoded symbols from the input stream.
 * "s" is the Golomb-Rice parameter.
 * "length" is the number of symbols in the block. */
void rice_decode_block(Word16 *block, Word16 s, UWord16 length, struct input_bit_stream *);

void huffman_decode(Word16 *block, Word16 s, UWord16 length, Word16 tbl, struct input_bit_stream *);

UWord16 rice_decode_unsigned(Word16 s, struct input_bit_stream *);

/* Returns the total number of (partial or total) bytes read from the stream since the stream was opened. */
UWord16 position_in_bytes(struct input_bit_stream *);

#endif /* !_INPUT_BIT_STREAM_H */


