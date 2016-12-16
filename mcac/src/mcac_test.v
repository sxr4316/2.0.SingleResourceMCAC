
module test;

reg [31:0]              clk_count = 'd0;
reg             testfail;

wire  scan_out0, scan_out1, scan_out2, scan_out3, scan_out4;

reg  clk, reset;
reg  scan_in0, scan_in1, scan_in2, scan_in3, scan_in4, scan_enable, test_mode;

reg rs, ws, cs, enc_s, enc_s_clk, enc_s_fs, dec_i, dec_i_clk, dec_i_fs;
reg [7:0] w_data;
reg [6:0] addrs;
reg i_uart0_tx_e, i_uart0_rts_e;
wire o_uart0_rx_e, o_uart0_cts_e;
reg i_uart0_tx_d, i_uart0_rts_d;
wire o_uart0_rx_d, o_uart0_cts_d;
reg [127:0] i_mm_wb_dat_e;
reg i_mm_wb_ack_e, i_mm_wb_err_e;
wire [31:0] o_mm_wb_adr_e;
wire [15:0] o_mm_wb_sel_e;
wire [127:0] o_mm_wb_dat_e;
wire o_mm_wb_we_e, o_mm_wb_cyc_e, o_mm_wb_stb_e;
reg [127:0] i_mm_wb_dat_d;
reg i_mm_wb_ack_d, i_mm_wb_err_d;
wire [31:0] o_mm_wb_adr_d;
wire [15:0] o_mm_wb_sel_d;
wire [127:0] o_mm_wb_dat_d;
wire o_mm_wb_we_d, o_mm_wb_cyc_d, o_mm_wb_stb_d;
wire [7:0] r_data;
wire enc_i, enc_i_clk, enc_i_fs, dec_s, dec_s_clk, dec_s_fs;

mcac #(
	.WB_DWIDTH(128),
	.WB_SWIDTH(16),
	.WB_SLAVES(10)
)
top (
        .reset(reset),
        .clk(clk),
	.test_mode(test_mode),
	.scan_enable(scan_enable),
        .scan_in0(scan_in0),
        .scan_in1(scan_in1),
        .scan_in2(scan_in2),
        .scan_in3(scan_in3),
        .scan_in4(scan_in4),
        .scan_out0(scan_out0),
        .scan_out1(scan_out1),
        .scan_out2(scan_out2),
        .scan_out3(scan_out3),
        .scan_out4(scan_out4),
	.enc_s(enc_s),
	.enc_s_clk(enc_s_clk),
	.enc_s_fs(enc_s_fs),
	.enc_i(enc_i),
	.enc_i_clk(enc_i_clk),
	.enc_i_fs(enc_i_fs),
	.dec_i(dec_i),
	.dec_i_clk(dec_i_clk),
	.dec_i_fs(dec_i_fs),
	.dec_s(dec_s),
	.dec_s_clk(dec_s_clk),
	.dec_s_fs(dec_s_fs),
	.rs(rs),
	.ws(ws),
	.cs(cs),
	.r_data(r_data),
	.w_data(w_data),
	.addrs(addrs),
	.i_uart0_tx_e(i_uart0_tx_e),
	.i_uart0_rts_e(i_uart0_rts_e),
	.o_uart0_rx_e(o_uart0_rx_e),
	.o_uart0_cts_e(o_uart0_cts_e),
	.i_uart0_tx_d(i_uart0_tx_d),
	.i_uart0_rts_d(i_uart0_rts_d),
	.o_uart0_rx_d(o_uart0_rx_d),
	.o_uart0_cts_d(o_uart0_cts_d),
	.o_mm_wb_adr_e(o_mm_wb_adr_e),
	.o_mm_wb_sel_e(o_mm_wb_sel_e),
	.o_mm_wb_we_e(o_mm_wb_we_e),
	.i_mm_wb_dat_e(i_mm_wb_dat_e),
	.o_mm_wb_dat_e(o_mm_wb_dat_e),
	.o_mm_wb_cyc_e(o_mm_wb_cyc_e),
	.o_mm_wb_stb_e(o_mm_wb_stb_e),
	.i_mm_wb_ack_e(i_mm_wb_ack_e),
	.i_mm_wb_err_e(i_mm_wb_err_e),
	.o_mm_wb_adr_d(o_mm_wb_adr_d),
	.o_mm_wb_sel_d(o_mm_wb_sel_d),
	.o_mm_wb_we_d(o_mm_wb_we_d),
	.i_mm_wb_dat_d(i_mm_wb_dat_d),
	.o_mm_wb_dat_d(o_mm_wb_dat_d),
	.o_mm_wb_cyc_d(o_mm_wb_cyc_d),
	.o_mm_wb_stb_d(o_mm_wb_stb_d),
	.i_mm_wb_ack_d(i_mm_wb_ack_d),
	.i_mm_wb_err_d(i_mm_wb_err_d)
    );


initial
begin
    $timeformat(-9,2,"ns", 16);
`ifdef SDFSCAN
    $sdf_annotate("sdf/mcac_tsmc18_scan.sdf", test.top);
`endif
    clk = 1'b0;
    reset = 1'b0;
    scan_in0 = 1'b0;
    scan_in1 = 1'b0;
    scan_in2 = 1'b0;
    scan_in3 = 1'b0;
    scan_in4 = 1'b0;
    scan_enable = 1'b0;
    test_mode = 1'b0;
    $finish;
end

endmodule
