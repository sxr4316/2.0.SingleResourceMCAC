/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: input_bit_stream.c
 Contents: Bit stream reader and entropy decoder functions.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include "g711llc_defines.h"
#include "tables.h"
#include "input_bit_stream.h"
#include "../utility/stack_profile.h"

static Word16 unary_decode(struct input_bit_stream *);
static void rice_decode_0(Word16 *block, UWord16 length, struct input_bit_stream *); /* for s==0 */
static void rice_decode_g0(Word16 *block, Word16 s, UWord16 length, struct input_bit_stream *); /* for s>0 */

UWord16 peek_bits_3(struct input_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2);
	UWord16 bits;

	bits = s_or(lshl(p->buffer[p->index], 8), p->buffer[p->index+1]);
	if (p->index >= 0) {
		bits = lshl(p->buffer[p->index], 8);
	}
	bits = lshl(bits, p->bits_in_buffer);
	SPR_LEAVE(scp0);
	return lshr(bits, 16 - 3);
}

UWord16 peek_bits_pc_max_codelen_largeframe(struct input_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(4);
	Word32 bits;

	bits = L_mac0(L_mac0(L_deposit_h(p->buffer[p->index]), p->buffer[p->index+2], 1), p->buffer[p->index+1], 256), add(p->bits_in_buffer, 8);
	if (p->index >= 0) {
		bits = L_deposit_h(p->buffer[p->index]);
	}
	bits = L_lshl(bits, add(p->bits_in_buffer, 8));

	assert(1 <= pc_max_codelen_largeframe && pc_max_codelen_largeframe <= 16);
	assert(pc_max_codelen_largeframe > 8); /* MODIFY this function if this is not true */

	SPR_LEAVE(scp0);
	return extract_l(L_lshr(bits, 32 - pc_max_codelen_largeframe));
}

void input_bit_stream_open(const Word16 *b, UWord16 size, UWord16 bit_offset, struct input_bit_stream *p)
{
	assert(b != 0);
	assert(bit_offset < 8);
	assert(bit_offset == 0 || size > 0);

	p->buffer_size = size; move16();
	p->index = sub(1, p->buffer_size);
	p->buffer = b - p->index; PTR2_MOVE();
	p->start_bit_offset = p->bits_in_buffer = bit_offset; move16(); move16();
}

UWord16 get_bits(Word16 length, struct input_bit_stream *p)
{
	const scopeid_t scp0 = SPR_ENTER(2+4);
	UWord16 l;
	Word32 bits;
	assert(0 < length && length <= 16);

	l = add(p->bits_in_buffer, length);

	bits = L_mac0(L_mac0(L_deposit_h(p->buffer[p->index]), p->buffer[p->index+2], 1), p->buffer[p->index+1], 256);
	if (p->index >= 0) {
		bits = L_deposit_h(p->buffer[p->index]);
	}

	bits = L_lshl(bits, add(p->bits_in_buffer, 8));

	p->index = add(p->index, lshr(l, 3));
	p->bits_in_buffer = s_and(l, 7);

	SPR_LEAVE(scp0);

	return extract_l( L_lshr(bits, sub(32, length)) );
}

UWord16 get_bits_le8(Word16 length, struct input_bit_stream *p)
{
	const scopeid_t scp0 = SPR_ENTER(2+2);

	UWord16 l;
	UWord16 bits;
	assert(0 < length && length <= 8);

	l = add(p->bits_in_buffer, length);

	bits = s_or(lshl(p->buffer[p->index], 8), p->buffer[p->index+1]);
	if (p->index >= 0) {
		bits = lshl(p->buffer[p->index], 8);
	}
	bits = lshl(bits, p->bits_in_buffer);

	p->index = add(p->index, lshr(l, 3));
	p->bits_in_buffer = s_and(l, 7);

	SPR_LEAVE(scp0);

	return lshr(bits, sub(16, length));
}

void skip_bits(Word16 length, struct input_bit_stream *p)
{
	const scopeid_t scp0 = SPR_ENTER(2);

	const UWord16 l = add(p->bits_in_buffer, length);
	p->index = add(p->index, lshr(l, 3));
	p->bits_in_buffer = s_and(l, 7);

	SPR_LEAVE(scp0);
}

UWord16 get_bit(struct input_bit_stream *p)
{
	const scopeid_t scp0 = SPR_ENTER(2+2);

	UWord16 l, bit;
	l = add(p->bits_in_buffer, 1);
	bit = lshr(p->buffer[p->index], sub(8, l));

	p->bits_in_buffer = s_and(l, 7);
	if (p->bits_in_buffer == 0)
		p->index = add(p->index, 1);

	SPR_LEAVE(scp0);

	return s_and(bit, 1);
}

