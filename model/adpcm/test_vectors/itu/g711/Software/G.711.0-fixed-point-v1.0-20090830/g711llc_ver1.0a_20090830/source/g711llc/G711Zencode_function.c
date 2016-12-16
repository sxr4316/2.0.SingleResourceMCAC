/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: G711Zencode_function.c
 Contents: G.711-LLC Min Max level encoder functions.
 Version: 1.00
 Revision Date: July 24, 2009
**************************************************************************/

#include <assert.h>
#include "g711llc_defines.h"
#include "../basicop/basicop.h"
#include "tables.h"
#include "G711Zencode_function.h"
#include "../utility/stack_profile.h"

/* Optimized for basic operations */
void G711Zcompress_analyze_low(BOOL mu_or_a, const Word16 *input_frame, Word16 *converted_frame, Word16 input_frame_size, Word16 *y_min, Word16 *y_max, Word16 *range)
{
	const scopeid_t scp0 = SPR_ENTER(2+2+2);
	const Word16 *from_linear;
	UWord16 i;
	Word16 in;

	/* Determine if input frame was from a A-law or mu-law encoded, then */
	/* map to desired {-128 .. 127} linear range                         */

	/* Also find y_min and y_max */
	*y_min = 127; move16(); /* init y_min */
	*y_max = -128; move16();   /* init y_max */

	from_linear = from_linear_alaw; PTR1_MOVE();
	if (mu_or_a != 0) {
		from_linear = from_linear_ulaw; PTR1_MOVE();
	}

	FOR (i=0; i<input_frame_size; i++) {
		in = from_linear[input_frame[i]]; move16(); /* CHECK_MOVE */
		*y_max = s_max(*y_max, in);
		*y_min = s_min(*y_min, in);
		converted_frame[i] = in; move16();
	}

	/* Find minimum number of bits needed to code the frame using */
	/* an explicit anchor (at *y_min and fd_min)                  */
	*range = add(sub(*y_max, *y_min), 1);

	SPR_LEAVE(scp0);
}

