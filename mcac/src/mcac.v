/*

Description: Top Level Interfacing of MCAC module.
		Contains encoder, decoder , config functionalities

Author: Siddharth Ramkrishnan, Adam Steenkamer

Revision History
//----------------------------------------------------------------------------
04/01/16	-	Siddharth Ramkrishnan	-	Initial Creation
04/03/16    - Adam Steenkamer - Fixed port names. Fixed some port declarations.
				Added declarations for signals used and not declared.
//----------------------------------------------------------------------------
*/

module mcac #(
           parameter WB_DWIDTH = 128,
           parameter WB_SWIDTH = 16,
           parameter WB_SLAVES = 10)
       (
           // common signals
           reset,
           clk,
           test_mode,
           scan_enable,
           scan_in0,
           scan_in1,
           scan_in2,
           scan_in3,
           scan_in4,
           scan_out0,
           scan_out1,
           scan_out2,
           scan_out3,
           scan_out4,
           // encoder ports
           enc_s,
           enc_s_clk,
           enc_s_fs,
           enc_i,
           enc_i_clk,
           enc_i_fs,
           // decoder ports
           dec_i,
           dec_i_clk,
           dec_i_fs,
           dec_s,
           dec_s_clk,
           dec_s_fs,
           // config ports
           rs,
           ws,
           cs,
           r_data,
           w_data,
           addrs,
           // Encoder UART ports
           i_uart0_tx_e,
           i_uart0_rts_e,
           o_uart0_rx_e,
           o_uart0_cts_e,
           // Decoder UART ports
           i_uart0_tx_d,
           i_uart0_rts_d,
           o_uart0_rx_d,
           o_uart0_cts_d,
           // Main Memory to encoder interface
           o_mm_wb_adr_e,
           o_mm_wb_sel_e,
           o_mm_wb_we_e,
           i_mm_wb_dat_e,
           o_mm_wb_dat_e,
           o_mm_wb_cyc_e,
           o_mm_wb_stb_e,
           i_mm_wb_ack_e,
           i_mm_wb_err_e,
           // Main memory to decoder interface
           o_mm_wb_adr_d,
           o_mm_wb_sel_d,
           o_mm_wb_we_d,
           i_mm_wb_dat_d,
           o_mm_wb_dat_d,
           o_mm_wb_cyc_d,
           o_mm_wb_stb_d,
           i_mm_wb_ack_d,
           i_mm_wb_err_d
       );

input
    reset,                      // system reset
    clk;                        // system clock

input
    scan_in0,                   // test scan mode data input
    scan_in1,                   // test scan mode data input
    scan_in2,                   // test scan mode data input
    scan_in3,                   // test scan mode data input
    scan_in4,                   // test scan mode data input
    scan_enable,                // test scan mode enable
    test_mode;                  // test mode

output
    scan_out0,                  // test scan mode data output
    scan_out1,                  // test scan mode data output
    scan_out2,                  // test scan mode data output
    scan_out3,                  // test scan mode data output
    scan_out4;                  // test scan mode data output

// System Input Ports
input
      rs,			// read strobe signal for config module
      ws,			// write strobe signal for config module
      cs,			// Chip Select signal for config module
      enc_s,		// encoder PCM input serial data
      enc_s_clk,	// encoder channel input clock
      enc_s_fs,		// encoder channel input frame sync
      dec_i,		// decoder ADPCM input serial data
      dec_i_clk,	// decoder channel input clock
      dec_i_fs;		// decoder channel input frame sync

input [7:0]
      w_data;		// data to be written into config module

input [6:0]
      addrs;		// address information for config module

// Encoder UART Interface I/O
input
    i_uart0_tx_e,
    i_uart0_rts_e;

output
    o_uart0_rx_e,
    o_uart0_cts_e;

// Decoder UART Interface I/O
input
    i_uart0_tx_d,
    i_uart0_rts_d;

output
    o_uart0_rx_d,
    o_uart0_cts_d;

// Main Memory to encoder Wishbone Interface
input [WB_DWIDTH-1:0]
      i_mm_wb_dat_e;

input
    i_mm_wb_ack_e,
    i_mm_wb_err_e;

output [31:0]
       o_mm_wb_adr_e;

output [WB_SWIDTH-1:0]
       o_mm_wb_sel_e;

output [WB_DWIDTH-1:0]
       o_mm_wb_dat_e;

output
    o_mm_wb_we_e,
    o_mm_wb_cyc_e,
    o_mm_wb_stb_e;

