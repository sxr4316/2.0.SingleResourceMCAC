/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: g711llc_decoder.c
 Contents: G.711-LLC decoder main control logic and decoder functions.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include "g711llc_defines.h"
#include "input_bit_stream.h"
#include "g711_itu.h"
#include "parcor.h"
#include "tables.h"
#include "mapping.h"
#include "G711Zdecode_function.h"
#include "fract_bits.h"
#include "multirun.h"
#include "g711llc_decoder.h"
#include "../utility/stack_profile.h"

static void get_signal_normal(const Word16 *d, UWord16 d_length, const Word16 *a, Word16 lpc_order, Word16 *x, const struct decoder_map_functions *engine, const Word16 *to_pcm)
{
	const scopeid_t scp0 = SPR_ENTER(320*2+2*2+4+2+2);

	Word16 tmp[320+MAX_PREDICTION_ORDER_SUPPORTED]; /* this could be removed with modulo addressing */
	Word16 i, j;
	Word32 s;
	Word16 s16;
	Word16 *yy = tmp; PTR1_MOVE();
	/* Orders used in this function: 1, 2, 3, 4, 5, 6, 8, 10, 12 */
	SWITCH (lpc_order) {
		case 1: {
			assert(d_length >= 1);
			
			/* i=0: */
			x[0] = engine->unmap_values(d[0]); move16(); FPTR_MOVE();
			
			/* i>=1: */
			/* Do the filtering. */
			FOR (i=1; i<d_length; ++i) {
				x[i] = engine->add_values(d[i], mult_r(a[1], to_pcm[x[i-1]])); move16(); FPTR_MOVE();
			}
			BREAK;
		}
		case 2: {
			assert(d_length >= 2);
		
			/* i=0: */
			x[0] = engine->unmap_values(d[0]); move16(); FPTR_MOVE();
			*yy++ = to_pcm[x[0]]; move16();
		
			/* i=1: */
			x[1] = engine->add_values(d[1], mult_r(a[1], yy[-1])); move16(); FPTR_MOVE();
			*yy++ = to_pcm[x[1]]; move16();
		
			/* i>=2: */
			/* Do the filtering. */
			FOR (i=2; i<d_length; ++i) {
				s = L_mult(a[2], yy[-1]);
				s16 = mac_r(L_shl(s,3), a[3], yy[-2]);
				/* s = L_shl(s, 3); */
			
				x[i] = engine->add_values(d[i], s16); move16(); FPTR_MOVE();
			
				*yy++ = to_pcm[x[i]]; move16();
			}
			BREAK;
		}
		case 3: {
			assert(d_length >= 3);
		
			/* i=0: */
			x[0] = engine->unmap_values(d[0]); move16(); FPTR_MOVE();
			*yy++ = to_pcm[x[0]]; move16();
		
			/* i=1: */
			x[1] = engine->add_values(d[1], mult_r(a[1], yy[-1])); move16(); FPTR_MOVE();
			*yy++ = to_pcm[x[1]]; move16();
		
			/* i=2: */
			{
				s = L_mult(a[2], yy[-1]);
				s16 = mac_r(L_shl(s,3), a[3], yy[-2]);
				/* s = L_shl(s, 3); */
			
				x[2] = engine->add_values(d[2], s16); move16(); FPTR_MOVE();
			
				*yy++ = to_pcm[x[2]]; move16();
			}
		
			/* i>=3: */
			/* Do the filtering. */
			FOR (i=3; i<d_length; ++i) {
				s = L_mult(a[4], yy[-1]);
				s = L_mac(s, a[5], yy[-2]);
				s16 = mac_r(L_shl(s,3), a[6], yy[-3]);
			
				x[i] = engine->add_values(d[i], s16); move16(); FPTR_MOVE();
			
				*yy++ = to_pcm[x[i]]; move16();
			}
			BREAK;
		}
		case 4: {
			assert(d_length >= 4);
		
			/* i=0: */
			x[0] = engine->unmap_values(d[0]); move16(); FPTR_MOVE();
			*yy++ = to_pcm[x[0]]; move16();
		
			/* i=1: */
			x[1] = engine->add_values(d[1], mult_r(a[1], yy[-1])); move16(); FPTR_MOVE();
			*yy++ = to_pcm[x[1]]; move16();
		
			/* i=2: */
			{
				s = L_mult(a[2], yy[-1]);
				s16 = mac_r(L_shl(s,3), a[3], yy[-2]);
				x[2] = engine->add_values(d[2], s16); move16(); FPTR_MOVE();
				*yy++ = to_pcm[x[2]]; move16();
			}
			/* i=3: */
			{
				s = L_mult(a[4], yy[-1]);
				s = L_mac(s, a[5], yy[-2]);
				s16 = mac_r(L_shl(s,3), a[6], yy[-3]);
				x[3] = engine->add_values(d[3], s16); move16(); FPTR_MOVE();
				*yy++ = to_pcm[x[3]]; move16();
			}
			/* i>=4: */
			/* Do the filtering. */
			FOR (i=4; i<d_length; ++i) {
				s = L_mult(a[7], yy[-1]);
				s = L_mac(s, a[8], yy[-2]);
				s = L_mac(s, a[9], yy[-3]);
				s16 = mac_r(L_shl(s,3), a[10], yy[-4]);
				x[i] = engine->add_values(d[i], s16); move16(); FPTR_MOVE();
				*yy++ = to_pcm[x[i]]; move16();
			}
			BREAK;
		}
		case 6: {
			const scopeid_t scp1 = SPR_ENTER(2);
			Word16 pp;
			/* i=0: */
			x[0] = engine->unmap_values(d[0]); move16(); FPTR_MOVE();
			*yy++ = to_pcm[x[0]]; move16();
		
			/* i=1: */
			x[1] = engine->add_values(d[1], mult_r(a[1], yy[-1])); move16(); FPTR_MOVE();
			*yy++ = to_pcm[x[1]]; move16();
		
			/* i=2: */
			{
				s = L_mult(a[2], yy[-1]);
				s16 = mac_r(L_shl(s,3), a[3], yy[-2]);
				x[2] = engine->add_values(d[2], s16); move16(); FPTR_MOVE();
				*yy++ = to_pcm[x[2]]; move16();
			}
			pp=3; move16();
			FOR (i=3; i<lpc_order; ++i) {
				s = L_mult(a[pp+1], yy[-1]);

				FOR(j=2; j<i; j++){
					s = L_mac(s, a[j+pp], yy[-j]);
				}
				s16 = mac_r(L_shl(s,3), a[i+pp], yy[-i]);
				x[i] = engine->add_values(d[i], s16); move16(); FPTR_MOVE();
				pp = add(pp,i);
				*yy++ = to_pcm[x[i]]; move16();
			}
			FOR (i=lpc_order; i<d_length; ++i) {
				s = L_mult(a[pp+1], yy[-1]);
				s = L_mac(s, a[pp+2], yy[-2]);
				s = L_mac(s, a[pp+3], yy[-3]);
				s = L_mac(s, a[pp+4], yy[-4]);
				s = L_mac(s, a[pp+5], yy[-5]);
				s16 = mac_r(L_shl(s,3), a[pp+lpc_order], yy[-lpc_order]);
				x[i] = engine->add_values(d[i], s16); move16(); FPTR_MOVE();
				*yy++ = to_pcm[x[i]]; move16();
			}
			SPR_LEAVE(scp1);
			BREAK;
		} 
		case 8: {
			const scopeid_t scp1 = SPR_ENTER(2);
			Word16 pp;
			/* i=0: */
			x[0] = engine->unmap_values(d[0]); move16(); FPTR_MOVE();
			*yy++ = to_pcm[x[0]]; move16();
		
			/* i=1: */
			x[1] = engine->add_values(d[1], mult_r(a[1], yy[-1])); move16(); FPTR_MOVE();
			*yy++ = to_pcm[x[1]]; move16();
		
			/* i=2: */
			{
				s = L_mult(a[2], yy[-1]);
				s16 = mac_r(L_shl(s,3), a[3], yy[-2]);
				x[2] = engine->add_values(d[2], s16); move16(); FPTR_MOVE();
				*yy++ = to_pcm[x[2]]; move16();
			}
			pp=3; move16();
			FOR (i=3; i<lpc_order; ++i) {
				s = L_mult(a[pp+1], yy[-1]);

				FOR(j=2; j<i; j++){
					s = L_mac(s, a[j+pp], yy[-j]);
				}
				s16 = mac_r(L_shl(s,3), a[i+pp], yy[-i]);
				x[i] = engine->add_values(d[i], s16); move16(); FPTR_MOVE();
				pp = add(pp,i);
				*yy++ = to_pcm[x[i]]; move16();
			}
			FOR (i=lpc_order; i<d_length; ++i) {
				s = L_mult(a[pp+1], yy[-1]);
				s = L_mac(s, a[pp+2], yy[-2]);
				s = L_mac(s, a[pp+3], yy[-3]);
				s = L_mac(s, a[pp+4], yy[-4]);
				s = L_mac(s, a[pp+5], yy[-5]);
				s = L_mac(s, a[pp+6], yy[-6]);
				s = L_mac(s, a[pp+7], yy[-7]);
				s16 = mac_r(L_shl(s,3), a[pp+lpc_order], yy[-lpc_order]);
				x[i] = engine->add_values(d[i], s16); move16(); FPTR_MOVE();
				*yy++ = to_pcm[x[i]]; move16();
			}
			SPR_LEAVE(scp1);
			BREAK;
		} 
		default: {
			const scopeid_t scp1 = SPR_ENTER(2);

			Word16 pp;
			/* i=0: */
			x[0] = engine->unmap_values(d[0]); move16(); FPTR_MOVE();
			*yy++ = to_pcm[x[0]]; move16();
		
			/* i=1: */
			x[1] = engine->add_values(d[1], mult_r(a[1], yy[-1])); move16(); FPTR_MOVE();
			*yy++ = to_pcm[x[1]]; move16();
		
			/* i=2: */
			{
				s = L_mult(a[2], yy[-1]);
				s16 = mac_r(L_shl(s,3), a[3], yy[-2]);
				x[2] = engine->add_values(d[2], s16); move16(); FPTR_MOVE();
				*yy++ = to_pcm[x[2]]; move16();
			}
			pp=3; move16();
			FOR (i=3; i<lpc_order; ++i) {
				s = L_mult(a[pp+1], yy[-1]);

				FOR(j=2; j<i; j++){
					s = L_mac(s, a[j+pp], yy[-j]);
				}
				s16 = mac_r(L_shl(s,3), a[i+pp], yy[-i]);
				x[i] = engine->add_values(d[i], s16); move16(); FPTR_MOVE();
				pp = add(pp,i);
				*yy++ = to_pcm[x[i]]; move16();
			}
			FOR (i=lpc_order; i<d_length; ++i) {
				s = L_mult(a[pp+1], yy[-1]);
				FOR(j=2; j<lpc_order; j++){
					s = L_mac(s, a[pp+j], yy[-j]);
				}
				s16 = mac_r(L_shl(s,3), a[pp+lpc_order], yy[-lpc_order]);
				x[i] = engine->add_values(d[i], s16); move16(); FPTR_MOVE();
				*yy++ = to_pcm[x[i]]; move16();
			}

			SPR_LEAVE(scp1);
		} /* end of default */
	} /* end of switch */

	SPR_LEAVE(scp0);
}

