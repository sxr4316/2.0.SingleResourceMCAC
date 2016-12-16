/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: g711llc_defines.h
 Contents: Macro and constant definitions.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

#ifndef _G711LLC_DEFINES_H
#define _G711LLC_DEFINES_H

#define PTR1_MOVE()	/* move16() */	/* this is for memory address copy to a pointer variable : Word16 *p = &(v[1]); */
#define PTR2_MOVE()	move16()	/* this is for memory address calculation of a pointer variable : Word16 *p = p0 + len; */
#define PTR3_MOVE()	move16()	/* this is for inclement an index variable in a loop : j+=i; */
/* #define WRD8_MOVE_IS_FREE */
#define FPTR_MOVE()	move16()	/* function pointer dereference */

/* No compress mode */
#define NO_COMPRESS_PREDICT_ORDER 4
#define NO_COMPRESS_PREDICT_SHIFT (NO_COMPRESS_PREDICT_ORDER >> 1) 

#ifdef WRD8_MOVE_IS_FREE
#	define WRD8_MOVE()
#else
#	define WRD8_MOVE() move16()
#endif

#define WIN_THR1 128

/* For Long Term Prediction */
#define NBITS           2
#define PMIN            10
#define PMAX            42
#define G1              24412		/* 24412.16 0.745 */
#define G2              23101		/* 23101.44 0.705   18186  0.555 */
#define PT              40

/* Maximum LPC prediction order supported. This is limited by the tables in tables.c. */
#define MAX_PREDICTION_ORDER_SUPPORTED  12

/* e-huffman encoding for residual */ 
/* e-Huffman table is used for frames > 40 samples */
#define NUM_HUFFMAN_TABLES	4

/*****************************************************************************/

enum {
	g711llc_default_hz = 8000,
	g711llc_default_frame_length = 160
};

#ifndef BOOL
#	define BOOL     int
#endif
#ifndef FALSE
#	define FALSE    0
#endif
#ifndef TRUE
#	define TRUE     1
#endif

#endif /* !_G711LLC_DEFINES_H */
