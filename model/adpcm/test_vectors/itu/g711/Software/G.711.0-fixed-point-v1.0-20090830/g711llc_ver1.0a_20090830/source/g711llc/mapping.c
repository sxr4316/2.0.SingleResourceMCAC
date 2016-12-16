/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: mapping.c
 Contents: G.711 PCM and Linear PCM subtraction functions.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#include "g711llc_defines.h"
#include "../basicop/basicop.h"
#include "tables.h"
#include "g711_itu.h"
#include "mapping.h"
#include "../utility/stack_profile.h"

/*****************************************************************************/
/* ENCODER SIDE **************************************************************/
/*****************************************************************************/

/* a: u-law value, b: PCM value */
static Word16 sub_ulaw_PMZero(Word16 a, Word16 b) {
	b = pcm_ul_pcm8_PMZero(b);
	return sub(from_linear_ulaw[a], b);
}

static Word16 map_ulaw_PMZero(Word16 a) { /* same as sub_ulaw_PMZero(a, 0) */
	return from_linear_ulaw[a];
}

/* a: u-law value, b: PCM value */
static Word16 sub_ulaw_NZero(Word16 a, Word16 b) {
	const scopeid_t scp0 = SPR_ENTER(2*2);
	Word16 aa, bb;
	aa = sub(a, 127);
	if (aa > 0) aa = sub(254, a);
	bb = pcm_ul_pcm8_NZero(b);
	SPR_LEAVE(scp0);
	return add(aa, bb);
}

static Word16 map_ulaw_NZero(Word16 a) { /* same as sub_ulaw_NZero(a, 0) */
	const scopeid_t scp0 = SPR_ENTER(2);
	Word16 aa = sub(a, 127);
	if (aa > 0) aa = sub(254, a);
	SPR_LEAVE(scp0);
	return add(aa, 1);
}

/* a: u-law value, b: PCM value */
static Word16 sub_ulaw_PZero(Word16 a, Word16 b) {
	const scopeid_t scp0 = SPR_ENTER(2*2);
	Word16 aa, bb;
	aa = sub(a, 127);
	if (aa > 0) aa = sub(255, a);
	bb = pcm_ul_pcm8_PZero(b);
	SPR_LEAVE(scp0);
	return sub(aa, bb);
}

static Word16 map_ulaw_PZero(Word16 a) { /* same as sub_ulaw_PZero(a, 0) */
	const scopeid_t scp0 = SPR_ENTER(2);
	Word16 aa = sub(a, 127);
	if (aa > 0) aa = sub(255, a);
	SPR_LEAVE(scp0);
	return aa;
}

/* a: u-law value, b: PCM value */
static Word16 sub_ulaw_MZero(Word16 a, Word16 b) {
	const scopeid_t scp0 = SPR_ENTER(2*2);
	Word16 aa, bb;
	aa = sub(a, 127);
	if (aa > 0) aa = sub(255, a);
	bb = pcm_ul_pcm8_MZero(b);
	SPR_LEAVE(scp0);
	return sub(aa, bb);
}

static Word16 map_ulaw_MZero(Word16 a) { /* same as sub_ulaw_MZero(a, 0) */
	const scopeid_t scp0 = SPR_ENTER(2);
	Word16 aa = sub(a, 127);
	if (aa > 0) aa = sub(255, a);
	SPR_LEAVE(scp0);
	return aa;
}

/* a: a-law value, b: PCM value */
static Word16 sub_alaw(Word16 a, Word16 b) {
	b = pcm_al_pcm8(b);
	return sub(from_linear_alaw[a], b);
}

static Word16 map_alaw(Word16 a) { /* same as sub_alaw(a, 0) */
	return from_linear_alaw[a];
}

