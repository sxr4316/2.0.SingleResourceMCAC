/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: autocorr.h
 Contents: Functions for calculating the autocorrelation of signals.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _AUTOCORR_H
#define _AUTOCORR_H

#include "../basicop/basicop.h"

struct g711llc_encoder;

typedef void (*autocorr_funcptr)(Word16 *x, UWord16 x_size, Word16 lpc_order, Word16 *r_h, Word16 *r_l, Word16 *Tflag_pre, struct g711llc_encoder *encoder, Word16 framelength_index);

/* Autocorrelation function pointers for each frame size: 40, 80, 160, 240, 320 */
extern const autocorr_funcptr autocorr_functions[5];

#endif /* !_AUTOCORR_H */

