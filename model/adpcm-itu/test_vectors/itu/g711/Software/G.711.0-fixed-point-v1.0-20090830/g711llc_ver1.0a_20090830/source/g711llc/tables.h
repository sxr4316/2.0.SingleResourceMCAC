/***************************************************************************
 ITU-T G.711.0 ANSI-C Source Code Release 1.0

 (C) Cisco Systems Inc., Huawei, NTT and Texas Instruments Incorporated.

 Filename: tables.h
 Contents: Declarations of Read Only tables.
 Version: 1.0
 Revision Date: July 24, 2009
**************************************************************************/

/*****************************************************************************
 * IMPORTANT: this file will be processed by a script to calculate the total *
 *            required ROM size.                                             *
 *                                                                           *
 * Rules:                                                                    *
 *  1. Declarations must begin with "extern const"                           *
 *  2. Declare only a single table per line.                                 *
 *  3. Always include the number of elements in array declarations.          *
 *  4. Arrays must contain simple elements (Word16, UWord16, pointers) only. *
 *****************************************************************************/
#ifndef _TABLES_H
#define _TABLES_H

#include "g711llc_defines.h"
#include "../basicop/basicop.h"

extern const Word8 max_prediction_orders[5];

/* From conversion_tables.c: *************************************************/
extern const Word16 tbl_ulaw_to_pcm[256];
extern const Word16 tbl_alaw_to_pcm[256];

/* From escaped_huffman.h: ***************************************************/
extern const Word16 hIndex_len[4];
extern const Word16 hIndex_value[4];
extern const Word8 dIndex[8];
extern const Word8 Huffman_maxCodeValue[4];

enum { huffman_read = 6 };
extern const Word16 huffman_table_len[4][8];
extern const Word16 huffman_table_value[4][8];
extern const Word16 diff_length[3][8];
extern const Word8 huffman_index[4][64];

/* From parcor.c: ************************************************************/
/* reconstruction levels for 1st and 2nd coefficients: */
extern const Word16 pc_expand01_largeframe[128];

enum { pc_max_codelen_largeframe = 13 }; /* You have to update this if you want to use more than 13 bits for the PARCOR coefficients! */

extern const Word8 pc_bits_largeframe_o1[1];
extern const Word8 pc_bits_largeframe_o5[5];
extern const Word8 pc_bits_largeframe_others[12];

/* lpc_order = 1 *************************************************************/
extern const Word8 pc_codes_largeframe_o1[8];
extern const Word8 pc_code_lengths_largeframe_o1[8];

/* lpc_order = 5,6 ************************************************************/
extern const Word8 pc_codes_largeframe_o5[80];
extern const Word8 pc_code_lengths_largeframe_o5[80];

/* lpc_order >= 7 *************************************************************/
extern const Word8 pc_codes_largeframe_others[144];
extern const Word8 pc_code_lengths_largeframe_others[144];

extern const Word8* const pc_bits_largeframe[13];
extern const Word8* const pc_codes_largeframe[13];
extern const Word8* const pc_code_lengths_largeframe[13];

extern const Word8 pc_ind_smallframe_3[8];
extern const Word8 pc_ind_smallframe_4[16];
extern const Word8 pc_ind_smallframe_5[32];
extern const Word8* pc_ind_smallframe[6];

extern const Word16 pc_val_smallframe_3[6];
extern const Word16 pc_val_smallframe_4[12];
extern const Word16 pc_val_smallframe_5[24];
extern const Word16* pc_val_smallframe[6];

extern const Word8 pc_bits_smallframe_n40_80o1[1];
extern const Word8 pc_bits_smallframe_n40o2_3[3];
extern const Word8 pc_bits_smallframe_n40o4[4];

extern const Word8 pc_bits_smallframe_n80o2[2];
extern const Word8 pc_bits_smallframe_n80o6[6];
extern const Word8 pc_bits_smallframe_n80o8[8];

extern const Word8* pc_bits_smallframe_n40[5];
extern const Word8* pc_bits_smallframe_n80[9];

extern const Word8** pc_bits_smallframe[2];

extern const Word8 pc_num_smallframe_n40_80o1[1];
extern const Word8 pc_num_smallframe_n40o2_3[3];
extern const Word8 pc_num_smallframe_n40o4[4];

extern const Word8 pc_num_smallframe_n80o2[2];
extern const Word8 pc_num_smallframe_n80o6[6];
extern const Word8 pc_num_smallframe_n80o8[8];

