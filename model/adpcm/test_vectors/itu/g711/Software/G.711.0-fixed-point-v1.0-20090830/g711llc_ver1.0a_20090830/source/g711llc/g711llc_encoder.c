/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: g711llc_encoder.c
 Contents: G.711-LLC encoder main control logic and encoder functions.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include "g711llc_defines.h"
#include "output_bit_stream.h"
#include "parcor.h"
#include "tables.h"
#include "window.h"
#include "autocorr.h"
#include "mapping.h"
#include "G711Zencode_function.h"
#include "fract_bits.h"
#include "multirun.h"
#include "g711llc_encoder.h"
#include "../utility/stack_profile.h"

void g711llc_encoder_init(struct g711llc_encoder *encoder)
{
	if (encoder == NULL)
		return;

	/* Variables for LTP pre-processing*/
	encoder->norm_avg    = 0; move16();
	encoder->noise_count = 0; move16();
	encoder->frame_count = 0; move16();
	encoder->avg_thr     = 0; move16();
	encoder->Tflag_buff  = 0; move16();
}

static void check_constant(const Word16 *x, UWord16 length, Word16 pzero, Word16 mzero, BOOL *constant, Word16 *pzero_count, Word16 *mzero_count, Word16 *pulse_pos)
{
	const scopeid_t scp0 = SPR_ENTER(2*2+2+2);
	Word16 i_pzero, i_mzero;
	Word16 c;
	UWord16 i;
	assert(x != NULL);
	assert(length > 0);

	*constant = TRUE; move16();
	*pzero_count = 0; move16();
	*mzero_count = 0; move16();

	c = x[0]; move16();
	FOR (i=0; i<length; ++i) {
		const scopeid_t scp1 = SPR_ENTER(2);
		const Word16 xi = x[i]; move16();

		if (sub(xi, c) != 0) {
			*constant = FALSE; move16();
		}
		i_pzero = sub(xi, pzero);
		i_mzero = sub(xi, mzero);
		if (i_pzero == 0)
			*pzero_count = add(*pzero_count, 1);
		if (i_mzero == 0)
			*mzero_count = add(*mzero_count, 1);
		if (L_mult0(i_pzero, i_mzero) != 0) {
			*pulse_pos = i; move16();
		}
		SPR_LEAVE(scp1);
	}
	SPR_LEAVE(scp0);
}

#define M MAX_PREDICTION_ORDER_SUPPORTED

/*
 * Parameters:
 *   m:  (input ) prediction order
 *   Rh: (input ) Rh[M+1] Vector of autocorrelations (msb)
 *   Rl: (input ) Rl[M+1] Vector of autocorrelations (lsb)
 *   rc: (output) rc[M]   Reflection coefficients. (Q15)
 *
 * Returns:
 *   Largest stable prediction order
 */

static Word16 levinson(Word16 m, const Word16 *Rh, const Word16 *Rl, Word16 *rc) {
	const scopeid_t scp0 = SPR_ENTER(2*2+2*2+2+3*2+(M+1)*2+2+(M+1)*2+2+3*4+(M+1)*4+2*2);

	Word16 i, j;
	Word16 hi, lo;
	Word16 Kh;                    /* reflection coefficient; hi and lo           */
	Word16 alp_h, alp_l, alp_exp; /* Prediction gain; hi lo and exponent         */
	Word16 _Ah[M+1], *Ah;         /* LPC coef. in double prec.                   */
	Word16 _Anh[M+1], *Anh;       /* LPC coef.for next iteration in double prec. */
	Word32 t0, t1, t2;            /* temporary variable                          */
	Word32 RR[M+1];
	Word16 stable_order, *B;

	assert(Rh != 0);
	assert(Rl != 0);
	assert(rc != 0);
	assert(0 <= m && m <= M);

	Ah = _Ah; PTR1_MOVE();

	/* K = A[1] = -R[1] / R[0] */
	RR[1] = L_Comp(Rh[1], Rl[1]); move32(); /* R[1] in Q31      */
	t2 = L_abs(RR[1]);                 /* abs R[1]         */
	t0 = Div_32(t2, Rh[0], Rl[0]);     /* R[1]/R[0] in Q31 */
	
	if (RR[1] < 0) t0 = L_negate(t0);  /* -R[1]/R[0]       */
	Kh = extract_h(t0);
	
	rc[0] = Kh; move16();
	t0 = L_shr(t0, 3);                 /* A[1] in Q28      */
	Ah[1] = extract_h(t0);

	/* Alpha = R[0] * (1-K**2) */
	t0 = L_mult(Kh, Kh);
	t0 = L_sub(0x7fffffffl, t0);       /* 1 - K*K  in Q31  */
	L_Extract(t0, &hi, &lo);           /* DPF format       */
	t0 = Mpy_32(Rh[0], Rl[0], hi, lo); /* Alpha in Q31     */

	/* Normalize Alpha */
	alp_exp = norm_l(t0);
	t0 = L_shl(t0, alp_exp);
	L_Extract(t0, &alp_h, &alp_l);     /* DPF format       */

	/*--------------------------------------*
	 * ITERATIONS  I=2 to M                 *
	 *--------------------------------------*/
	stable_order = 1; move16();
	Anh = _Anh; PTR1_MOVE();
	FOR (i=2; i<=m; ++i) {
		/* t0 = SUM ( R[j]*A[i-j] ,j=1,i-1 ) +  R[i] */

		t0 = 0; move32();
		FOR (j=1; j<i; j++){
			t0 = L_add(t0, L_mls(RR[j], Ah[i-j]));
		}
		t0 = L_shl(t0, 3);             /* result in Q28 -> convert to Q31 */
		                               /* No overflow possible            */
		RR[i] = L_Comp(Rh[i], Rl[i]); move16();
		t0 = L_sub(t0, RR[i]);         /* add R[i] in Q31                 */

		/* K = -t0 / Alpha */

		t1 = L_abs(t0);
		t2 = Div_32(t1, alp_h, alp_l); /* abs(t0)/Alpha                   */
		if (t0 > 0) t2 = L_negate(t2); /* K =-t0/Alpha                    */
		t2 = L_shl(t2, alp_exp);       /* denormalize; compare to Alpha   */
		Kh = extract_h(t2);            /* K in DPF                        */
		if (sub(abs_s(Kh), 32750) > 0) { /* Test for unstable filter. If unstable keep old A(z) */
			Kh = 0; move16();
		}
		rc[i-1] = Kh; move16();
		if (Kh != 0) {
			stable_order = i; move16();
		}

		/*------------------------------------------*
		 *  Compute new LPC coeff. -> An[i]         *
		 *  An[j]= A[j] + K*A[i-j]     , j=1 to i-1 *
		 *  An[i]= K                                *
		 *------------------------------------------*/
		FOR (j=1; j<i; ++j) {
			Anh[j] = sub( Ah[j], mult_r(Ah[i-j], Kh)); move16();
		}

		Anh[i]= shr(Kh,3); move16();
		/* Alpha = Alpha * (1-K**2) */
		t0 = L_mult(Kh, Kh);                  /* K*K      in Q15 */
		t0 = L_sub(0x7fffffffl, t0);          /* 1 - K*K  in Q31 */
		L_Extract(t0, &hi, &lo);              /* DPF format      */
		t0 = Mpy_32(alp_h, alp_l, hi, lo);    /* Alpha in Q31    */

		/* Normalize Alpha */
		j = norm_l(t0);
		t0 = L_shl(t0, j);
		L_Extract(t0, &alp_h, &alp_l);        /* DPF format      */
		alp_exp = add(alp_exp, j);            /* Add normalization to alp_exp */

		/* Swap Ah and Anh pointers */
		B = Ah; PTR1_MOVE();
		Ah = Anh; PTR1_MOVE();
		Anh = B; PTR1_MOVE();
	}

	SPR_LEAVE(scp0);

	return stable_order;
}

/* Get the prediction residual.
 * Parameters:
 *   x:            (input ) signal in intermediate format
 *   x_length:     (input ) number of samples in x
 *   x_org:        (input ) signal in a-law or mu-law format
 *   a:            (input ) prediction coefficients Q12
 *   lpc_order:    (input ) prediction order
 *   d:            (output) prediction residual (length: x_length)
 */

