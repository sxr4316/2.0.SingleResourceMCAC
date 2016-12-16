/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: g711_itu.h
 Contents: Linear PCM to G.711 PCM conversion routines.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _G711_ITU_H
#define _G711_ITU_H

#include "g711llc_defines.h"
#include "../basicop/basicop.h"

extern Word16 pcm_ul_pcm8_PMZero(Word16 in);
extern Word16 pcm_ul_pcm8_PZero(Word16 in);
extern Word16 pcm_ul_pcm8_MZero(Word16 in);
extern Word16 pcm_ul_pcm8_NZero(Word16 in);

extern Word16 pcm_al_pcm8(Word16 in);

#endif /* !_G711_ITU_H */