const struct encoder_map_functions enc_map_functions_ulaw[4] = {
	{ &sub_ulaw_NZero,  &map_ulaw_NZero  },
	{ &sub_ulaw_PZero,  &map_ulaw_PZero  },
	{ &sub_ulaw_MZero,  &map_ulaw_MZero  },
	{ &sub_ulaw_PMZero, &map_ulaw_PMZero }
};

const struct encoder_map_functions enc_map_functions_alaw = {
	&sub_alaw, &map_alaw
};

/*****************************************************************************/
/* DECODER SIDE **************************************************************/
/*****************************************************************************/

/* a: u-law value, b: PCM value */
Word16 add_ulaw_PMZero(Word16 a, Word16 b) {
	const scopeid_t scp0 = SPR_ENTER(2);
	Word16 bb;
	bb = pcm_ul_pcm8_PMZero(b);
	a = add(a,bb);
	if (a >= 0) a = sub(127, a);
	a = add(a, 128);
	SPR_LEAVE(scp0);
	return a;
}

Word16 unmap_ulaw_PMZero(Word16 a) { /* same as add_ulaw_PMZero(a, 0) */
	if (a >= 0) a = sub(127, a);
	return add(a, 128);
}

/* a: u-law value, b: PCM value */
Word16 add_ulaw_NZero(Word16 a, Word16 b) {
	const scopeid_t scp0 = SPR_ENTER(2);
	Word16 bb;
	bb = pcm_ul_pcm8_NZero(b);
	a = sub(a, bb);
	if (a >= 0) a = sub(127, a);
	SPR_LEAVE(scp0);
	return add(a, 127);
}

Word16 unmap_ulaw_NZero(Word16 a) { /* same as add_ulaw_NZero(a, 0) */
	if (a > 0) a = sub(129, a);
	return add(a, 126);
}

/* a: u-law value, b: PCM value */
Word16 add_ulaw_PZero(Word16 a, Word16 b) {
	const scopeid_t scp0 = SPR_ENTER(2);
	Word16 bb;
	bb = pcm_ul_pcm8_PZero(b);
	a = add(a, bb);
	if (a >= 0)	a = sub(128, a);
	SPR_LEAVE(scp0);
	return add(a, 127);
}

Word16 unmap_ulaw_PZero(Word16 a) { /* same as add_ulaw_PZero(a, 0) */
	if (a >= 0)	a = sub(128, a);
	return add(a, 127);
}

/* a: u-law value, b: PCM value */
Word16 add_ulaw_MZero(Word16 a, Word16 b) {
	const scopeid_t scp0 = SPR_ENTER(2);
	Word16 bb;
	bb = pcm_ul_pcm8_MZero(b);
	a = add(a, bb);
	if (a > 0) a = sub(128, a);
	SPR_LEAVE(scp0);
	return add(a, 127);
}

Word16 unmap_ulaw_MZero(Word16 a) { /* same as add_ulaw_MZero(a, 0) */
	if (a > 0) a = sub(128, a);
	return add(a, 127);
}

/* a: a-law value, b: PCM value */
Word16 add_alaw(Word16 a, Word16 b) {
	const scopeid_t scp0 = SPR_ENTER(2*2);
	Word16 aa, bb;
	bb = pcm_al_pcm8(b);
	a = add(a, bb);
	aa = add(from_linear_alaw[a+128], 128);
	SPR_LEAVE(scp0);
	return aa;
}

Word16 unmap_alaw(Word16 a) { /* same as add_alaw(a, 0) */
	return add(from_linear_alaw[a+128], 128);
}

const struct decoder_map_functions dec_map_functions_ulaw[4] = {
	{ &add_ulaw_NZero,  &unmap_ulaw_NZero  },
	{ &add_ulaw_PZero,  &unmap_ulaw_PZero  },
	{ &add_ulaw_MZero,  &unmap_ulaw_MZero  },
	{ &add_ulaw_PMZero,	&unmap_ulaw_PMZero }
};

const struct decoder_map_functions dec_map_functions_alaw = {
	&add_alaw, &unmap_alaw
};
