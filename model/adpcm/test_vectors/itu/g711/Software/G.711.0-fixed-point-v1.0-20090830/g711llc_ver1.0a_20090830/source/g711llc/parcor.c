/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: parcor.c
 Contents: Functions related to the PARCOR coefficients.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include "g711llc_defines.h"
#include "output_bit_stream.h"
#include "input_bit_stream.h"
#include "tables.h"
#include "parcor.h"
#include "../utility/stack_profile.h"

#define Q8_1	0x0100
#define Q12_1	0x1000
#define Q13_1	0x2000
#define Q14_1	0x4000
#define Q15_1	0x7fff
#define Q16_1	0x00010000l
#define Q24_1	0x01000000l
#define Q27_1	0x08000000l
#define Q28_1	0x10000000l
#define Q30_1	0x40000000l
#define Q31_1	0x7fffffffl

/* Helper function for quantizing the first two PARCOR coefficients.
 *
 * Applies the compander function and quantizes the coefficient.
 */
static Word16 quantize01_largeframe(Word16 i_par, Word16 bits) {
	const scopeid_t scp0 = SPR_ENTER(2*2);
	Word16 i_a, tmp;
	IF (sub(i_par, 30801) > 0) {
		tmp = add(mult_r(i_par, -23648), 19552); /* 1.2.12 */
		i_a = shl(tmp, sub(bits, 12));
	} ELSE IF (sub(i_par, 24576) > 0) {
		tmp = add(mult_r(i_par, -31103), 18529); /* 1.1.14 */
		i_a = shl(tmp, sub(bits, 14));
	} ELSE {
		tmp = add(mult_r(i_par, -24209), 8559); /* 1.0.15 */
		i_a = shl(tmp, sub(bits, 15));
	}
	SPR_LEAVE(scp0);
	return i_a;
}

static void quantize_parcor_largeframe(const Word16 *i_par, Word16 lpc_order, Word16 *asi, Word16 flen_ind) {
	const scopeid_t scp0 = SPR_ENTER(3*2);
	Word16 i, pcb, i_a;
	assert(i_par != 0);
	assert(asi != 0);

	(void)flen_ind;

	/* first coefficient: */
	asi[0] = quantize01_largeframe(i_par[0], (Word16 )pc_bits_largeframe[lpc_order][0]); WRD8_MOVE(); move16(); /* CHECK_MOVE for Word8 */

	/* second coefficient: */
	IF (sub(lpc_order, 1) > 0) {
		asi[1] = quantize01_largeframe(negate(i_par[1]), (Word16 )pc_bits_largeframe[lpc_order][1]); WRD8_MOVE(); move16(); /* CHECK_MOVE for Word8 */

		IF (sub(lpc_order, 2) > 0) { /* FOR must iterate at least once */
			/* the remaining coeffs: */
			FOR (i=2; i<lpc_order; ++i) {
				pcb = (Word16 )pc_bits_largeframe[lpc_order][i]; move16(); /* CHECK_MOVE for Word8 */

				i_a = shl(i_par[i], sub(pcb, 15));

				asi[i] = i_a; move16();
			}
		}
	}
	SPR_LEAVE(scp0);
}

static void reconstruct_parcor_largeframe(const Word16 *asi, Word16 lpc_order, Word16 *i_par, Word16 flen_ind) {
	const scopeid_t scp0 = SPR_ENTER(4*2);

	Word16 scale, offset, pcb, i;

	(void)flen_ind;

	scale = sub(6, (Word16 )pc_bits_largeframe[lpc_order][0]); WRD8_MOVE(); /* CHECK_MOVE for Word8 */
	offset = shr(sub(shl(1, scale), 1), 1);
	i_par[0] = negate(pc_expand01_largeframe[add(shl(asi[0], scale),offset)+64]); move16();

	/* second coefficient: */
	IF (sub(lpc_order,1) > 0) {
		scale = sub(6, (Word16 )pc_bits_largeframe[lpc_order][1]); WRD8_MOVE(); /* CHECK_MOVE for Word8 */
		offset = shr(sub(shl(1, scale), 1), 1);
		i_par[1] = pc_expand01_largeframe[add(shl(asi[1], scale),offset)+64]; move16();

		IF (sub(lpc_order, 2) > 0) { /* FOR must iterate at least once */
			FOR (i=2; i<lpc_order; ++i) {
				pcb = pc_bits_largeframe[lpc_order][i]; WRD8_MOVE(); /* CHECK_MOVE for Word8 */
				i_par[i] = add(shl(asi[i], sub(15, pcb)), shl(1, sub(14, pcb))); move16();
			}
		}
	}
	SPR_LEAVE(scp0);
}