/* Optimized for basic operations */
Word16 G711Zcompress_encode(Word16 *input_frame, Word16 input_frame_size, Word16 *output_frame, Word16 y_min, Word16 y_max, Word16 num_bits)
{
	const scopeid_t scp0 = SPR_ENTER(2+2+3*2+2+2+2*2);

	int output_frame_size;
	BOOL explicit_anchor_flag;
	Word16 n_codepoint, a_codepoint, y_anchor;
	UWord16 sample_byte_count;
	Word16 bits_left_in_byte;                      /* accounting for bits unused in byte */
	Word16 i, j;
	explicit_anchor_flag = FALSE; move16();
	
	assert(input_frame_size % 8 == 0); /* Test to make sure frame_size is an integer multiple of 8 samples */

	/* If the minimum number of bits is 8, then y_anchor=-128 by default.*/
	/* Set n_codepoint, a_codepoint and y_anchor and don't bother with   */
	/* the num_bits !=8 logic below. Note: The explicit_anchor_flag was  */
	/* initialized to false (so I don't need to set it here).            */
	IF (sub(num_bits, 8) == 0) {
		n_codepoint = 0; move16();    /* set the N and A bits for an 8 bit encoding */
		a_codepoint = 30; move16();
		y_anchor = -128; move16();
	} ELSE {
		const scopeid_t scp1 = SPR_ENTER(2+2+2+2);
		Word16 y_tmp;
		Word16 samples_codepoint;
		Word16 y_tentative_anchor;
		Word16 num_bits_codepoint;

		IF (sub(y_min, -29) >= 0) {
			IF (sub(y_min, -17) >= 0) {
				IF (sub(y_min, 1) >= 0) {
					move16(); move16();
					y_tentative_anchor = 1; a_codepoint = 0;
				} ELSE IF (sub(y_min, -7) >= 0) {
					move16(); 
					y_tentative_anchor = y_min; 
					a_codepoint = sub(1, y_min);
				} ELSE /* IF (sub(y_min, -17) >= 0)*/ {
					y_tmp = shr(sub(y_min, 1), 1);
					y_tentative_anchor = add(shl(y_tmp, 1), 1); 
					a_codepoint = sub(68, shr(add(y_min, 127), 1));
				}
			} ELSE { /* -29 */
				IF (sub(y_min, -23) >= 0) {
					IF (sub(y_min, -20) >= 0) {
						move16(); move16();
						y_tentative_anchor = -20; a_codepoint = 14;
					} ELSE {
						move16(); move16();
						y_tentative_anchor = -23; a_codepoint = 15;
					}
				} ELSE {
					IF (sub(y_min, -26) >= 0) {
						move16(); move16();
						y_tentative_anchor = -26; a_codepoint = 16;
					} ELSE { 
						move16(); move16();
						y_tentative_anchor = -29; a_codepoint = 17;
					}
				}
			} 
		} ELSE {
			IF (sub(y_min, -68) >= 0) {
				IF (sub(y_min, -48) >= 0) {
					y_tmp = shr(y_min, 2);
					y_tentative_anchor = shl(y_tmp, 2); 
					a_codepoint = sub(42, shr(add(y_min, 128), 2));
				} ELSE /*IF (sub(y_min, -68) >= 0)*/ {
					IF (sub(y_min, -58) >= 0) {
						IF (sub(y_min, -53) >= 0) {
							move16(); move16();
							y_tentative_anchor = -53; a_codepoint = 23;
						} ELSE {
							move16(); move16();
							y_tentative_anchor = -58; a_codepoint = 24;
						}
					} ELSE {
						IF (sub(y_min, -63) >= 0) {
							move16(); move16();
							y_tentative_anchor = -63; a_codepoint = 25;
						} ELSE {
							move16(); move16();
							y_tentative_anchor = -68; a_codepoint = 26;
						}
					}
				}
			} ELSE {
				IF (sub(y_min, -80) >= 0) {
					IF (sub(y_min, -74) >= 0) {
						move16(); move16();
						y_tentative_anchor = -74; a_codepoint = 27;
					} ELSE {
						move16(); move16();
						y_tentative_anchor = -80; a_codepoint = 28;
					}
				} ELSE {
					IF (sub(y_min, -87) >= 0) {
						move16(); move16();
						y_tentative_anchor = -87; a_codepoint = 29;
					} ELSE {
						move16(); move16();
						y_tentative_anchor = -128; explicit_anchor_flag = TRUE;
					}
				}
			}
		}

		/* Find number of bits per sample using fixed codepoint anchors */
		samples_codepoint = sub(y_max, y_tentative_anchor);
		num_bits_codepoint = sub(15, norm_s(samples_codepoint));
		if (samples_codepoint == 0) {
			num_bits_codepoint = 0; move16();
		}
		test();
		IF ((sub(num_bits_codepoint, num_bits) == 0) && (explicit_anchor_flag == 0)) {
			y_anchor = y_tentative_anchor; move16();
		} ELSE {
			move16();
			move16();
			move16();
			y_anchor = y_min;         /* need explicit anchor, set it to y_min, */
			a_codepoint = 31;                 /* set a_codepoint and set anchor */
			explicit_anchor_flag = TRUE;  /* set flag if it hasn't been set yet */
		}
		move16();
		n_codepoint = num_bits;           /* set n_codepoint for num_bits != 8 case */
		SPR_LEAVE(scp1);
	} /* Close the "else" (of "if(num_bits==8)" statement) way above */

	/* Packing Routine begins here */
	/* Pack the overhead byte(s): n_codepoint, a_codepoint and */
	/* and explicit_anchor (if required) */
	/*sample_byte_count = (input_frame_size*num_bits)/8;*/
	sample_byte_count = mult(shl(input_frame_size, 1), shl(num_bits, 11));

	/* Include 1st overhead byte in count */
	output_frame_size = add(sample_byte_count, 1); 
	output_frame[0] = s_or(shl(n_codepoint, 5), a_codepoint); move16(); /* Pack "N" and "A" bits */

	j = 1; move16();                                   /* "j" is output_frame array index */

	IF (explicit_anchor_flag != 0) {
		output_frame[j] = add(y_anchor, 128); move16();               /* Pack the explicit anchor byte */
		j = add(j, 1);
		output_frame_size = add(output_frame_size, 1);    /* Include this byte in size count */
	}

	/* If num_bits == 0, put value representing how many samples of   */
	/* y_anchor are to be replicated in the unzipped output (which is */
	/* 8 times the value placed here).                                */
	/* If num_bits !=0, then pack the bits into the compressed frame  */
	bits_left_in_byte = 8; move16();
	output_frame[j] = 0; move16();

	FOR (i=0; i<input_frame_size; ++i) {
		/* Case: Entire sample fits in present byte here */
		bits_left_in_byte = sub(bits_left_in_byte, num_bits);
		IF (bits_left_in_byte >= 0) {
			const scopeid_t scp1 = SPR_ENTER(2);
			Word16 temp = shl(sub(input_frame[i], y_anchor), bits_left_in_byte);
			output_frame[j] = s_or(output_frame[j], temp); move16();
			IF (bits_left_in_byte == 0) {
				j = add(j, 1);            /* go onto next output buffer byte */
				output_frame[j] = 0; move16();
				bits_left_in_byte = 8; move16(); /* refill bits_left_in_byte */
			}
			SPR_LEAVE(scp1);
		}
		/* Case: Part of sample fits in present byte (put other part in */
		/*       subsequent byte).                                      */
		ELSE {
			const scopeid_t scp1 = SPR_ENTER(2+2);
			/* pack as many bits as possible */
			Word16 in = sub(input_frame[i], y_anchor);
			Word16 temp = lshl(in, bits_left_in_byte);
			assert(0 <= temp && temp <= 255);
			output_frame[j] = s_or(output_frame[j], temp); move16();
			/* go onto next output buffer byte */
			j = add(j, 1);
			/* pack remaining bits */
			bits_left_in_byte = add(8, bits_left_in_byte);  /* set bits_left_in_byte */
			output_frame[j] = shl(in, bits_left_in_byte); move16();
			SPR_LEAVE(scp1);
		}
	} /* closing for loop */

	SPR_LEAVE(scp0);
	return output_frame_size;
}

