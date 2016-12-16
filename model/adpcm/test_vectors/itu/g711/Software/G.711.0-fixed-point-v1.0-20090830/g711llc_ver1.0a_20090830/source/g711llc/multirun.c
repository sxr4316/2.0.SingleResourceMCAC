/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: multirun.c
 Contents: G.711-LLC Value-location coder functions.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#include <assert.h>
#include "tables.h"
#include "input_bit_stream.h"
#include "output_bit_stream.h"
#include "multirun.h"
#include "../utility/stack_profile.h"

/* Indexing function for contiguous coding */
static Word16 next_offset(Word16 levels, Word16 noncodeindex, Word16 offset, Word16 *increment, Word16 *toggle);

/*------------------------------------------------------------------*/
Word16 multirun_analyze(Word16 framelength_index, Word16 input_size, const Word16 *lindata, Word16 y_max, Word16 y_min, mr_hist_t *hist)
{
	const scopeid_t scp0 = SPR_ENTER(2+2+2);
	Word16 index;
	Word16 level; 
	Word16 maxcount;

	/* Zero-based number of levels to encode */
	hist->levels = sub(y_max, y_min);
	IF (sub(hist->levels, MR_MAXMINDIFF) > 0) {
		SPR_LEAVE(scp0);
		return -1;
	}

	/* Test for special cases to encode */
	SWITCH (hist->levels) {
		case 1:
			IF (y_min != 0) {
				SPR_LEAVE(scp0);
				return -1;
			}
			BREAK;
		case 2:
			IF (sub(y_min, -3) <= 0) {
				SPR_LEAVE(scp0);
				return -1;
			}
			IF (sub(y_max, 2) >= 0) {
				SPR_LEAVE(scp0);
				return -1;
			}
			BREAK;
		case 3:
			IF (sub(y_min, -2) != 0) {
				SPR_LEAVE(scp0);
				return -1;
			}
			IF (sub(framelength_index, 2) <= 0) {
				SPR_LEAVE(scp0);
				return -1;
			}
			BREAK;
		default:
			assert( FALSE );
			SPR_LEAVE(scp0);
			return -1;
	}


	/* Calculate histogram data for multirun.  */

	/* Clear histogram data */
	FOR (index=0; index<=hist->levels; ++index) {
		hist->count[index] = 0; move16();
	}

	/* Gather histogram info about input frame data */
	FOR (index=0; index<input_size; ++index) {
		const scopeid_t scp1 = SPR_ENTER(2);
		Word16 countindex;
		countindex = sub(lindata[index], y_min);
		hist->count[countindex] = add(hist->count[countindex], 1); move16(); /* CHECK_MOVE */
		SPR_LEAVE(scp1);
	}

	/* Determine max count level */
	maxcount = 0; move16();

	FOR (level=0; level<=hist->levels; ++level) {
		maxcount = s_max(maxcount, hist->count[level]);
		move16();
		if (sub(hist->count[level], maxcount) == 0) {
			hist->maxcountidx = level; move16();
		}
	}

	/* Only run-length encode if maxcount level is 0 */
	IF (add(hist->maxcountidx, y_min) != 0) {
		SPR_LEAVE(scp0);
		return -1;
	}


	/* Use run-length encoding, remember y_min and y_max */
	hist->minlevel = y_min; move16();
	hist->maxlevel = y_max; move16();

	SPR_LEAVE(scp0);
	return 0;
}




/*-----------------------------------------------------------------*/