static void normal_prediction(const Word16 *x, UWord16 x_length, 
				const Word16 *x_org, const Word16 *a, Word16 lpc_order, 
				Word16 *d, const struct encoder_map_functions *engine)
{
	const scopeid_t scp0 = SPR_ENTER(2*2+4);
	Word16 i, s16;
	Word32 s;
	SWITCH (lpc_order) {
		case 1:
			/* i=0: */
			d[0] = engine->map_values(x_org[0]); move16(); FPTR_MOVE();
			
			FOR (i=1; i<x_length; ++i) {
				d[i] = engine->sub_values(x_org[i], mult_r(a[1], x[i-1])); move16(); FPTR_MOVE();
			}
			BREAK;
		case 2:
			/* i=0: */
			d[0] = engine->map_values(x_org[0]); move16(); FPTR_MOVE();
			/* i=1: */
			d[1] = engine->sub_values(x_org[1], mult_r(a[1], x[0])); move16(); FPTR_MOVE();
			
			FOR (i=2; i<x_length; ++i) {
				s = L_mult(a[2], x[i-1]);
				s16 = mac_r(L_shl(s,3), a[3], x[i-2]);
				d[i] = engine->sub_values(x_org[i], s16); move16(); FPTR_MOVE();
			}
			BREAK;
		case 3:
			/* i=0: */
			d[0] = engine->map_values(x_org[0]); move16(); FPTR_MOVE();
			/* i=1: */
			d[1] = engine->sub_values(x_org[1], mult_r(a[1], x[0])); move16(); FPTR_MOVE();
			/* i=2: */
			{
				s = L_mult(a[2], x[1]);
				s16 = mac_r(L_shl(s,3), a[3], x[0]);
				d[2] = engine->sub_values(x_org[2], s16); move16(); FPTR_MOVE();
			}
			/* i>=3: */
			FOR (i=3; i<x_length; ++i) {
				s = L_mult(a[4], x[i-1]);
				s = L_mac(s, a[5], x[i-2]);
				s16 = mac_r(L_shl(s,3), a[6], x[i-3]);
				d[i] = engine->sub_values(x_org[i], s16); move16(); FPTR_MOVE();
			}
			BREAK;
		case 4:
			/* i=0: */
			d[0] = engine->map_values(x_org[0]); move16(); FPTR_MOVE();
			/* i=1: */
			d[1] = engine->sub_values(x_org[1], mult_r(a[1], x[0])); move16(); FPTR_MOVE();
			/* i=2: */
			{
				s = L_mult(a[2], x[1]);
				s16 = mac_r(L_shl(s,3), a[3], x[0]);
				d[2] = engine->sub_values(x_org[2], s16); move16(); FPTR_MOVE();
			}
			/* i=3: */
			{
				s = L_mult(a[4], x[2]);
				s = L_mac(s, a[5], x[1]);
				s16 = mac_r(L_shl(s,3), a[6], x[0]);
				d[3] = engine->sub_values(x_org[3], s16); move16(); FPTR_MOVE();
			}
			/* i>=4: */
			FOR (i=4; i<x_length; ++i) {
				s = L_mult(a[7], x[i-1]);
				s = L_mac(s, a[8], x[i-2]);
				s = L_mac(s, a[9], x[i-3]);
				s16 = mac_r(L_shl(s,3), a[10], x[i-4]);
				d[i] = engine->sub_values(x_org[i], s16); move16(); FPTR_MOVE();
			}
			BREAK;
		case 6:
			/* i=0: */
			d[0] = engine->map_values(x_org[0]); move16(); FPTR_MOVE();
			/* i=1: */
			d[1] = engine->sub_values(x_org[1], mult_r(a[1], x[0])); move16(); FPTR_MOVE();
			/* i=2: */
			{
				s = L_mult(a[2], x[1]);
				s16 = mac_r(L_shl(s,3), a[3], x[0]);
				d[2] = engine->sub_values(x_org[2], s16); move16(); FPTR_MOVE();
			}
			/* i=3: */
			{
				s = L_mult(a[4], x[2]);
				s = L_mac(s, a[5], x[1]);
				s16 = mac_r(L_shl(s,3), a[6], x[0]);
				d[3] = engine->sub_values(x_org[3], s16); move16(); FPTR_MOVE();
			}
			/* i=4: */
			{
				s = L_mult(a[7], x[3]);
				s = L_mac(s, a[8], x[2]);
				s = L_mac(s, a[9], x[1]);
				s16 = mac_r(L_shl(s,3), a[10], x[0]);
				d[4] = engine->sub_values(x_org[4], s16); move16(); FPTR_MOVE();
			}
			/* i=5: */
			{
				s = L_mult(a[11], x[4]);
				s = L_mac(s, a[12], x[3]);
				s = L_mac(s, a[13], x[2]);
				s = L_mac(s, a[14], x[1]);
				s16 = mac_r(L_shl(s,3), a[15], x[0]);
				d[5] = engine->sub_values(x_org[5], s16); move16(); FPTR_MOVE();
			}
			/* i>=6: */
			FOR (i=6; i<x_length; ++i) {
				s = L_mult(a[16], x[i-1]);
				s = L_mac(s, a[17], x[i-2]);
				s = L_mac(s, a[18], x[i-3]);
				s = L_mac(s, a[19], x[i-4]);
				s = L_mac(s, a[20], x[i-5]);
				s16 = mac_r(L_shl(s,3), a[21], x[i-6]);
				d[i] = engine->sub_values(x_org[i], s16); move16(); FPTR_MOVE();
			}
			BREAK;	
		case 8:
			/* i=0: */
			d[0] = engine->map_values(x_org[0]); move16(); FPTR_MOVE();
			/* i=1: */
			d[1] = engine->sub_values(x_org[1], mult_r(a[1], x[0])); move16(); FPTR_MOVE();
			/* i=2: */
			{
				s = L_mult(a[2], x[1]);
				s16 = mac_r(L_shl(s,3), a[3], x[0]);
				d[2] = engine->sub_values(x_org[2], s16); move16(); FPTR_MOVE();
			}
			/* i=3: */
			{
				s = L_mult(a[4], x[2]);
				s = L_mac(s, a[5], x[1]);
				s16 = mac_r(L_shl(s,3), a[6], x[0]);
				d[3] = engine->sub_values(x_org[3], s16); move16(); FPTR_MOVE();
			}
			/* i=4: */
			{
				s = L_mult(a[7], x[3]);
				s = L_mac(s, a[8], x[2]);
				s = L_mac(s, a[9], x[1]);
				s16 = mac_r(L_shl(s,3), a[10], x[0]);
				d[4] = engine->sub_values(x_org[4], s16); move16(); FPTR_MOVE();
			}
			/* i=5: */
			{
				s = L_mult(a[11], x[4]);
				s = L_mac(s, a[12], x[3]);
				s = L_mac(s, a[13], x[2]);
				s = L_mac(s, a[14], x[1]);
				s16 = mac_r(L_shl(s,3), a[15], x[0]);
				d[5] = engine->sub_values(x_org[5], s16); move16(); FPTR_MOVE();
			}
			/* i=6: */
			{
				s = L_mult(a[16], x[5]);
				s = L_mac(s, a[17], x[4]);
				s = L_mac(s, a[18], x[3]);
				s = L_mac(s, a[19], x[2]);
				s = L_mac(s, a[20], x[1]);
				s16 = mac_r(L_shl(s,3), a[21], x[0]);
				d[6] = engine->sub_values(x_org[6], s16); move16(); FPTR_MOVE();
			}
			/* i=7: */
			{
				s = L_mult(a[22], x[6]);
				s = L_mac(s, a[23], x[5]);
				s = L_mac(s, a[24], x[4]);
				s = L_mac(s, a[25], x[3]);
				s = L_mac(s, a[26], x[2]);
				s = L_mac(s, a[27], x[1]);
				s16 = mac_r(L_shl(s,3), a[28], x[0]);
				d[7] = engine->sub_values(x_org[7], s16); move16(); FPTR_MOVE();
			}
			/* i>=8: */
			FOR (i=8; i<x_length; ++i) {
				s = L_mult(a[29], x[i-1]);
				s = L_mac(s, a[30], x[i-2]);
				s = L_mac(s, a[31], x[i-3]);
				s = L_mac(s, a[32], x[i-4]);
				s = L_mac(s, a[33], x[i-5]);
				s = L_mac(s, a[34], x[i-6]);
				s = L_mac(s, a[35], x[i-7]);
				s16 = mac_r(L_shl(s,3), a[36], x[i-8]);
				d[i] = engine->sub_values(x_org[i], s16); move16(); FPTR_MOVE();
			}
			BREAK;			
		default: assert(!"NEVER HAPPEN"); exit(1);
	} /* end of swith */
	SPR_LEAVE(scp0);
}

