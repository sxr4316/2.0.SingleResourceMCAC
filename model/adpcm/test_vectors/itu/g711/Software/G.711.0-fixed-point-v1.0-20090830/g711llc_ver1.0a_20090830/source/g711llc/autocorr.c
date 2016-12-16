/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: autocorr.c
 Contents: Functions for calculating the autocorrelation of signals.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#include <assert.h>
#include "g711llc_defines.h"
#include "g711llc_encoder.h"
#include "tables.h"
#include "autocorr.h"
#include "../utility/stack_profile.h"

/* Calculate the autocorrelation coefficients
 *
 * Parameters:
 *  x:         (input ) input signal
 *  x_size:    (input ) number of samples in the input signal
 *  lpc_order: (input ) prediction order
 *  r_h, r_l:  (output) pointer to the autocorrelation coefficients (DPF format)
 *
 * Returns:
 *  normalization factor
 */
static Word16 autocorr(Word16 *x, UWord16 x_size, Word16 lpc_order, Word16 *r_h, Word16 *r_l)
{
	const scopeid_t scp0 = SPR_ENTER(2+2+2+4);

	UWord16 i;
	Word16 j;
	Word16 norm;
	Word32 sum;

	/* Compute r[0] and test for overflow */
	DO {
		Overflow = 0; move16();
		assert(x_size > 1);
		sum = L_mac(1, x[0], x[0]); /* Avoid case of all zeros */
		FOR (i=1; i<x_size; ++i)
			sum = L_mac(sum, x[i], x[i]);

		/* If overflow divide x[] by 4 */
		IF (Overflow != 0) {
			FOR (i=0; i<x_size; ++i) {
				x[i] = shr(x[i], 2); move16();
			}
		}
	} WHILE (Overflow != 0);

	/* Normalization of r[0] */
	norm = norm_l(sum);
	sum = L_shl(sum, norm);
	L_Extract(sum, &r_h[0], &r_l[0]); /* Put in DPF format (see oper_32b) */
	
	/* r[1] to r[lpc_order] */
	FOR (i=1; i<=lpc_order; ++i) {
		const scopeid_t scp1 = SPR_ENTER(2);

		const Word16 upper_bound = sub(x_size, i);
		assert(1 < upper_bound);
		sum = L_mult(x[0], x[i]);
		FOR (j=1; j<upper_bound; ++j)
			sum = L_mac(sum, x[j], x[j+i]);
		sum = L_shl(sum, norm);
		L_Extract(sum, &r_h[i], &r_l[i]);

		SPR_LEAVE(scp1);
	}

	SPR_LEAVE(scp0);

	return norm;
}

/* Calculate the autocorrelation coefficients (normal version).
 *
 * Parameters:
 *  x:         (input ) input signal
 *  x_size:    (input ) number of samples in the input signal
 *  lpc_order: (input ) the prediction order
 *  r_h, r_l:  (output) pointer to the autocorrelation coefficients (DPF format)
 *  Tflag_pre: (DUMMY )
 *  encode:    (DUMMY )
 */
static void autocorr_normal(Word16 *x, UWord16 x_size, Word16 lpc_order, Word16 *r_h, Word16 *r_l, Word16 *Tflag_pre, struct g711llc_encoder *encoder, Word16 framelength_index)
{
	(void)Tflag_pre;
	(void)encoder;
	(void)framelength_index;

	(void)autocorr(x, x_size, lpc_order, r_h, r_l);
}

/* Calculate the autocorrelation coefficients (with bandwidth extension).
 *
 * Parameters:
 *  x:         (input ) input signal
 *  x_size:    (input ) number of samples in the input signal
 *  lpc_order: (input ) the prediction order
 *  r_h, r_l:  (output) pointer to the autocorrelation coefficients (DPF format)
 *  Tflag_pre: (output) LTP flag
 *  encoder:   (input ) pointer to the encoder persistent storage
 */
static void autocorr_largeframe(Word16 *x, UWord16 x_size, Word16 lpc_order, Word16 *r_h, Word16 *r_l, Word16 *Tflag_pre, struct g711llc_encoder *encoder, Word16 framelength_index)
{
	const scopeid_t scp0 = SPR_ENTER(2+2+2+2+4+5*2);
	
	UWord16 i;
	Word16 norm;
	Word16 lag_tab_0;

	/* Bandwidth expansion */
	Word16 ENER;
	Word32 sum_temp;
	Word16 pre_norm_thr, pre_Tflag_count, pre_frame_count, offset, ct; 

	assert(x_size == 160 || x_size == 240 || x_size == 320);

	norm = autocorr(x, x_size, lpc_order, r_h, r_l);

	ENER = s_max(sub(15, framelength_index), 12); /* 13 12 12 */
	pre_norm_thr = sub(17, framelength_index); /* 15 14 13 */
	offset = sub(ENER, 10); /* 3 2 2 */
	pre_frame_count = shl(1, offset); /* 8 4 4 */
	pre_Tflag_count = autocorr_pre_Tflag_count[framelength_index-2]; move16();

	ct = sub(encoder->frame_count, pre_frame_count);
	IF (ct > 0) { /* Most common case */
		IF (sub(sub(encoder->avg_thr, norm), 2) < 0) {
			*Tflag_pre = 0; move16();
			encoder->norm_avg = add(encoder->norm_avg, norm);
			encoder->noise_count = add(encoder->noise_count, 1);
			IF (sub(encoder->noise_count, 4) == 0) {
				encoder->avg_thr = shr(encoder->norm_avg, 2);
				encoder->norm_avg = 0; move16();
				encoder->noise_count = 0; move16();
			}
		}
	} ELSE IF (ct < 0) {
		encoder->avg_thr = add(encoder->avg_thr, norm);
		encoder->frame_count = add(encoder->frame_count, 1);
	} ELSE {
		encoder->avg_thr = shr(encoder->avg_thr, offset);
		encoder->frame_count = add(encoder->frame_count, 1);
	}

	if (sub(norm, pre_norm_thr) > 0) {
		*Tflag_pre = 0; move16();
	}

	if (s_and(encoder->Tflag_buff, pre_Tflag_count) > 0) {
		*Tflag_pre = 1; move16();
	}

	lag_tab_0 = add(autocorr_lag_tab[0], 30);
	lag_tab_0 = sub(lag_tab_0, add(norm, ENER));
	if (sub(norm, ENER) >= 0)
		lag_tab_0 = add(lag_tab_0, 16);
	sum_temp = L_shl(Mpy_32_16(r_h[0], r_l[0], lag_tab_0), 1);

	IF (Overflow == 0) {
		L_Extract(sum_temp, &r_h[0], &r_l[0]);
	}
	Overflow = 0; move16();

	/* r[1] to r[lpc_order] */
	FOR (i=1; i<=lpc_order; ++i) {
		L_Extract(L_shl(Mpy_32_16(r_h[i], r_l[i], autocorr_lag_tab[i]), 1), &r_h[i], &r_l[i]);
	}

	SPR_LEAVE(scp0);
}

const autocorr_funcptr autocorr_functions[] = {
	&autocorr_normal,
	&autocorr_normal,
	&autocorr_largeframe,
	&autocorr_largeframe,
	&autocorr_largeframe
};