Word16 multirun_encode(const Word16 *input_frame, 
                       Word16 input_frame_size, 
                       mr_hist_t *hist,
                       Word16 *output,
                       Word16 output_size)
{
	const scopeid_t scp0 = SPR_ENTER(2+2+2+2+2+2+MR_MAXFRAMESIZE*2+2+2*2+sizeof(struct output_bit_stream));
	Word16 index;
	Word16 encodeoffset;
	Word16 toggle;
	Word16 increment;
	Word16 direction;
	Word16 streamsize;
	Word16 mod_inputstream[MR_MAXFRAMESIZE];
	const Word16 *inputstream;
	Word16 levelcount, t;
	struct output_bit_stream bitstream;
	
	/* Initialize the output bitstream */
	output_bit_stream_open(output, output_size, 0, &bitstream);


	/* Output zero-based number of levels encoded */
	/* This is now one of four special cases */
	/* put_bits_le8(hist->levels, MR_MAXLEVELBITS, &bitstream); */
	SWITCH (hist->levels) {
		case 3:
			put_bits_le8(0, MR_SCBITS, &bitstream);
			direction = 0; move16();
			t = sub(hist->count[3], add(hist->count[0], hist->count[1]));
			if (t >= 0) {
				direction = 1; move16();
			}
			BREAK;
		case 2:
			index = 2; move16();
			direction = 0; move16();

			IF (sub(hist->minlevel, -1) == 0) {
				index = 1; move16();
				t = sub(hist->count[2], hist->count[0]);
				if (t >= 0) {
					direction = 1; move16();
				}
			}
			put_bits_le8(index, MR_SCBITS, &bitstream);
			BREAK;
		case 1:
			put_bits_le8(3, MR_SCBITS, &bitstream);
			direction = 1; move16();
			BREAK;
		default:
			assert( FALSE );
	}

	/* Determine coding direction and encode it*/
	/* Direction (above or below maxcount level) to start encoding */
	put_bit(direction, &bitstream);

	/* Preparation for contiguous coding */
	encodeoffset = -1; move16();
	if (direction > 0) {
		encodeoffset = 1; move16();
	}
	toggle = 1; move16();
	increment = 0; move16();

	
	/* Input stream remaining to be encoded. */
	streamsize = input_frame_size; move16();
	inputstream = input_frame; PTR1_MOVE();

	/* Loop over all levels that will be encoded.  Level having max counts will not be 
	encoded, since it is defined as the remaining locations after all other levels
	encoded. */
	FOR (levelcount=1; levelcount<=hist->levels; ++levelcount) {
		const scopeid_t scp1 = SPR_ENTER(2+2+2+MR_MAXFRAMESIZE*2+2+2+2+2+2);
		Word16 encodelevel;
		Word16 encodecount;
		Word16 inputindex;
		UWord16 counts[MR_MAXFRAMESIZE];
		UWord16 cstartindex;
		UWord16 riceprm;
		UWord16 lenruncode;
		Word16 streambits;
		Word16 numcounts;

		move16();
		IF( hist->count[ add(hist->maxcountidx, encodeoffset)] == 0 )
		{
			/* The level has no occurrences */
			put_bits_le8(5, MR_EXPONENTBITS, &bitstream);
		}
		ELSE
		{
		    /* Level to be encoded */
		    encodelevel = add(hist->minlevel, add(hist->maxcountidx, encodeoffset));

		    /* Extract the level into a buffer for run-length coding.
		    Also determine run-length counts so that modified NTT rice 
		    parameter estimator can be used. */
		    encodecount = 0; move16();
		    inputindex = 0; move16();
		    cstartindex = 0; move16();
		    FOR (index=0; index<streamsize; ++index) {
			    move16();
			    IF (sub(inputstream[index], encodelevel) != 0) {
				    /* Level is not the level currently being encoded */ 
				    /* runstream[index] = 0; move16(); */

				    /* Save this level not to be encoded in the reduced input stream */
				    mod_inputstream[inputindex++] = inputstream[index]; move16();
			    } ELSE {

				    /* Level to be encoded found in inputstream */
				    /* runstream[index] = 1; move16(); */

				    /* Record count of run prior to encodelevel */
				    counts[encodecount++] = sub(index, cstartindex); move16(); /* CHECK_MOVE */
				    cstartindex = add(index, 1);
			    }
		    }
		
		    /* Last run if it exists */
		    numcounts = encodecount; move16();
		    index = sub(streamsize, cstartindex); 
		    IF (index > 0) {
			    counts[encodecount] = index; move16(); /* CHECK_MOVE */
			    numcounts = add(numcounts, 1);
		    }
		
		    /* Number of bits needed to encode length of current runstream.  This is also
		    the number of bits needed to identify a location within the runstream. */
		    streambits = sub(15, norm_s(sub(streamsize, 1)));

		    /* Determine number of bits needed to encode the runstream.  Use modified NTT 
		       function as defined above. */
		    riceprm = get_rice_parameter_unsigned(counts, numcounts, 1, MR_MAXEXPONENT, &lenruncode); 
	
		    /* Manage special cases in which coding should not use run-length, but other methods that
		    provide shorter codes.  This only occurs if MR_EXPONENTBITS >=3 because the special cases are
		    encoded as special exponents. */

		    IF ((MR_EXPONENTBITS-3) >= 0 && sub(streamsize, lenruncode) < 0) {
			    /* Runlength encoding is larger than the stream,
			    so just encode as a zero/one stream. */

			    /* Special exponent code that flags zero/one encoding */
			    put_bits_le8(0, MR_EXPONENTBITS, &bitstream);

			    /* Encode count now is bits remaining to 0/1 encode */
			    encodecount = streamsize; move16();
			    numcounts = sub(numcounts, 1);
			    FOR (index = 0; index < numcounts; index++) {
				    unary_encode(counts[index], &bitstream);
				    encodecount = sub(encodecount, add(counts[index], 1)); 
			    }

			    t = sub(counts[numcounts], encodecount);
			    IF (t < 0) {
				    unary_encode(counts[numcounts], &bitstream);
			    } ELSE {
				    FOR (index = 0; index < encodecount; index++) {
					    put_bit(0, &bitstream);
				    }
			    }
		    } ELSE {
			    /* Check if direct coding of locations of levels is more efficient.  This occurs if direct
			    coding of each location of the level to encode takes fewer bits than run-length coding.*/
			    test(); test(); test();
			    IF ((MR_EXPONENTBITS-3) >= 0 &&
				    (((MR_DIRECTBITS-2) == 0 && encodecount > 0 && sub(encodecount, 4) <= 0) || 
				     ((MR_DIRECTBITS-3) >= 0 && sub((1 << MR_DIRECTBITS), encodecount) > 0)) &&
				    (sub(add(MR_DIRECTBITS, extract_l(L_mult0(streambits, encodecount))), lenruncode) < 0)) {

				    const scopeid_t scp2 = SPR_ENTER(2+2);

				    Word16 exponent;
				    Word16 encodelocation;
				    /* Exponent code that indicates direct encoding */
				    exponent = sub((1 << MR_EXPONENTBITS), 1);

				    /* Use NTT bit handling function to share common code functionality. */
				    put_bits_le8(exponent, MR_EXPONENTBITS, &bitstream);

				    /* Output number of locations where encodelevel exists */
				    if (MR_DIRECTBITS == 2) {
					    put_bits_le8(sub(encodecount, 1), MR_DIRECTBITS, &bitstream);
				    } else {
					    put_bits_le8(encodecount, MR_DIRECTBITS, &bitstream);
				    }

				    encodelocation = 0; move16();
				    FOR (index=0; index<encodecount; ++index) {
					    encodelocation = add(encodelocation, counts[index]);
					    put_bits(encodelocation, streambits, &bitstream);
					    encodelocation = add(encodelocation, 1);
				    }
				    SPR_LEAVE(scp2);
			    }
			    /* Run-length code if special cases do not apply */
			    ELSE {
				    /* Encode Rice parameter */
				    put_bits_le8(riceprm, MR_EXPONENTBITS, &bitstream);

				    /* Output the run-length coded bitstream using an NTT function. */
				    rice_encode_block_unsigned(counts, riceprm, numcounts, &bitstream);
			    }
		    }

		    /* Update the size of the remaining data to encode */
		    streamsize = inputindex; move16();
		    inputstream = mod_inputstream; PTR1_MOVE();
		}

		/* Update the encode offset of next level to encode if contiguous coding */
		encodeoffset = next_offset(hist->levels, hist->maxcountidx, encodeoffset, &increment, &toggle);

		SPR_LEAVE(scp1);
	}

	/* Encoding completed */

	SPR_LEAVE(scp0);
	return length_in_bytes(&bitstream);
}