static void get_residual_largeframe(const Word16 *x,
                                    UWord16 x_length,
                                    const Word16 *x_org, 
                                    const Word16 *a,
                                    Word16 lpc_order,
                                    Word16 *d,
                                    Word16 *xn,
                                    const struct encoder_map_functions *engine)
{
	const scopeid_t scp0 = SPR_ENTER(2+4+2);
	UWord16 i;
	Word32 s;
	Word16 pred;

	assert(x != NULL && d != NULL && xn != NULL && engine != NULL);
	assert(lpc_order == 0 || (a != NULL && x_org != NULL));
	assert(0 <= lpc_order && lpc_order <= MAX_PREDICTION_ORDER_SUPPORTED);
	assert(x_length == 160 || x_length == 240 || x_length == 320);

	SWITCH (lpc_order) {
		case 1:
			/* i=0: */
			d[0] = engine->map_values(x_org[0]); move16(); FPTR_MOVE();
		
			FOR (i=1; i<x_length; ++i) {
				xn[i] = mult_r(a[1], x[i-1]); move16();
			}
			BREAK;
		case 5: {
			const scopeid_t scp1 = SPR_ENTER(2*2+2);
			Word16 pp, j;
			const UWord16 upper_bound = s_min(x_length, lpc_order);
			d[0] = engine->map_values(x_org[0]); move16(); FPTR_MOVE();
			/* i=1: */
			pred = mult_r(a[1], x[0]);
			d[1] = engine->sub_values(x_org[1], pred); move16(); FPTR_MOVE();
			s = L_mult(a[2], x[1]);
			pred = mac_r(L_shl(s,3), a[3], x[0]);
			d[2] = engine->sub_values(x_org[2], pred); move16(); FPTR_MOVE();
			pp=3; move16();
			FOR (i=3; i<upper_bound; ++i) {
				s = L_mult(a[1+pp], x[i-1]);
				FOR (j=2; j<i; ++j){
					s = L_mac(s, a[j+pp], x[i-j]);
				}
				pred = mac_r(L_shl(s,3), a[i+pp], x[0]);
				pp = add(pp, i);
				d[i] = engine->sub_values(x_org[i], pred); move16(); FPTR_MOVE();
			}
			FOR (i=lpc_order; i<x_length; ++i) {
				s = L_mult(a[1+pp], x[i-1]);
				s = L_mac(s, a[2+pp], x[i-2]);
				s = L_mac(s, a[3+pp], x[i-3]);
				s = L_mac(s, a[4+pp], x[i-4]);
				xn[i] = mac_r(L_shl(s,3), a[lpc_order+pp], x[i-lpc_order]);move16();
			}
			SPR_LEAVE(scp1);
			BREAK;
		}
		case 8: {
			const scopeid_t scp1 = SPR_ENTER(2*2+2);
			Word16 pp, j;
			const UWord16 upper_bound = s_min(x_length, lpc_order);
			d[0] = engine->map_values(x_org[0]); move16(); FPTR_MOVE();
			/* i=1: */
			pred = mult_r(a[1], x[0]);
			d[1] = engine->sub_values(x_org[1], pred); move16(); FPTR_MOVE();
			/* i=2: */
			s = L_mult(a[2], x[1]);
			pred = mac_r(L_shl(s,3), a[3], x[0]);
			d[2] = engine->sub_values(x_org[2], pred); move16(); FPTR_MOVE();
			pp=3; move16();
			FOR (i=3; i<upper_bound; ++i) {
				s = L_mult(a[1+pp], x[i-1]);
				FOR (j=2; j<i; ++j){
					s = L_mac(s, a[j+pp], x[i-j]);
				}
				pred = mac_r(L_shl(s,3), a[i+pp], x[0]);
				pp = add(pp, i);
				d[i] = engine->sub_values(x_org[i], pred); move16(); FPTR_MOVE();
			}
			FOR (i=lpc_order; i<x_length; ++i) {
				s = L_mult(a[1+pp], x[i-1]);
				s = L_mac(s, a[2+pp], x[i-2]);
				s = L_mac(s, a[3+pp], x[i-3]);
				s = L_mac(s, a[4+pp], x[i-4]);
				s = L_mac(s, a[5+pp], x[i-5]);
				s = L_mac(s, a[6+pp], x[i-6]);
				s = L_mac(s, a[7+pp], x[i-7]);
				xn[i] = mac_r(L_shl(s,3), a[lpc_order+pp], x[i-lpc_order]);move16();
			}	
			SPR_LEAVE(scp1);
			BREAK;
		}
		case 10: {
			const scopeid_t scp1 = SPR_ENTER(2*2+2);
			Word16 pp, j;
			const UWord16 upper_bound = s_min(x_length, lpc_order);
			d[0] = engine->map_values(x_org[0]); move16(); FPTR_MOVE();
			/* i=1: */
			pred = mult_r(a[1], x[0]);
			d[1] = engine->sub_values(x_org[1], pred); move16(); FPTR_MOVE();
			/* i=2: */
			s = L_mult(a[2], x[1]);
			pred = mac_r(L_shl(s,3), a[3], x[0]);
			d[2] = engine->sub_values(x_org[2], pred); move16(); FPTR_MOVE();
			pp=3; move16();
			FOR (i=3; i<upper_bound; ++i) {
				s = L_mult(a[1+pp], x[i-1]);
				FOR (j=2; j<i; ++j){
					s = L_mac(s, a[j+pp], x[i-j]);
				}
				pred = mac_r(L_shl(s,3), a[i+pp], x[0]);
				pp = add(pp, i);
				d[i] = engine->sub_values(x_org[i], pred); move16(); FPTR_MOVE();
			}
			FOR (i=lpc_order; i<x_length; ++i) {
				s = L_mult(a[1+pp], x[i-1]);
				s = L_mac(s, a[2+pp], x[i-2]);
				s = L_mac(s, a[3+pp], x[i-3]);
				s = L_mac(s, a[4+pp], x[i-4]);
				s = L_mac(s, a[5+pp], x[i-5]);
				s = L_mac(s, a[6+pp], x[i-6]);
				s = L_mac(s, a[7+pp], x[i-7]);
				s = L_mac(s, a[8+pp], x[i-8]);
				s = L_mac(s, a[9+pp], x[i-9]);
				xn[i] = mac_r(L_shl(s,3), a[lpc_order+pp], x[i-lpc_order]);move16();
			}
			SPR_LEAVE(scp1);
			BREAK;
		}
		case 12: {
			const scopeid_t scp1 = SPR_ENTER(2*2+2);
			Word16 pp, j;
			const UWord16 upper_bound = s_min(x_length, lpc_order);
			d[0] = engine->map_values(x_org[0]); move16(); FPTR_MOVE();
			/* i=1: */
			pred = mult_r(a[1], x[0]);
			d[1] = engine->sub_values(x_org[1], pred); move16(); FPTR_MOVE();
			/* i=2: */
			s = L_mult(a[2], x[1]);
			pred = mac_r(L_shl(s,3), a[3], x[0]);
			d[2] = engine->sub_values(x_org[2], pred); move16(); FPTR_MOVE();
			pp=3; move16();
			FOR (i=3; i<upper_bound; ++i) {
				s = L_mult(a[1+pp], x[i-1]);
				FOR (j=2; j<i; ++j){
					s = L_mac(s, a[j+pp], x[i-j]);
				}
				pred = mac_r(L_shl(s,3), a[i+pp], x[0]);
				pp = add(pp, i);
				d[i] = engine->sub_values(x_org[i], pred); move16(); FPTR_MOVE();
			}
			FOR (i=lpc_order; i<x_length; ++i) {
				s = L_mult(a[1+pp], x[i-1]);
				s = L_mac(s, a[2+pp], x[i-2]);
				s = L_mac(s, a[3+pp], x[i-3]);
				s = L_mac(s, a[4+pp], x[i-4]);
				s = L_mac(s, a[5+pp], x[i-5]);
				s = L_mac(s, a[6+pp], x[i-6]);
				s = L_mac(s, a[7+pp], x[i-7]);
				s = L_mac(s, a[8+pp], x[i-8]);
				s = L_mac(s, a[9+pp], x[i-9]);
				s = L_mac(s, a[10+pp], x[i-10]);
				s = L_mac(s, a[11+pp], x[i-11]);
				xn[i] = mac_r(L_shl(s,3), a[lpc_order+pp], x[i-lpc_order]);move16();
			}
			SPR_LEAVE(scp1);
			BREAK;
		}
		default: assert(!"NEVER HAPPEN"); exit(1);
	} /* end of switch */
	SPR_LEAVE(scp0);
}

static Word16 get_rice_parameter_from_mean(Word16 i_mean, Word16 framelength_index) {
	const scopeid_t scp0 = SPR_ENTER(2);
	Word16 mul;

	mul = 32125; move16(); /* Divisor: 1.02 */
	if (framelength_index > 0) {
		mul = 27962 /* 28007 */; move16(); /* Divisor: 1.17 */
	}
	i_mean = mult(i_mean, mul); /* Divide */
	SPR_LEAVE(scp0);
	return s_and(s_max(sub(8, norm_s(i_mean)), 0), 7);
}

/* Estimate the optimal Golomb-Rice parameter */
static Word16 get_rice_parameter(const Word16 *x, UWord16 N, Word16 *a_mean, Word16 framelength_index) {
	const scopeid_t scp0 = SPR_ENTER(2*2+4+2);
	Word16 n, s;
	Word32 L_mean;
	/* Calculate L1 norm of the residual */
	Word16 i_mean = 0; move16(); /* Q0 */

	assert(x != 0);

	FOR (n=0; n<N; ++n)
		i_mean = add(i_mean, abs_s(x[n]));

	SWITCH (N) {
		case  80:
			L_mean = L_mult(i_mean, 26214);  /* Q0 + Q22 */
			i_mean = extract_l(L_shl(L_mean, -15)); /* Q7 */
			BREAK;
		case  79:
			L_mean = L_mult(i_mean, 26546);  /* Q0 + Q22 */
			i_mean = extract_l(L_shl(L_mean, -15)); /* Q7 */
			BREAK;
		case  78:
			L_mean = L_mult(i_mean, 26887);  /* Q0 + Q22 */
			i_mean = extract_l(L_shl(L_mean, -15)); /* Q7 */
			BREAK;
		case  39:
			L_mean = L_mult(i_mean, 26887);  /* Q0 + Q21 */
			i_mean = extract_l(L_shl(L_mean, -14)); /* Q7 */
			BREAK;
		case  38:
			L_mean = L_mult(i_mean, 27594);  /* Q0 + Q21 */
			i_mean = extract_l(L_shl(L_mean, -14)); /* Q7 */
			BREAK;
		default : /* others */
			L_mean = L_mult(i_mean, div_s(1, N)); /* Q0 + Q15 +1 = Q16 */
			i_mean = extract_l(L_shl(L_mean, 7-16));     /* Q7 */
	}

	s = get_rice_parameter_from_mean(i_mean, framelength_index);
	*a_mean= i_mean; move16();
	SPR_LEAVE(scp0);
	return s;
}

static void low_pass_downsample(const Word16 *y, Word16 y2[])
{
	const scopeid_t scp0 = SPR_ENTER(2+2+2);
	Word16 j;
	const Word16 *r;
	Word16 temp = shr(add(y[0], y[1]), 1);
	/* mean move filter. */
	y2[0] = y[0]; move16();
	r = &y[2]; PTR1_MOVE(); 
	FOR (j=1; j<80; ++j)
	{
		y2[j] = shr(add(*r++, temp), 1); move16();
		temp = shr(add(*r++, y2[j]), 1);
	}
	SPR_LEAVE(scp0);
}

