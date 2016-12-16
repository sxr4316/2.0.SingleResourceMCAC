/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: window.c
 Contents: Window functions
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#include "g711llc_defines.h"
#include "tables.h"
#include "window.h"
#include "../utility/stack_profile.h"

/* Apply the specified window(s) to the signal.
 * Parameters:
 *   x:            (input ) original signal
 *   x_size:       (input ) number of samples in x
 *   xd:           (output) windowed signal
 *   win32_1:      (input ) ptr to the window for the first part of the signal
 *   win32_2:      (input ) ptr to the END of the window for the second part
 *   dd:           (input ) number of samples at the edged to window
 */
static void window_impl_step1(const Word16 *x, Word16 x_size, Word16 *xd, const Word16 *win32_1, const Word16 *win32_2, Word16 dd)
{
	const scopeid_t scp0 = SPR_ENTER(2+2);

	const UWord16 upper_bound = sub(x_size, dd);
	Word16 n;

	FOR (n=0; n<dd; ++n) {
		xd[n] = mult(x[n], *win32_1); move16(); /* Q0 + Q15 + 1 - Q16 = Q0 */
		++win32_1;
	}
	FOR (; n<upper_bound; ++n) {
		xd[n] = x[n]; move16();
	}
	FOR (; n<x_size; ++n) {
		--win32_2;
		xd[n] = mult(x[n], *win32_2); move16(); /* Q0 + Q15 + 1 - Q16 = Q0 */
	}

	SPR_LEAVE(scp0);
}

/* Apply the specified window(s) to the signal.
 * Parameters:
 *   x:            (input ) original signal
 *   x_size:       (input ) number of samples in x
 *   xd:           (output) windowed signal
 *   win32_1:      (input ) ptr to the window for the first part of the signal
 *   win32_2:      (input ) ptr to the END of the window for the second part
 *   dd:           (input ) number of samples at the edged to window
 */
static void window_impl_step2(const Word16 *x, Word16 x_size, Word16 *xd, const Word16 *win32_1, const Word16 *win32_2, Word16 dd)
{
	const scopeid_t scp0 = SPR_ENTER(2+2);

	const UWord16 upper_bound = sub(x_size, dd);
	Word16 n;

	FOR (n=0; n<dd; ++n) {
		xd[n] = mult(x[n], *win32_1); move16(); /* Q0 + Q15 + 1 - Q16 = Q0 */
		win32_1 += 2;
	}
	FOR (; n<upper_bound; ++n) {
		xd[n] = x[n]; move16();
	}
	FOR (; n<x_size; ++n) {
		win32_2 -= 2;
		xd[n] = mult(x[n], *win32_2); move16(); /* Q0 + Q15 + 1 - Q16 = Q0 */
	}

	SPR_LEAVE(scp0);
}

static void window_40(const Word16 *x, Word16 *xd)
{
	const scopeid_t scp0 = SPR_ENTER(2*2+3*2);

	const Word16 *win32_1, *win32_2;
	Word16 n, abs_x0, abs_x39;

	win32_1 = win40_1; PTR1_MOVE();
	abs_x0 = abs_s(x[0]);
	if (sub(abs_x0, WIN_THR1) < 0) {
		win32_1 = win40_2; PTR1_MOVE();
	}

	win32_2 = win40_1; PTR1_MOVE();
	abs_x39 = abs_s(x[39]);
	if (sub(abs_x39, WIN_THR1) < 0) {
		win32_2 = win40_2; PTR1_MOVE();
	}

	/* Separate implementation to avoid FOR loop overhead: */

	xd[0] = mult(x[0], win32_1[0]); move16(); /* Q0 + Q15 + 1 - Q16 = Q0 */
	xd[1] = mult(x[1], win32_1[1]); move16(); /* Q0 + Q15 + 1 - Q16 = Q0 */
	xd[2] = mult(x[2], win32_1[2]); move16(); /* Q0 + Q15 + 1 - Q16 = Q0 */
	xd[3] = mult(x[3], win32_1[3]); move16(); /* Q0 + Q15 + 1 - Q16 = Q0 */

	FOR (n=4; n<36; ++n) {
		xd[n] = x[n]; move16();
	}

	xd[36] = mult(x[36], win32_2[3]); move16(); /* Q0 + Q15 + 1 - Q16 = Q0 */
	xd[37] = mult(x[37], win32_2[2]); move16(); /* Q0 + Q15 + 1 - Q16 = Q0 */
	xd[38] = mult(x[38], win32_2[1]); move16(); /* Q0 + Q15 + 1 - Q16 = Q0 */
	xd[39] = mult(x[39], win32_2[0]); move16(); /* Q0 + Q15 + 1 - Q16 = Q0 */

	SPR_LEAVE(scp0);
}

static void window_80(const Word16 *x, Word16 *xd)
{
	const scopeid_t scp0 = SPR_ENTER(2*2+2*2);

	const Word16 *win32_1, *win32_2;
	Word16 abs_x0, abs_x79;

	win32_1 = win80_1; PTR1_MOVE();
	abs_x0 = abs_s(x[0]);
	if (sub(abs_x0, WIN_THR1) < 0) {
		win32_1 = win80_2; PTR1_MOVE();
	}

	win32_2 = win80_1; PTR1_MOVE();
	abs_x79 = abs_s(x[79]);
	if (sub(abs_x79, WIN_THR1) < 0) {
		win32_2 = win80_2; PTR1_MOVE();
	}
	window_impl_step1(x, 80, xd, win32_1, win32_2 + 8, 8);

	SPR_LEAVE(scp0);
}

static void window_160(const Word16 *x, Word16 *xd)
{
	window_impl_step2(x, 160, xd, cosrect_win32, cosrect_win32 + 32, 16);
}

static void window_240(const Word16 *x, Word16 *xd)
{
	window_impl_step1(x, 240, xd, cosrect_win24, cosrect_win24 + 24, 24);
}

static void window_320(const Word16 *x, Word16 *xd)
{
	window_impl_step1(x, 320, xd, cosrect_win32, cosrect_win32 + 32, 32);
}

void (*const window_functions[5])(const Word16*, Word16*) = {
	&window_40,
	&window_80,
	&window_160,
	&window_240,
	&window_320
};