/*-----------------------------------------------------------------*/
Word16 multirun_decode(Word16 mu,
                       Word16 num_samples,
                       struct input_bit_stream *bitstream, 
                       Word16 *output_frame)
{
	const scopeid_t scp0 = SPR_ENTER(2+2+2+2+2+2+2+2+MR_MAXFRAMESIZE*2+2+2);

	Word16 index;
	Word16 levels;
	Word16 direction;
	Word16 noncodeindex;
	Word16 decodeoffset;
	Word16 toggle;
	Word16 increment;
	Word16 curlength;
	Word16 curindices[ MR_MAXFRAMESIZE ];
	Word16 level;
	const Word16 *from_linear;

	/* Read the number of levels encoded.  This is zero-based */
	/* Now read special case bits */
	/* levels = get_bits_le8(MR_MAXLEVELBITS, bitstream); */
	index = get_bits_le8(MR_SCBITS, bitstream);

	/* Decode the special cases.  Levels are zero-based. */
	SWITCH (index) {
		case 0:
			levels = 3; move16();
			noncodeindex = 2; move16();
			BREAK;
		case 1:
			levels = 2; move16();
			noncodeindex = 1; move16();
			BREAK;
		case 2:
			levels = 2; move16();
			noncodeindex = 2; move16();
			BREAK;
		case 3:
			levels = 1; move16();
			noncodeindex = 0; move16();
	}

	/* Starting direction of levels from max count level */
	direction = get_bit(bitstream);

	/* Parameters based on direction */
	decodeoffset = -1; move16();
	if (direction > 0) {
		decodeoffset = 1; move16();
	}
	toggle = 1; move16();
	increment = 0; move16();

	/* The max count level.  All other levels are known from this level
	and the above index, since levels are contiguous. This level is
	not coded. */
	/* noncodelevel = get_bits_le8(8, bitstream); */

	/* Special cases have noncodelevel = 128 */
	/* noncodelevel = 128; move16(); */


	/* Prepare to read the run-length coded levels */
	curlength = num_samples; move16();

	FOR (index=0; index<num_samples; ++index) {
		curindices[index] = index; move16();
	}

	/* Decode each level and place it in output vector */
	FOR (level=1; level<=levels; ++level) {
		const scopeid_t scp1 = SPR_ENTER(2+2+MR_MAXFRAMESIZE*2+2+2+2+2);

		Word16 decodelevel;
		Word16 exponent;
		Word16 codeindices[MR_MAXFRAMESIZE];
		Word16 numindices;
		Word16 codeindex;
		Word16 curindex;
		Word16 startindex;

		numindices = 0; move16();

		/* Now non-code level known to be special case of 128 */
		/* decodelevel = add(noncodelevel, decodeoffset); */
		decodelevel = decodeoffset; move16();


		/* Determine type of encoding of the level */
		exponent = get_bits_le8(MR_EXPONENTBITS, bitstream);

		/* Check if a level with no occurrences */
		IF( sub( exponent, 5 ) != 0 )
		{
		    /* Check the special cases first */

		    /* Check for straightforward bit encoding */
		    IF (exponent == 0 && (MR_EXPONENTBITS >= 3)) {
			    /* For now, read bit-by-bit, but this could be more efficiently
			    implemented by doing 16 bits at a time, and then the final bits */
			    curindex = 0; move16();
			    assert(curlength > 0);
			    FOR (index=curlength; index > 0; index -= 16 ) {
				    const scopeid_t scp2 = SPR_ENTER(2+2+2);
				    Word16 bitstoget;
				    Word16 codebits;
				    Word16 bitindex;

				    bitstoget = s_min(index, 16);
				    codebits = get_bits(bitstoget, bitstream);
				    codebits = lshl(codebits, sub(16, bitstoget));
				    FOR (bitindex = 0; bitindex < bitstoget; ++bitindex) {
					    const scopeid_t scp3 = SPR_ENTER(2);
					    Word16 codebit;
					    codebit = s_and(codebits, (Word16)0x8000);

					    if (codebit != 0) {
						    codeindices[numindices++] = curindex; move16();
					    }
					    codebits = lshl(codebits, 1);
					    curindex = add(curindex, 1);

					    SPR_LEAVE(scp3);
				    }
				    SPR_LEAVE(scp2);
			    }
		    }
		    /* Check for direct coding of codeindices */
		    ELSE IF (sub(exponent, (1 << MR_EXPONENTBITS)-1) == 0 && (MR_EXPONENTBITS >= 3)) {
			    const scopeid_t scp2 = SPR_ENTER(2+2);
			    Word16 idx;
			    Word16 streambits;

			    /* Number of indices to retrieve.  IF MR_DIRECTBITS == 2 it is zero-based */
			    numindices = get_bits_le8(MR_DIRECTBITS, bitstream);

			    /* Number of bits needed to encode length of current runstream.  This is also
			    the number of bits needed to identify a location within the runstream. */
			    streambits = sub(15, norm_s(sub(curlength,1)));

			    if (MR_DIRECTBITS == 2) {
				    numindices = add(numindices, 1);
			    }

			    FOR (idx=0; idx<numindices; ++idx) {
				    codeindices[idx] = get_bits(streambits, bitstream); move16(); /* CHECK_MOVE */
			    }
			    SPR_LEAVE(scp2);
		    }
		    /* Run-length coding */
		    ELSE {
			    /* Modified version of NTT run-length code */
			    index = 0; move16();
			    WHILE (sub(index, curlength) < 0) {
				    UWord16 count = rice_decode_unsigned(exponent, bitstream);
				    index = add(index, count);

				    IF (sub(index, curlength) < 0) {
					    codeindices[numindices++] = index; move16();
				    }

				    index = add(index, 1);
			    }
		    }

		    /* Place the decode level value at the locations specified by the code indices */

		    IF (0 < numindices) {
			    FOR (index=0; index<numindices; ++index) {
				    output_frame[curindices[codeindices[index]]] = decodelevel; move16();
			    }
		    }

		    /* Reduce the curindices array by those values that are in the codeindices */
		    curindex = 0; move16();
		    startindex = 0; move16();
		    IF (0 < numindices) {
			    FOR (codeindex=0; codeindex<numindices; ++codeindex) {
				    move16();
				    IF (sub(startindex, codeindices[codeindex]) < 0) {
					    FOR (index = startindex; index < codeindices[codeindex]; ++index) {
						    curindices[curindex++] = curindices[index]; move16();
					    }
				    }
				    startindex = add(codeindices[codeindex], 1); 
			    }
		    }

		    /* Fill in any remaining indices after the end of codeindices, and
		    define the current length of the reduced curindices */
		    IF (sub(startindex, curlength) < 0) {
			    FOR (index=startindex; index<curlength; ++index) {
				    curindices[curindex++] = curindices[index]; move16();
			    }
		    }
		    curlength = curindex; move16();
		}

		/* Update the encode offset of next level to encode if contiguous coding */
		decodeoffset = next_offset(levels, noncodeindex, decodeoffset, &increment, &toggle);

		SPR_LEAVE(scp1);
	}

	/* Put the noncode level in the remaining locations after decoding all other levels */
	FOR (index=0; index<curlength; ++index) {
		output_frame[curindices[index]] = 0; move16();
	}

	/* Convert the frame from "linear" representation to mu-law or a-law */
	from_linear = from_linear_alaw + 128; PTR1_MOVE();
	if (mu != 0) {
		from_linear = from_linear_ulaw + 128; PTR1_MOVE();
	}

	FOR (index=0; index<num_samples; ++index) {
		output_frame[index] = add(from_linear[output_frame[index]], 128); move16();
	}

	SPR_LEAVE(scp0);
	return num_samples;
}


