/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: output_bit_stream.c
 Contents: Bit stream writer and entropy coder functions.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include "g711llc_defines.h"
#include "tables.h"
#include "output_bit_stream.h"
#include "../utility/stack_profile.h"

static void rice_encode_0(const Word16 *block, UWord16 length, struct output_bit_stream *); /* for s==0 */
static void rice_encode_g0(const Word16 *block, Word16 s, UWord16 length, struct output_bit_stream *); /* for s>0 */

void output_bit_stream_open(Word16 *b, UWord16 size, UWord16 bit_offset, struct output_bit_stream *p)
{
	assert(p != NULL);
	assert(b != NULL);
	assert(bit_offset < 8);
	assert(size >= 3);

	p->buffer_size = sub(size, 3); /* 3 bytes safety */
	p->buffer = b + p->buffer_size; PTR2_MOVE();
	p->index = negate(p->buffer_size);
	p->bits_in_buffer = bit_offset; move16();

	assert(size > 0);
	p->buffer[p->index] = s_and(p->buffer[p->index], s_xor(sub(lshl(1, sub(8, bit_offset)), 1), 0xff)); move16();
}

void put_bits(Word16 bits, Word16 length, struct output_bit_stream *p)
{
	const scopeid_t scp0 = SPR_ENTER(3*2);
	UWord16 l, bl, bh;
	assert(p != NULL);
	assert(0 <= length && length <= 16);
	assert(0 <= bits && bits < (1<<length));
	l = add(p->bits_in_buffer, length);
	bl = lshl(bits, sub(24, l));
	bh = lshl(bits, sub(8, l));

	assert((p->buffer[p->index] & 0xff00) == 0);
	assert((bh & 0xff00) == 0);
	p->buffer[p->index] = s_or(bh, p->buffer[p->index]); move16();
	p->buffer[p->index+1] = lshr(bl, 8); move16();
	p->buffer[p->index+2] = s_and(bl, 0xff); move16();

	p->index = add(p->index, lshr(l, 3));
	p->bits_in_buffer = s_and(l, 7);

	if (p->index > 0) { p->index = 0; move16(); }

	SPR_LEAVE(scp0);
}

void put_bits_le8(Word16 bits, Word16 length, struct output_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2+2);
	UWord16 l;
	UWord16 b;
	assert(p != NULL);
	assert(0 <= length && length <= 8);
	assert(0 <= bits && bits < (1<<length));
	l = add(p->bits_in_buffer, length);
	b = lshl(bits, sub(16, l));

	assert((p->buffer[p->index] & 0xff00) == 0);
	p->buffer[p->index] = s_or(lshr(b, 8), p->buffer[p->index]); move16();
	p->buffer[p->index+1] = s_and(b, 0xff); move16();

	p->index = add(p->index, lshr(l, 3));
	p->bits_in_buffer = s_and(l, 7);

	if (p->index > 0) { p->index = 0; move16(); }

	SPR_LEAVE(scp0);
}

void put_bit(Word16 bit, struct output_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2);
	UWord16 l;
	assert(p != NULL);
	l = add(p->bits_in_buffer, 1);

	p->buffer[p->index] = s_or(lshl(bit, sub(8, l)), p->buffer[p->index]); move16();
	p->buffer[p->index+1] = 0; move16();

	p->bits_in_buffer = s_and(l, 7);
	if (p->bits_in_buffer == 0)
		p->index = add(p->index, 1);

	if (p->index > 0) { p->index = 0; move16(); }

	SPR_LEAVE(scp0);
}

UWord16 length_in_bytes(struct output_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2);
	UWord16 result;
	assert(p != NULL);
	result = add(p->buffer_size, p->index);
	if (p->bits_in_buffer > 0)
		result = add(result, 1);
	SPR_LEAVE(scp0);
	return result;
}

