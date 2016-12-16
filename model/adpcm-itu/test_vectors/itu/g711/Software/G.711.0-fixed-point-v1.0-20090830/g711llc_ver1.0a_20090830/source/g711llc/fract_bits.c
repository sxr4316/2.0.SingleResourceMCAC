/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: fract_bits.c
 Contents: Encoding of number of levels by blocked fractional-bit
           representation.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#include <assert.h>
#include "tables.h"
#include "input_bit_stream.h"
#include "output_bit_stream.h"
#include "fract_bits.h"
#include "../utility/stack_profile.h"

/*-----------------------------------------------------------------------
level_exists

This is a helper function to determine if alevel exists in the data.
------------------------------------------------------------------------*/
static Word16 level_exists(const Word16* input, Word16 input_size, Word16 level)
{
	const scopeid_t scp0 = SPR_ENTER(2*2);

	Word16 sample, exists;

	exists = 0; move16();

	/* Check for missing level */	
	FOR (sample=0; sample<input_size; ++sample) {
		move16();
		if (sub(*input++, level) == 0) {
			exists = 1; move16();
		}
	}
	
	SPR_LEAVE(scp0);

	return exists;
}


/*---------------------------------------------------------------------------------
fract_bits_analyze

This function receives as input a frame of data and its length, the minimum and maximum
frame values and the range.  From this data it determines if the frame can be encoded using
fractional bit encoding, and determines the proper enum constant for the frame type
which will be used to signal a special header byte.  It also determines the frame
size of the fractional-bit encoded frame.
----------------------------------------------------------------------------------*/
fract_frame_t fract_bits_analyze(Word16 framelength_index,
                                 Word16 input_size, 
                                 const Word16 *input, 
                                 Word16 y_max,
                                 Word16 y_min,
                                 Word16 range,
                                 Word16 *estimated_size, 
                                 Word16 *exists_minus1)
{
	const scopeid_t scp0 = SPR_ENTER(2*2+2);

	Word16 codeindex, t;
	fract_frame_t fb_type;

	fb_type = 0; move16();

	test();
	IF ((sub(y_min, -5) <= 0) || (sub(y_max, 3) >= 0)) {
		SPR_LEAVE(scp0);
		return fb_type;
	}

	/* Return if range too low or y_min to high for special cases */
	IF (s_or(sub(range, 1), sub(1, y_min)) <= 0) {
		SPR_LEAVE(scp0);
		return fb_type;
	}

	/* Special case condition tests */

	/* Initialize flag indicating existence of level -1 */
	*exists_minus1 = 1; move16();
	codeindex = sub(range, 2);


	/* Tests for all frame sizes */


	/* Two level special case 2/2 */
	IF (sub(range, 2) == 0) {
		IF (y_min == 0) {
			t = sub(bytes_per_frame[codeindex][framelength_index], *estimated_size);
			IF (t < 0) {
				*estimated_size = bytes_per_frame[codeindex][framelength_index]; move16();
				fb_type = add(fb_2lev128anc40, fb_type_offsets[framelength_index]);
			}
		}
		SPR_LEAVE(scp0);
		return fb_type;
	}

	/* 4 level special cases 4/4 4/3 */
	IF (s_or(sub(range, 4), sub(y_min, -2)) == 0) {
		*exists_minus1 = level_exists(input, input_size, -1);

		if (*exists_minus1 <= 0)
			codeindex = sub(codeindex, 1);

		t = sub(bytes_per_frame[codeindex][framelength_index], *estimated_size);
		IF (t < 0) {
			*estimated_size = bytes_per_frame[codeindex][framelength_index]; move16();
			fb_type = add(fb_4lev126anc40z127, fb_type_offsets[framelength_index]);
			if (*exists_minus1 > 0)
				fb_type = add(fb_4lev126anc40, fb_type_offsets[framelength_index]);
		}
		SPR_LEAVE(scp0);
		return fb_type;
	}

	/* 3 level special case 3/2 */
	IF (s_or(sub(range, 3), sub(y_min, -2)) == 0) {
		*exists_minus1 = level_exists(input, input_size, -1);

		IF (*exists_minus1 == 0) {
			/* For all frame sizes */
			/* fb_type = fb_3lev126anc40z127; move16(); */
			codeindex = sub(codeindex, 1);
			t = sub(bytes_per_frame[codeindex][framelength_index], *estimated_size);
			IF (t < 0) {
				*estimated_size = bytes_per_frame[codeindex][framelength_index]; move16();
				fb_type = add(fb_3lev126anc40z127, fb_type_offsets[framelength_index]);
			}
			SPR_LEAVE(scp0);
			return fb_type;
		}
	}


	/* Return for framesizes 160 or greater */
	IF (sub(framelength_index, 2) >= 0) {
		SPR_LEAVE(scp0);
		return fb_type;
	}

	/* Cases for 80 and 40 sample frames both 3/3 */
	IF (sub(range, 3) == 0) {
		IF (sub(y_min, -1) == 0) {
			/* fb_type = fb_3lev127anc40; move16(); */
			t = sub(bytes_per_frame[codeindex][framelength_index], *estimated_size);
			IF (t < 0) {
				*estimated_size = bytes_per_frame[codeindex][framelength_index]; move16();
				fb_type = add(fb_3lev127anc40, fb_type_offsets[framelength_index]);
			}
			SPR_LEAVE(scp0);
			return fb_type;
		} ELSE IF(sub(y_min, -2) == 0) {
			/* This has already been calculated for range 3 in all framesize test */
			IF (*exists_minus1 > 0) {
				/* fb_type = fb_3lev126anc40; move16(); */
				t = sub(bytes_per_frame[codeindex][framelength_index], *estimated_size);
				IF (t < 0) {
					*estimated_size = bytes_per_frame[codeindex][framelength_index]; move16();
					fb_type = add(fb_3lev126anc40, fb_type_offsets[framelength_index]);
				}
				SPR_LEAVE(scp0);
				return fb_type;
			}
		}
	}

	/* Return if framelength 80 or greater */
	IF (framelength_index > 0) {
		SPR_LEAVE(scp0);
		return fb_type;
	}

	/* Cases for 40 samples per frame only */

	/* 5 levels special cases - one case has been removed, fb_5lev125anc40 */
	IF (sub(range, 5) == 0) {
		IF (sub(y_min, -2) == 0) {
			*exists_minus1 = level_exists(input, input_size, -1);

			if (*exists_minus1 <= 0)
				codeindex = sub(codeindex, 1);

			t = sub(bytes_per_frame[codeindex][framelength_index], *estimated_size);
			IF (t < 0) {
				*estimated_size = bytes_per_frame[codeindex][framelength_index]; move16();
				fb_type = fb_5lev126anc40z127; move16();
				if (*exists_minus1 > 0) {
					fb_type = fb_5lev126anc40; move16();
				}
			}
			SPR_LEAVE(scp0);
			return fb_type;
		}

		IF (sub(y_min, -3) == 0) {
			*exists_minus1 = level_exists(input, input_size, -1);

			IF (*exists_minus1 <= 0) {
				codeindex = sub(codeindex, 1);

				t = sub(bytes_per_frame[codeindex][framelength_index], *estimated_size);
				IF (t < 0) {
					*estimated_size = bytes_per_frame[codeindex][framelength_index]; move16();
					fb_type = fb_5lev125anc40z127; move16();
				}
			}
			SPR_LEAVE(scp0);
			return fb_type;
		}
	}

	/* 6 levels special cases */
	ELSE IF (sub(range, 6) == 0) {
		IF (sub(y_min, -3) == 0) {
			*exists_minus1 = level_exists(input, input_size, -1);

			if (*exists_minus1 <= 0)
				codeindex = sub(codeindex, 1);

			t = sub(bytes_per_frame[codeindex][framelength_index], *estimated_size);
			IF (t < 0) {
				*estimated_size = bytes_per_frame[codeindex][framelength_index]; move16();
				fb_type = fb_6lev125anc40z127; move16();
				if (*exists_minus1 > 0) {
					fb_type = fb_6lev125anc40; move16();
				}
			}
			SPR_LEAVE(scp0);
			return fb_type;
		}
	}
	/* 7 level special case */
	ELSE IF (s_or(sub(range,7), sub(y_min, -4)) == 0) {
		*exists_minus1 = level_exists(input, input_size, -1);

		IF (*exists_minus1 <= 0) {
			/* fb_type = fb_7lev124anc40z127; move16(); */
			codeindex = sub(codeindex, 1);
			t = sub(bytes_per_frame[codeindex][framelength_index], *estimated_size);
			IF (t < 0) {
				*estimated_size = bytes_per_frame[codeindex][framelength_index]; move16();
				fb_type = fb_7lev124anc40z127; move16();
			}
			SPR_LEAVE(scp0);
			return fb_type;
		}
	}

	/* No special case identified */

	SPR_LEAVE(scp0);

	return fb_type;
}

