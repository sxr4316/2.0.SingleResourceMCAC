/* encode sign and mangnitude to pcm */
long
inv_g711(is, imag, aLaw)
long	is, imag;
int	aLaw;
{
	long	mag, i, s, q, inv;
	long	ymag = 0;
	short	iesp;

	if ( !aLaw ) {
		/* process u-law pcm */
/*
 *
 * In order to simplify the encoding process, the original linear magnitude
 * is biased by adding 33 which shifts the encoding range from (0 - 8158) to
 * (33 - 8191). The result can be seen in the following encoding table:
 *
 *	Biased Linear Input Code	Compressed Code
 *	------------------------	---------------
 *	00000001wxyza			    000wxyz
 *	0000001wxyzab			    001wxyz
 *	000001wxyzabc			    010wxyz
 *	00001wxyzabcd			    011wxyz
 *	0001wxyzabcde			    100wxyz
 *	001wxyzabcdef			    101wxyz
 *	01wxyzabcdefg			    110wxyz
 *	1wxyzabcdefgh			    111wxyz
 *
 * Each biased linear code has a leading 1 which identifies the segment
 * number. The value of the segment number is equal to 7 minus the number
 * of leading 0's. The quantization interval is directly available as the
 * four bits wxyz.  * The trailing bits (a - h) are ignored.
 *
 * Ordinarily the complement of the resulting code word is used for
 * transmission, and so the code word is complemented before it is returned.
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 *
 */
		mag = imag + 33;
		if (mag >= 0x2000) {
			ymag = 0x7f;
		} else {
			for (i = 12; i >= 5; i--) {
				if (mag & (1 << i))
					break;
			}
			s = i - 5;
			q = (mag & ( 0x0f << (i - 4))) >> (i - 4);
			ymag = (s << 4) + q;
		}
		inv = is ? ymag | 0x80 : ymag & 0x7f;
		/* invert bits */
		inv = inv ^ 0xff;

	} else {
		/* process a-law pcm */
/*
 *
 *	Linear Input Code	        Compressed Code
 *	------------------------	---------------
 *	0000000wxyza			    000wxyz
 *	0000001wxyza			    001wxyz
 *	000001wxyzab			    010wxyz
 *	00001wxyzabc			    011wxyz
 *	0001wxyzabcd			    100wxyz
 *	001wxyzabcde			    101wxyz
 *	01wxyzabcdef			    110wxyz
 *	1wxyzabcdefg			    111wxyz
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 *
 */

		ymag = imag;
	    iesp = 7;
	    for (i = 1; i <= 7; ++i)
	    {
			ymag += ymag;
			if (ymag >= 4096)
				break;
			iesp = 7 - i;
	    }
	
	    ymag &= 4095;
	
	    ymag = (ymag >> 8);
	    inv = (is == 0) ? ymag + (iesp << 4) : ymag + (iesp << 4) + 128;
	    //
	    // Sign bit inversion //
	    inv ^= 0x55;
	    inv ^= 0x80;
	}
	return(inv);
}

