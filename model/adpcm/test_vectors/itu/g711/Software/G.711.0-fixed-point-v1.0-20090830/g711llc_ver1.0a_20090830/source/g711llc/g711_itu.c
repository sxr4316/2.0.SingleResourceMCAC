/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: g711_itu.c
 Contents: Linear PCM to G.711 PCM conversion routines.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#include <assert.h>
#include "tables.h"
#include "g711_itu.h"
#include "../utility/stack_profile.h"

static Word16 pcm_ul_pcm8_half(Word16 in) {
	const scopeid_t scp0 = SPR_ENTER(2+2+2+2+2);

	Word16 i;          /* aux.var.                                */
	Word16 absno;      /* absolute value of linear (input) sample */
	Word16 segno;      /* segment (Table 2/G711, column 1)        */
	Word16 low_nibble; /* low nibble of log companded sample      */
	Word16 lo;

	/* -------------------------------------------------------------------- */
	/* Change from 14 bit left justified to 14 bit right justified          */
	/* Compute absolute value; adjust for easy processing                   */
	/* -------------------------------------------------------------------- */
	if (in < 0)                    /* NB: 33 is the difference value        */
		absno = sub(32, in);       /* between the thresholds for            */
	if (in >= 0)                   /* A-law and u-law.                      */
		absno = add(in, 33);

	absno = s_min(absno, 0x1fff);  /* limitation to "absno" < 8192 */

	/* Determination of sample's segment */
	i = shr(absno, 6);
	segno = sub(9, clz_lut[i]);

	/* Mounting the low-nibble of the log PCM sample */
	assert(((absno >> segno) >> 4) == 1);
	low_nibble = shr(absno, segno); /* right shift of mantissa */

	lo = add(shl(segno, 4), low_nibble);

	SPR_LEAVE(scp0);

	return lo;
}

Word16 pcm_ul_pcm8_PMZero(Word16 in) {
	const scopeid_t scp0 = SPR_ENTER(2);

	Word16 lo;

	lo = pcm_ul_pcm8_half(in);

	if (in >= 0) lo = sub(lo, 32);
	if (in < 0)  lo = sub(31, lo);

	SPR_LEAVE(scp0);

	return lo;
}

Word16 pcm_ul_pcm8_NZero(Word16 in) {
	const scopeid_t scp0 = SPR_ENTER(2);

	Word16 lo;

	lo = pcm_ul_pcm8_half(in);

	if (in >= 0) lo = sub(33, lo);
	if (in < 0) lo = sub(lo, 32);

	SPR_LEAVE(scp0);

	return lo;
}

Word16 pcm_ul_pcm8_PZero(Word16 in) {
	const scopeid_t scp0 = SPR_ENTER(2);

	Word16 lo;

	lo = pcm_ul_pcm8_half(in);

	if (in < 0)  lo = sub(32, lo);
	if (in >= 0) lo = sub(lo, 32);

	SPR_LEAVE(scp0);

	return lo;
}

Word16 pcm_ul_pcm8_MZero(Word16 in) {
	return pcm_ul_pcm8_PZero(in);
}

Word16 pcm_al_pcm8(Word16 in) {
	const scopeid_t scp0 = SPR_ENTER(2+2+2+2+2);

	Word16 i;          /* aux.var.                                */
	Word16 absno;      /* absolute value of linear (input) sample */
	Word16 segno;      /* segment (Table 2/G711, column 1)        */
	Word16 low_nibble; /* low nibble of log companded sample      */
	Word16 lo;

	/* -------------------------------------------------------------------- */
	/* Change from 14 bit left justified to 14 bit right justified          */
	/* Compute absolute value; adjust for easy processing                   */
	/* -------------------------------------------------------------------- */
	if (in < 0)
		absno = sub(-1, in);
	if (in >= 0) {
		absno = in; move16();
	}

	absno = s_min(absno, 4095);  /* limitation to "absno" < 4096 */

	/* Determination of sample's segment */
	i = shr(absno, 6);

	segno = sub(9, clz_lut[i]);

	/* Mounting the low-nibble of the log PCM sample */
	low_nibble = shr(absno, segno); /* right shift of mantissa */

	lo = add(shl(segno, 4), low_nibble);
	if (in >= 0) lo = sub(lo, 16);
	if (in < 0)  lo = sub(15, lo);

	SPR_LEAVE(scp0);

	return lo;
}