static Word16 ltp_pcm_residual(Word16 nsubframe, const Word16 *res, Word16 input_size, const Word16 *T, Word16 *ltp)
{
	const scopeid_t scp0 = SPR_ENTER(3*2+2*4+2+2+2);
	Word16 i, k, end;
	Word32 eng_ltp, eng_orig;
	Word16 ltp_better;
	Word16 len;
	const Word16 one_before_last = sub(nsubframe, 2);

	len = sub(input_size, T[0]);
	SWITCH (nsubframe) {
		case 2: len = shr(len, 1); BREAK;
		case 3: len = mult(len, 10923); BREAK;
	}

	ltp_better = 0; move16();

	i = T[0]; move16();
	end = add(i, len);
	eng_orig = 1; move32();
	eng_ltp = 0; move32();
	FOR (k=0; k<nsubframe; ++k) {
		const scopeid_t scp1 = SPR_ENTER(2*2+2);
		Word16 gain, ltp_res;
		const Word16 *const r = res - T[k]; PTR2_MOVE();
		gain = G1; move16();
		move16(); /* needed because of T[k] below */
		if (sub(T[k], PT) >= 0) {
			gain = G2; move16();
		}

		FOR (; i<end; ++i) {
			eng_orig = L_mac(eng_orig, res[i], res[i]);

			ltp[i] = mult_r(gain, r[i]); move16();

			ltp_res = sub(res[i], ltp[i]);
			eng_ltp = L_mac(eng_ltp, ltp_res, ltp_res);
		}
		end = add(end, len);
		if (sub(k, one_before_last) == 0) { /* the last subframe can be longer */
			end = input_size; move16();
		}
		SPR_LEAVE(scp1);
	}

	eng_orig = L_sub(eng_orig, L_shr(eng_orig, 3));
	if (L_sub(eng_orig, eng_ltp) > 0) {
		ltp_better = 1; move16();
	}
	SPR_LEAVE(scp0);
	return ltp_better;
}

static void ltp_aulaw_residual(const Word16 *au_input, const Word16 *xn, Word16 *residual, Word16 input_size, const Word16 T0, const Word16 *ltp, const struct encoder_map_functions *engine)
{
	const scopeid_t scp0 = SPR_ENTER(2*2);
	Word16 i, pred;
	FOR (i=0; i<T0; ++i) {
		residual[i] = engine->sub_values(au_input[i], xn[i]); move16(); FPTR_MOVE();
	}
	FOR (i=T0; i<input_size; ++i) {
		pred = add(xn[i], ltp[i]);
		residual[i] = engine->sub_values(au_input[i], pred); move16(); FPTR_MOVE();
	}
	SPR_LEAVE(scp0);
}

static void ltp_open_loop(const Word16 *y, const Word16 *y2, Word16 input_size, Word16 *T)
{
	const scopeid_t scp0 = SPR_ENTER(80*2+2+2*4+5*2+2*2+2);

	/* wtmp is downsampled signal. */
	/* to get two maximam value to get pitch. */

	Word16 tp[80], tpp;
	Word32 ss_tmp, tmp;
	Word16 p01 = PMAX, i, j, smin, p0;
	Word16 qmin, qmax;
	Word16 smax = abs_s(y2[PMAX]); move16();
	/* Get p01 */
	FOR (i=PMAX+1; i<80; ++i) {
		tpp = abs_s(y2[i]);
		smax = s_max(smax, tpp);
		if (sub(smax, tpp) == 0) {
			p01 = i; move16();
		}
	}

	smin = s_max(sub(p01, 15), PMAX);
	smax = s_min(add(p01, 15), 80);

	qmin = sub(smin, PMAX);
	qmax = sub(smax, PMIN);

	/* Calculate tp[] */
	FOR (i=qmin; i<qmax; ++i) {
		tp[i] = mult_r(G1, y2[i]); move16();
	}

	/* Get p0 */
	p0 = PMIN; move16();
	ss_tmp = 0; move32();
	FOR (i=smin; i<smax; ++i) {
		tpp = sub(y2[i], tp[i-p0]);
		assert(qmin <= (i-p0) && (i-p0) < qmax);
		ss_tmp = L_mac(ss_tmp, tpp, tpp);
	}
	FOR (j=PMIN+1; j<PMAX; ++j) {
		move16();
		assert(qmin <= (p01-j) && (p01-j) < qmax);
		IF (s_xor(y2[p01], tp[p01-j]) >= 0) {
			tmp = 0; move32();
			FOR (i=smin; i<smax; ++i) {
				tpp = sub(y2[i], tp[i-j]);
				assert(qmin <= (i-j) && (i-j) < qmax);
				tmp = L_mac(tmp, tpp, tpp);
			}

			if (L_sub(ss_tmp, tmp) > 0) {
				p0 = j; move16();
			}
			ss_tmp = L_min(ss_tmp, tmp);
		}
	}

	IF (sub(p0, 20) < 0) {
		/* Pitch is 2*p0 */
		*T = add(p0, p0);
	} ELSE {
		const scopeid_t scp1 = SPR_ENTER(2+4*4+5*2);

		const Word16 twice_p0 = add(p0, p0);
		Word32 E_tmp0, E_tmp1, E_tmp2, E_tmp3;
		Word16 upper_bound, E0_h, E1_h, E2_h, E3_h;

		/* Check p and 2*p */
		*T = p0; move16();

		/* Common part of p0 case and 2*p0 case: */
		E_tmp2 = 0; move32();
		E_tmp3 = 1; move32();
		FOR (j=twice_p0; j<input_size; ++j) {
			E_tmp2 = L_mac(E_tmp2, y[j], y[j-twice_p0]);
			E_tmp3 = L_mac(E_tmp3, y[j-twice_p0], y[j-twice_p0]);
		}

		IF (E_tmp2 >= 0) {
			/* Part specific to the p0 case: */
			E_tmp0 = 0; move32();
			E_tmp1 = E_tmp3; move32();
			FOR (j=p0; j<input_size; ++j) {
				E_tmp0 = L_mac(E_tmp0, y[j], y[j-p0]);
			}
			upper_bound = sub(input_size, p0);
			FOR (j=sub(input_size, twice_p0); j<upper_bound; ++j) {
				E_tmp1 = L_mac(E_tmp1, y[j], y[j]);
			}

			E0_h = extract_h(E_tmp0);
			E1_h = extract_h(E_tmp1);

			E2_h = extract_h(E_tmp2);
			E3_h = extract_h(E_tmp3);

			if (L_sub(L_mult0(E2_h, E1_h), L_mult0(E3_h, E0_h)) > 0) {
				*T = twice_p0; move16();
			}
		}
		SPR_LEAVE(scp1);
	}
	SPR_LEAVE(scp0);
}

static void Pitch_fine_research(Word16 nsubframe, const Word16 *res, Word16 T[], Word16 input_size)
{
	const scopeid_t scp0 = SPR_ENTER(5*2+2*2+2*2+2*4+2*2+2+2*2);
	Word16 i, j, k, smin, smax, E_h[2], E0_h, E1_h; 
	Word32 E_tmp0, E_tmp1; 
	Word16 exp, exp2;
	Word16 sub_len;
	const Word16 *p0, *p1; 

	/* for the first subframe. */
	smin = s_max(sub(T[0], 3), 20);
	smax = s_min(add(T[0], 3), 83);
	assert(smin < smax);
	/* fix the subframe length  temporarily only for the first subframe. */
	SWITCH (nsubframe) {
		case 1: sub_len = sub(input_size, smax); BREAK;
		case 2: sub_len = shr(sub(input_size, smax), 1); BREAK; /* divide by 2 */
		case 3: sub_len = mult(sub(input_size, smax), 10923); BREAK; /* divide by 3 */
		default: assert(FALSE); SPR_LEAVE(scp0); return; /* never happens */
	}

	p0 = res; PTR1_MOVE();
	p1 = p0 + smin; PTR2_MOVE();
	E_tmp0 = L_mult(p0[0], p1[0]);
	E_tmp1 = L_mac(1, p1[0], p1[0]);
	FOR (j=1; j<sub_len; ++j) {
		E_tmp0 = L_mac(E_tmp0,p0[j],p1[j]);
		E_tmp1 = L_mac(E_tmp1,p1[j],p1[j]);
	}

	exp2 = norm_l(E_tmp1);
	E0_h = extract_h(L_shl(E_tmp0, exp2));
	E1_h = extract_h(L_shl(E_tmp1, exp2));

	T[0] = smin; move16();		/* the first subframe optimize pitch. */

	smin = add(smin, 1);

	FOR (i=smin; i<=smax; ++i) {
		p1 = p0 + i; PTR2_MOVE();
		E_tmp0 = L_mult(p0[0], p1[0]);
		FOR (j=1; j<sub_len; ++j) {
			E_tmp0 = L_mac(E_tmp0, p0[j], p1[j]);
		}
		E_tmp1 = L_msu(E_tmp1, p1[-1], p1[-1]);
		E_tmp1 = L_mac(E_tmp1, p1[sub_len-1], p1[sub_len-1]);

		exp = norm_l(E_tmp1);
		E_h[0] = extract_h(L_shl(E_tmp0, exp));
		E_h[1] = extract_h(L_shl(E_tmp1, exp));

		IF (L_sub(L_mult0(E_h[0], E1_h), L_mult0(E_h[1], E0_h)) > 0) {
			E0_h = E_h[0]; move16();
			E1_h = E_h[1]; move16();
			T[0] = i; move16();		/* the first subframe optimize pitch. */
		}
	}
	
	/* re-compute subframe length according to the first subframe pitch.. */
	p0 = &res[T[0]]; PTR2_MOVE();

	FOR (k=1; k<nsubframe; ++k) {
		p0 = p0 + sub_len; PTR2_MOVE();

		smin = sub(T[k-1], NBITS);
		smax = add(T[k-1], NBITS);

		assert(smin < smax);
		p1 = p0 - smin; move16();
		E_tmp0 = L_mult(p0[0], p1[0]);
		FOR (j=1; j<sub_len; ++j) {
			E_tmp0 = L_mac(E_tmp0, p0[j], p1[j]);
		}
		T[k] = smin; move16();		/* the first subframe optimize pitch. */

		smin = add(smin, 1);

		FOR (i=smin; i<=smax; ++i) {
			p1 = p0 - i; PTR2_MOVE();
			E_tmp1 = L_mult(p0[0], p1[0]);
			FOR (j=1; j<sub_len; ++j) {
				E_tmp1 = L_mac(E_tmp1, p0[j], p1[j]);
			}
			if (L_sub(E_tmp1, E_tmp0) > 0) {
				T[k] = i; move16();		/* the first subframe optimize pitch. */
			}
			E_tmp0 = L_max(E_tmp0, E_tmp1);
		}
	}
	SPR_LEAVE(scp0);
}