extern const Word8* pc_num_smallframe_n40[5];
extern const Word8* pc_num_smallframe_n80[9];

extern const Word8** pc_num_smallframe[2];

extern const Word8 pc_rice_smallframe_n40_80_o1_enc_dec[6];
extern const Word8 pc_rice_smallframe_n40_o2_enc[18];
extern const Word8 pc_rice_smallframe_n40_o2_dec[18];
extern const Word8 pc_rice_smallframe_n40_o3_enc[24];
extern const Word8 pc_rice_smallframe_n40_o3_dec[24];
extern const Word8 pc_rice_smallframe_n40_o4_enc[42];
extern const Word8 pc_rice_smallframe_n40_o4_dec[42];
extern const Word8 pc_rice_smallframe_n80_o2_enc[36];
extern const Word8 pc_rice_smallframe_n80_o2_dec[36];
extern const Word8 pc_rice_smallframe_n80_o6_enc[84];
extern const Word8 pc_rice_smallframe_n80_o6_dec[84];
extern const Word8 pc_rice_smallframe_n80_o8_enc[102];
extern const Word8 pc_rice_smallframe_n80_o8_dec[102];

extern const Word8* pc_riceval_smallframe_enc_n40[5];
extern const Word8* pc_riceval_smallframe_dec_n40[5];
extern const Word8* pc_riceval_smallframe_enc_n80[9];
extern const Word8* pc_riceval_smallframe_dec_n80[9];

extern const Word8** pc_riceval_smallframe_enc[2];
extern const Word8** pc_riceval_smallframe_dec[2];

extern const Word8 pc_quantize_smallframe_nonzero[6];

extern const Word8 pc_ricepara_smallframe[6];

extern const Word8 box_l4[4];
extern const Word8 box_8[4];
extern const Word8 box_10[4];
extern const Word8 box_12[4];

/* From input_bit_stream.c: **************************************************/
extern const Word16 clz_lut[256];

/* From G711Zdecode_function.c: **********************************************/
extern const Word16 from_linear_ulaw[256];
extern const Word16 from_linear_alaw[256];
extern const Word8 y_anchor_table[30];
extern const Word16 G711Z_mask[9];

/* From G711Zencode_function.c: **********************************************/
extern const Word8 tab_anchor[39];
extern const Word8 tab_codepoint[39];
extern const Word8 tab_anchor1[12];
extern const Word8 tab_codepoint1[12];
extern const Word8 tab_codepoint2[5];

/* From g711llc_encoder_engine_ntt.c: ****************************************/
extern const Word16 cosrect_win32[32];
extern const Word16 cosrect_win24[24];
extern const Word16 win40_1[4];
extern const Word16 win40_2[4];
extern const Word16 win80_1[8];
extern const Word16 win80_2[8];

extern const Word8 pm_max_rice_params[5];

extern const Word8 const_pm_zero_rice_value40[6];
extern const Word8 const_pm_zero_rice_len40[6];
extern const Word8 const_pm_zero_rice_value80[7];
extern const Word8 const_pm_zero_rice_len80[7];
extern const Word8 const_pm_zero_rice_value320[10];
extern const Word8 const_pm_zero_rice_len320[10];

extern const Word8 *const const_pm_zero_rice_values[5];
extern const Word8 *const const_pm_zero_rice_lengths[5];
extern const Word8 pulse_pos_lengths[5];

extern const UWord8 p_order[13];
extern const UWord8 ipar_multipliers[5];

extern const Word16 ave_multiplier[3];
extern const Word16 qave_multiplier[3];

extern const Word8 num_range[9];

extern const Word16 diff_bit_num[5];

extern const Word16 autocorr_lag_tab[MAX_PREDICTION_ORDER_SUPPORTED+1];

extern const UWord8 Rice_map[16];
extern const UWord8 Rice_map_inv[16];

/* From g711llc_decoder_engine_ntt.c: ****************************************/
extern const Word8 map_ss0[5][8];
extern const Word8 map_ss1[5][8];

/* From autocorr.c: **********************************************************/
extern const Word16 autocorr_pre_Tflag_count[3];

/* From fract_bits.c: ********************************************************/
extern const Word8 bits_per_block[5];
extern const Word8 samp_per_block[5];
extern const Word16 bytes_per_frame[5][5];
extern const Word16 fb_type_offsets[5];

#endif /* !_TABLES_H */

