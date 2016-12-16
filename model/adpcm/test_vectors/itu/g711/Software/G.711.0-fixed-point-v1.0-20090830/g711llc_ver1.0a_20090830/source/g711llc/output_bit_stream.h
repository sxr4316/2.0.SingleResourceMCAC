/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: output_bit_stream.h
 Contents: Bit stream writer and entropy coder functions.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _OUTPUT_BIT_STREAM_H
#define _OUTPUT_BIT_STREAM_H

#include "g711llc_defines.h"
#include "../basicop/basicop.h"

struct output_bit_stream
{
	Word16 *buffer;
	Word16 index;
	Word16 buffer_size;
	Word16 bits_in_buffer;
};

void output_bit_stream_open(Word16 *buffer, UWord16 buffer_size, UWord16 bit_offset, struct output_bit_stream *);

/* Write the specified bits to the output stream on the specified length */
void put_bits(Word16 bits, Word16 length, struct output_bit_stream *);
/* Write the specified bits to the output stream on the specified length (length <= 8) */
void put_bits_le8(Word16 bits, Word16 length, struct output_bit_stream *);
/* Write a single bit the output stream */
void put_bit(Word16 bit, struct output_bit_stream *);

/* Encode one symbol with Golomb-Rice code to the input stream. "s" is
 * the Golomb-Rice parameter. */
void rice_encode(Word16 symbol, Word16 s, struct output_bit_stream *);
/* Encode a block of symbols with Golomb-Rice code to the input stream.
 * "s" is the Golomb-Rice parameter.
 * "length" is the number of symbols in the block. */
void rice_encode_block(const Word16 *block, Word16 s, UWord16 length, struct output_bit_stream *);
void rice_encode_block_s5(const Word16 *block, UWord16 length, struct output_bit_stream *);

/* Encode a block of symbols with e-huffman code
 * "s" is the Golomb-Rice parameter -1.
 * "length" is the number of symbols in the block */
void get_kj_values(const Word16 *block, Word16 *kv, Word16 *jv, Word16 s, UWord16 length);
Word16 get_huffman_index(const Word16 *block, UWord16 length);
void huffman_encode(const Word16 *block, const Word16 *jv, Word16 s, UWord16 length, Word16 tbl_index, struct output_bit_stream *p);

void unary_encode(Word16 k, struct output_bit_stream *);
UWord16 count_rice_bits(Word16 symbol, Word16 s);
void rice_encode_unsigned(UWord16 symbol, Word16 s, struct output_bit_stream *);
void rice_encode_block_unsigned(const UWord16 *block, Word16 s, UWord16 length, struct output_bit_stream *);

/* Returns the total number of (partial or total) bytes on the stream since relative to the first byte. */
UWord16 length_in_bytes(struct output_bit_stream *);

#define DEF_MINEXPONENT	0
#define DEF_MAXEXPONENT	9

Word16 get_rice_parameter_unsigned(const UWord16 *x, UWord16 N, UWord16 minexponent, UWord16 maxexponent, UWord16 *bitlength);

#endif /* !_OUTPUT_BIT_STREAM_H */