static void get_signal_largeframe(Word16 nsubframe, const Word16 *d, UWord16 d_length, const Word16 *a, Word16 lpc_order, Word16 *x, const Word16 *T, const struct decoder_map_functions *engine, const Word16 *to_pcm)
{
	const scopeid_t scp0 = SPR_ENTER((320+2*MAX_PREDICTION_ORDER_SUPPORTED)*2+5*2+3*2+3*2+4+2+2*2);
	Word16 tmp[320+2*MAX_PREDICTION_ORDER_SUPPORTED];
	Word16 i, j, k, end, nsub_len, gain[3], temp, pred, s16;
	Word32 s;
	const Word16 one_before_last = sub(nsubframe, 2);
	Word16 *yy, *res;
	yy = tmp + MAX_PREDICTION_ORDER_SUPPORTED; PTR1_MOVE();
	res = tmp; PTR1_MOVE();

	nsub_len = sub(sub(d_length, lpc_order), T[0]);
	SWITCH (nsubframe) {
		case 2: nsub_len = shr(nsub_len, 1); BREAK;
		case 3: nsub_len = mult(nsub_len, 10923); BREAK;
	}

	FOR (i=0; i<nsubframe; ++i) {
		gain[i] = G2; move16();
		move16();
		if (sub(T[i], PT) < 0) {
			gain[i] = G1; move16();
		}
	}

	/* lpc_orders used in this function: 1, 5, 8, 10, 12 */
	IF (sub(lpc_order, 1) == 0) {
		/* i=0: */
		x[0] = engine->unmap_values(d[0]); move16(); FPTR_MOVE();
		*yy++ = to_pcm[x[0]]; move16();
		/* i>=1: */
		end = add(lpc_order, T[0]);
		FOR (i=lpc_order; i<end; ++i) {
			temp = mult_r(a[1], yy[-1]); /* lpc predictive signal. */		
			x[i] = engine->add_values(d[i], temp); move16(); FPTR_MOVE();
			*yy++ = to_pcm[x[i]]; move16();
			res[i] = sub(yy[-1], temp); move16();		/* lpc residual signal. */
		}

		end = add(i, nsub_len);
		FOR (k=0; k<nsubframe; ++k) {
			const scopeid_t scp1 = SPR_ENTER(2);

			const Word16 *const r = res - T[k]; PTR2_MOVE();
			FOR (; i<end; ++i) {
				temp = mult_r(a[1], yy[-1]); /* lpc predictive signal. */		
				/* lpc predictive signal. */
				pred = add(temp, mult_r(gain[k], r[i]));		/* lpc + ltp. */
				x[i] = engine->add_values(d[i], pred); move16(); FPTR_MOVE();
				*yy++ = to_pcm[x[i]]; move16();
				res[i] = sub(yy[-1], temp); move16();		/* lpc residual signal. */
			}
			end = add(end, nsub_len);
			if (sub(k, one_before_last) == 0) { /* the last subframe can be longer */
				end = d_length; move16();
			}

			SPR_LEAVE(scp1);
		}
	} ELSE {
		const scopeid_t scp1 = SPR_ENTER(2);

		Word16 pp;

		assert(lpc_order >= 4);

		/* i=0: */
		x[0] = engine->unmap_values(d[0]); move16(); FPTR_MOVE();
		*yy++ = to_pcm[x[0]]; move16();
		
		/* i=1: */
		x[1] = engine->add_values(d[1], mult_r(a[1], yy[-1])); move16(); FPTR_MOVE();
		*yy++ = to_pcm[x[1]]; move16();
		
		/* i=2: */
		{
			s = L_mult(a[2], yy[-1]);
			s16 = mac_r(L_shl(s, 3), a[3], yy[-2]);
			x[2] = engine->add_values(d[2], s16); move16(); FPTR_MOVE();
			*yy++ = to_pcm[x[2]]; move16();
		}
		pp = 3; move16();
		FOR (i=3; i<lpc_order; ++i) {
			s = L_mult(a[pp+1], yy[-1]);

			FOR (j=2; j<i; ++j) {
				s = L_mac(s, a[j+pp], yy[-j]);
			}
			s16 = mac_r(L_shl(s, 3), a[i+pp], yy[-i]);
			x[i] = engine->add_values(d[i], s16); move16(); FPTR_MOVE();
			pp = add(pp, i);
			*yy++ = to_pcm[x[i]]; move16();
		}

		/* Do the filtering. */
		end = add(lpc_order, T[0]);
		
		SWITCH(lpc_order) {
			case 5: {
				FOR (i=lpc_order; i<end; ++i) {
					s = L_mult(a[1+pp], yy[-1]);

					s = L_mac(s, a[2+pp], yy[-2]);
					s = L_mac(s, a[3+pp], yy[-3]);
					s = L_mac(s, a[4+pp], yy[-4]);

					temp = mac_r(L_shl(s, 3), a[lpc_order+pp], yy[-lpc_order]);
					/* lpc predictive signal. */		
					x[i] = engine->add_values(d[i], temp); move16(); FPTR_MOVE();
					*yy++ = to_pcm[x[i]]; move16();
					res[i] = sub(yy[-1], temp); move16();		/* lpc residual signal. */
				}

				end = add(i, nsub_len);
				FOR (k=0; k<nsubframe; ++k) {
					const scopeid_t scp2 = SPR_ENTER(2);

					const Word16 *const r = res - T[k]; PTR2_MOVE();
					FOR (; i<end; ++i) {
						s = L_mult(a[1+pp], yy[-1]);
						s = L_mac(s, a[2+pp], yy[-2]);
						s = L_mac(s, a[3+pp], yy[-3]);
						s = L_mac(s, a[4+pp], yy[-4]);

						temp = mac_r(L_shl(s, 3), a[lpc_order+pp], yy[-lpc_order]);
						/* lpc predictive signal. */
						pred = add(temp, mult_r(gain[k], r[i]));		/* lpc + ltp. */
						x[i] = engine->add_values(d[i], pred); move16(); FPTR_MOVE();

						*yy++ = to_pcm[x[i]]; move16();
						res[i] = sub(yy[-1], temp); move16();		/* lpc residual signal. */
					}
					end = add(end, nsub_len);
					if (sub(k, one_before_last) == 0) { /* the last subframe can be longer */
						end = d_length; move16();
					}

					SPR_LEAVE(scp2);
				}
				BREAK;
			}
			case 8: {
				FOR (i=lpc_order; i<end; ++i) {
					s = L_mult(a[1+pp], yy[-1]);
					s = L_mac(s, a[2+pp], yy[-2]);
					s = L_mac(s, a[3+pp], yy[-3]);
					s = L_mac(s, a[4+pp], yy[-4]);
					s = L_mac(s, a[5+pp], yy[-5]);
					s = L_mac(s, a[6+pp], yy[-6]);
					s = L_mac(s, a[7+pp], yy[-7]);

					temp = mac_r(L_shl(s, 3), a[lpc_order+pp], yy[-lpc_order]);
					/* lpc predictive signal. */		
					x[i] = engine->add_values(d[i], temp); move16(); FPTR_MOVE();
					*yy++ = to_pcm[x[i]]; move16();
					res[i] = sub(yy[-1], temp); move16();		/* lpc residual signal. */
				}

				end = add(i, nsub_len);
				FOR (k=0; k<nsubframe; ++k) {
					const scopeid_t scp2 = SPR_ENTER(2);

					const Word16 *const r = res - T[k]; PTR2_MOVE();
					FOR (; i<end; ++i) {
						s = L_mult(a[1+pp], yy[-1]);
						s = L_mac(s, a[2+pp], yy[-2]);
						s = L_mac(s, a[3+pp], yy[-3]);
						s = L_mac(s, a[4+pp], yy[-4]);
						s = L_mac(s, a[5+pp], yy[-5]);
						s = L_mac(s, a[6+pp], yy[-6]);
						s = L_mac(s, a[7+pp], yy[-7]);

						temp = mac_r(L_shl(s, 3), a[lpc_order+pp], yy[-lpc_order]);
						/* lpc predictive signal. */
						pred = add(temp, mult_r(gain[k], r[i]));		/* lpc + ltp. */
						x[i] = engine->add_values(d[i], pred); move16(); FPTR_MOVE();

						*yy++ = to_pcm[x[i]]; move16();
						res[i] = sub(yy[-1], temp); move16();		/* lpc residual signal. */
					}
					end = add(end, nsub_len);
					if (sub(k, one_before_last) == 0) { /* the last subframe can be longer */
						end = d_length; move16();
					}

					SPR_LEAVE(scp2);
				}
				BREAK;
			}
			case 10: {
				FOR (i=lpc_order; i<end; ++i) {
					s = L_mult(a[1+pp], yy[-1]);
					s = L_mac(s, a[2+pp], yy[-2]);
					s = L_mac(s, a[3+pp], yy[-3]);
					s = L_mac(s, a[4+pp], yy[-4]);
					s = L_mac(s, a[5+pp], yy[-5]);
					s = L_mac(s, a[6+pp], yy[-6]);
					s = L_mac(s, a[7+pp], yy[-7]);
					s = L_mac(s, a[8+pp], yy[-8]);
					s = L_mac(s, a[9+pp], yy[-9]);

					temp = mac_r(L_shl(s, 3), a[lpc_order+pp], yy[-lpc_order]);
					/* lpc predictive signal. */		
					x[i] = engine->add_values(d[i], temp); move16(); FPTR_MOVE();
					*yy++ = to_pcm[x[i]]; move16();
					res[i] = sub(yy[-1], temp); move16();		/* lpc residual signal. */
				}

				end = add(i, nsub_len);
				FOR (k=0; k<nsubframe; ++k) {
					const scopeid_t scp2 = SPR_ENTER(2);

					const Word16 *const r = res - T[k]; PTR2_MOVE();
					FOR (; i<end; ++i) {
						s = L_mult(a[1+pp], yy[-1]);
						s = L_mac(s, a[2+pp], yy[-2]);
						s = L_mac(s, a[3+pp], yy[-3]);
						s = L_mac(s, a[4+pp], yy[-4]);
						s = L_mac(s, a[5+pp], yy[-5]);
						s = L_mac(s, a[6+pp], yy[-6]);
						s = L_mac(s, a[7+pp], yy[-7]);
						s = L_mac(s, a[8+pp], yy[-8]);
						s = L_mac(s, a[9+pp], yy[-9]);

						temp = mac_r(L_shl(s, 3), a[lpc_order+pp], yy[-lpc_order]);
						/* lpc predictive signal. */
						pred = add(temp, mult_r(gain[k], r[i]));		/* lpc + ltp. */
						x[i] = engine->add_values(d[i], pred); move16(); FPTR_MOVE();

						*yy++ = to_pcm[x[i]]; move16();
						res[i] = sub(yy[-1], temp); move16();		/* lpc residual signal. */
					}
					end = add(end, nsub_len);
					if (sub(k, one_before_last) == 0) { /* the last subframe can be longer */
						end = d_length; move16();
					}

					SPR_LEAVE(scp2);
				}
				BREAK;
			}

			case 12: {
				FOR (i=lpc_order; i<end; ++i) {
					s = L_mult(a[1+pp], yy[-1]);
					s = L_mac(s, a[2+pp], yy[-2]);
					s = L_mac(s, a[3+pp], yy[-3]);
					s = L_mac(s, a[4+pp], yy[-4]);
					s = L_mac(s, a[5+pp], yy[-5]);
					s = L_mac(s, a[6+pp], yy[-6]);
					s = L_mac(s, a[7+pp], yy[-7]);
					s = L_mac(s, a[8+pp], yy[-8]);
					s = L_mac(s, a[9+pp], yy[-9]);
					s = L_mac(s, a[10+pp], yy[-10]);
					s = L_mac(s, a[11+pp], yy[-11]);

					temp = mac_r(L_shl(s, 3), a[lpc_order+pp], yy[-lpc_order]);
					/* lpc predictive signal. */		
					x[i] = engine->add_values(d[i], temp); move16(); FPTR_MOVE();
					*yy++ = to_pcm[x[i]]; move16();
					res[i] = sub(yy[-1], temp); move16();		/* lpc residual signal. */
				}

				end = add(i, nsub_len);
				FOR (k=0; k<nsubframe; ++k) {
					const scopeid_t scp2 = SPR_ENTER(2);

					const Word16 *const r = res - T[k]; PTR2_MOVE();
					FOR (; i<end; ++i) {
						s = L_mult(a[1+pp], yy[-1]);

						s = L_mac(s, a[2+pp], yy[-2]);
						s = L_mac(s, a[3+pp], yy[-3]);
						s = L_mac(s, a[4+pp], yy[-4]);
						s = L_mac(s, a[5+pp], yy[-5]);
						s = L_mac(s, a[6+pp], yy[-6]);
						s = L_mac(s, a[7+pp], yy[-7]);
						s = L_mac(s, a[8+pp], yy[-8]);
						s = L_mac(s, a[9+pp], yy[-9]);
						s = L_mac(s, a[10+pp], yy[-10]);
						s = L_mac(s, a[11+pp], yy[-11]);

						temp = mac_r(L_shl(s, 3), a[lpc_order+pp], yy[-lpc_order]);
						/* lpc predictive signal. */
						pred = add(temp, mult_r(gain[k], r[i]));		/* lpc + ltp. */
						x[i] = engine->add_values(d[i], pred); move16(); FPTR_MOVE();

						*yy++ = to_pcm[x[i]]; move16();
						res[i] = sub(yy[-1], temp); move16();		/* lpc residual signal. */
					}
					end = add(end, nsub_len);
					if (sub(k, one_before_last) == 0) { /* the last subframe can be longer */
						end = d_length; move16();
					}

					SPR_LEAVE(scp2);
				}
				BREAK;
			}
			default: assert(!"NEVER HAPPEN"); exit(1);
		}

		SPR_LEAVE(scp1);
	}

	SPR_LEAVE(scp0);
}