/* Optimized for basic operations */
Word16 G711Zcompress_encode_low(Word16 *input_frame, Word16 input_frame_size, Word16 *output_frame, Word16 y_min, Word16 y_max, Word16 num_bits)
{
	const scopeid_t scp0 = SPR_ENTER(2+2+2*2+2+2+2+2+2+2);
	Word16 y_tmp;
	int output_frame_size;
	BOOL explicit_anchor_flag;
	Word16 a_codepoint, y_anchor;
	UWord16 sample_byte_count;
	Word16 j;
	Word16 num_bits_codepoint;
	Word16 samples_codepoint;
	Word16 y_tentative_anchor;

	explicit_anchor_flag = FALSE; move16();

	assert(input_frame_size % 8 == 0); /* Test to make sure frame_size is an integer multiple of 8 samples */


	/* If the minimum number of bits is 8, then y_anchor=-128 by default.*/
	/* Set n_codepoint, a_codepoint and y_anchor and don't bother with   */
	/* the num_bits !=8 logic below. Note: The explicit_anchor_flag was  */
	/* initialized to false (so I don't need to set it here).            */

	IF(sub(y_min, 1)>=0) {
		move16(); move16();
		y_tentative_anchor = 1; a_codepoint =  0;
	} ELSE IF (sub(y_min, -7) >= 0) { /* 8 - 1 */
		move16(); 
		y_tentative_anchor = y_min; 
		a_codepoint = sub(1, y_min);
	} ELSE IF (sub(y_min, -17) >= 0) { /* 13 - 9 */
		y_tmp = shr(sub(y_min, 1), 1);
		y_tentative_anchor = add(shl(y_tmp, 1), 1); 
		a_codepoint = sub(68, shr(add(y_min, 127), 1));
	} ELSE IF (sub(y_min, -29) >= 0) { /* 17 - 14 */
		/* CHECK_MOVE 20090530 */
		y_tmp = sub(y_min, -29);
		y_tentative_anchor = (Word16 )tab_anchor1[y_tmp]; move16(); 
		a_codepoint = (Word16 )tab_codepoint1[y_tmp]; move16();
	} ELSE IF (sub(y_min, -48) >= 0) { /* 22- 18 */
		y_tmp = shr(y_min, 2);
		y_tentative_anchor = shl(y_tmp, 2);
		a_codepoint = sub(42, shr(add(y_min, 128), 2));
	} ELSE IF (sub(y_min, -87) >= 0){ /* 29 - 23*/
		/* CHECK_MOVE 20090530 */
		y_tmp = sub(y_min, -87);
		y_tentative_anchor = (Word16 )tab_anchor[y_tmp]; move16();
		a_codepoint = (Word16 )tab_codepoint[y_tmp]; move16();
	} ELSE {
		move16(); move16();
		y_tentative_anchor = -128; explicit_anchor_flag = TRUE;
	}
	/* Find number of bits per sample using fixed codepoint anchors */
	samples_codepoint = add(sub(y_max, y_tentative_anchor), 1);

	IF (sub(samples_codepoint,  4) <= 0 ) {
		num_bits_codepoint = (Word16 )tab_codepoint2[samples_codepoint]; move16();
	} ELSE {
		move16();
		num_bits_codepoint = 3;
	}	

	test();
	IF ((sub(num_bits_codepoint, num_bits) == 0) && (explicit_anchor_flag == 0)) {
		y_anchor = y_tentative_anchor; move16();
		/* Packing Routine begins here */
		/* Pack the overhead byte(s): n_codepoint, a_codepoint and */
		/* and explicit_anchor (if required) */
		/*sample_byte_count = (input_frame_size*num_bits)/8;*/

		sample_byte_count = extract_l(L_mult0(5, num_bits)); /* only for 40 sample*/

		/* Include 1st overhead byte in count */
		output_frame_size = add(sample_byte_count, 1); 
		output_frame[0] = s_or(shl(num_bits/*n_codepoint*/, 5), a_codepoint); move16(); /* Pack "N" and "A" bits */
		j = 1; move16();        /* "j" is output_frame array index */
	} ELSE {
		sample_byte_count = extract_l(L_mult0(5, num_bits)); /* only for 40 sample*/
		/* Include 1st overhead byte in count */
		y_anchor = y_min; move16();
		/* set a_codepoint and set anchor  a_codepoint = 31;*/
		output_frame[0] = s_or(shl(num_bits, 5), 31); move16(); /* Pack "N" and "A" bits */
		output_frame[1] = add(y_min, 128); move16();               /* Pack the explicit anchor byte */
		j = 2; move16();
		output_frame_size = add(sample_byte_count, 2);    /* Include this byte in size count */
	}

	IF (sub(num_bits, 2) == 0) {/*only for 40 sample *//*num_bits ==2 or 1 */
		const scopeid_t scp1 = SPR_ENTER(3*2);
		Word16 i, temp, i2; 
		FOR(i=0; i<10; i++){
			i2=shl(i,2);
			temp = shl(sub(input_frame[i2], y_anchor), 6);
			temp = s_or(temp,shl(sub(input_frame[i2+1], y_anchor), 4));
			temp = s_or(temp,shl(sub(input_frame[i2+2], y_anchor), 2));
			temp = s_or(temp, sub(input_frame[i2+3], y_anchor));
			output_frame[j+i] = temp; move16();
		}
		SPR_LEAVE(scp1);
	} ELSE /*(num_bits == 1)*/ {/*only for 40 sample */
		const scopeid_t scp1 = SPR_ENTER(4*2);
		Word16 i, ii, i3, temp; 
		FOR(i=0; i<5; i++){
			i3=shl(i,3);
			temp = shl(sub(input_frame[i3], y_anchor),7);
			temp = s_or(temp,sub(input_frame[i3+7], y_anchor));
			i3=add(i3,7);
			FOR(ii=6; ii>0; ii--){
				temp = s_or(temp,shl(sub(input_frame[i3-ii], y_anchor), ii));
			}
			output_frame[j+i] = temp; move16();
		}
		SPR_LEAVE(scp1);
	}	
	SPR_LEAVE(scp0);
	return output_frame_size;
}

