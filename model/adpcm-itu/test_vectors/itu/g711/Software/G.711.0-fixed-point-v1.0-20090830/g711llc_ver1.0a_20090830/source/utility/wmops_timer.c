/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: wmops_timer.c
 Contents: WMOPS timer functions.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#include <stdio.h>
#include "../basicop/basicop.h" /* setCounter(), Init_WMOPS_counter(), WMOPS_output(), Reset_WMOPS_counter(), fwc() */
#ifdef _MSC_VER
#	include <sys/timeb.h> /* timeb, ftime() */
#elif defined(HAVE_SYS_TIME_H)
#	include <sys/time.h> /* timeval, gettimeofday() */
#endif
#include "wmops_timer.h"

static int wmops_timer_id;
static double start_time, end_time;

/* Query the current wall-clock time */
static double get_time(void)
{
#ifdef _MSC_VER
	struct timeb t;
	ftime(&t);
	return (double)(t.time) + (double)(t.millitm) / 1000.0;
#elif defined(HAVE_SYS_TIME_H)
	struct timeval t;
	gettimeofday(&t, 0);
	return (double)(t.tv_sec) + (double)(t.tv_usec) / 1000000.0;
#else
	return 0;
#endif
}

void wmops_timer_init(const char *name)
{
	start_time = get_time();
	end_time = start_time;
	wmops_timer_id = getCounterId((char*)name);
	setCounter(wmops_timer_id);
	Init_WMOPS_counter();
}

void wmops_timer_start(void)
{
	setCounter(wmops_timer_id);
	Init_WMOPS_counter();
	start_time = get_time();
}

void wmops_timer_end(void)
{
	end_time = get_time();
}

void wmops_timer_next_frame(BOOL verbose)
{
	setCounter(wmops_timer_id);
	fwc();
	if (verbose)
		WMOPS_output(0);
	Reset_WMOPS_counter();
}

void wmops_timer_show(double play_time, const char *message)
{
	const double wallclock_time = start_time < end_time ? (end_time - start_time) : 0;
	printf("\n%s took %.3f sec", message, wallclock_time);

	if (play_time != 0)
		printf(" (%.1f x real-time)\n", (wallclock_time>0 ? play_time / wallclock_time : 0.0));
	else
		puts("\n");

	setCounter(wmops_timer_id);
	fwc();
	WMOPS_output(1);
}

void wmops_timer_show_default(double play_time)
{
	wmops_timer_show(play_time, "Processing");
}