void rice_encode(Word16 symbol, Word16 s, struct output_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2);
	Word16 k;
	assert(0 <= s && s < 8);
	assert(p != NULL);
	IF (s > 0) {
		const scopeid_t scp1 = SPR_ENTER(2*2+2+2);
		Word16 i, j;
		const Word16 s1 = sub(s, 1);
		const Word16 ss = shl(1, s1); 
		i = symbol; move16();
		if (symbol < 0)
			i = negate(i);
		if (symbol < 0)
			i = sub(i, 1);
		k = shr(i, s1);
		j = s_and(i, sub(ss, 1));

		if (symbol >= 0)
			j = s_or(j, ss);

		unary_encode(k, p);
		put_bits_le8(j, s, p);

		SPR_LEAVE(scp1);
	} ELSE {
		k = shl(symbol, 1);
		if (symbol < 0)
			k = negate(k);
		if (symbol < 0)
			k = sub(k, 1);
		unary_encode(k, p);
	}

	SPR_LEAVE(scp0);
}

void rice_encode_block(const Word16 *block, Word16 s, UWord16 length, struct output_bit_stream *p) {
	assert(block != NULL);
	assert(0 <= s && s <= 8);

	IF (s == 0)
		rice_encode_0(block, length, p);
	ELSE
		rice_encode_g0(block, s, length, p);
}

void rice_encode_block_s5(const Word16 *block, UWord16 length, struct output_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2);
	UWord16 i;
	enum { M = 31 }; /* shl(1,5)-1*/
	assert(block != NULL);
	FOR (i=0; i<length; i++) {
		const scopeid_t scp1 = SPR_ENTER(2);
		/* split symbol into (k,j)-pair: */
		Word16 symbol = shl(block[i], 1);
		if (symbol < 0)
			symbol = s_xor(symbol, -1);
		put_bits(s_or(32, s_and(symbol, M)), add(lshr(symbol, 5), 6), p);
		SPR_LEAVE(scp1);
	}
	SPR_LEAVE(scp0);
}

void rice_encode_0(const Word16 *block, UWord16 length, struct output_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2);
	UWord16 i;
	assert(p != NULL);
	assert(block != NULL);
	FOR (i=0; i<length; ++i) {
		const scopeid_t scp1 = SPR_ENTER(2);
		Word16 k;
		/* obtain k in a case when s=0: */
		k = shl(block[i], 1);
		assert((block[i] < 0) == (k < 0));
		if (k < 0) k = s_xor(k, -1);

		/* send run of k 0s followed by a 1-bit */
		unary_encode(k, p);
		SPR_LEAVE(scp1);
	}
	SPR_LEAVE(scp0);
}

void rice_encode_g0(const Word16 *block, Word16 s, UWord16 length, struct output_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2+2+2);
	const UWord16 M0 = shl(1, s);
	const UWord16 M1 = sub(M0, 1);
	UWord16 i;

	FOR (i=0; i<length; ++i) {
		const scopeid_t scp1 = SPR_ENTER(2+2*2);
		Word16 symbol;
		UWord16 k, j;
		/* split symbol into (k,j)-pair: */
		symbol = shl(block[i], 1);
		if (symbol < 0)
			symbol = s_xor(symbol, -1);
		k = lshr(symbol, s);
		j = s_and(symbol, M1);

		/* send run of k 0s followed by a 1-bit */
		unary_encode(k, p);

		/* send j */
		put_bits_le8(j, s, p);

		SPR_LEAVE(scp1);
	}
	SPR_LEAVE(scp0);
}


void unary_encode(Word16 k, struct output_bit_stream *p) {
	/* send run of k 0s followed by a 1-bit */
	p->bits_in_buffer = add(p->bits_in_buffer, sub(k, 7)); /* (bits_in_buffer+k+1)-8 */

	WHILE (p->bits_in_buffer > 0) {
		p->buffer[++p->index] = 0; move16();
		if (p->index > 0) { p->index = 0; move16(); }
		p->bits_in_buffer = sub(p->bits_in_buffer, 8);
	}

	p->buffer[p->index] = s_or(p->buffer[p->index], lshr(1, p->bits_in_buffer)); move16();

	if (p->bits_in_buffer == 0) {
		p->buffer[++p->index] = 0; move16();
	}
	p->bits_in_buffer = s_and(p->bits_in_buffer, 7);

	if (p->index > 0) { p->index = 0; move16(); }
}