// Main Memory to decoder Wishbone Interface
input [WB_DWIDTH-1:0]
      i_mm_wb_dat_d;

input
    i_mm_wb_ack_d,
    i_mm_wb_err_d;

output [31:0]
       o_mm_wb_adr_d;

output [WB_SWIDTH-1:0]
       o_mm_wb_sel_d;

output [WB_DWIDTH-1:0]
       o_mm_wb_dat_d;

output
    o_mm_wb_we_d,
    o_mm_wb_cyc_d,
    o_mm_wb_stb_d;

// System Output Ports
output [7:0]
       r_data;		// data read from config

output
       enc_i,		// Encoder ADPCM output serial data
       enc_i_clk,	// Encoder chhanel output clock
       enc_i_fs,	// Encoder channel output frame sync
       dec_s,		// Decoder PCM output serial data
       dec_s_clk,	// Decoder channel output clock
       dec_s_fs;	// Decoder channel output frame sync

// Config Module Wishbone Interface internal signals
wire [31:0]
     o_cfg_wb_adr_e,
     o_cfg_wb_adr_d;

wire [WB_SWIDTH-1:0]
     o_cfg_wb_sel_e,
     o_cfg_wb_sel_d;

wire
    o_cfg_wb_we_e,
    o_cfg_wb_we_d;

wire [WB_DWIDTH-1:0]
     o_cfg_wb_dat_d,
     o_cfg_wb_dat_e,
     i_cfg_wb_dat_d,
     i_cfg_wb_dat_e;

wire
    o_cfg_wb_cyc_d,
    o_cfg_wb_cyc_e,
    o_cfg_wb_stb_d,
    o_cfg_wb_stb_e;

wire
    i_cfg_wb_ack_e,
    i_cfg_wb_err_e,
    i_cfg_wb_ack_d,
    i_cfg_wb_err_d;


assign i_cfg_wb_dat_d[63:32] = i_cfg_wb_dat_d[31:0];
assign i_cfg_wb_dat_d[95:64] = i_cfg_wb_dat_d[31:0];
assign i_cfg_wb_dat_d[127:96] = i_cfg_wb_dat_d[31:0];

assign i_cfg_wb_dat_e[63:32] = i_cfg_wb_dat_e[31:0];
assign i_cfg_wb_dat_e[95:64] = i_cfg_wb_dat_e[31:0];
assign i_cfg_wb_dat_e[127:96] = i_cfg_wb_dat_e[31:0];



// Port map modules

CFG_INT CFG_INST0(
            .reset(reset),
            .clk(clk),
            .scan_in0(),
            .scan_in1(),
            .scan_in2(),
            .scan_in3(),
            .scan_in4(),
            .scan_enable(scan_enable),
            .test_mode(test_mode),
            .scan_out0(),
            .scan_out1(),
            .scan_out2(),
            .scan_out3(),
            .scan_out4(),
            .rs(rs),
            .ws(ws),
            .cs(cs),
            .w_data(w_data),
            .addrs(addrs),
            .r_data(r_data),

            .i_wb_adr_e(o_cfg_wb_adr_e),
            .i_wb_sel_e(o_cfg_wb_sel_e[3:0]),
            .i_wb_we_e (o_cfg_wb_we_e),
            .o_wb_dat_e(i_cfg_wb_dat_e[31:0]),
            .i_wb_dat_e(o_cfg_wb_dat_e[31:0]),
            .i_wb_cyc_e(o_cfg_wb_cyc_e),
            .i_wb_stb_e(o_cfg_wb_stb_e),
            .o_wb_ack_e(i_cfg_wb_ack_e),
            .o_wb_err_e(i_cfg_wb_err_e),

            .i_wb_adr_d(o_cfg_wb_adr_d),
            .i_wb_sel_d(o_cfg_wb_sel_d[3:0]),
            .i_wb_we_d (o_cfg_wb_we_d),
            .o_wb_dat_d(i_cfg_wb_dat_d[31:0]),
            .i_wb_dat_d(o_cfg_wb_dat_d[31:0]),
            .i_wb_cyc_d(o_cfg_wb_cyc_d),
            .i_wb_stb_d(o_cfg_wb_stb_d),
            .o_wb_ack_d(i_cfg_wb_ack_d),
            .o_wb_err_d(i_cfg_wb_err_d)
        );

