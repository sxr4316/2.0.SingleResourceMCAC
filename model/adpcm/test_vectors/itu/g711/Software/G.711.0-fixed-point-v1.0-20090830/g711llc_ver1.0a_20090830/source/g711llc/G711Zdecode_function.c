/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: G711Zdecode_function.c
 Contents: G.711-LLC Min Max level decoder function.
 Version: 1.00
 Revision Date: July 24, 2009
**************************************************************************/

#include <assert.h>
#include "g711llc_defines.h"
#include "../basicop/basicop.h"
#include "tables.h"
#include "G711Zdecode_function.h"
#include "../utility/stack_profile.h"

Word16 G711Zdecompress(BOOL mu_or_a, const Word16 *g711z_input_frame, Word16 *g711z_input_frame_length, Word16 *g711z_output_frame, Word16 output_frame_length)
{
	const scopeid_t scp0 = SPR_ENTER(2+2+2+2+2*2+2+2);

	UWord16 j;
	UWord16 n_codepoint;
	UWord16 a_codepoint;
	UWord16 num_bits, y_anchor;
	UWord16 number_of_samples;
	const Word16 *from_linear;

	assert(mu_or_a == 0 || mu_or_a == 1);
	assert(*g711z_input_frame_length >= 2); /* minimum input frame length is 2 bytes */

	/* Start processing by decomposing overhead byte */
	j = 1; move16();                             /* j is the g711z byte index */
	n_codepoint = s_and(lshr(g711z_input_frame[0], 5), 0x07);
	a_codepoint = s_and(g711z_input_frame[0], 0x1f);

	test();
	IF ((n_codepoint == 0) && (sub(a_codepoint, 30) == 0)) {
		num_bits = 8; move16();          /* a_codepoint=30 and n_codepoint=0 signals */
		y_anchor = -128; move16();       /* num_bits=8 and 8 bit encodings are anchored at -128 */
	} ELSE {
		num_bits = n_codepoint; move16();  /* num_bits is known here */
	}

	/* find y_anchor (if num_bits != 8) */
	IF (sub(num_bits, 8) != 0) {
		assert(a_codepoint != 30); /* unexpected reserved code found - exit/report error */
		IF (sub(a_codepoint, 31) == 0) { /* explicit anchor case */
			y_anchor = sub(g711z_input_frame[j], 128);
			j = add(j, 1);
		} ELSE {
			assert(a_codepoint < 30);
			y_anchor = (Word16 )y_anchor_table[a_codepoint]; move16();
		}
	}                 /* end of "if(num_bits !=8)" statement */

	/* For num_bits=0 case, initialize the output frame with zeros (y_anchor added later) */
	IF (sub(num_bits, 1) == 0) { /*only for 40 */
		const scopeid_t scp1 = SPR_ENTER(2*2);
		Word16 i, ii;
		number_of_samples = output_frame_length; move16();
		FOR(i=0; i<5; i++){
			g711z_output_frame[i*8] = add(shr(g711z_input_frame[j+i], 7), y_anchor); move16();
			g711z_output_frame[i*8+7] = add(s_and(g711z_input_frame[j+i], 1), y_anchor); move16();
			FOR(ii=6; ii>0; ii--){
				g711z_output_frame[i*8+7-ii] = add(s_and(shr(g711z_input_frame[j+i], ii), 1), y_anchor); move16();
			}
		}
		*g711z_input_frame_length = add(j,5);/* Store the number of processed bytes */
		SPR_LEAVE(scp1);
	} 
	ELSE IF (sub(num_bits, 2) == 0){
		const scopeid_t scp1 = SPR_ENTER(2);
		Word16 i;
		number_of_samples = output_frame_length; move16();
		FOR(i=0; i<10; i++){
			g711z_output_frame[i*4] = add(shr(g711z_input_frame[j+i], 6), y_anchor); move16();
			g711z_output_frame[i*4+1] = add(s_and(shr(g711z_input_frame[j+i], 4), 3), y_anchor); move16();
			g711z_output_frame[i*4+2] = add(s_and(shr(g711z_input_frame[j+i], 2), 3), y_anchor); move16();
			g711z_output_frame[i*4+3] = add(s_and(g711z_input_frame[j+i], 3), y_anchor); move16();
		}
		*g711z_input_frame_length = add(j, 10);
		SPR_LEAVE(scp1);
	}
	ELSE IF (sub(num_bits, 4) == 0) {
		const scopeid_t scp1 = SPR_ENTER(2*2);
		Word16 i;
		number_of_samples = output_frame_length; move16();
		FOR(i=0; i<20; i++){
			g711z_output_frame[i*2] = add(shr(g711z_input_frame[j+i], 4), y_anchor); move16();
			g711z_output_frame[i*2+1] = add(s_and(g711z_input_frame[j+i], 0xf), y_anchor); move16();
		}
		*g711z_input_frame_length = add(j, 20);
		SPR_LEAVE(scp1);
	}ELSE { /* Unpacking routine begins here (for num_bits !=0 case) */
		const scopeid_t scp1 = SPR_ENTER(2+2);
		UWord16 i;
		Word16 bits_left_in_byte;
		number_of_samples = output_frame_length; move16();
		i = 0; move16();                       /* "i" is the sample counter (g711z_input_frame) index */
		bits_left_in_byte = 8; move16();                            /* bit accounting for unused bits */

		WHILE (sub(i, number_of_samples) < 0) { /* Do while more samples to process */
			/* Case 1: Entire sample is in present byte */
			IF (sub(bits_left_in_byte, num_bits) >= 0) {
				const scopeid_t scp2 = SPR_ENTER(2);

				UWord16 uint8_temp1;
				bits_left_in_byte = sub(bits_left_in_byte, num_bits);
				uint8_temp1 = lshr(g711z_input_frame[j], bits_left_in_byte);

				g711z_output_frame[i] = add(s_and(uint8_temp1, G711Z_mask[num_bits]), y_anchor); move16(); /* mask MSBs not in sample */ /* CHECK_MOVE */
				i = add(i, 1);
				if (bits_left_in_byte == 0)
					j = add(j, 1);                               /* go onto next RGL sample byte */
				if (bits_left_in_byte == 0) {
					bits_left_in_byte = 8; move16();                 /* refill bits_left_in_byte */
				}

				SPR_LEAVE(scp2);
			} /* close of Case 1 "if" */
			/* Case 2: Part of sample in present byte and the other part is in */
			/* the subsequent byte */
			ELSE {
				const scopeid_t scp2 = SPR_ENTER(2*2+2);

				UWord16 uint8_temp1, uint8_temp2;
				UWord16 num_bits_unpacked = sub(num_bits, bits_left_in_byte); /*accounting for bits in subsequent byte*/

				assert(1 <= num_bits && num_bits <= 7);
				uint8_temp1 = s_and(g711z_input_frame[j], G711Z_mask[bits_left_in_byte]); /* move16(); */ /* mask MSBs not in sample */ /* CHECK_MOVE */

				/* first part of sample */
				uint8_temp1 = lshl(uint8_temp1, num_bits_unpacked);
				/* second part of sample */
				j = add(j, 1);
				bits_left_in_byte = sub(8, num_bits_unpacked); /* set this counter */
				uint8_temp2 = lshr(g711z_input_frame[j], bits_left_in_byte);
				/* put sample together */
				g711z_output_frame[i] = add(s_or(uint8_temp1, uint8_temp2), y_anchor); move16();
				i = add(i, 1);

				SPR_LEAVE(scp2);
			} /* close of Case 2 "else if" */

		} /* close of while statement */
		if (sub(bits_left_in_byte, 8) == 0)
			j = sub(j, 1);                               /* go back one byte */

		*g711z_input_frame_length = add(j, 1); /* Store the number of processed bytes */

		SPR_LEAVE(scp1);
	}   /* close of else of "if( num_bits == 0 )" way above */
	/* Map the 0 - 255 linear values to mu-law or A-law G711 output format */
	from_linear = from_linear_alaw + 128; PTR1_MOVE();
	if (mu_or_a != 0) {
		from_linear = from_linear_ulaw + 128; PTR1_MOVE();
	}
	FOR (j=0; j<number_of_samples; j++) {
		g711z_output_frame[j] = add(from_linear[g711z_output_frame[j]], 128); move16(); /* CHECK_MOVE */
	}
	SPR_LEAVE(scp0);
	return number_of_samples;
}