/* e-huffman */
void get_kj_values(const Word16 *block, Word16 *kvalues, Word16 *jvalues, Word16 s, UWord16 length) {
	IF (s == 0) {
		const scopeid_t scp0 = SPR_ENTER(2);
		UWord16 i;
		FOR (i=0; i<length; i++) {
			const scopeid_t scp1 = SPR_ENTER(2);
			/* obtain k in a case when s=0: */
			Word16 symbol = shl(block[i], 1);
			if (symbol < 0)
				symbol = s_xor(symbol, -1);
			kvalues[i] = symbol; move16();
			SPR_LEAVE(scp1);
		}
		SPR_LEAVE(scp0);
	} ELSE {
		const scopeid_t scp0 = SPR_ENTER(2+2+2);
		const UWord16 M0 = shl(1, s);
		const UWord16 M1 = sub(M0, 1);
		UWord16 i;
		FOR (i=0; i<length; i++) {
			const scopeid_t scp1 = SPR_ENTER(2);
			/* split symbol into (k,j)-pair: */
			Word16 symbol = shl(block[i], 1);
			if (symbol < 0)
				symbol = s_xor(symbol, -1);
			/* block[i] = lshr(symbol, s); move16(); */
			kvalues[i] = lshr(symbol, s); move16();
			jvalues[i] = s_and(symbol, M1); move16();
			SPR_LEAVE(scp1);
		}
		SPR_LEAVE(scp0);
	}
}
Word16 get_huffman_index(const Word16 *block, UWord16 length) {
	const scopeid_t scp0 = SPR_ENTER(2*2+NUM_HUFFMAN_TABLES*2+8*2+2*2+4);
	UWord16 i, index;
	Word16 size[NUM_HUFFMAN_TABLES];

	Word16 hist[8];
	Word16 j, diff;
	Word32 acc;
	FOR (i=0; i<8; ++i) {
		hist[i]=0; move16();
	}
	FOR (i=0; i<length; ++i) {
		j = s_min(block[i], 7);
		hist[j] = add(hist[j], 1); move16();
	}
/*	fprintf(stdout, "%5d %5d %5d %5d \n", hist[0], hist[1], hist[2], hist[7]); */

	size[3] = 1; move16(); /* penalty by hIndex[3][0] */
	index = 3; move16();

	FOR (i=0; i<(NUM_HUFFMAN_TABLES-1); ++i) {
		acc = L_mult0(diff_length[i][0], hist[0]);
		FOR (j=1; j<8; ++j) {
			acc = L_mac0(acc, diff_length[i][j], hist[j]);
		}
		size[i] = add(extract_l(acc), hIndex_value[i]); move16(); /* penalty */
		diff = sub(size[index], size[i]);
		if (diff>0) { index=i; move16(); }
/*				fprintf(stdout, "%5d %5d ", index, size[index]); */
	}
	SPR_LEAVE(scp0);
	return index;
}

void huffman_encode_0(const Word16 symbol, Word16 tbl, Word16 maxCodeValue, struct output_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2);
	Word16 k;
	k = sub(symbol, maxCodeValue);
	IF (k < 0) {	/* the symbol value is on the huffman table */
		put_bits_le8(huffman_table_value[tbl][symbol], huffman_table_len[tbl][symbol], p);
	} ELSE {		/* the symbol value requires an additional code (unary code) representation */
		/* fprintf(stdout, "0, k = %d, %d\n", 0, k); */
		put_bits_le8(huffman_table_value[tbl][maxCodeValue], huffman_table_len[tbl][maxCodeValue], p);
		/* send run of k 0s followed by a 1-bit */
		unary_encode(k, p);
	}
	SPR_LEAVE(scp0);
}