/*----------------------------------------------------------------------*/
Word16 fract_bits_encode(const Word16 *input_frame, 
                         Word16 input_frame_size, 
                         Word16 *output, 
                         Word16 output_size,
                         Word16 y_max,
                         Word16 y_min,
                         Word16 exists_minus1)
{
	const scopeid_t scp0 = SPR_ENTER(2+sizeof(struct output_bit_stream));

	Word16 levels;
	struct output_bit_stream bitstream;

	/* Initialize the output_frame bitstream - use NTT's functionality for output */
	output_bit_stream_open(output, output_size, 0, &bitstream);

	/* levels and index into the tables */
	levels = add(sub(y_max, y_min), exists_minus1);

	/* Special cases where straighforward bit coding is used */

	/* Encode two levels.  Two cases now exist.  Onee case of two 
	level coding is when there are three levels with y_min = -2
	and level -1 is missing. The other is when y_min = 0 and
	y_max = 1.*/
	IF (sub(levels, 2) == 0) {
		const scopeid_t scp1 = SPR_ENTER(2+2);

		/* Encode with a single bit */
		Word16 index;
		const Word16* input = input_frame; PTR1_MOVE();

		IF (exists_minus1 <= 0) {
			FOR (index=0; index<input_frame_size; ++index) {
				put_bit(shr(sub(*input++, y_min), 1), &bitstream);
			}
		} ELSE {
			FOR (index=0; index<input_frame_size; ++index) {
				put_bit(sub(*input++, y_min), &bitstream);
			}
		}

		SPR_LEAVE(scp1);
	}

	/* Encode four levels. */
	ELSE IF (sub(levels, 4) == 0) {
		const scopeid_t scp1 = SPR_ENTER(2+2);

		/* Encode with two bits */
		Word16 index;
		const Word16* input = input_frame; PTR1_MOVE();

		IF (exists_minus1 > 0) {
			FOR (index=0; index<input_frame_size; ++index) {
				put_bits_le8(sub(*input++, y_min), 2, &bitstream);
			}
		} ELSE {
			FOR (index=0; index<input_frame_size; ++index) {
				const scopeid_t scp2 = SPR_ENTER(2+2);

				Word16 bits;
				Word16 in = *input++; move16();
				bits = sub(in, y_min);
				
				assert(in != -1);
				if (in >= 0)
					bits = sub(bits, 1);

				put_bits_le8(bits, 2, &bitstream);

				SPR_LEAVE(scp2);
			}
		}

		SPR_LEAVE(scp1);
	}
	/* All other encodings use fractional-bit */
	ELSE
	{
		const scopeid_t scp1 = SPR_ENTER(2+2+2+2+2+6*2+2+2+4);

		Word16 index;
		Word16 samples_per_block;
		Word16 block_bit_size;
		Word16 sample_count;
		Word16 packword;
		Word16 powers[6];
		Word16 order;
		Word16 idx;
		Word32 t;

		/* Index into fractional-bit arrays */
		index = sub(levels, 2);

		/* Maximum number of samples to put in a block, and the bit-size of the
		block of optimal size */
		samples_per_block = (Word16 )samp_per_block[index]; move16();
		block_bit_size = (Word16 )bits_per_block[index]; move16();

		/* Order of the polynomial calculations */
		order = sub(samples_per_block, 1);

		/* Powers for the polynomial */
		powers[0] = levels; move16();
		FOR (idx=1; idx<order; ++idx) {
			t = L_mult0(powers[idx-1], levels);
			powers[idx] = extract_l(t); /* CHECK_MOVE: complexity included in extract_l()? */
		}

		/* Loop to do fractional-bit encode of each sample in the frame into
		blocks, and write the block data to the bitstream. */
		FOR (sample_count = input_frame_size; sample_count > 0; sample_count -= samples_per_block) {
			const scopeid_t scp2 = SPR_ENTER(2+4);

			Word16 coef;
			Word32 poly;

			/* Determine how many samples are to be put into next block */

			/* Full block can be filled */

			/* Put first sample in least-significant coefficient position */
			IF (exists_minus1 > 0) {
				poly = L_deposit_l(sub(*input_frame++, y_min));

				/* Build up the block polynomial with remaining samples */
				FOR (coef=0; coef<order; ++coef) {
					poly = L_mac0(poly, sub(*input_frame++, y_min), powers[coef]);
				}
			} ELSE {
				const scopeid_t scp3 = SPR_ENTER(2+2);

				Word16 p;
				Word16 in = *input_frame++; move16();
				p = sub(in, y_min);
				assert(in != -1);
				if (in >= 0)
					p = sub(p, 1);
				poly = L_deposit_l(p);

				/* Build up the block polynomial with remaining samples */
				FOR (coef=0; coef<order; ++coef) {
					in = *input_frame++; move16();
					p = sub(in, y_min);
					assert(in != -1);
					if (in >= 0)
						p = sub(p, 1);

					poly = L_mac0(poly, p, powers[coef]);
				}

				SPR_LEAVE(scp3);
			}

			/* Pack the block_bit_size bits of poly into the bitstream */

			/* least significant portion of poly first */
			packword = extract_l(poly);
			put_bits(packword, s_min(block_bit_size, 16), &bitstream);

			SPR_LEAVE(scp2);
		}

		SPR_LEAVE(scp1);
	}

	SPR_LEAVE(scp0);
	/* Return the size of the encoded data in whole bytes. */
	return length_in_bytes(&bitstream);
}