static void calculate_parcor(const Word16 *x, Word16 input_size, Word16 *max_order, Word16 *ipar, Word16 framelength_index, Word16 *Tflag_pre, struct g711llc_encoder *encoder)
{
	const scopeid_t scp0 = SPR_ENTER((MAX_PREDICTION_ORDER_SUPPORTED+1)*2+(MAX_PREDICTION_ORDER_SUPPORTED+1)*2+320*2);

	Word16 corr_h[MAX_PREDICTION_ORDER_SUPPORTED+1], corr_l[MAX_PREDICTION_ORDER_SUPPORTED+1];
	Word16 xd[320];

	/* Apply window function */
	window_functions[framelength_index](x, xd); FPTR_MOVE();
	/* Calculate autocorrelation */
	autocorr_functions[framelength_index](xd, input_size, *max_order, corr_h, corr_l, Tflag_pre, encoder, framelength_index); /* max_order might be decreased here */ FPTR_MOVE();

	/* Levinson-Durbin algorithm to calculate the PARCOR coefficients */
	*max_order = levinson(*max_order, corr_h, corr_l, ipar); move16();
	
	SPR_LEAVE(scp0);
}

static void put_normal_frame_flag_40_80(struct output_bit_stream *out, Word16 Tflag)
{
	(void)Tflag;
	put_bit(1, out); /* put 1 */
}

static void put_normal_frame_flag_160(struct output_bit_stream *out, Word16 Tflag)
{
	assert(Tflag == 0 || Tflag == 1);
	/* Put [LTP flag +] normal frame flag */
	put_bits_le8(1, add(shl(Tflag, 1), 1), out); /* put [00]1 */
}

static void put_normal_frame_flag_240_320(struct output_bit_stream *out, Word16 Tflag)
{
	/* Put [LTP flag +] normal frame flag */
	put_bits_le8(3, add(Tflag, 2), out); /* put [0]11 */
}

static void (*const put_normal_frame_flag_functions[5])(struct output_bit_stream *, Word16) = {
	&put_normal_frame_flag_40_80,
	&put_normal_frame_flag_40_80,
	&put_normal_frame_flag_160,
	&put_normal_frame_flag_240_320,
	&put_normal_frame_flag_240_320
};