enc #(.WB_DWIDTH(WB_DWIDTH), .WB_SWIDTH(WB_SWIDTH), .WB_SLAVES(WB_SLAVES)) ENC_INST0(
        .reset(reset),
        .clk(clk),
        .enc_s(enc_s),
        .enc_s_clk(enc_s_clk),
        .enc_s_fs(enc_s_fs),
        .scan_in0(),
        .scan_in1(),
        .scan_in2(),
        .scan_in3(),
        .scan_in4(),
        .scan_enable(scan_enable),
        .test_mode(test_mode),
        .scan_out0(),
        .scan_out1(),
        .scan_out2(),
        .scan_out3(),
        .scan_out4(),
        .i_uart0_tx(i_uart0_tx_e),
        .i_uart0_rts(i_uart0_rts_e),
        .o_uart0_cts(o_uart0_cts_e),
        .o_uart0_rx(o_uart0_rx_e),
        .enc_o(enc_i),
        .enc_o_clk(enc_i_clk),
        .enc_o_fs(enc_i_fs),

        .o_mm_wb_adr(o_mm_wb_adr_e),
        .o_mm_wb_sel(o_mm_wb_sel_e),
        .o_mm_wb_we (o_mm_wb_we_e),
        .i_mm_wb_dat(i_mm_wb_dat_e),
        .o_mm_wb_dat(o_mm_wb_dat_e),
        .o_mm_wb_cyc(o_mm_wb_cyc_e),
        .o_mm_wb_stb(o_mm_wb_stb_e),
        .i_mm_wb_ack(i_mm_wb_ack_e),
        .i_mm_wb_err(i_mm_wb_err_e),

        .o_cfg_wb_adr(o_cfg_wb_adr_e),
        .o_cfg_wb_sel(o_cfg_wb_sel_e),
        .o_cfg_wb_we (o_cfg_wb_we_e),
        .i_cfg_wb_dat(i_cfg_wb_dat_e),
        .o_cfg_wb_dat(o_cfg_wb_dat_e),
        .o_cfg_wb_cyc(o_cfg_wb_cyc_e),
        .o_cfg_wb_stb(o_cfg_wb_stb_e),
        .i_cfg_wb_ack(i_cfg_wb_ack_e),
        .i_cfg_wb_err(i_cfg_wb_err_e)

    );

dec #(.WB_DWIDTH(WB_DWIDTH), .WB_SWIDTH(WB_SWIDTH), .WB_SLAVES(WB_SLAVES)) DEC_INST0(
        .reset(reset),
        .clk(clk),
        .dec_s(dec_i),
        .dec_s_clk(dec_i_clk),
        .dec_s_fs(dec_i_fs),
        .scan_in0(),
        .scan_in1(),
        .scan_in2(),
        .scan_in3(),
        .scan_in4(),
        .scan_enable(scan_enable),
        .test_mode(test_mode),
        .scan_out0(),
        .scan_out1(),
        .scan_out2(),
        .scan_out3(),
        .scan_out4(),
        .i_uart0_tx(i_uart0_tx_d),
        .i_uart0_rts(i_uart0_rts_d),
        .o_uart0_cts(o_uart0_cts_d),
        .o_uart0_rx(o_uart0_rx_d),
        .dec_o(dec_s),
        .dec_o_clk(dec_s_clk),
        .dec_o_fs(dec_s_fs),

        .o_mm_wb_adr(o_mm_wb_adr_d),
        .o_mm_wb_sel(o_mm_wb_sel_d),
        .o_mm_wb_we (o_mm_wb_we_d),
        .i_mm_wb_dat(i_mm_wb_dat_d),
        .o_mm_wb_dat(o_mm_wb_dat_d),
        .o_mm_wb_cyc(o_mm_wb_cyc_d),
        .o_mm_wb_stb(o_mm_wb_stb_d),
        .i_mm_wb_ack(i_mm_wb_ack_d),
        .i_mm_wb_err(i_mm_wb_err_d),

        .o_cfg_wb_adr(o_cfg_wb_adr_d),
        .o_cfg_wb_sel(o_cfg_wb_sel_d),
        .o_cfg_wb_we (o_cfg_wb_we_d),
        .i_cfg_wb_dat(i_cfg_wb_dat_d),
        .o_cfg_wb_dat(o_cfg_wb_dat_d),
        .o_cfg_wb_cyc(o_cfg_wb_cyc_d),
        .o_cfg_wb_stb(o_cfg_wb_stb_d),
        .i_cfg_wb_ack(i_cfg_wb_ack_d),
        .i_cfg_wb_err(i_cfg_wb_err_d)
    );

endmodule // mcac