static void encode_parcor_largeframe(const Word16 *asi, Word16 lpc_order, struct output_bit_stream *out, Word16 flen_ind) {
	const scopeid_t scp0 = SPR_ENTER(2+2+2);
	const Word8 *codewords = pc_codes_largeframe[lpc_order];
	const Word8 *codelengths = pc_code_lengths_largeframe[lpc_order];
	Word16 i;

	(void)flen_ind;

	/* Output the codes for the PARCOR coefficients */
	FOR (i=0; i<lpc_order; ++i) {
		const scopeid_t scp1 = SPR_ENTER(2*2);
		UWord16 half_range, index;

		half_range = shl(1, pc_bits_largeframe[lpc_order][i]); WRD8_MOVE(); /* CHECK_MOVE for Word8 */
		index = add(asi[i], half_range); /* make sure it's non-negative */
		put_bits((Word16 )codewords[index], (Word16 )codelengths[index], out); WRD8_MOVE(); WRD8_MOVE(); /* CHECK_MOVE for Word8 */
		codewords += 2*half_range; PTR2_MOVE(); /* CHECK_MOVE!! */
		codelengths += 2*half_range; PTR2_MOVE(); /* CHECK_MOVE!! */
		SPR_LEAVE(scp1);
	}
	SPR_LEAVE(scp0);
}

static void decode_parcor_largeframe(struct input_bit_stream *in, Word16 lpc_order, Word16 *asi, Word16 flen_ind) {
	const scopeid_t scp0 = SPR_ENTER(2+2+2);
	const Word8 *codewords = pc_codes_largeframe[lpc_order];
	const Word8 *codelengths = pc_code_lengths_largeframe[lpc_order];
	Word16 i;

	(void)flen_ind;

	FOR (i=0; i<lpc_order; ++i) {
		const scopeid_t scp1 = SPR_ENTER(3*2);
		UWord16 full_range, next_bits, index;

		full_range = shl(2, pc_bits_largeframe[lpc_order][i]); WRD8_MOVE(); /* CHECK_MOVE for Word8 */

		next_bits = peek_bits_pc_max_codelen_largeframe(in);
		index = 0; move16();
		WHILE (sub(index, full_range) < 0) {
			const scopeid_t scp2 = SPR_ENTER(2*2);
			Word16 cl, ci;
			cl = codelengths[index]; move16(); /* CHECK_MOVE for Word8 */
			ci = codewords[index]; move16(); /* CHECK_MOVE for Word8 */
			assert(cl <= pc_max_codelen_largeframe);
			IF (sub(lshr(next_bits, sub(pc_max_codelen_largeframe, cl)), ci) == 0) {
				skip_bits(cl, in);
				SPR_LEAVE(scp2);
				BREAK;
			}
			index = add(index, 1);
			SPR_LEAVE(scp2);
		}
		assert(index < full_range);

		asi[i] = sub(index, shr(full_range, 1)); move16();

		codewords += full_range; PTR2_MOVE(); /* CHECK_MOVE!! */
		codelengths += full_range; PTR2_MOVE(); /* CHECK_MOVE!! */
		SPR_LEAVE(scp1);
	}
	SPR_LEAVE(scp0);
}

static UWord16 count_parcor_bits_largeframe(const Word16 *asi, Word16 lpc_order, Word16 flen_ind) {
	const scopeid_t scp0 = SPR_ENTER(2+2+2);
	const Word8 *codelengths = pc_code_lengths_largeframe[lpc_order];
	Word16 i;
	UWord16 result = 0; move16();

	(void)flen_ind;

	FOR (i=0; i<lpc_order; ++i) {
		const scopeid_t scp1 = SPR_ENTER(2*2);
		UWord16 half_range, index;
		half_range = shl(1, pc_bits_largeframe[lpc_order][i]); WRD8_MOVE(); /* CHECK_MOVE for Word8 */
		index = add(asi[i], half_range); /* make sure it's non-negative */
		result = add(result, codelengths[index]); WRD8_MOVE(); /* CHECK_MOVE for Word8 */
		codelengths += 2*half_range; PTR2_MOVE(); /* CHECK_MOVE!! */
		SPR_LEAVE(scp1);
	}
	SPR_LEAVE(scp0);
	return result;
}