Word16 g711llc_decode_frame(const Word16 *input, UWord16 *input_size, Word16 *output, UWord16 output_size, BOOL mu_law)
{
	const scopeid_t scp0 = SPR_ENTER(2*2+3*2+2*2+6*2+2*2+sizeof(struct input_bit_stream)+2+2*2);

	Word16 pzero, mzero;
	Word16 num_samples, large_frame, framelength_index;
	Word16 i, j;
	Word16 frame_type_flag1, frame_type_flag2, frame_type_flag3, frame_type_flag4, frame_type_flag5, frame_type_flag6;
	Word16 pulse_pos, pulse_flag;
	struct input_bit_stream in;
	enum { /* DO NOT REORDER OR INSERT */
		uncompressed, const_plus_zero, const_minus_zero, const_value, pm_zero_01, cisco_compressed,
		pm_zero_rice_minus, pm_zero_rice_plus, normal_n, normal_p, normal_m, normal_pm, normal_no_compress
	} frame_type;
	Word16 Tflag = 0, nsubframe; move16();

	if (mu_law != 0) {
		pzero = 0xff; move16();
	}
	if (mu_law != 0) {
		mzero = 0x7f; move16();
	}
	if (mu_law == 0) {
		pzero = 0xd5; move16();
	}
	if (mu_law == 0) {
		mzero = 0x55; move16();
	}

	input_bit_stream_open(input, *input_size, 0, &in);

	frame_type_flag1 = get_bits_le8(2, &in);
	SWITCH (frame_type_flag1) {
		case 1: num_samples = 40; move16(); BREAK;
		case 2: num_samples = 80; move16(); BREAK;
		case 3: num_samples = 160; move16(); BREAK;
		default: /* 0 */
			frame_type_flag2 = get_bits_le8(2, &in);
			SWITCH (frame_type_flag2) {
				case  2: num_samples = 240; move16(); BREAK;
				case  3: num_samples = 320; move16(); BREAK;
				default: /* 0 or 1 */
					frame_type_flag3 = get_bits_le8(4, &in);
					test();
					IF (frame_type_flag3 == 0 && frame_type_flag2 == 0) { 
						*input_size = 1; move16();
						SPR_LEAVE(scp0);
						return 0;
					} ELSE {
						const scopeid_t scp1 = SPR_ENTER(2);
						fract_frame_t fb_type;
						fb_type = s_or(s_or(shl(frame_type_flag1, 6), shl(frame_type_flag2, 4)), frame_type_flag3);

						/* Test for header signaling use of special case fractional-bit coding */
						test();
						IF (sub(fb_type, FB_MIN_TYPE_HEADER) >= 0 && sub(fb_type, FB_MAX_TYPE_HEADER) <= 0) { 
							/* Use fractional-bit decoding */
							num_samples = fract_bits_decode(mu_law, &in, output, output_size, fb_type);
							*input_size = position_in_bytes(&in); move16();
							SPR_LEAVE(scp1);
							SPR_LEAVE(scp0);
							return num_samples;
						} else {
							SPR_LEAVE(scp1);
							SPR_LEAVE(scp0);
							return -3; /* invalid first byte */
						}
					}
			}
	}
	framelength_index = shr(sub(num_samples, 16), 6);
	large_frame = sub(num_samples, 160);

	setFrameRate(g711llc_default_hz, num_samples);

	IF (large_frame <= 0) {
		frame_type_flag3 = get_bit(&in);
		IF (frame_type_flag3 == 0) {
			frame_type_flag4 = get_bits_le8(2, &in);
			SWITCH (frame_type_flag4) {
				case 0x00:
					frame_type = add(get_bits_le8(3, &in), uncompressed);
					IF (sub(frame_type, 6) == 0 ) {
						num_samples = multirun_decode(mu_law, num_samples, &in, output);
						*input_size = position_in_bytes(&in); move16();
						SPR_LEAVE(scp0);
						return num_samples;
					}
					/* Only for error checking: */
					if (frame_type > cisco_compressed) {
						SPR_LEAVE(scp0);
						return -4; /* invalid frame type 1 */
					}
					BREAK;

				case 0x01:
					Tflag = 1; move16();
					BREAK;
				case 0x02:
					frame_type = pm_zero_rice_plus; move16();
					BREAK;
				case 0x03:
					frame_type = pm_zero_rice_minus; move16();
					BREAK;
			}
		}
		test();
		IF ((Tflag != 0) && (framelength_index == 0)) {
			frame_type = normal_no_compress; move16();
		} ELSE {
			test();
			IF (frame_type_flag3 != 0 || Tflag != 0) {
				test();
				IF (mu_law != 0 && framelength_index > 0) {
					IF (get_bit(&in) == 0) {
						frame_type = normal_pm; move16();
					} ELSE {
						frame_type = add(get_bits_le8(2, &in), normal_n);
						/* Only for error checking: */
						if (frame_type > normal_m) {
							SPR_LEAVE(scp0);
							return -5; /* invalid frame type 1 */
						}
					}
				} ELSE {
					frame_type = normal_pm; move16();
				}
			}
		}/* IF ( (sub(Tflag,1) == 0) && (sub(num_samples,40) == 0))...ELSE...*/
	} ELSE {
		frame_type_flag3 = get_bit(&in);
		IF (frame_type_flag3 == 0) {
			frame_type_flag4 = get_bit(&in);
			IF (frame_type_flag4 == 0) {
				frame_type = add(get_bits_le8(2, &in), uncompressed);
			} ELSE {
				frame_type_flag5 = get_bit(&in);
				IF (frame_type_flag5 == 0) {
					frame_type_flag6 = get_bit(&in);
					IF (frame_type_flag6 != 0) {
						num_samples = multirun_decode(mu_law, num_samples, &in, output);
						*input_size = position_in_bytes(&in); move16();
						SPR_LEAVE(scp0);
						return num_samples;
					}
					frame_type = pm_zero_01; move16();
					/* Only for error checking */
					if (frame_type_flag6 != 0) {
						SPR_LEAVE(scp0);
						return -7; /* invalid frame type 6 (rice01?!) */
					}
				} ELSE {
					Tflag = 1; move16();
				}
			}
		}
		test();
		IF (frame_type_flag3 != 0 || Tflag != 0) {
			frame_type_flag4 = 1; move16();
			IF (Tflag == 0)
				frame_type_flag4 = get_bit(&in);
			IF (frame_type_flag4 == 0) {
				frame_type = sub(pm_zero_rice_plus, get_bit(&in));
			} ELSE {
				test();
				IF (mu_law != 0 && framelength_index > 0) {
					IF (get_bit(&in) == 0) {
						frame_type = normal_pm; move16();
					} ELSE {
						frame_type = add(get_bits_le8(2, &in), normal_n);
						/* Only for error checking: */
						if (frame_type > normal_m) {
							SPR_LEAVE(scp0);
							return -9; /* invalid frame type 1 */
						}
					}
				} ELSE {
					frame_type = normal_pm; move16();
				}
			}
		}
	}

	SWITCH (frame_type) {
		case uncompressed: {
			enum { start = 1 };
			FOR (i=0; i<num_samples; ++i) {
				output[i] = input[start + i]; move16();
			}
			*input_size = add(start, num_samples);
			SPR_LEAVE(scp0);
			return num_samples;
		}
		case pm_zero_01: {
			const scopeid_t scp1 = SPR_ENTER(2*2);
			/* 1 bit per sample encoding */
			Word16 zeros[2];
			zeros[0] = pzero; move16();
			zeros[1] = mzero; move16();
			FOR (i=0; i<num_samples; ++i) {
				output[i] = zeros[get_bit(&in)]; move16();
			}
			SPR_LEAVE(scp1);
			BREAK;
		}
		case pm_zero_rice_plus:
		case pm_zero_rice_minus: {
			const scopeid_t scp1 = SPR_ENTER(3*2);
			/* Golomb-Rice encoding */
			Word16 more_zero, less_zero, s, pulse_value;
			IF (sub(frame_type, pm_zero_rice_minus) == 0) {
				more_zero = mzero; move16();
				less_zero = pzero; move16();
			} ELSE {
				more_zero = pzero; move16();
				less_zero = mzero; move16();
			}

			{
				const scopeid_t scp2 = SPR_ENTER(2);
				Word16 t;
				s = get_bits_le8(2, &in);
				IF (s == 0) {
					pulse_flag = 1; move16();
					s = get_bits_le8(2, &in);
				} ELSE {
					pulse_flag = 0; move16();
				}
				SWITCH (num_samples) {
					case  40: {
						IF (sub(s, 3) == 0) {
							t = get_bit(&in);
							IF (t != 0) {
								s = add(4, get_bit(&in));
							}
						}
						BREAK;
					}				
					case  80: {
						IF (sub(s, 3) == 0)
							s = add(3, get_bits_le8(2, &in));
						BREAK;
					}				
					case 160:
					case 240:
					case 320: {
						IF (sub(s, 3) == 0) {
							t = get_bits_le8(2, &in);
							SWITCH (t) {
								case 0:
								case 1:
									s = add(3, t); BREAK;
								case 2:
									s = add(5, get_bit(&in)); BREAK;
								default:
									t = get_bit(&in);
									if (t == 0) { s = 7; move16(); }  
									IF (t != 0) {
										s = add(6, get_bits_le8(2, &in));
									}
									BREAK;
							}
						}
						BREAK;
					}
				}

				IF (pulse_flag != 0) {
					pulse_pos = get_bits((Word16 )pulse_pos_lengths[framelength_index], &in); WRD8_MOVE(); /* CHECK_MOVE for Word8 */

					pulse_value = sub(more_zero, 1);
					if (get_bit(&in) != 0) {
						pulse_value = sub(less_zero, 1);
					}
					pulse_value = add(pulse_value, rice_decode(0, &in));
				}
				i = 0; move16();
				WHILE (sub(i, num_samples) < 0) {
					const scopeid_t scp3 = SPR_ENTER(2);

					UWord16 count = rice_decode_unsigned(s, &in);
					IF (0 < count) {
						FOR (j=0; j<count; ++j) {
							assert(i < output_size);
							output[i] = more_zero; move16();
							i = add(i, 1);
						}
					}
					IF (sub(i, num_samples) < 0) {
						assert(i < output_size);
						output[i] = less_zero; move16();
						i = add(i, 1);
					}

					SPR_LEAVE(scp3);
				}
				if (pulse_flag != 0) {
					output[pulse_pos] = pulse_value; move16();
				}
				SPR_LEAVE(scp2);
			}
			SPR_LEAVE(scp1);
			BREAK;
		}
		case const_value: {
			const scopeid_t scp1 = SPR_ENTER(2);
			Word16 value = get_bits_le8(8, &in);
			FOR (i=0; i<num_samples; ++i) {
				output[i] = value; move16();
			}
			SPR_LEAVE(scp1);
			BREAK;
		}
		case const_plus_zero: {
			FOR (i=0; i<num_samples; ++i) {
				output[i] = pzero; move16();
			}
			BREAK;
		}
		case const_minus_zero: {
			FOR (i=0; i<num_samples; ++i) {
				output[i] = mzero; move16();
			}
			BREAK;
		}
		case normal_n:
		case normal_p:
		case normal_m:
		case normal_pm: {
			const scopeid_t scp1 = SPR_ENTER(2+2+4*2+MAX_PREDICTION_ORDER_SUPPORTED*2+2+4*2+2*2+2+2*2+2+2+(MAX_PREDICTION_ORDER_SUPPORTED*(MAX_PREDICTION_ORDER_SUPPORTED+1)+1)*2+3*2+2+2+2);
			/* Normal frame */
			Word16 lpc_order;
			const Word8 *box;

			/* e-huffman */
			UWord16 huff_index[4];
			Word16 ipar[MAX_PREDICTION_ORDER_SUPPORTED];
			Word16 ss, s0[4], subdivision, subblock_size;
			Word16 tbl_index;
			Word16 progressive_s0, progressive_s1;
			Word16 skip_samples;
			Word16 frame_ind;

			Word16 icof[MAX_PREDICTION_ORDER_SUPPORTED*(MAX_PREDICTION_ORDER_SUPPORTED+1)+1];
			Word16 T[3];
			const struct decoder_parcor_functions *parcor_functions;
			const struct decoder_map_functions *map_functions;
			const Word16 *to_pcm;

			lpc_order = max_prediction_orders[framelength_index]; move16();

			/* DEC_ORDER */
			box = get_box(lpc_order);
			lpc_order = box[get_bits_le8(2, &in)]; move16();

			frame_ind=shr(sub(num_samples, 1),6);

			parcor_functions = &decoder_parcor_functions_smallframe;
			if (large_frame >= 0)
				parcor_functions = &decoder_parcor_functions_largeframe;

			/* Read the prediction coefficients */
			{
				const scopeid_t scp2 = SPR_ENTER(MAX_PREDICTION_ORDER_SUPPORTED*2);

				Word16 asi[MAX_PREDICTION_ORDER_SUPPORTED];

				parcor_functions->decode_parcor(&in, lpc_order, asi, frame_ind); FPTR_MOVE();

				/* Reconstruct PARCOR coefficients */
				parcor_functions->reconstruct_parcor(asi, lpc_order, ipar, frame_ind); FPTR_MOVE();

				SPR_LEAVE(scp2);
			}

			/* Decode the residual */
			/* subblock */
			subdivision = s_max(1, framelength_index);
			subblock_size = s_min(num_samples, 80);

			nsubframe = 1; move16(); /* Debug Harada */
			IF (large_frame >= 0) /* >= 160 */
			{
				IF (Tflag != 0)
				{
					nsubframe = sub(framelength_index, 1);
					T[0] = add(get_bits_le8(6, &in), 20);

					IF (sub(1, nsubframe) < 0) {
						FOR (i = 1; i < nsubframe; i++) {
							T[i] = sub(T[i-1], rice_decode(0, &in)); move16();
						}
					}
				}

				IF (large_frame > 0) { /* >= 240 */
					IF (get_bit(&in) == 0) {
						subdivision = 1; move16();
						subblock_size = num_samples; move16();
					}
				}
			}
			ss = rice_decode_unsigned(1, &in); 
			if(Tflag) ss = add(8, ss);
			ss = (Word16)Rice_map_inv[ss]; move16();
			s0[0] = ss; move16();

			IF (framelength_index > 0) { /* for n>=80 */
				IF (sub(1, subdivision) < 0) {
					FOR (i=1; i<subdivision; ++i) {
						s0[i] = add(s0[i-1], rice_decode(0, &in)); move16();
					}
				}

				test(); move16();
				IF (large_frame == 0 && sub(s0[1], s0[0]) == 0) {
					subblock_size = num_samples; move16();
					subdivision = 1; move16();
				}

				FOR (i=0; i<subdivision; ++i) {
					huff_index[i] = (Word16 )dIndex[peek_bits_3(&in)]; move16();
					skip_bits(hIndex_len[huff_index[i]], &in); /* skip read bits */
				}
			}

			tbl_index = 0; move16();
			assert(ss <= 7);
			IF (Tflag == 0) {
				move16();
				if (sub(ipar[0], 26500) > 0)
					tbl_index = add(tbl_index, 1);
				if (sub(ipar[0], 30800) > 0)
					tbl_index = add(tbl_index, 2);		/* 3 */
				test(); move16();
				IF ((sub(lpc_order,1) > 0) && (add(ipar[1], 28000) < 0)) {
					tbl_index = 4; move16();
				}
			} ELSE {
				move16();
				if (sub(ipar[0], 26500) > 0)
					tbl_index = add(tbl_index, 2); /* 2 */
				if (sub(ipar[0], 30800) > 0)
					tbl_index = add(tbl_index, 2);	/* 4 */
			}

			progressive_s0 = (Word16 )map_ss0[tbl_index][ss]; move16();
			progressive_s1 = (Word16 )map_ss1[tbl_index][ss]; move16();

			output[0] = rice_decode(progressive_s0, &in); move16();
			IF (sub(lpc_order, 1) > 0) {
				output[1] = rice_decode(progressive_s1, &in); move16();
			}

			skip_samples = s_min(lpc_order, 2);

			IF (framelength_index > 0) {
				huffman_decode(output+skip_samples, s0[0], sub(subblock_size, skip_samples), huff_index[0], &in); /* First EC block is 'skip_samples' samples shorter */
				IF (sub(1, subdivision) < 0) {
					FOR (i=1; i<subdivision; ++i) {
						huffman_decode(output+subblock_size*i, s0[i], subblock_size, huff_index[i], &in); /* First EC block is 'skip_samples' samples shorter */
					}
				}
			} ELSE
				rice_decode_block(output+skip_samples, s0[0], sub(subblock_size, skip_samples), &in); 

			par2cof(ipar, lpc_order, icof);

			/* Reconstruct the signal */
			IF (mu_law != 0) {
				map_functions = &dec_map_functions_ulaw[frame_type - normal_n]; PTR2_MOVE();
				to_pcm = tbl_ulaw_to_pcm; PTR1_MOVE();
			} ELSE {
				assert(frame_type == normal_pm);
				map_functions = &dec_map_functions_alaw; PTR2_MOVE();
				to_pcm = tbl_alaw_to_pcm; PTR1_MOVE();
			}
			test();
			IF ((large_frame >= 0) && (Tflag != 0)) {
				get_signal_largeframe(nsubframe, output, num_samples, icof, lpc_order, output, T, map_functions, to_pcm);
			} ELSE
				get_signal_normal(output, num_samples, icof, lpc_order, output, map_functions, to_pcm);

			assert(output_size >= num_samples);
			SPR_LEAVE(scp1);
			BREAK;
		}
		case normal_no_compress: {
			const scopeid_t scp1 = SPR_ENTER(NO_COMPRESS_PREDICT_ORDER*2);

			Word16 sign_buf[NO_COMPRESS_PREDICT_ORDER];
			const struct decoder_map_functions *map_functions;

			map_functions = &dec_map_functions_alaw; PTR1_MOVE();
			if (mu_law != 0) {
				map_functions = &dec_map_functions_ulaw[3]; PTR1_MOVE();
			}
			FOR (i=0; i<NO_COMPRESS_PREDICT_ORDER; ++i) {
				const scopeid_t scp2 = SPR_ENTER(2*2);

				Word16 o, s;
				o = get_bits_le8(7, &in);
				s = get_bit(&in);
				if (o == 0) {
					o = lshl(s, 7);
				}
				sign_buf[i] = s; move16();
				output[i] = o; move16();

				SPR_LEAVE(scp2);
			}
			rice_decode_block(output+NO_COMPRESS_PREDICT_ORDER, 5, sub(num_samples, NO_COMPRESS_PREDICT_ORDER), &in);
			FOR (i=NO_COMPRESS_PREDICT_ORDER; i<num_samples; ++i) {
				output[i] = sub(shr(add(add(add(output[i-1], output[i-2]), output[i-3]), output[i-4]), NO_COMPRESS_PREDICT_SHIFT), output[i]); move16();
			}
			FOR (i=0; i<NO_COMPRESS_PREDICT_ORDER; ++i) {
				const scopeid_t scp2 = SPR_ENTER(2);

				Word16 o = output[i]; move16();
				move16();
				if (sign_buf[i] > 0) {
					o = negate(o);
				}
				output[i] = map_functions->unmap_values(o); move16(); FPTR_MOVE();

				SPR_LEAVE(scp2);
			}
			FOR (i=NO_COMPRESS_PREDICT_ORDER; i<num_samples; ++i) {
				const scopeid_t scp2 = SPR_ENTER(2);

				Word16 o = output[i]; move16();
				if (get_bit(&in) > 0) {
					o = negate(o);
				}
				output[i] = map_functions->unmap_values(o); move16(); FPTR_MOVE();

				SPR_LEAVE(scp2);
			}
			assert(output_size >= num_samples);
			SPR_LEAVE(scp1);
			BREAK;
		}
		case cisco_compressed: {
			const scopeid_t scp1 = SPR_ENTER(2);
			Word16 processed_bytes = *input_size; move16();
			assert(num_samples == 40);

			if (G711Zdecompress(mu_law, input+1, &processed_bytes, output, num_samples) < 0) {
				SPR_LEAVE(scp1);
				SPR_LEAVE(scp0);
				return -50; /* Cisco codec returned error */;
			}

			*input_size = add(processed_bytes, 1);
			SPR_LEAVE(scp1);
			SPR_LEAVE(scp0);
			return num_samples;
		}
		default: assert(FALSE); SPR_LEAVE(scp0); return -1; /* never happens */
	}
	*input_size = position_in_bytes(&in); move16();

	SPR_LEAVE(scp0);

	return num_samples;
}