/*-----------------------------------------------------------------------
fract_bits_decode

This function decodes the bits that were encoded by fract_bits_encode.
Note that this function assumes that upon call the bitstream is at the
beginning of a byte.
---------------------------------------------------------------------*/
Word16 fract_bits_decode(Word16 mu_or_a,
                         struct input_bit_stream *bitstream, 
                         Word16 *output_frame, 
                         Word16 output_frame_size,
                         fract_frame_t frame_type )
{
	const scopeid_t scp0 = SPR_ENTER(2+2+2+2+2+2+2);

	Word16 levels;
	Word16 y_min;
	Word16 num_samples;
	Word16 exists127;
	Word16 j;
	const Word16 *from_linear;
	Word16 *output;

	/* Initialize level existing flag */
	exists127 = 1; move16();

	/* Working output pointer */
	output = output_frame; move16();

	/* Decode the fractional-bit frame type */
	SWITCH (frame_type)
	{

	case fb_2lev128anc40 :
		levels = 2; move16();
		num_samples = 40; move16();
		y_min = 0; move16();
		BREAK;

	case fb_3lev126anc40 :
		levels = 3; move16();
		num_samples = 40; move16();
		y_min = -2; move16();
		BREAK;
	case  fb_3lev126anc40z127 :
		levels = 2; move16();
		num_samples = 40; move16();
		y_min = -2; move16();
		exists127 = 0; move16();
		BREAK;
	case fb_3lev127anc40 :
		levels = 3; move16();
		num_samples = 40; move16();
		y_min = -1; move16();
		BREAK;
	case fb_4lev126anc40 :
		levels = 4; move16();
		num_samples = 40; move16();
		y_min = -2; move16();
		BREAK;
	case fb_4lev126anc40z127 :
		levels = 3; move16();
		num_samples = 40; move16();
		y_min = -2; move16();
		exists127 = 0; move16();
		BREAK;
	case fb_5lev125anc40z127 :
		levels = 4; move16();
		num_samples = 40; move16();
		y_min = -3; move16();
		exists127 = 0; move16();
		BREAK;
	case fb_5lev126anc40 :
		levels = 5; move16();
		num_samples = 40; move16();
		y_min = -2; move16();
		BREAK;
	case fb_5lev126anc40z127 :
		levels = 4; move16();
		num_samples = 40; move16();
		y_min = -2; move16();
		exists127 = 0; move16();
		BREAK;
	case fb_6lev125anc40 :
		levels = 6; move16();
		num_samples = 40; move16();
		y_min = -3; move16();
		BREAK;
	case fb_6lev125anc40z127 :
		levels = 5; move16();
		num_samples = 40; move16();
		y_min = -3; move16();
		exists127 = 0; move16();
		BREAK;
	case fb_7lev124anc40z127 :
		levels = 6; move16();
		num_samples = 40; move16();
		y_min = -4; move16();
		exists127 = 0; move16();
		BREAK;

	case fb_2lev128anc80 :
		levels = 2; move16();
		num_samples = 80; move16();
		y_min = 0; move16();
		BREAK;

	case fb_4lev126anc80 :
		levels = 4; move16();
		num_samples = 80; move16();
		y_min = -2; move16();
		BREAK;
	case fb_3lev126anc80z127 :
		levels = 2; move16();
		num_samples = 80; move16();
		y_min = -2; move16();
		exists127 = 0; move16();
		BREAK;
	case fb_4lev126anc80z127 :
		levels = 3; move16();
		num_samples = 80; move16();
		y_min = -2; move16();
		exists127 = 0; move16();
		BREAK;
	case fb_3lev126anc80 :
		levels = 3; move16();
		num_samples = 80; move16();
		y_min = -2; move16();
		BREAK;
	case fb_3lev127anc80 :
		levels = 3; move16();
		num_samples = 80; move16();
		y_min = -1; move16();
		BREAK;

	case fb_2lev128anc160 :
		levels = 2; move16();
		num_samples = 160; move16();
		y_min = 0; move16();
		BREAK;

	case fb_4lev126anc160 :
		levels = 4; move16();
		num_samples = 160; move16();
		y_min = -2; move16();
		BREAK;
	case fb_3lev126anc160z127 :
		levels = 2; move16();
		num_samples = 160; move16();
		y_min = -2; move16();
		exists127 = 0; move16();
		BREAK;
	case fb_4lev126anc160z127 :
		levels = 3; move16();
		num_samples = 160; move16();
		y_min = -2; move16();
		exists127 = 0; move16();
		BREAK;

	case fb_2lev128anc240 :
		levels = 2; move16();
		num_samples = 240; move16();
		y_min = 0; move16();
		BREAK;

	case fb_4lev126anc240 :
		levels = 4; move16();
		num_samples = 240; move16();
		y_min = -2; move16();
		BREAK;
	case fb_3lev126anc240z127 :
		levels = 2; move16();
		num_samples = 240; move16();
		y_min = -2; move16();
		exists127 = 0; move16();
		BREAK;
	case fb_4lev126anc240z127 :
		levels = 3; move16();
		num_samples = 240; move16();
		y_min = -2; move16();
		exists127 = 0; move16();
		BREAK;

	case fb_2lev128anc320 :
		levels = 2; move16();
		num_samples = 320; move16();
		y_min = 0; move16();
		BREAK;

	case fb_4lev126anc320 :
		levels = 4; move16();
		num_samples = 320; move16();
		y_min = -2; move16();
		BREAK;
	case fb_3lev126anc320z127 :
		levels = 2; move16();
		num_samples = 320; move16();
		y_min = -2; move16();
		exists127 = 0; move16();
		BREAK;
	case fb_4lev126anc320z127 :
		levels = 3; move16();
		num_samples = 320; move16();
		y_min = -2; move16();
		exists127 = 0; move16();
		BREAK; 
	default :
		assert( FALSE );
	}


	assert( output_frame_size >= num_samples );
	setFrameRate(g711llc_default_hz, num_samples);

	/* Special cases where straighforward bit coding is used */

	/* Decode two levels.  Right now there are two cases of
	this, and that is when there are three levels with y_min = -2
	and level -1 is missing, or y_min = 0 and y_max = 1. */
	IF (sub(levels, 2) == 0) {
		const scopeid_t scp1 = SPR_ENTER(2+2);

		Word16 sample, d;
		d = 1; move16();
		if (exists127 <= 0) {
			d = 2; move16();
		}
		FOR (sample=0; sample<num_samples; ++sample) {
			const scopeid_t scp2 = SPR_ENTER(2);

			Word16 bit = get_bit(bitstream);
			if (bit == 0) {
				*output++ = y_min; move16();
			}
			if (bit != 0) {
				*output++ = add(y_min, d); move16();
			}

			SPR_LEAVE(scp2);
		}

		SPR_LEAVE(scp1);
	}
	/* Decode four-level data, which may have missing level -1 */
	ELSE IF (sub(levels, 4) == 0) {
		const scopeid_t scp1 = SPR_ENTER(2);

		Word16 sample;

		/* If the level -1 is not missing, there are contiguous levels */
		IF (exists127 > 0) {
			FOR (sample=0; sample<num_samples; ++sample) {
				const scopeid_t scp2 = SPR_ENTER(2);

				Word16 value = get_bits_le8(2, bitstream);
				*output++ = add(value, y_min); move16();

				SPR_LEAVE(scp2);
			}
		} ELSE {
			/* Level -1 missing */
			FOR (sample=0; sample<num_samples; ++sample) {
				const scopeid_t scp2 = SPR_ENTER(2);

				Word16 value = get_bits_le8(2, bitstream);
				value = add(value, y_min);
				if (add(value, 1) >= 0)
					value = add(value, 1);
				*output++ = value; move16();

				SPR_LEAVE(scp2);
			}
		}

		SPR_LEAVE(scp1);
	}
	/* All other cases use fractional-bit decoding */
	ELSE {
		const scopeid_t scp1 = SPR_ENTER(2+2+2+2+6*2+2+2+4);

		Word16 index;
		Word16 samples_per_block;
		Word16 block_bit_size;
		Word16 sample_count;
		Word16 powers[6];
		Word16 order;
		Word16 idx;
		Word32 t;

		/* Index into data tables */
		index = sub(levels, 2);

		/* fractional-bit decoding parameters */
		samples_per_block = (Word16 )samp_per_block[index]; move16();
		block_bit_size = (Word16 )bits_per_block[index]; move16();

		/* Order of the polynomial calculations for full block */
		order = sub(samples_per_block, 1);

		/* Powers for the polynomial */
		powers[0] = levels; move16();
		FOR (idx=1; idx<order; ++idx) {
			t = L_mult0(powers[idx-1], levels);
			powers[idx] = extract_l(t); /* CHECK_MOVE: complexity included in extract_l()? */
		}

		/* Loop to extract all of the blocks and extract samples */
		FOR (sample_count = num_samples; sample_count > 0; sample_count -= samples_per_block) {
			const scopeid_t scp2 = SPR_ENTER(6*2+2);

			Word16 coef[6];
			Word16 block;

			/* Determine how many samples are in the next block */

			/* Full block can be read */

			/* Obtain the block of coded data */

			/* Obtain the least-significant portion of the block */
			block = get_bits(block_bit_size, bitstream);


			/* Search to extract the coefficients of the polynomial of the block */
			FOR (idx=order; idx>0; --idx) {
				const scopeid_t scp3 = SPR_ENTER(2*2);

				Word16 cidx, pidx;
				cidx = 0; move16();
				pidx = powers[idx-1]; move16();

				WHILE (sub(block, pidx) >= 0) {
					block = sub(block, pidx);
					cidx = add(cidx, 1);
				}

				coef[idx] = cidx; move16();

				SPR_LEAVE(scp3);
			}

			/* Zero'th coefficient is remaining value after all higher order subtractions */
			coef[0] = block; move16();

			/* Output the coefficients of the block */

			/* output data biased by y_min.  There may be cases of missing level 127. */
			IF (exists127 > 0) {
				/* Data is contiguous */
				FOR (idx=0; idx<=order; ++idx) {
					*output++ = add(coef[idx], y_min); move16();
				}
			} ELSE {
				/* Data is missing level 127 */
				FOR (idx=0; idx<=order; ++idx) {
					const scopeid_t scp3 = SPR_ENTER(2);

					Word16 val = add(coef[idx], y_min);
					if (add(val, 1) >= 0)
						val = add(val, 1);
					*output++ = val; move16();

					SPR_LEAVE(scp3);
				}
			}

			SPR_LEAVE(scp2);
		}

		SPR_LEAVE(scp1);
	}


	/* This code for conversion from the linear to mu/a-law format borrowed from the Cisco decoder */

	/* Map the 0 - 255 "linear" values to mu-law or A-law G711 output format */
	from_linear = from_linear_alaw + 128; PTR1_MOVE();
	if (mu_or_a) {
		from_linear = from_linear_ulaw + 128; PTR1_MOVE();
	}
	FOR (j=0; j<num_samples; ++j) {
		output_frame[j] = add(from_linear[output_frame[j]], 128); move16(); /* CHECK_MOVE */
	}

	SPR_LEAVE(scp0);
	return num_samples;
}