/*------------------------------------------------------------------------------------
  next_offset

  This function advances the offset for contiguous coding.  In contiguouse coding
  the levels are encoded (and therefore decoded) by starting with the level above or 
  below the level that has maximum counts, depending on whether more samples have levels
  above or below the maximum count level.  The coding then moves alternately above and
  below the maximum count level encoding levels farther away from the max count level. This 
  is done to try to capture the levels with most counts first and run-length encode those, 
  then encode levels with sparse numbers of samples later, in an attempt to spread the
  run-length coding evenly.
  -----------------------------------------------------------------------------------*/
static Word16 next_offset(Word16 levels, Word16 noncodeindex, Word16 offset, Word16 *increment, Word16 *toggle)
{
	const scopeid_t scp0 = SPR_ENTER(2);
	Word16 cindex;

	/* Toggle sign every level unless only levels above or below 
	noncode level remain */
	IF (*toggle > 0) {
		offset = negate(offset);

		/* Increment offset every other time if toggling sign */
		IF (*increment > 0) {
			if (offset >= 0) {
				offset = add(offset, 2);
			}
			offset = sub(offset, 1);

			*increment = 0; move16();
		} ELSE {
			*increment = 1; move16();
		}

		cindex = add(noncodeindex, offset);

		/* Check if offset puts index out of bounds.  The rest of the
		indices are all on one side of the noncodeindex */
		test();
		IF (cindex < 0 || sub(cindex, levels ) > 0) {
			offset = negate(offset);
			IF (*increment > 0) {
				if (offset >= 0) {
					offset = add(offset, 2);
				}
				offset = sub(offset, 1);
			}
			*toggle = 0; move16();
			*increment = 1; move16();
		}
	} ELSE {
		/*If toggling disabled, only level indices on one side of
		non-coded index remain to be encoded, so just increment. */

		if (offset >= 0)
			offset = add(offset, 2);
		offset = sub(offset, 1);
	}

	SPR_LEAVE(scp0);

	return offset;
}
