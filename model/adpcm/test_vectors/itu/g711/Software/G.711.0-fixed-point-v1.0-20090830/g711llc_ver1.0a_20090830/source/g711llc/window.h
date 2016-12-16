/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: window.h
 Contents: Window functions
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _WINDOW_H
#define _WINDOW_H

#include "../basicop/basicop.h"

/* Window function pointers for each frame size: 40, 80, 160, 240, 320 */
extern void (*const window_functions[5])(const Word16*, Word16*);

#endif /* !_WINDOW_H */

