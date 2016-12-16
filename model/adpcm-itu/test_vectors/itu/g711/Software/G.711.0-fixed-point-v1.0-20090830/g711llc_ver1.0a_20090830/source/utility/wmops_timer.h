/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: wmops_timer.h
 Contents: WMOPS timer functions.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _WMOPS_TIMER_H
#define _WMOPS_TIMER_H

#include "../g711llc/g711llc_defines.h"

void wmops_timer_init(const char *name);
void wmops_timer_start(void);
void wmops_timer_end(void);
void wmops_timer_next_frame(BOOL verbose);
void wmops_timer_show(double play_time, const char *message);
void wmops_timer_show_default(double play_time);

#endif /* !_WMOPS_TIMER_H */


