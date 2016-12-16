/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: stack_profile.h
 Contents: Stack use profiler for checking the worst-case RAM use.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _STACK_PROFILE_H
#define _STACK_PROFILE_H

/* Enable/disable stack profiling here: */
/* #define ENABLE_STACK_PROFILING */

typedef unsigned int scopeid_t;

#ifdef ENABLE_STACK_PROFILING
#	include <stdio.h>
#	if __STDC_VERSION__ >= 199901L
#		define SPR_FUNCTION_NAME        __func__
#	elif (defined(__GNUC__) || defined(_MSC_VER))
#		define SPR_FUNCTION_NAME        __FUNCTION__
#	else
#		define SPR_FUNCTION_NAME       "(unknown)"
#	endif
#	define SPR_ENTER(size)                           __spr_enter_scope(size, __FILE__, __LINE__, SPR_FUNCTION_NAME)
#	define SPR_LEAVE(scopeid)                        __spr_leave_scope(scopeid, __FILE__, __LINE__, SPR_FUNCTION_NAME)
#	define SPR_RESET(input_file_name)                __spr_reset_stack_counters(input_file_name)
#	define SPR_NEXT_FRAME()                          __spr_next_frame()
#	define SPR_PRINT(file, frame_length)             __spr_print_stack_profile(file, frame_length)
#	define SPR_UPDATE_WORST(filename, frame_length)  __spr_update_worst_stack_profile(filename, frame_length)
scopeid_t __spr_enter_scope(unsigned int size, const char *file, unsigned int line, const char *function_name);
void __spr_leave_scope(scopeid_t scopeid, const char *file, unsigned int line, const char *function_name);
void __spr_reset_stack_counters(const char *input_file_name);
void __spr_next_frame(void);
void __spr_print_stack_profile(FILE *file, int frame_length);
void __spr_update_worst_stack_profile(const char *filename, int frame_length);
#else /* ENABLE_STACK_PROFILING */
#	define SPR_ENTER(size)                           0
#	define SPR_LEAVE(scopeid)                        (void)scopeid
#	define SPR_RESET(input_file_name)                do { } while (0)
#	define SPR_NEXT_FRAME()                          do { } while (0)
#	define SPR_PRINT(file, frame_length)             do { } while (0)
#	define SPR_UPDATE_WORST(filename, frame_length)  do { } while (0)
#endif /* ENABLE_STACK_PROFILING */

#endif /* !_STACK_PROFILE_H */