UWord16 position_in_bytes(struct input_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2);
	UWord16 result = add(p->index, sub(p->buffer_size, 1));
	if (sub(p->bits_in_buffer, p->start_bit_offset) > 0)
		result = add(result, 1);
	SPR_LEAVE(scp0);
	return result;
}

Word16 rice_decode(Word16 s, struct input_bit_stream *p)
{
	const scopeid_t scp0 = SPR_ENTER(2*2+2);
	UWord16 j, cnt;
	Word16 v;
	assert(0 <= s && s <= 7);

	/* scan run of 1s: */
	cnt = unary_decode(p);

	/* read last s bits: */
	IF (s != 0) {
		const scopeid_t scp1 = SPR_ENTER(2*2);
		Word16 s1, s2;
		j = get_bits_le8(s, p); move16();

		s1 = sub(s, 1);
		s2 = shl(1, s1);

		IF (s_and(j, s2) != 0)
			v = s_or(shl(cnt, s1), s_and(j, sub(s2, 1)));
		ELSE
			v = s_xor(s_or(shl(cnt, s1), j), -1);

		SPR_LEAVE(scp1);
	} ELSE {
		/* convert cnt back to a symbol: */
		IF (s_and(cnt, 1) != 0)
			v = shr(s_xor((Word16)cnt,-1), 1);
		ELSE
			v = shr(cnt, 1);
	}
	SPR_LEAVE(scp0);
	return v;
}

void rice_decode_0(Word16 *block, UWord16 length, struct input_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2);
	UWord16 i;
	FOR (i=0; i<length; ++i) {
		const scopeid_t scp1 = SPR_ENTER(2+2);
		UWord16 cnt = unary_decode(p);
		UWord16 b = s_and(cnt, 1);

		cnt = lshr(cnt, 1);

		if (b != 0) cnt = s_xor(cnt, -1);

		block[i] = cnt; move16();
		SPR_LEAVE(scp1);
	}
	SPR_LEAVE(scp0);
}

void rice_decode_g0(Word16 *block, Word16 s, UWord16 length, struct input_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2);
	const Word16 M = sub(shl(1, s), 1);
	UWord16 i;
	FOR (i=0; i<length; ++i) {
		const scopeid_t scp1 = SPR_ENTER(3*2);
		UWord16 cnt, j, symbol;
		cnt = unary_decode(p);

		j = lshl(p->buffer[p->index],8);
		j = s_or(j, p->buffer[p->index+1]);
		if (p->index >= 0) {
			j = lshl(p->buffer[p->index], 8);
		}

		p->bits_in_buffer = add(p->bits_in_buffer, s);
		j = s_and(lshr(j, sub(16, p->bits_in_buffer)), M);
		p->index = add(p->index, lshr(p->bits_in_buffer, 3));
		p->bits_in_buffer = s_and(p->bits_in_buffer, 7);

		symbol = s_or(shl(cnt, s), s_and(j, M));
		if (s_and(j, 1) != 0)
			symbol = s_xor(symbol, -1);
		block[i] = shr(symbol, 1); move16();
		SPR_LEAVE(scp1);
	}
	SPR_LEAVE(scp0);
}

void rice_decode_block(Word16 *block, Word16 s, UWord16 length, struct input_bit_stream *p) {
	assert(block != 0);
	assert(0 <= s && s <= 7);

	IF (s == 0)
		rice_decode_0(block, length, p);
	ELSE
		rice_decode_g0(block, s, length, p);
}

void huffman_decode_0(Word16 *block, UWord16 length, Word16 tbl, struct input_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2+2);
	UWord16 i;
	Word16 maxCodeValue = (Word16 )Huffman_maxCodeValue[tbl]; move16(); /* CHECK_MOVE for Word8 */
	FOR (i=0; i<length; ++i) {
		const scopeid_t scp1 = SPR_ENTER(2*2+2*2+2);
		UWord16 cntb, b;
		UWord16 code, index;
		Word16 v;
		
		code = s_or(lshl(p->buffer[p->index], 8), p->buffer[p->index+1]);
		if (p->index >= 0) {
			code = lshl(p->buffer[p->index], 8);
		}

		index = s_and(lshr(code, sub(16-huffman_read, p->bits_in_buffer)), 0x3F);
		v = (Word16 )huffman_index[tbl][index]; move16(); /* CHECK_MOVE for Word8 but needed anyway */
		cntb = add(p->bits_in_buffer, huffman_table_len[tbl][v]);

		p->bits_in_buffer = s_and(cntb, 7);
		p->index = add(p->index, lshr(cntb, 3));

		IF (sub(v, maxCodeValue) >= 0) {
			v = add(unary_decode(p), maxCodeValue);
		}
		b = s_and(v, 1);
		v = lshr(v, 1);
		if (b != 0) v = s_xor(v, -1);
		block[i] = v; move16();
		SPR_LEAVE(scp1);
	}
	SPR_LEAVE(scp0);
}