static Word16 quantize_smallframe(Word16 i_par, Word16 bits) {
	const scopeid_t scp0 = SPR_ENTER(4*2+2+2);
	Word16 i_a, shiftbit, hi, nz;
	UWord16 tmppar;
	const Word8 *qparind_vec = pc_ind_smallframe[bits];

	shiftbit = sub(16, bits);
	tmppar = abs_s(i_par);
	hi = shr(tmppar, shiftbit);
	if (i_par < 0) { nz = (Word16 )pc_quantize_smallframe_nonzero[bits]; move16(); }
	if (i_par < 0) { hi = add(hi, nz); }
	i_a = qparind_vec[hi]; move16();
	SPR_LEAVE(scp0);
	return i_a;
}

static void quantize_parcor_smallframe(const Word16 *i_par, Word16 lpc_order, Word16 *asi, Word16 framelength_index) {
	const scopeid_t scp0 = SPR_ENTER(2);
	Word16 i;
	assert(i_par != 0);
	assert(asi != 0);
	FOR (i=0; i<lpc_order; ++i) {
		asi[i] = quantize_smallframe(i_par[i], (Word16 )pc_bits_smallframe[framelength_index][lpc_order][i]); WRD8_MOVE(); move16(); /* CHECK_MOVE for Word8 */
	}
	SPR_LEAVE(scp0);
}

static void reconstruct_parcor_smallframe(const Word16 *asi, Word16 lpc_order, Word16 *i_par, Word16 framelength_index) {
	const scopeid_t scp0 = SPR_ENTER(2);
	Word16 i;
	FOR (i=0; i<lpc_order; ++i) {
		const scopeid_t scp1 = SPR_ENTER(2);
		const Word16 *qparval_vec = pc_val_smallframe[pc_bits_smallframe[framelength_index][lpc_order][i]];
		i_par[i] = qparval_vec[asi[i]]; move16();
		SPR_LEAVE(scp1);
	}
	SPR_LEAVE(scp0);
}

static void encode_parcor_smallframe(const Word16 *asi, Word16 lpc_order, struct output_bit_stream *out, Word16 framelength_index) {
	const scopeid_t scp0 = SPR_ENTER(2+2+2);
	const Word8 *qparrice_vec = pc_riceval_smallframe_enc[framelength_index][lpc_order];
	const Word8 *qparnum = pc_num_smallframe[framelength_index][lpc_order];
	Word16 i;
	FOR (i=0; i<lpc_order; ++i) {
		const scopeid_t scp1 = SPR_ENTER(2);
		const UWord16 range = qparnum[i]; move16();
		rice_encode_unsigned((Word16 )qparrice_vec[asi[i]], (Word16 )pc_ricepara_smallframe[pc_bits_smallframe[framelength_index][lpc_order][i]], out); WRD8_MOVE(); WRD8_MOVE(); /* CHECK_MOVE for Word8 */
		qparrice_vec += range; PTR2_MOVE(); /* CHECK_MOVE!! */
		SPR_LEAVE(scp1);
	}
	SPR_LEAVE(scp0);
}

static void decode_parcor_smallframe(struct input_bit_stream *in, Word16 lpc_order, Word16 *asi, Word16 framelength_index) {
	const scopeid_t scp0 = SPR_ENTER(2+2+2);
	const Word8 *qparrice_vec = pc_riceval_smallframe_dec[framelength_index][lpc_order];
	const Word8 *qparnum = pc_num_smallframe[framelength_index][lpc_order];
	Word16 i;
	FOR (i=0; i<lpc_order; ++i) {
		const scopeid_t scp1 = SPR_ENTER(2);
		const UWord16 range = qparnum[i]; move16();
		asi[i] = (Word16 )qparrice_vec[rice_decode_unsigned((Word16 )pc_ricepara_smallframe[(Word16 )pc_bits_smallframe[framelength_index][lpc_order][i]], in)]; move16(); WRD8_MOVE(); WRD8_MOVE(); WRD8_MOVE(); /* CHECK_MOVE for Word8 */
		qparrice_vec += range; PTR2_MOVE(); /* CHECK_MOVE!! */
		SPR_LEAVE(scp1);
	}
	SPR_LEAVE(scp0);
}