void huffman_encode(const Word16 *block, const Word16 *jv, Word16 s, UWord16 length, Word16 tbl, struct output_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2);
	const Word16 maxCodeValue = (Word16 )Huffman_maxCodeValue[tbl]; move16();

	IF (s > 0) {
		const scopeid_t scp1 = SPR_ENTER(2+2+2);
		const UWord16 shifted_Huffman_maxCodeValue_bits_p1 = lshl(huffman_table_value[tbl][maxCodeValue], add(s, 1));
		const UWord16 shifted_Huffman_maxCodeValue_length_p1 = add(huffman_table_len[tbl][maxCodeValue], add(s, 1));
		UWord16 i;
		FOR (i = 0; i< length; i++) {
			const scopeid_t scp2 = SPR_ENTER(2);
			Word16 k;
			k = sub(block[i], maxCodeValue);
			IF (k < 0) {	/* the symbol value is on the huffman table */
				const scopeid_t scp3 = SPR_ENTER(2);
				/* put the code + jv together */
				Word16 symbol = add(k, maxCodeValue);
				put_bits(s_or(lshl(huffman_table_value[tbl][symbol], s), jv[i]), add(huffman_table_len[tbl][symbol], s), p);
				SPR_LEAVE(scp3);
			} ELSE {		/* the symbol value requires an additional code (unary code) representation */
				const scopeid_t scp3 = SPR_ENTER(2);
				/* fprintf(stdout, "s, k = %d, %d\n", s, k); */
				Word16 k2;
				k2 = shr(k, 1);
				put_bits(s_or(shifted_Huffman_maxCodeValue_bits_p1, s_or(shl(jv[i], 1), s_and(k, 0x01))), shifted_Huffman_maxCodeValue_length_p1, p);
				/* send run of k 0s followed by a 1-bit */
				unary_encode(k2, p);
				SPR_LEAVE(scp3);
			}
			SPR_LEAVE(scp2);
		}
		SPR_LEAVE(scp1);
	} ELSE {
		const scopeid_t scp1 = SPR_ENTER(2);
		UWord16 i;
		FOR (i = 0; i< length; i++) {
			huffman_encode_0(block[i], tbl, maxCodeValue, p);
		}
		SPR_LEAVE(scp1);
	}	
	SPR_LEAVE(scp0);
}

void rice_encode_unsigned(UWord16 symbol, Word16 s, struct output_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2+2);
	UWord16 k = lshr(symbol, s);
	UWord16 j = s_and(symbol, sub(shl(1, s), 1));
	assert(0 <= s && s <= 15);

	unary_encode(k, p);

	IF (s > 0)
		put_bits_le8(j, s, p);
	SPR_LEAVE(scp0);
}

void rice_encode_block_unsigned(const UWord16 *block, Word16 s, UWord16 length, struct output_bit_stream *p) {
	const scopeid_t scp0 = SPR_ENTER(2);
	UWord16 i;
	assert(block != 0);
	assert(0 <= s && s <= 8);

	FOR (i=0; i<length; ++i)
		rice_encode_unsigned(block[i], s, p);
	SPR_LEAVE(scp0);
}

UWord16 count_rice_bits(Word16 symbol, Word16 s) {
	assert(0 <= s && s <= 7);

	IF (s > 0) {
		const scopeid_t scp0 = SPR_ENTER(2);
		UWord16 i = shr(symbol, 15);
		i = s_xor(i, symbol);
		SPR_LEAVE(scp0);
		return add(add(lshr(i, (s-1)), s), 1); /* k + s + 1 bits */
	} ELSE {
		const scopeid_t scp0 = SPR_ENTER(2);
		UWord16 i = shr(symbol, 15);
		i = s_xor(i, shl(symbol, 1));
		SPR_LEAVE(scp0);
		return add(i, 1); /* (s=0) k+s+1 bits */
	}
}

/* Estimate the optimal Golomb-Rice parameter */
Word16 get_rice_parameter_unsigned(const UWord16 *x, UWord16 N, UWord16 minexponent, UWord16 maxexponent, UWord16 *bitlength) {
	const scopeid_t scp0 = SPR_ENTER(2+2*2+2);
	UWord16 min_bit_length;
	UWord16 Ns, i;
	Word16 s;

	assert(x != 0);
	assert(minexponent <= maxexponent);

	min_bit_length = 10000; move16();
	Ns = extract_l(L_mult0(minexponent, N));
	s = minexponent; move16();
	WHILE (sub(s, maxexponent) <= 0) {
		const scopeid_t scp1 = SPR_ENTER(2);
		/* Calculate Rice-coded bit length */
		UWord16 bit_length = N; move16();
		FOR (i=0; i<N; ++i)
			bit_length = add(bit_length, lshr(x[i], s));
		bit_length = add(bit_length, Ns);

		/* Select minimal */
		IF (sub(bit_length, min_bit_length) < 0) {
			min_bit_length = bit_length; move16();
		} ELSE {
			SPR_LEAVE(scp1);
			BREAK;
		}

		Ns = add(Ns, N);
		s = add(s, 1);
		SPR_LEAVE(scp1);
	}
	if (bitlength != NULL) {
		*bitlength = min_bit_length; move16();
	}
	SPR_LEAVE(scp0);
	return sub(s, 1);
}