void huffman_decode_g0(Word16 *block, Word16 s, UWord16 length, Word16 tbl, struct input_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2+2+2);
	const Word16 M = sub(shl(1, s), 1);
	UWord16 i;
	Word16 maxCodeValue = (Word16 )Huffman_maxCodeValue[tbl]; move16();
	FOR (i=0; i<length; ++i) {
		const scopeid_t scp1 = SPR_ENTER(4*2+2*2+2);
		UWord16 cntb, b, j, symbol;
		UWord16 code, index;
		Word16 v;
			
		code = s_or(lshl(p->buffer[p->index], 8), p->buffer[p->index+1]);
		if (p->index >= 0) {
			code = lshl(p->buffer[p->index], 8);
		}

		index = s_and(lshr(code, sub(16-huffman_read, p->bits_in_buffer)), 0x3F);
		v = (Word16 )huffman_index[tbl][index]; move16(); /* CHECK_MOVE for Word8 but needed anyway */
		cntb = add(p->bits_in_buffer, huffman_table_len[tbl][v]);
		p->bits_in_buffer = s_and(cntb, 7);
		p->index = add(p->index, lshr(cntb, 3));

		j = lshl(p->buffer[p->index],8);
		j = s_or(j, p->buffer[p->index+1]);
		if (p->index >= 0) {
			j = lshl(p->buffer[p->index], 8);
		}

		p->bits_in_buffer = add(p->bits_in_buffer, s);
		j = s_and(lshr(j, sub(16, p->bits_in_buffer)), M);
		p->index = add(p->index, lshr(p->bits_in_buffer, 3));
		p->bits_in_buffer = s_and(p->bits_in_buffer, 7);

		IF (sub(v, maxCodeValue) >= 0) {
			b = get_bit(p);
			v = add(shl(unary_decode(p), 1), maxCodeValue);
			if (b != 0) v = add(v, 1);
		}

		symbol = s_or(shl(v, s), s_and(j, M));
		if (s_and(j, 1) != 0)
			symbol = s_xor(symbol, -1);
		block[i] = shr(symbol, 1); move16();
		SPR_LEAVE(scp1);
	}
	SPR_LEAVE(scp0);
}

void huffman_decode(Word16 *block, Word16 s, UWord16 length, Word16 tbl, struct input_bit_stream *p) {
	IF (s == 0) {
		huffman_decode_0(block, length, tbl, p);
	} ELSE {
		huffman_decode_g0(block, s, length, tbl, p);
	}
}

UWord16 rice_decode_unsigned(Word16 s, struct input_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2+2);
	UWord16 j;
	/* scan run of 0s and one 1 bit: */
	UWord16 cnt = unary_decode(p);
	assert(0 <= s && s <= 8);

	/* read last s bits: */
	IF (s != 0) {
		j = get_bits_le8(s, p);
	} else
		j = 0; /* Only to prevent debugger error in VC++ */

	SPR_LEAVE(scp0);

	return s_or(shl(cnt, s), s_and(j, sub(shl(1, s), 1)));
}

Word16 unary_decode(struct input_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2+2);
	const UWord16 head_byte = s_and(lshl(p->buffer[p->index], p->bits_in_buffer), 0xff);
	UWord16 cnt;
	p->bits_in_buffer = sub(8, p->bits_in_buffer);
	cnt = s_min(p->bits_in_buffer, clz_lut[head_byte]);

	p->bits_in_buffer = sub(cnt, p->bits_in_buffer);
	IF (p->bits_in_buffer >= 0) {
		const scopeid_t scp1 = SPR_ENTER(2);
		UWord16 d;
		WHILE ((d = p->buffer[++p->index]) == 0) {
			move16();
			cnt = add(cnt, 8);
		}
		cnt = add(cnt, clz_lut[d]);
		p->bits_in_buffer = sub(clz_lut[d], 8);
		SPR_LEAVE(scp1);
	}
	p->bits_in_buffer = add(p->bits_in_buffer, 9);
	p->index = add(p->index, lshr(p->bits_in_buffer, 3));
	p->bits_in_buffer = s_and(p->bits_in_buffer, 7);
	SPR_LEAVE(scp0);
	return cnt;
}