static UWord16 count_parcor_bits_smallframe(const Word16 *asi, Word16 lpc_order, Word16 framelength_index) {
	const scopeid_t scp0 = SPR_ENTER(2+2+2+2);
	const Word8 *qparrice_vec = pc_riceval_smallframe_enc[framelength_index][lpc_order];
	const Word8 *qparnum = pc_num_smallframe[framelength_index][lpc_order];
	Word16 i;
	UWord16 result = lpc_order; move16();

	FOR (i=0; i<lpc_order; ++i) {
		const scopeid_t scp1 = SPR_ENTER(2);
		const Word16 range = (Word16 )qparnum[i]; move16();
		result = add(result, (Word16 )qparrice_vec[asi[i]]); move16(); /* CHECK_MOVE for Word8 */
		qparrice_vec += range; PTR2_MOVE(); /* CHECK_MOVE!! */
		SPR_LEAVE(scp1);
	}
	SPR_LEAVE(scp0);
	return result;
}

void par2cof(const Word16 *rc, Word16 m, Word16 *A) {
	const scopeid_t scp0 = SPR_ENTER(4*2);
	Word16 atmp1, atmp3, atmp6, atmp;
	assert(rc != 0);
	assert(0 <= m /*&& m <= M*/);
	assert(A != 0);
/* A[1]; 1st                              */
/* A[2],A[3]; 2nd                         */
/* A[4],A[5];A[6]; 3rd                    */
/* A[p*(p-1)/2+1],,,A[p*(p-1)/2+p]; pth   */

/*	A[1] = shr_r(rc[0], 3); move16(); */ /* A:Q12 */
	SWITCH (m) {
		case 1: 
			A[1] = rc[0]; move16(); /* A:Q15 */
			BREAK;
		case 2: { /* optimized for m=2: */
			/* i=2: */
			atmp1= shr_r(rc[0], 3); /* A:Q12 */
			A[1] = rc[0]; move16(); /* A:Q15*/
			A[2] = sub(atmp1, mult_r(atmp1, rc[1])); move16(); /* 1 */
			A[3] = rc[1]; move16(); /* 2 Q15*/
			BREAK;
		}
		case 3: { /* optimized for m=3: */
			/* i=2: */
			atmp1= shr_r(rc[0], 3); /* A:Q12 */
			A[1] = rc[0]; move16(); /* A:Q15*/
			A[2] = sub(atmp1, mult_r(atmp1,rc[1])); move16(); /* 1 */
			A[3] = rc[1]; move16(); /* 2 Q15*/
			atmp3 = shr_r(rc[1], 3); /* 2 */
			/* i=3: */
			A[4] = sub(A[2], mult_r(atmp3, rc[2])); move16(); /* A[1],1,2 */
			A[5] = sub(atmp3, mult_r(A[2], rc[2])); move16(); /* A[2],2 */
			A[6] = rc[2]; move16(); /* A[3] */
			BREAK;
		}
		case 4: { /* optimized for m=4: */
			/* i=2: */
			atmp1= shr_r(rc[0], 3); /* A:Q12 */
			A[1] = rc[0]; move16(); /* A:Q15*/
			A[2] = sub(atmp1, mult_r(atmp1, rc[1])); move16(); /* 1 */
			A[3] = rc[1]; move16(); /* 2 Q15*/
			atmp3 = shr_r(rc[1], 3); /* 2 */
			/* i=3: */
			A[4] = sub(A[2], mult_r(atmp3, rc[2])); move16(); /* A[1],1,2 */
			A[5] = sub(atmp3, mult_r(A[2], rc[2])); move16(); /* A[2],2 */
			A[6] = rc[2]; move16(); /* A[3] */
			atmp6 = shr_r(rc[2], 3); move16(); /* A[3] */
			/* i=4: */
			A[7] = sub(A[4], mult_r(atmp6, rc[3])); move16(); /* 1,1,3 */
			A[8] = sub(A[5], mult_r(A[5], rc[3])); move16(); /* 2,2,2 */
			A[9] = sub(atmp6, mult_r(A[4], rc[3])); move16(); /* 3 */
			A[10] = rc[3]; move16(); /* 4 */

			BREAK;
		}
		default: { /* generic case: */ /* CHECK_MOVE!! */
			const scopeid_t scp1 = SPR_ENTER(2+2*2);
			Word16 i, j;
			/* i=2: */
			atmp1= shr_r(rc[0], 3); /* A:Q12 */
			++A; *A = rc[0]; move16(); /* A:Q15*/
			++A; *A = sub(atmp1, mult_r(atmp1, rc[1])); move16(); /* 1 */
			++A; *A = rc[1]; move16();/* 2 */
			atmp3= shr_r(rc[1], 3); 
			/* i=3: */
			++A; *A = sub(A[-2], mult_r(atmp3, rc[2])); move16(); /* A[1],1,2 */
			++A; *A = sub(atmp3, mult_r(A[-3], rc[2])); move16(); /* A[2],2 */
			++A; *A = rc[2]; move16(); /* A[3] */
			atmp6 = shr_r(rc[2], 3); /* A[3] */
			/* i=4: */
			++A; *A = sub(A[-3], mult_r(atmp6, rc[3])); move16(); /* 1,1,3 */
			++A; *A = sub(A[-3], mult_r(A[-3], rc[3])); move16(); /* 2,2,2 */
			++A; *A = sub(atmp6, mult_r(A[-5], rc[3])); move16(); /* 3 */
			++A; *A = rc[3]; move16(); 
			atmp= shr_r(rc[3],3);
			/* i>=5: */
			FOR (i=4; i<m; ++i) {
				/*------------------------------------------*
				 *  Compute new LPC coeff. -> An[i]         *
				 *  An[j]= A[j] + K*A[i-j]     , j=1 to i-1 *
				 *  An[i]= K                                *
				 *------------------------------------------*/
				Word16 *AA = A; PTR1_MOVE();
				++A; *A = sub(A[-i], mult_r(atmp, rc[i])); move16();
				FOR (j=2; j<i; ++j) {
					++A; *A = sub(A[-i], mult_r(*--AA, rc[i])); move16();
				}
				++A; *A = sub(atmp, mult_r(*--AA, rc[i])); move16();
				++A; *A = rc[i]; move16(); /* Q15 */
				atmp = shr_r(rc[i], 3);  /* Q12 */
			}
			SPR_LEAVE(scp1);
		}
	}
	SPR_LEAVE(scp0);
}