Word16 g711llc_encode_frame(const Word16 *input, Word16 frame_length, Word16 *output, UWord16 output_size, BOOL mu_law, struct g711llc_encoder *encoder)
{
	const scopeid_t scp0 = SPR_ENTER(2*2+2+2+MAX_PREDICTION_ORDER_SUPPORTED*2+4*2+2+sizeof(struct output_bit_stream)+2+2*2+2+2+2*2+3*2+sizeof(mr_hist_t)+2*320+2*2+2*2);

	Word16 encoded_size, i;
	UWord16 framelength_index;
	Word16 order_index;
	Word16 ipar[MAX_PREDICTION_ORDER_SUPPORTED];
	Word16 s0[4];
	Word16 large_frame;
	struct output_bit_stream out;
	Word16 frame_length_bits;
	Word16 pulse_pos, pulse_num;
	mr_hist_t mr_hist;
	Word16 temp_input[320];
	Word16 y_min, y_max, range;
	BOOL constant;
	Word16 frame_length_flag;
	Word16 pzero_count, mzero_count;
	Word16 const_flag_bits, pm_zero_flag;
	Word16 pzero, mzero;
	
	ipar[0] = 23000; move16(); /* CHECK_MOVE */ /* initialize the first parcor parameter */
	
	assert(input != 0);
	assert(frame_length == 0 || frame_length == 40 || frame_length == 80 || frame_length == 160 || frame_length == 240 || frame_length == 320);
	assert(output != 0);
	assert(output_size >= frame_length + 1);
	assert(output_size >= frame_length + 1 + 3); /* output_bit_stream.c implementation artifact: see comment in ../utility/g711llc_encode_file.c */

	IF (frame_length == 0) {
		output[0] = 0; move16();
		SPR_LEAVE(scp0);
		return 1;
	}

	encoded_size = 0; move16();
	framelength_index = shr(sub(frame_length, 16), 6);
	large_frame = sub(frame_length, 160);

	/* Try to compress the input signal */
	output_bit_stream_open(output, output_size, 0, &out);

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

	check_constant(input, frame_length, pzero, mzero, &constant, &pzero_count, &mzero_count, &pulse_pos);
	pulse_num = sub(frame_length, add(pzero_count, mzero_count));
	const_flag_bits = 6; move16();
	pm_zero_flag = 2; move16();
	frame_length_bits = 2; move16();
	frame_length_flag = add(framelength_index, 1);
	IF (large_frame > 0) {
		const_flag_bits = 4; move16();
		pm_zero_flag = 4; move16();
		frame_length_bits = 4; move16();
		frame_length_flag = sub(framelength_index, 1);
	}

	/* Put the (sub)frame length */
	put_bits_le8(frame_length_flag, frame_length_bits, &out);

	IF (sub(pzero_count, frame_length) == 0) { /* check for constant +zero */
		put_bits_le8(1, const_flag_bits, &out); /* put 0001 or 00 0001 */
		encoded_size = length_in_bytes(&out); move16();
	} ELSE IF (sub(mzero_count, frame_length) == 0) { /* check for constant -zero */
		put_bits_le8(2, const_flag_bits, &out); /* put 0010 or 00 0010 */
		encoded_size = length_in_bytes(&out); move16();
	} ELSE IF (constant != 0) { /* check for general constant value */
		put_bits_le8(3, const_flag_bits, &out); /* put 0011 or 00 0011 */
		put_bits_le8(input[0], 8, &out); /* put the common constant value */
		encoded_size = length_in_bytes(&out); move16();
	} ELSE IF (sub(pulse_num, 2) < 0) { /* check for signal consisting of +zero and -zero only */
		const scopeid_t scp1 = SPR_ENTER(2+3*2+320*2+2*2+2);
		Word16 pulse_value;
		Word16 more_zero, less_zero, s;
		UWord16 counts[320];
		UWord16 counts_length, pos;
		Word16 try_01;

		/* Decide which zero has more occurences in the input signal */
		IF (sub(mzero_count, pzero_count) > 0) {
			more_zero = mzero; move16();
			less_zero = pzero; move16();
			put_bits_le8(add(pm_zero_flag, 1), 3, &out); /* put 101 or 011 */
		} ELSE {
			more_zero = pzero; move16();
			less_zero = mzero; move16();
			put_bits_le8(pm_zero_flag, 3, &out); /* put 100 or 010 */
		}

		/* Find the lenght of the (more_zero,...,more_zero,less_zero) runs */
		counts_length = 0; move16();
		pos = 0; move16();


		WHILE (sub(pos, frame_length) < 0) {
			const scopeid_t scp2 = SPR_ENTER(2+2);
			Word16 count;
			UWord16 start_pos = pos; move16();

			test(); /* for WHILE */
			move16(); /* arithmetic test is performed on an array variable */
			WHILE (sub(pos, frame_length) < 0 && sub(input[pos], less_zero) != 0) {
				test(); /* for WHILE */
				move16(); /* arithmetic test is performed on an array variable */
				pos = add(pos, 1);
			}
			count = sub(pos, start_pos);
			counts[counts_length++] = count; move16();
			pos = add(pos, 1);
			SPR_LEAVE(scp2);
		}

		assert(counts_length > 0);
		/* Estimate the optimal Rice parameter */
		s = s_min(get_rice_parameter_unsigned(counts, counts_length, DEF_MINEXPONENT, DEF_MAXEXPONENT, NULL), (Word16 )pm_max_rice_params[framelength_index]);
		WRD8_MOVE(); /* CHECK_MOVE for Word8 */

		try_01 = shr(frame_length, 3);
		test();
		IF (s > 0 || pulse_num != 0) {
			const scopeid_t scp2 = SPR_ENTER(2+2);
			Word16 const_pm_zero_rice_value, const_pm_zero_rice_length;

			const_pm_zero_rice_value = (Word16 )const_pm_zero_rice_values[framelength_index][s]; move16();
			const_pm_zero_rice_length = (Word16 )const_pm_zero_rice_lengths[framelength_index][s]; move16();

			if (pulse_num != 0) {
				const_pm_zero_rice_length = add(const_pm_zero_rice_length, 2); /* add 00 prefix */
			}

			/* Put the Rice parameter */
			put_bits_le8(const_pm_zero_rice_value, const_pm_zero_rice_length, &out);

			IF (pulse_num != 0) {
				/* Put the pulse position */
				put_bits(pulse_pos, (Word16 )pulse_pos_lengths[framelength_index], &out); WRD8_MOVE(); /* CHECK_MOVE for Word8 */

				pulse_value = input[pulse_pos]; move16();

				IF (sub(abs_s(sub(pulse_value, more_zero)), abs_s(sub(pulse_value, less_zero))) < 0) { 
					put_bit(0, &out);
					rice_encode(sub(add(pulse_value, 1), more_zero), 0, &out);
				} ELSE {
					put_bit(1, &out);
					rice_encode(sub(add(pulse_value, 1), less_zero), 0, &out);
				}
			}

			/* Put the counts */
			rice_encode_block_unsigned(counts, s, counts_length, &out);
			try_01 = sub(sub(length_in_bytes(&out), 1), try_01);

			if (pulse_num != 0) {
				try_01 = -1; move16();
			}
			SPR_LEAVE(scp2);
		}

		/* Simple 01 coding */
		IF (try_01 > 0) {
			/* reset the output stream */
			output_bit_stream_open(output, output_size, frame_length_bits, &out); /* the subframe length is already written */

			/* Simple 01 encoding of the signal */
			put_bits_le8(4, const_flag_bits, &out); /* put 0100 or 00 0100 */

			FOR (i=0; i<frame_length; ++i) {
				const scopeid_t scp2 = SPR_ENTER(2);
				Word16 b = sub(input[i], pzero);
				if (b != 0) {
					b = 1; move16();
				}
				put_bit(b, &out); /* put 0 for pzero, 1 for mzero */
				SPR_LEAVE(scp2);
			}
		}
		encoded_size = length_in_bytes(&out); move16();
		SPR_LEAVE(scp1);
	} ELSE {
		test();
		G711Zcompress_analyze_low(mu_law, input, temp_input, frame_length, &y_min, &y_max, &range);
		IF ((framelength_index > 0) && (multirun_analyze(framelength_index, frame_length, temp_input, y_max, y_min, &mr_hist) >= 0)) {
			const scopeid_t scp1 = SPR_ENTER(2);
			Word16 tmp;
			
			tmp = multirun_encode(&temp_input[0], frame_length, &mr_hist, &output[1], sub(output_size, 1));
			encoded_size = add(tmp, 1);

			/* Set header using formerly reserved states */
			SWITCH (framelength_index) {
				case 0 : output[0] = 0x46; move16(); BREAK;
				case 1 : output[0] = 0x86; move16(); BREAK;
				case 2 : output[0] = 0xc6; move16(); BREAK;
				case 3 : output[0] = 0x25; move16(); BREAK;
				case 4 : output[0] = 0x35; move16(); BREAK;
			}
			SPR_LEAVE(scp1);
		} ELSE { /* normal frame */
			const scopeid_t scp1 = SPR_ENTER(320*2+2+2+(MAX_PREDICTION_ORDER_SUPPORTED*(MAX_PREDICTION_ORDER_SUPPORTED+1)/2)*2+2+2*2+2*2+2+2+2+2+3*2+2+2+2+2*2+2+2+2);

			Word16 x[320];
			Word16 max_order;
			Word16 org_max_order;
			Word16 asi[MAX_PREDICTION_ORDER_SUPPORTED*(MAX_PREDICTION_ORDER_SUPPORTED+1)/2];
			Word16 lpc_order;
			Word16 skip_samples;
			Word16 subblock_size, subdivision;
			Word16 ss;

			/* LTP */
			Word16 Tflag;
			Word16 nsubframe;
			Word16 T[3]; /* detected pitches for each subframe */
			Word16 Tflag_pre;
			const struct encoder_map_functions *map_fn;
			const struct encoder_parcor_functions *parcor_functions;
			const Word16 *to_pcm;

			/* adaptive S for initial value */
			Word16 progressive_s0, progressive_s1;
			Word16 tbl_index;

			/* Set up flag indicating +-zeroes */
			Word16 pm_flag = 0; move16();
			if (mu_law == 0) { pm_flag = 3; move16(); } /* no PM_Zero for A-law */
			if (framelength_index == 0) { pm_flag = 3; move16(); } /* no PM_Zero for n=40 */

			if (pzero_count > 0)
				pm_flag = s_or(pm_flag, 1);
			if (mzero_count > 0)
				pm_flag = s_or(pm_flag, 2);

			/* Set up subtraction routines (for get_residual(), etc.) */
			if (mu_law == 0) { map_fn = &enc_map_functions_alaw; PTR1_MOVE(); }
			if (mu_law != 0) { map_fn = &enc_map_functions_ulaw[pm_flag]; PTR1_MOVE(); }

			/* LTP flag, only for 160, 240, 320 */
			Tflag_pre = 1; move16();

			/* Set the maximum prediction order */
			max_order = max_prediction_orders[framelength_index]; move16();
			org_max_order = max_order; move16();

			/* Convert the input signal to the intermediate PCM format */
			if (mu_law == 0) { to_pcm = tbl_alaw_to_pcm; PTR1_MOVE(); }
			if (mu_law != 0) { to_pcm = tbl_ulaw_to_pcm; PTR1_MOVE(); }
			FOR (i=0; i<frame_length; ++i) {
				assert(0 <= input[i] && input[i] <= 255);
				x[i] = to_pcm[input[i]]; move16();
			}

			/* Calculate the PARCOR coefficients */
			calculate_parcor(x, frame_length, &max_order, ipar, framelength_index, &Tflag_pre, encoder);

			parcor_functions = &encoder_parcor_functions_smallframe; PTR1_MOVE();
			if (large_frame >= 0) {
				parcor_functions = &encoder_parcor_functions_largeframe; PTR1_MOVE();
			}

			/* Decide the actual LPC order */
			/* reduced candidates for opt. order */
			{
				const scopeid_t scp2 = SPR_ENTER(6*2+2);
				Word16 ecost, mcost, sum, order, o, p_ord;
				const Word16 mul = (Word16 )ipar_multipliers[framelength_index]; move16();

				IF (framelength_index == 0) {
					const scopeid_t scp3 = SPR_ENTER(4*2);
					Word16 costs[4];

					FOR (lpc_order=1; lpc_order<=max_order; ++lpc_order) {
						p_ord = (Word16 )p_order[lpc_order]; WRD8_MOVE(); /* CHECK_MOVE for Word8 */
						parcor_functions->quantize_parcor(ipar, lpc_order, asi + p_ord, framelength_index);
						costs[lpc_order-1] = parcor_functions->count_parcor_bits(asi + p_ord, lpc_order, framelength_index); move16();
					}
					sum = mult(mult(ipar[0], ipar[0]), mul);
					/* residual cost estimation for "order" */
					lpc_order = 1; move16();
					mcost = sub(costs[0], sum);  /* total cost estimation for "order" */
					IF (sub(1, max_order) < 0) {
						FOR (i=1; i<max_order; ++i) {
							sum = add(sum, mult(mult(ipar[i], ipar[i]), mul));
							/* residual cost estimation for "order" */
							ecost = sub(costs[i], sum);  /* total cost estimation for "order" */
							if (sub(ecost, mcost) < 0) { /* mininum selection */
								lpc_order = add(i, 1);
							}
							mcost = s_min(mcost, ecost);
						}
					}
					order_index = sub(lpc_order, 1);
					SPR_LEAVE(scp3);
				} ELSE {
					const scopeid_t scp3 = SPR_ENTER(2);
					const Word8 *box = get_box(org_max_order); /* get the 4-length vector of possible prediction orders */

					/* i=0: */
					order_index = 0; move16(); /* index in the box for now */
					order = box[0]; move16();
					/* Calculate the residual cost estimations since the last checked order */
					sum = mult(mult(ipar[order-1], ipar[order-1]), mul);
					o = order; move16();
					parcor_functions->quantize_parcor(ipar, order, asi, framelength_index); FPTR_MOVE();
					mcost= sub(parcor_functions->count_parcor_bits(asi, order, framelength_index), sum); FPTR_MOVE(); /* total cost estimation for "order" */
					/* i=1: */
					order = box[1]; move16();
					IF (sub(order, max_order) <= 0) { /* do not allow orders larger than the max order */
						/* Calculate the residual cost estimations since the last checked order */
						FOR (; o<order; ++o) sum = add(sum, mult(mult(ipar[o], ipar[o]), mul));
						p_ord = (Word16 )p_order[order]; WRD8_MOVE(); /* CHECK_MOVE for Word8 */
						parcor_functions->quantize_parcor(ipar, order, asi+p_ord, framelength_index); FPTR_MOVE();
						ecost = sub(parcor_functions->count_parcor_bits(asi+p_ord, order, framelength_index), sum); FPTR_MOVE(); /* total cost estimation for "order" */

						if (sub(ecost, mcost) < 0) {
							order_index = 1; move16();
						}
						mcost = s_min(mcost, ecost);

						/* i=2: */
						order = box[2]; move16();
						IF (sub(order, max_order) <= 0) { /* do not allow orders larger than the max order */
							/* Calculate the residual cost estimations since the last checked order */
							FOR (; o<order; ++o) sum = add(sum, mult(mult(ipar[o], ipar[o]), mul));
							p_ord = (Word16 )p_order[order]; WRD8_MOVE(); /* CHECK_MOVE for Word8 */
							parcor_functions->quantize_parcor(ipar, order, asi+p_ord, framelength_index); FPTR_MOVE();
							ecost = sub(parcor_functions->count_parcor_bits(asi+p_ord, order, framelength_index), sum); FPTR_MOVE(); /* total cost estimation for "order" */

							if (sub(ecost, mcost) < 0) {
								order_index = 2; move16();
							}
							mcost = s_min(mcost, ecost);

							/* i=3: */
							order = box[3]; move16();
							IF (sub(order, max_order) <= 0) { /* do not allow orders larger than the max order */
								/* Calculate the residual cost estimations since the last checked order */
								FOR (; o<order; ++o) sum = add(sum, mult(mult(ipar[o], ipar[o]), mul));
								p_ord = (Word16 )p_order[order]; WRD8_MOVE(); /* CHECK_MOVE for Word8 */
								parcor_functions->quantize_parcor(ipar, order, asi+p_ord, framelength_index); FPTR_MOVE();
								ecost = sub(parcor_functions->count_parcor_bits(asi+p_ord, order, framelength_index), sum); FPTR_MOVE(); /* total cost estimation for "order" */

								if (sub(ecost, mcost) < 0) {
									order_index = 3; move16();
								}
							}
						}
					}
					lpc_order = box[order_index]; move16(); /* convert index to order */
					SPR_LEAVE(scp3);
				}
				SPR_LEAVE(scp2);
			}

			/* Convert the quantized PARCOR coefficients to direct LPC coefficients */
			{
				const scopeid_t scp2 = SPR_ENTER(320*2);
				Word16 residual[320];

				parcor_functions->reconstruct_parcor(asi+(Word16 )p_order[lpc_order], lpc_order, ipar, framelength_index); FPTR_MOVE();
				WRD8_MOVE(); /* CHECK_MOVE for Word8 */
				{
					const scopeid_t scp3 = SPR_ENTER((MAX_PREDICTION_ORDER_SUPPORTED*(MAX_PREDICTION_ORDER_SUPPORTED+1)/2+1)*2);
					Word16 icof[MAX_PREDICTION_ORDER_SUPPORTED*(MAX_PREDICTION_ORDER_SUPPORTED+1)/2+1]; /* TM20080510 */

					par2cof(ipar, lpc_order, icof);

					Tflag = 0; move16();
					nsubframe = sub(framelength_index, 1);

					/* Get the prediction residual */
					IF (nsubframe <= 0) {
						normal_prediction(x, frame_length, input, icof, lpc_order, residual, map_fn);
					} ELSE {
						const scopeid_t scp4 = SPR_ENTER(320*2+2);

						Word16 xn[320];
						Word16 temp;
					
						/* This fills residual[0..lpc_order-1] and xn[lpc_order..input_size-1] */
						get_residual_largeframe(x, frame_length, input, icof, lpc_order, residual, xn, map_fn);

						IF (Tflag_pre != 0) {
							{
								const scopeid_t scp5 = SPR_ENTER(80*2);
								Word16 x2[80];
								low_pass_downsample(x, x2);
								ltp_open_loop(x, x2, frame_length, T);
								SPR_LEAVE(scp5);
							}

							FOR (i=lpc_order; i<frame_length; ++i) {
								residual[i] = sub(x[i], xn[i]); move16();
							}

							/* to compute residual. */
							temp = sub(frame_length, lpc_order);

							Pitch_fine_research(nsubframe, &residual[lpc_order], T, temp);

							/* Re-use "x" */
							Tflag = ltp_pcm_residual(nsubframe, &residual[lpc_order], temp, T, x);
							IF (Tflag != 0) {
								/* change pcm signal into a/u law. */
								ltp_aulaw_residual(&input[lpc_order], &xn[lpc_order], &residual[lpc_order], temp, T[0], x, map_fn);
							} ELSE {
								FOR (i=lpc_order; i<frame_length; ++i) {
									residual[i] = map_fn->sub_values(input[i], xn[i]); move16();
								}
							}
						} ELSE {
							FOR (i=lpc_order; i<frame_length; ++i) {
								residual[i] = map_fn->sub_values(input[i], xn[i]); move16();
							}
						}
						encoder->Tflag_buff = add(lshl(encoder->Tflag_buff, 1), Tflag);

						SPR_LEAVE(scp4);
					}
					SPR_LEAVE(scp3);
				}

				/* Coding starts here **********************************/

				/* Put normal frame flag */
				put_normal_frame_flag_functions[framelength_index](&out, Tflag); FPTR_MOVE();
				IF (s_min(mu_law, framelength_index) > 0) { /* Put the PM flag only for u-law and n>=80 */
					/* Signal the presence of +/- zeroes */
					IF (sub(pm_flag, 3) == 0) {
						put_bit(0, &out);
					} ELSE {
						put_bits_le8(s_or(pm_flag, 4), 3, &out);
					}
				}

				/* Put the flag for lpc_order */
				put_bits_le8(order_index/*lpc_order*/, 2, &out); /* signal the index of the best order on 2 bits */

				/* Encode and output codes for the PARCOR coefficients */
				parcor_functions->encode_parcor(asi+(Word16 )p_order[lpc_order], lpc_order, &out, framelength_index); FPTR_MOVE();
				WRD8_MOVE(); /* CHECK_MOVE for Word8 */
				IF (Tflag != 0) {
					put_bits_le8(sub(T[0], 20), 6, &out);
					IF (sub(1, nsubframe) < 0) {
						FOR (i=1; i<nsubframe; ++i) {
							const scopeid_t scp3 = SPR_ENTER(2);
							Word16 T_diff = sub(T[i-1], T[i]);
							put_bits_le8(1, diff_bit_num[T_diff+2], &out);
							SPR_LEAVE(scp3);
						}
					}
				}

				/* Encode the residual */
				skip_samples = s_min(lpc_order, 2);

				{
					const scopeid_t scp3 = SPR_ENTER(320*2+320*2+4*2);
					/* e-huffman */
					Word16 jvalues[320];
					Word16 kvalues[320];
					UWord16 huff_index[4];
					IF (large_frame < 0) { /* Frame lengths 40 and 80, no subdivision */
						const scopeid_t scp4 = SPR_ENTER(2*2);

						Word16 a_mean, ssout;

						subblock_size = frame_length; move16();
						subdivision = 1; move16();
						ss = get_rice_parameter(&(residual[skip_samples]), sub(frame_length, skip_samples), &a_mean, framelength_index);
						ss = s_min(ss, 7);

						s0[0] = ss; move16();
						ssout = (Word16 )Rice_map[ss]; move16();

						rice_encode_unsigned(ssout, 1, &out);
						IF (framelength_index != 0) { /* Only for frame length=80 */
							get_kj_values(&(residual[skip_samples]), &(kvalues[skip_samples]), &(jvalues[skip_samples]), s0[0], sub(frame_length, skip_samples));
							huff_index[0] = get_huffman_index(&(kvalues[skip_samples]), sub(frame_length, skip_samples)); move16();/* First EC block is 'skip_samples' samples shorter */
							put_bits_le8(hIndex_value[huff_index[0]], hIndex_len[huff_index[0]], &out);
						}
						SPR_LEAVE(scp4);
					} ELSE { /* For frames of 160 or more samples, we subdivide into 80 sample subblocks */
						const scopeid_t scp4 = SPR_ENTER(4*2+4*2+2+4*2+2*2+2*2+2);
						Word16 t[4], a_mean[4], ssout;
						Word16 diffa[4], ave, qave;
						BOOL t_is_zero, diffa_less_than_qave;
						Word16 s1;

						subblock_size = 80; move16();
						subdivision = framelength_index; move16(); /* 160: 2, 240: 3, 320: 4 */

						s0[0] = s_min(7, get_rice_parameter(&residual[skip_samples], sub(subblock_size, skip_samples), &a_mean[0], framelength_index)); move16();

						ave = a_mean[0]; move16();
						t_is_zero = TRUE; move16();
						FOR (i=1; i<subdivision; ++i) {
							s0[i] = s_min(7, get_rice_parameter(&residual[i*subblock_size], subblock_size, &a_mean[i], framelength_index)); move16();
							ave = add(ave, a_mean[i]);
							t[i-1] = sub(s0[i], s0[i-1]); move16();
							if (t[i-1] != 0) {
								t_is_zero = FALSE; move16();
							}
						}
						ave = mult(ave, (Word16 )ave_multiplier[subdivision-2]); /* average of the average magnitudes of the subblocks */ WRD8_MOVE(); /* CHECK_MOVE for Word8 */
						qave = mult(ave, (Word16 )qave_multiplier[subdivision-2]); WRD8_MOVE(); /* CHECK_MOVE for Word8 */

						diffa_less_than_qave = TRUE; move16();
						FOR (i=0; i<subdivision; ++i) {
							diffa[i] = abs_s(sub(a_mean[i], ave)); move16();
							if (sub(diffa[i], qave) >= 0) {
								diffa_less_than_qave = FALSE; move16();
							}
						}

						s1 = 1; move16(); /* 1 means that we subdivide */
						IF (s_or(t_is_zero, diffa_less_than_qave) != 0) {
							s1 = 0; move16(); /* 0 means the we do not subdivide */
							subdivision = 1; move16();
							s0[0] = get_rice_parameter_from_mean(ave, framelength_index); move16();
							subblock_size = frame_length; move16();
						}

						/* Signal subdivision here for frame length >= 240 */
						IF (large_frame > 0) {
							put_bit(s1, &out);
						}
						ss = s0[0]; move16();
						ssout = ss; move16();

						if (Tflag) ssout = add(8, ss);
						ssout = (Word16 )Rice_map[ssout]; move16();
						rice_encode_unsigned(ssout, 1, &out);

						IF (s_or(s1, large_frame) != 0) { /* There is a special flag when (!s1 && input_size==160) */
							rice_encode_block(t, 0, subdivision-1, &out);
						} ELSE /* Signal no subdivision for frame length == 160 */
							put_bit(1, &out);

						/* e-huffman encode for residuals (n>=160)*/
						get_kj_values(&(residual[skip_samples]), &(kvalues[skip_samples]), &(jvalues[skip_samples]), s0[0], sub(subblock_size, skip_samples));
						huff_index[0] = get_huffman_index(&(kvalues[skip_samples]), sub(subblock_size, skip_samples)); move16(); /* First EC block is 'skip_samples' samples shorter */
						put_bits_le8(hIndex_value[huff_index[0]], hIndex_len[huff_index[0]], &out);
						IF (sub(1,subdivision) < 0) {
							FOR (i=1; i<subdivision; ++i) {
								get_kj_values(&(residual[subblock_size*i]), &(kvalues[subblock_size*i]), &(jvalues[subblock_size*i]), s0[i], subblock_size);
								huff_index[i] = get_huffman_index(&(kvalues[subblock_size*i]), subblock_size); move16();
								put_bits_le8(hIndex_value[huff_index[i]], hIndex_len[huff_index[i]], &out);
							}
						}
						SPR_LEAVE(scp4);
					}

					/* adaptive S for initial value */
					assert(ss <= 7);
					tbl_index = 0; move16();
					IF (Tflag != 0) {
						if (sub(ipar[0], 26500) > 0)
							tbl_index = add(tbl_index, 2); /*2*/
						if (sub(ipar[0], 30800) > 0)
							tbl_index = add(tbl_index, 2); /*4*/
					} ELSE {
						if (sub(ipar[0], 26500) > 0)
							tbl_index = add(tbl_index, 1);
						if (sub(ipar[0], 30800) > 0)
							tbl_index = add(tbl_index, 2);		/*3*/
						test();
						IF (sub(lpc_order,1) > 0 && (L_add(ipar[1], 28000) <0)) {
							tbl_index = 4; move16();
						}	
					}

					progressive_s0 = (Word16 )map_ss0[tbl_index][ss]; move16();
					progressive_s1 = (Word16 )map_ss1[tbl_index][ss]; move16();

					rice_encode(residual[0], progressive_s0, &out);
					IF (sub(lpc_order, 1) > 0)		
						rice_encode(residual[1], progressive_s1, &out);

					/* subblock */
					IF (framelength_index != 0) {
						huffman_encode(&(kvalues[skip_samples]), &(jvalues[skip_samples]), s0[0], sub(subblock_size, skip_samples), huff_index[0], &out); /* First EC block is 'skip_samples' samples shorter */
						IF (sub(1, subdivision) < 0) {
							FOR (i=1; i<subdivision; ++i)
								huffman_encode(&(kvalues[subblock_size*i]), &(jvalues[subblock_size*i]), s0[i], subblock_size, huff_index[i], &out); 
						}
					} ELSE {
						rice_encode_block(&(residual[skip_samples]), s0[0], sub(subblock_size, skip_samples), &out); 
						assert(subdivision == 1);
					}

					encoded_size = length_in_bytes(&out); move16();
					SPR_LEAVE(scp3);
				}
				SPR_LEAVE(scp2);
			}

			IF (sub(encoded_size, 6) > 0) {
				const scopeid_t scp2 = SPR_ENTER(2*2+2+2+2);
				Word16 num_bits, estimated_size;
				Word16 alt_compressed_size;
				Word16 existslevel;
				fract_frame_t fb_type;

				IF ((fb_type = fract_bits_analyze(framelength_index, frame_length, temp_input, y_max, y_min, range, &encoded_size, &existslevel)) != 0) {
					/* Encode frame using fractional-bit compression */
					alt_compressed_size = fract_bits_encode(&temp_input[0], frame_length, &output[1], sub(output_size, 1), y_max, y_min, existslevel);
					output[0] = fb_type; move16();
					encoded_size = add(alt_compressed_size, 1);
				} ELSE IF (framelength_index == 0) {
					/* Check if the Cisco codec compresses better */
					IF (sub(ipar[0], 22000) > 0) {
						IF (sub(range, 4) <= 0) {
							estimated_size = (Word16 )num_range[range]; move16();
							IF (sub(estimated_size, encoded_size) < 0) {
								assert(range != 0);
								num_bits = sub(8, clz_lut[range-1]);
								alt_compressed_size = G711Zcompress_encode_low(&temp_input[0], frame_length, &output[1], y_min, y_max, num_bits);
								output[0] = s_or(s_and(output[0], 0xc0), 0x05); move16(); /* xx00 0101 */
								encoded_size = add(alt_compressed_size, 1);
							}
						}
					} ELSE {
						const scopeid_t scp3 = SPR_ENTER(2*2);
						Word16 UU, LL;
						IF (sub(range, 4) <= 0) {
							assert(range != 0);
							num_bits = sub(8, clz_lut[range-1]);
							estimated_size = (Word16 )num_range[range]; move16();
							IF (sub(estimated_size, encoded_size) < 0) {
								alt_compressed_size = G711Zcompress_encode_low(&temp_input[0], frame_length, &output[1], y_min, y_max, num_bits);
								output[0] = s_or(s_and(output[0], 0xc0), 0x05); move16(); /* xx00 0101 */
								encoded_size = add(alt_compressed_size, 1);
							}
						} ELSE /*IF(sub(order_index,2)<=0)*/ {
							assert(range != 0);
							num_bits = s_min(7, sub(8, clz_lut[range-1]));

							UU = shl(1, num_bits);
							LL = sub(UU, shr(UU, 2));

							test();
							IF (sub(range,LL) > 0 && sub(range,UU) <= 0) {
								estimated_size = add(extract_l(L_mult0(5, num_bits)), 2);
								IF (sub(estimated_size, encoded_size) < 0) {
									alt_compressed_size = G711Zcompress_encode(&temp_input[0], frame_length, &output[1], y_min, y_max, num_bits);
									output[0] = s_or(s_and(output[0], 0xc0), 0x05); move16(); /* xx00 0101 */
									encoded_size = add(alt_compressed_size, 1);
								}
							}
						}
						SPR_LEAVE(scp3);
					}
				}
				SPR_LEAVE(scp2);
			}
			SPR_LEAVE(scp1);
		} /* end of normal frame */
		/* fprintf(stdout,"%5d encoded \n", encoded_size); */
	}

	/* Check if the input signal was compressible or not */
	IF (framelength_index == 0) {
		IF (sub(add(encoded_size, 1), frame_length) > 0) {
			const scopeid_t scp1 = SPR_ENTER(40*2+40*2+40*2+2+2);
			Word16 i_input[40], i_filter[40];
			Word16 sign_flag[40], in;
			const struct encoder_map_functions *map_fn;

			assert(output_size >= 1);
			if (mu_law == 0) { map_fn = &enc_map_functions_alaw; PTR1_MOVE(); }
			if (mu_law != 0) { map_fn = &enc_map_functions_ulaw[3]; PTR1_MOVE(); }
			FOR (i=0; i<frame_length; ++i) {
				in = map_fn->map_values(input[i]);
				sign_flag[i] = 0; move16();
				if (in < 0) {
					sign_flag[i] = 1; move16();
				}
				if (in < 0) {
					in = negate(in);
				}
				i_input[i] = in; move16();
			}
			/* high pass filter */
			FOR (i=NO_COMPRESS_PREDICT_ORDER; i<frame_length; ++i) {
				i_filter[i] =sub(shr(add(add(add(i_input[i-1], i_input[i-2]), i_input[i-3]), i_input[i-4]), NO_COMPRESS_PREDICT_SHIFT), i_input[i]); move16();
			}
			/* reset the output stream */
			output_bit_stream_open(output, output_size, frame_length_bits, &out); /* the subframe length is already written */
			/* Put frame mode flag with reserved LTP flag for 40 */
			put_bits_le8(1, 3, &out); /* put 00 and 1 */
			/* output bitstream */
			FOR (i=0; i<NO_COMPRESS_PREDICT_ORDER; ++i) {
				assert(0 <= i_input[i] && i_input[i] <= 128);
				in = s_or(shl(s_and(i_input[i], 127), 1), sign_flag[i]);
				put_bits_le8(in, 8, &out);
			}
			rice_encode_block_s5(&(i_filter[NO_COMPRESS_PREDICT_ORDER]), sub(frame_length, NO_COMPRESS_PREDICT_ORDER), &out);		   
			FOR (i=NO_COMPRESS_PREDICT_ORDER; i<frame_length; ++i) {
				put_bit(sign_flag[i], &out);
			}
			encoded_size = length_in_bytes(&out); move16();
			IF (sub(encoded_size, frame_length) > 0) {     	
				/* Put the (sub)frame length */
				output[0] = 0x40; move16();
				assert(output_size >= frame_length + 1);
				FOR (i=0; i<frame_length; ++i) {
					output[1+i] = input[i]; move16();
				}
				encoded_size = add(frame_length, 1);
			}
			SPR_LEAVE(scp1);
		}/* end for IF (sub(encoded_size+1, frame_length) > 0) */
	} ELSE { /* else for IF (sub(frame_length, 40) == 0) */
		IF (sub(encoded_size, frame_length) > 0) {
			/* Put the (sub)frame length */
			SWITCH (frame_length) {
				case  80: output[0] = 0x80; move16(); BREAK;
				case 160: output[0] = 0xc0; move16(); BREAK;
				case 240: output[0] = 0x20; move16(); BREAK;
				case 320: output[0] = 0x30; move16(); BREAK;
			}
			assert(output_size >= frame_length + 1);
			FOR (i=0; i<frame_length; ++i) {
				output[1+i] = input[i]; move16();
			}
			encoded_size = add(frame_length, 1);
		}
	}
	SPR_LEAVE(scp0);
	return encoded_size;
}