const Word8 *get_box(Word16 max_order) {
	SWITCH (max_order) {
		case 1:
		case 2:
		case 3:
		case 4:
			/* Use predefined box "box_l4" */
			return box_l4;
		case 8:
			/* Use predefined box "box_8" */
			return box_8;
		case 10:
			/* Use predefined box "box_10" */
			return box_10;
		case 12:
			/* Use predefined box "box_12" */
			return box_12;
	}
	assert(!"NEVER HAPPEN");
	exit(EXIT_FAILURE);
	return NULL;
}

const struct encoder_parcor_functions encoder_parcor_functions_largeframe = {
	&quantize_parcor_largeframe,
	&reconstruct_parcor_largeframe,
	&encode_parcor_largeframe,
	&count_parcor_bits_largeframe
};

const struct encoder_parcor_functions encoder_parcor_functions_smallframe = {
	&quantize_parcor_smallframe,
	&reconstruct_parcor_smallframe,
	&encode_parcor_smallframe,
	&count_parcor_bits_smallframe
};

const struct decoder_parcor_functions decoder_parcor_functions_largeframe = {
	&reconstruct_parcor_largeframe,
	&decode_parcor_largeframe
};

const struct decoder_parcor_functions decoder_parcor_functions_smallframe = {
	&reconstruct_parcor_smallframe,
	&decode_parcor_smallframe
};


