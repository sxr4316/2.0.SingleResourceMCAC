`include "../include/global_timescale.vh"
`include "../include/system_config_defines.vh"
`include "../include/global_defines.vh"

`define TB_LOAD_MEM
`define RESET_MEM_TICKS 2
`define RESET_TICKS 2+`RESET_MEM_TICKS
`define SER_CLK_PERIOD 488.281
`define CLK_PERIOD (1000/`AMBER_CLK_FREQ)
`define AMBER_LOAD_MAIN_MEM

`define AMBER_SIM_CTRL 4

//`define SINGLECHANNEL
`define ERRORCHECK
`define STANDALONE
//`define TIE_IN

module test();

`include "../include/debug_functions.vh"
`include "../include/system_functions.vh"
`include "../include/memory_configuration.vh"

reg                     sysrst;
reg [31:0]              clk_count = 'd0;

integer                 log_file;

`ifdef AMBER_LOAD_MAIN_MEM
integer                 main_mem_file;
reg     [31:0]          main_mem_file_address;
reg     [31:0]          main_mem_file_data;
reg     [127:0]         main_mem_file_data_128;
integer                 main_mem_line_count;
reg     [22:0]          mm_ddr3_addr;
`endif

integer                 boot_mem_file;
reg     [31:0]          boot_mem_file_address;
reg     [31:0]          boot_mem_file_data;
reg     [127:0]         boot_mem_file_data_128;
integer                 boot_mem_line_count;
integer                 fgets_return;
reg     [120*8-1:0]     line;
reg     [120*8-1:0]     aligned_line;
integer                 timeout = 0;

// signals for the test uart
wire                    o_uart0_cts_e;
wire                    o_uart0_rx_e;
wire                    i_uart0_rts_e;
wire                    i_uart0_tx_e;
wire                    o_uart0_cts_d;
wire                    o_uart0_rx_d;
wire                    i_uart0_rts_d;
wire                    i_uart0_tx_d;

reg  [7*8-1:0]          core_str;

wire  scan_out0, scan_out1, scan_out2, scan_out3, scan_out4;
reg  clk, uart_clk, reset;
reg  scan_in0, scan_in1, scan_in2, scan_in3, scan_in4, scan_enable, test_mode;

reg enc_s, enc_s_clk, enc_s_fs;
reg dec_i, dec_i_fs;
wire dec_in, dec_fs_in;
wire dec_s, dec_i_clk, dec_s_clk, dec_s_fs;
wire enc_i, enc_i_clk, enc_i_fs;

wire	[31:0]	o_mm_wb_adr_e;
wire	[15:0]	o_mm_wb_sel_e;
wire			o_mm_wb_we_e;       
wire	[127:0]	i_mm_wb_dat_e;
wire	[127:0]	o_mm_wb_dat_e;
wire			o_mm_wb_cyc_e;
wire			o_mm_wb_stb_e;
wire			i_mm_wb_ack_e;
wire			i_mm_wb_err_e;

wire	[31:0]	o_mm_wb_adr_d;
wire	[15:0]	o_mm_wb_sel_d;
wire			o_mm_wb_we_d;       
wire	[127:0]	i_mm_wb_dat_d;
wire	[127:0]	o_mm_wb_dat_d;
wire			o_mm_wb_cyc_d;
wire			o_mm_wb_stb_d;
wire			i_mm_wb_ack_d;
wire			i_mm_wb_err_d;

reg rs, ws, cs;
reg [7:0] w_data;
reg [6:0] addrs;
wire [7:0] r_data;

// parameterize the paths
reg [2*8-1:0]  rates [0:3];      // 4 rates (16kbps, 24kbps, 32kbps, 40kbps)
reg [4*8-1:0]  laws [0:3];       // 2 laws (a-law, u-law)
reg [6*8-1:0]  operations [0:3]; // 3 operations (normal or overload, or I for decoder)
reg [12*8-1:0] types [0:1];      // (reset, homing)
reg [6*8-1:0]  models [0:1];     // dec or enc
integer        rate, law, operation, type, model, j;
reg program_start;
integer delay_start;

reg [4:0] ch_count_e, tdmo_ch_count_e;
reg [2:0] bit_count_e, tdmo_bit_count_e;
reg [4:0] ch_count_d, tdmo_ch_count_d;
reg [2:0] bit_count_d, tdmo_bit_count_d;

reg [23:0] data_in_vec_e  [0:636160];
reg [23:0] data_out_vec_e [0:636160];
reg [23:0] vectorSizes_e  [0:19384];
reg [23:0] testCases    [0:19879];

reg [23:0] data_in_vec_d  [0:636160];
reg [23:0] data_out_vec_d [0:636160];
reg [23:0] vectorSizes_d  [0:19384];

integer test_channel_e;
reg [23:0] channels_e [0:19879];
integer test_channel_d;
reg [23:0] channels_d [0:19879];

reg [7:0] data_out_e [31:0];
reg [7:0] data_out_d [31:0];
integer next_channel;

integer i, test_count_e, tdmo_test_count_e, test_count_d, tdmo_test_count_d, loop_e, loop_d;

localparam integer UART_BIT_PERIOD = (1000000000 / `AMBER_UART_BAUD);
localparam integer SYS_CLK_PER = `CLK_PERIOD;
localparam integer CLKS_PER_BIT =   (UART_BIT_PERIOD / SYS_CLK_PER);
`define UART_CLK_PERIOD (CLKS_PER_BIT*`CLK_PERIOD)

reg config_done = 1'b0;

mcac
`ifndef NETLIST
#(
	.WB_DWIDTH(128),
	.WB_SWIDTH(16),
	.WB_SLAVES(10)
)
`endif
top (
	.reset(reset),
	.clk(clk),
	.scan_in0(scan_in0),
	.scan_in1(scan_in1),
	.scan_in2(scan_in2),
	.scan_in3(scan_in3),
	.scan_in4(scan_in4),
	.scan_enable(scan_enable),
	.test_mode(test_mode),
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

	.dec_i(dec_in),
	.dec_i_clk(dec_i_clk),
	.dec_i_fs(dec_fs_in),
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

// ======================================
// Instantiate Testbench Encoder UART
// ======================================
tb_uart tb_uart_e (
    .clk            ( uart_clk  ),
    .reset          ( reset     ),
    .scan_enable    (scan_enable),
    .test_mode      ( test_mode ),
    .i_uart_cts_n   ( o_uart0_cts_e ),          // Clear To Send
    .i_uart_rxd     ( o_uart0_rx_e  ),
    .o_uart_rts_n   ( i_uart0_rts_e ),          // Request to Send
    .o_uart_txd     ( i_uart0_tx_e  )
);

// ======================================
// Instantiate Testbench Decoder UART
// ======================================
tb_uart tb_uart_d (
    .clk            ( uart_clk  ),
    .reset          ( reset     ),
    .scan_enable    (scan_enable),
    .test_mode      ( test_mode ),
    .i_uart_cts_n   ( o_uart0_cts_d ),          // Clear To Send
    .i_uart_rxd     ( o_uart0_rx_d  ),
    .o_uart_rts_n   ( i_uart0_rts_d ),          // Request to Send
    .o_uart_txd     ( i_uart0_tx_d  )
);

// ======================================
// Instantiate Encoder Main Memory
// ======================================
main_mem #(
	.WB_DWIDTH(128),
	.WB_SWIDTH(16)
)
main_mem_e (
	.clk(clk),
	.reset(reset),
	.scan_enable(scan_enable),
	.test_mode(test_mode),
	.i_mem_ctrl(1'b0),  // 0=128MB, 1=32MB
	.i_wb_adr(o_mm_wb_adr_e),
	.i_wb_sel(o_mm_wb_sel_e),
	.i_wb_we(o_mm_wb_we_e),
	.o_wb_dat(i_mm_wb_dat_e),
	.i_wb_dat(o_mm_wb_dat_e),
	.i_wb_cyc(o_mm_wb_cyc_e),
	.i_wb_stb(o_mm_wb_stb_e),
	.o_wb_ack(i_mm_wb_ack_e),
	.o_wb_err(i_mm_wb_err_e)
);

// ======================================
// Instantiate Decoder Main Memory
// ======================================
main_mem #(
	.WB_DWIDTH(128),
	.WB_SWIDTH(16)
)
main_mem_d (
	.clk(clk),
	.reset(reset),
	.scan_enable(scan_enable),
	.test_mode(test_mode),
	.i_mem_ctrl(1'b0),  // 0=128MB, 1=32MB
	.i_wb_adr(o_mm_wb_adr_d),
	.i_wb_sel(o_mm_wb_sel_d),
	.i_wb_we(o_mm_wb_we_d),
	.o_wb_dat(i_mm_wb_dat_d),
	.i_wb_dat(o_mm_wb_dat_d),
	.i_wb_cyc(o_mm_wb_cyc_d),
	.i_wb_stb(o_mm_wb_stb_d),
	.o_wb_ack(i_mm_wb_ack_d),
	.o_wb_err(i_mm_wb_err_d)
);

always #(`CLK_PERIOD/2) clk = ~clk;

always #(`UART_CLK_PERIOD/2) uart_clk = ~uart_clk;

// initialize path parameters
initial begin
  rates[0]      = "40";
  rates[1]      = "32";
  rates[2]      = "24";
  rates[3]      = "16";
  laws[0]       = "AA";
  laws[1]       = "MM";  // m for u-Law
  laws[2]       = "AM";
  laws[3]       = "MA";
  operations[0] = "nrm";
  operations[1] = "ovr";
  operations[2] = "i";
  types[0]      = "reset";
  types[1]      = "homing";
  models[0]     = "enc";
  models[1]     = "dec";
end

initial begin
    $timeformat(-9,2,"ns", 16);
`ifdef SDFSCAN
    $sdf_annotate("sdf/mcac_tsmc065_scan.sdf", test.top);
`endif
	clk = 1'b0;
	enc_s = 1'b0;
	dec_i = 1'b0;
	enc_s_fs = 1'b0;
	dec_i_fs = 1'b0;
	uart_clk = 1'b0;
	scan_in0 = 1'b0;
	scan_in1 = 1'b0;
	scan_in2 = 1'b0;
	scan_in3 = 1'b0;
	scan_in4 = 1'b0;
	scan_enable = 1'b0;
	test_mode = 1'b0;

	reset = 1'd0;
	@(posedge clk)
		reset = #1 1'd1;
	repeat(`RESET_MEM_TICKS)
		@(negedge clk);
	reset = #1 1'd0;

	addrs = 7'h00;
	cs = 1'b1;
	rs = 1'b0;
	ws = 1'b0;
	w_data = 8'h00;

`ifdef STANDALONE
	model = 0;//testCases[0];
	type = 0;//testCases[1];
	operation= 0;//testCases[2];
	law = 2;//testCases[3];
	rate = 0;//testCases[4];
`else
	// get the test case from the file
	$readmemh({"./MCACTests.t"},testCases);
	model = testCases[0];
	type = testCases[1];
	operation = testCases[2];
	law = testCases[3];
	rate = testCases[4];
`endif
	$display("MCAC %s %s %s %s law", rates[rate], operations[operation], types[type], laws[law]);

	// load vectors for the specific case
	$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[0], "/", types[type],"/", operations[operation],"/",rates[rate],"/","vectorSize.t"}, vectorSizes_e);
	loop_e = vectorSizes_e[0];
	$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[1], "/", types[type],"/", operations[operation],"/",rates[rate],"/","vectorSize.t"}, vectorSizes_d);
	loop_d = vectorSizes_d[0];
`ifndef SINGLECHANNEL
	loop_e = loop_e / 32;
	loop_d = loop_d / 32;
`endif
	$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[0], "/", types[type],"/", operations[operation],"/",rates[rate],"/","channel.t"}, channels_e);
	test_channel_e = channels_e[0];
	$display("Current channel being tested with main vectors is: %d", test_channel_e);
	$display("Other channels being tested with null vectors");
	$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[0], "/", types[type],"/", operations[operation],"/",rates[rate],"/","input.t"}, data_in_vec_e);
	$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[0], "/", types[type],"/", operations[operation],"/",rates[rate],"/","output.t"}, data_out_vec_e);

	$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[1], "/", types[type],"/", operations[operation],"/",rates[rate],"/","channel.t"}, channels_d);
	test_channel_d = channels_d[0];
	$display("Current channel being tested with main vectors is: %d", test_channel_d);
	$display("Other channels being tested with null vectors");           
	$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[1], "/", types[type],"/", operations[operation],"/",rates[rate],"/","input.t"}, data_in_vec_d);
	$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[1], "/", types[type],"/", operations[operation],"/",rates[rate],"/","output.t"}, data_out_vec_d);

end

wire tie_in;

`ifdef TIE_IN
	assign tie_in = 1'b1;
`else
	assign tie_in = 1'b0;
`endif

assign dec_i_clk = (tie_in) ? (enc_i_clk) : (enc_s_clk);
assign dec_in = (tie_in) ? (enc_i) : (dec_i);
assign dec_fs_in = (tie_in) ? (enc_i_fs) : (dec_i_fs);

reg [7:0] ser_count;
always @ (posedge clk, posedge reset) begin
	if(reset) begin
		enc_s_clk <= 1'b0;
		ser_count <= 8'h00;
	end else if (ser_count != 8'h3B) begin
		ser_count <= ser_count + 1'b1;
		enc_s_clk <= enc_s_clk;
	end else if (ser_count == 8'h3B) begin
		ser_count <= 8'h00;
		enc_s_clk <= ~ enc_s_clk;
	end
end

reg d_law = 1'b0;
reg e_law = 1'b0;
always @(rate, law) begin
	case (rates[rate])
		"40": w_data[2:1] = 2'b00;
		"32": w_data[2:1] = 2'b01;
		"24": w_data[2:1] = 2'b10;
		"16": w_data[2:1] = 2'b11;
	endcase
	case (laws[law])
		"AA": begin
			e_law = 1'b1;
			d_law = 1'b1;
		end
		"MM": begin
			e_law = 1'b0;
			d_law = 1'b0;
		end
		"AM": begin
			e_law = 1'b1;
			d_law = 1'b0;
		end
		"MA": begin
			e_law = 1'b0;
			d_law = 1'b1;
		end
	endcase
	addrs[5] = 1'b0;
	for (next_channel = 0; next_channel<32; next_channel=next_channel+1) begin
		w_data[0] = d_law;
		addrs[4:0] = next_channel[4:0];
		ws = 1'b1;
		@(posedge clk);
		ws = 1'b0;
		@(posedge clk);
	end
	addrs[5] = 1'b1;
	for (next_channel = 0; next_channel<32; next_channel=next_channel+1) begin
		w_data[0] = e_law;
		addrs[4:0] = next_channel[4:0];
		ws = 1'b1;
		@(posedge clk);
		ws = 1'b0;
		@(posedge clk);
	end
	config_done = 1'b1;
end


reg e_mail, d_mail;
always @(posedge reset, posedge clk) begin
	if (reset) begin
		program_start <= 1'b0;
		e_mail <= 1'b0;
		d_mail <= 1'b0;
	end else begin
		if(config_done == 1 && program_start == 0) begin
			addrs = 7'b1000000;
			@(posedge clk);
			rs = 1'b1;
			@(posedge clk);
			rs = 1'b0;
			if (r_data == 1) begin
				d_mail = 1'b1;
			end
			addrs = 7'b1100000;
			@(posedge clk);
			rs = 1'b1;
			@(posedge clk);
			rs = 1'b0;
			if (r_data == 1) begin
				e_mail = 1'b1;
			end
			if (e_mail & d_mail) begin
				program_start <= 1'b1;
			end
		end
	end
end

// ======================================
// Counter of system clock ticks        
// ======================================
always @ ( posedge `U_SYSTEM.clk )
    clk_count <= clk_count + 1'd1;

// ======================================
// Encoder feed into TDMI     
// ======================================
always @(posedge reset, posedge enc_s_clk) begin
	if (reset | ~program_start) begin
		ch_count_e <= 'd0;
		test_count_e <= 'd0;
		bit_count_e <= 'd7;
	end else begin
		if (test_count_e < loop_e)  begin
			if (ch_count_e == 5'd0) begin
				enc_s_fs <= 1'b1;
			end else begin
				enc_s_fs <= 1'b0;
			end
	`ifdef SINGLECHANNEL
			if (ch_count_e == 0) begin
				enc_s <= data_in_vec_e[test_count_e][bit_count_e];
			end else begin
				enc_s <= 1'b0;
			end
	`else
			enc_s <= data_in_vec_e[32*test_count_e + ch_count_e][bit_count_e];
	`endif
			bit_count_e <= bit_count_e - 1'b1;
			if (bit_count_e == 3'd0) begin
				ch_count_e <= ch_count_e + 1'b1;
				if (ch_count_e == 5'b11111) begin
					test_count_e <= test_count_e + 1'b1;
				end
			end
		end
	end
end

`ifndef TIE_IN
// ======================================
// Decoder feed into TDMI     
// ======================================
always @(posedge reset, posedge dec_i_clk) begin
	if (reset | ~program_start) begin
		ch_count_d <= 'd0;
		test_count_d <= 'd0;
		bit_count_d <= 'd7;
	end else begin
		if (test_count_d < loop_d)  begin
			if (ch_count_d == 5'd0) begin
				dec_i_fs <= 1'b1;
			end else begin
				dec_i_fs <= 1'b0;
			end
	`ifdef SINGLECHANNEL
			if (ch_count_d == 0) begin
				dec_i <= data_in_vec_d[test_count_d][bit_count_d];
			end else begin
				dec_i <= 1'b0;
			end
	`else
			dec_i <= data_in_vec_d[32*test_count_d + ch_count_d][bit_count_d];
	`endif
			bit_count_d <= bit_count_d - 1'b1;
			if (bit_count_d == 3'd0) begin
				ch_count_d <= ch_count_d + 1'b1;
				if (ch_count_d == 5'b11111) begin
					test_count_d <= test_count_d + 1'b1;
				end
			end
		end
	end
end
`endif

// ======================================
// Capture data from Encoder TDMO
// ======================================
reg tdmo_fs_prev_e;
reg captured_data_e;
always @(posedge reset, negedge enc_i_clk) begin
	if (reset | ~program_start) begin
		tdmo_fs_prev_e <= 1'b0;
		captured_data_e <= 1'b0;
		tdmo_test_count_e <= 'd0;
		tdmo_ch_count_e <= 'd0;
		tdmo_bit_count_e <= 'd7;
	end else begin
		if (tdmo_test_count_e <= loop_e) begin
			if (enc_i_fs & ~tdmo_fs_prev_e) begin
				data_out_e[0][7] <= enc_i;
				tdmo_bit_count_e <= 3'b110;
				tdmo_ch_count_e <= 5'b00000;
			end else begin
				data_out_e[tdmo_ch_count_e][tdmo_bit_count_e] <= enc_i;
				tdmo_bit_count_e <= tdmo_bit_count_e - 1'b1;
				if (tdmo_test_count_e == 'd0 && tdmo_ch_count_e == 5'b11111 && tdmo_bit_count_e == 3'b001) begin
					tdmo_bit_count_e <= 3'd7;
					tdmo_test_count_e <= tdmo_test_count_e + 1'b1;
					tdmo_ch_count_e <= 5'b00000;
				end else if (tdmo_bit_count_e == 3'd0) begin
						captured_data_e <= 1'b1;
						tdmo_bit_count_e <= 3'd7;
						tdmo_ch_count_e <= tdmo_ch_count_e + 1'b1;
						if (tdmo_ch_count_e == 5'b11111) begin
							tdmo_test_count_e <= tdmo_test_count_e + 1'b1;
							tdmo_ch_count_e <= 5'b00000;
						end
				end else begin
`ifdef ERRORCHECK
					if (captured_data_e && (tdmo_bit_count_e == 3'd7)) begin
`ifdef SINGLECHANNEL
					if ((tdmo_ch_count_e == 1) && (tdmo_test_count_e >= 1)) begin
						if (data_out_e[0] != data_out_vec_e[tdmo_test_count_e-1][7:0]) begin
							$display("%t TESTBENCHERROR: failure found for channel 0 and test number %d, when output data = %h but should be %h",$time,tdmo_test_count_e,data_out_e[0],data_out_vec_e[tdmo_test_count_e-1][7:0]); 
							$finish;
						end
					end
`else
					if (tdmo_test_count_e >= 1) begin
						if (tdmo_ch_count_e == 0) begin
							if (data_out_e[31] != data_out_vec_e[32*tdmo_test_count_e-33][7:0]) begin
								$display("%t TESTBENCHERROR: failure found for channel %h and test number %d, when output data = %h but should be %h",$time,tdmo_ch_count_e,tdmo_test_count_e,data_out_e[31],data_out_vec_e[32*tdmo_test_count_e-33][7:0]); 
								$finish;
							end
						end else begin
							if (data_out_e[tdmo_ch_count_e-1] != data_out_vec_e[32*tdmo_test_count_e+tdmo_ch_count_e-33][7:0]) begin
								$display		("%t TESTBENCHERROR: failure found for channel %h and test number %d, when output data = %h but should be %h",$time,tdmo_ch_count_e,tdmo_test_count_e,data_out_e[tdmo_ch_count_e-1],data_out_vec_e[32*tdmo_test_count_e+tdmo_ch_count_e-33][7:0]); 
								$finish;
							end
							else if ((tdmo_test_count_e % 100)==0 && tdmo_ch_count_e == 1) begin // print passing once in a while
								$display("%t Passing tests so far. Currently on test number %d", $time, tdmo_test_count_e);
							end
						end
					end
`endif
					end
`endif
					captured_data_e <= 1'b0;
				end
			end
			tdmo_fs_prev_e <= enc_i_fs;
		end else begin
			$display("%t TESTBENCHFINISHED: simulation complete",$time);
			$finish;
		end
	end
end

// ======================================
// Capture data from Decoder TDMO
// ======================================
reg tdmo_fs_prev_d;
reg captured_data_d;
always @(posedge reset, negedge dec_s_clk) begin
	if (reset | ~program_start) begin
		tdmo_fs_prev_d <= 1'b0;
		captured_data_d <= 1'b0;
		tdmo_test_count_d <= 'd0;
		tdmo_ch_count_d <= 'd0;
		tdmo_bit_count_d <= 'd7;
	end else begin
		if (tdmo_test_count_d <= loop_d) begin
			if (dec_s_fs & ~tdmo_fs_prev_d) begin
				data_out_d[0][7] <= dec_s;
				tdmo_bit_count_d <= 3'b110;
				tdmo_ch_count_d <= 5'b00000;
			end else begin
				data_out_d[tdmo_ch_count_d][tdmo_bit_count_d] <= dec_s;
				tdmo_bit_count_d <= tdmo_bit_count_d - 1'b1;
				if (tdmo_test_count_d == 'd0 && tdmo_ch_count_d == 5'b11111 && tdmo_bit_count_d == 3'b001) begin
					tdmo_bit_count_d <= 3'd7;
					tdmo_test_count_d <= tdmo_test_count_d + 1'b1;
					tdmo_ch_count_d <= 5'b00000;
				end else if (tdmo_bit_count_d == 3'd0) begin
						captured_data_d <= 1'b1;
						tdmo_bit_count_d <= 3'd7;
						tdmo_ch_count_d <= tdmo_ch_count_d + 1'b1;
						if (tdmo_ch_count_d == 5'b11111) begin
							tdmo_test_count_d <= tdmo_test_count_d + 1'b1;
							tdmo_ch_count_d <= 5'b00000;
						end
				end else begin
`ifdef ERRORCHECK
					if (captured_data_d && (tdmo_bit_count_d == 3'd7)) begin
`ifdef SINGLECHANNEL
					if ((tdmo_ch_count_d == 1) && (tdmo_test_count_d >= 2)) begin
						if (data_out_d[0] != data_out_vec_d[tdmo_test_count_d-2][7:0]) begin
							$display("%t TESTBENCHERROR: failure found for channel 0 and test number %d, when output data = %h but should be %h",$time,tdmo_test_count_d,data_out_d[0],data_out_vec_d[tdmo_test_count_d-1][7:0]); 
							$finish;
						end
					end
`else
					if (tdmo_test_count_d >= 2) begin
						if (tdmo_ch_count_d == 0) begin
							if (data_out_d[31] != data_out_vec_d[32*tdmo_test_count_d-65][7:0]) begin
								$display("%t TESTBENCHERROR: failure found for channel %h and test number %d, when output data = %h but should be %h",$time,tdmo_ch_count_d,tdmo_test_count_d,data_out_d[31],data_out_vec_d[32*tdmo_test_count_d-33][7:0]); 
								$finish;
							end
						end else begin
							if (data_out_d[tdmo_ch_count_d-1] != data_out_vec_d[32*tdmo_test_count_d+tdmo_ch_count_d-65][7:0]) begin
								$display		("%t TESTBENCHERROR: failure found for channel %h and test number %d, when output data = %h but should be %h",$time,tdmo_ch_count_d,tdmo_test_count_d,data_out_d[tdmo_ch_count_d-1],data_out_vec_d[32*tdmo_test_count_d+tdmo_ch_count_d-33][7:0]); 
								$finish;
							end
							else if ((tdmo_test_count_d % 100)==0 && tdmo_ch_count_d == 1) begin // print passing once in a while
								$display("%t Passing tests so far. Currently on test number %d", $time, tdmo_test_count_d);
							end
						end
					end
`endif
					end
`endif
					captured_data_d <= 1'b0;
				end
			end
			tdmo_fs_prev_d <= dec_s_fs;
		end else begin
			$display("%t TESTBENCHFINISHED: simulation complete",$time);
			$finish;
		end
	end
end

// ======================================
// Initialize Boot Memory
// ======================================
always @ ( posedge `U_SYSTEM.reset ) begin
		repeat(`RESET_MEM_TICKS)
			@ ( negedge `U_SYSTEM.clk );
        $display("Load boot memory from %s", `BOOT_MEM_FILE);
        boot_mem_line_count   = 0;
        boot_mem_file         = $fopen(`BOOT_MEM_FILE,    "r");
        if (boot_mem_file == 0)
            begin
		`ifndef NETLIST
            	`TB_ERROR_MESSAGE
		`endif
            $display("TESTBENCHERROR: Can't open input file %s", `BOOT_MEM_FILE);
            $finish;
            end
        
        if (boot_mem_file != 0)
            begin  
            fgets_return = 1;
            while (fgets_return != 0)
                begin
                fgets_return        = $fgets(line, boot_mem_file);
                boot_mem_line_count = boot_mem_line_count + 1;
                aligned_line        = align_line(line);
                
                // if the line does not start with a comment
                if (aligned_line[120*8-1:118*8] != 16'h2f2f)
                    begin
                    // check that line doesnt start with a '@' or a blank
                    if (aligned_line[120*8-1:119*8] != 8'h40 && aligned_line[120*8-1:119*8] != 8'h00)
                        begin
                        $display("Format ERROR in input file %s, line %1d. Line must start with a @, not %08x", 
                                 `BOOT_MEM_FILE, boot_mem_line_count, aligned_line[118*8-1:117*8]);
                        `ifndef NETLIST
            			`TB_ERROR_MESSAGE
				`endif
                        end
                    
                    if (aligned_line[120*8-1:119*8] != 8'h00)
                        begin
                        boot_mem_file_address  =   hex_chars_to_32bits (aligned_line[119*8-1:111*8]);
                        boot_mem_file_data     =   hex_chars_to_32bits (aligned_line[110*8-1:102*8]);
                        
                        boot_mem_file_data_128 = top.ENC_INST0.boot_mem_inst0.u_mem.mem[boot_mem_file_address[BOOT_MSB:4]];
                        top.ENC_INST0.boot_mem_inst0.u_mem.mem[boot_mem_file_address[BOOT_MSB:4]] = 
                              insert_32_into_128 ( boot_mem_file_address[3:2], 
                                                   boot_mem_file_data_128, 
                                                   boot_mem_file_data );
                        top.DEC_INST0.boot_mem_inst0.u_mem.mem[boot_mem_file_address[BOOT_MSB:4]] = 
                              insert_32_into_128 ( boot_mem_file_address[3:2], 
                                                   boot_mem_file_data_128, 
                                                   boot_mem_file_data );
                        `ifdef AMBER_LOAD_MEM_DEBUG
                            $display ("Load Boot Mem: PAddr: 0x%08x, Data 0x%08x", 
                                        boot_mem_file_address, boot_mem_file_data);
                        `endif   
                        end
                    end  
                end
                
            $display("Read in %1d lines", boot_mem_line_count);      
            end

        // Grab the test name from memory    
        timeout   = `AMBER_TIMEOUT   ;  
        core_str = "amber25";      
        $display("Core %s, log file %s, timeout %0d, test name %0s ", core_str, `AMBER_LOG_FILE, timeout, `AMBER_TEST_NAME );          
        log_file = $fopen(`AMBER_LOG_FILE, "a");                               
end

// ======================================
// Initialize Main Memory
// ======================================
`ifdef AMBER_LOAD_MAIN_MEM
always @ ( posedge `U_SYSTEM.reset ) begin
	repeat(`RESET_MEM_TICKS)
		@ ( negedge `U_SYSTEM.clk );
        $display("Load enc main memory from %s", `ENC_MAIN_MEM_FILE);

        main_mem_file   = $fopen(`ENC_MAIN_MEM_FILE, "r");
            
        // Read RAM File
        main_mem_line_count   = 0;
        
        if (main_mem_file == 0)
            begin
            $display("ERROR: Can't open input file %s", `ENC_MAIN_MEM_FILE);
            `ifndef NETLIST
			`TB_ERROR_MESSAGE
		`endif
            end

        if (main_mem_file != 0)
            begin  
            fgets_return = 1;
            while (fgets_return != 0)
                begin
                fgets_return        = $fgets(line, main_mem_file);
                main_mem_line_count = main_mem_line_count + 1;
                aligned_line        = align_line(line);
                
                // if the line does not start with a comment
                if (aligned_line[120*8-1:118*8] != 16'h2f2f)
                    begin
                    // check that line doesnt start with a '@' or a blank
                    if (aligned_line[120*8-1:119*8] != 8'h40 && aligned_line[120*8-1:119*8] != 8'h00)
                        begin
                        $display("Format ERROR in input file %s, line %1d. Line must start with a @, not %08x", 
                                 `MAIN_MEM_FILE, main_mem_line_count, aligned_line[118*8-1:117*8]);
                        `ifndef NETLIST
            			`TB_ERROR_MESSAGE
				`endif
                        end
                    
                    if (aligned_line[120*8-1:119*8] != 8'h00)
                        begin
                        main_mem_file_address =   hex_chars_to_32bits (aligned_line[119*8-1:111*8]);
                        main_mem_file_data    =   hex_chars_to_32bits (aligned_line[110*8-1:102*8]);

                      // Fast simulation model of main memory
                      // U_RAM - Can either point to simple or Xilinx DDR3 model. 
                      // Set in hierarchy_defines.v
                      
                      main_mem_file_data_128 = test.main_mem_e.ram[main_mem_file_address[31:4]];
                      test.main_mem_e.ram [main_mem_file_address[31:4]] = 
                          insert_32_into_128 ( main_mem_file_address[3:2], 
                                               main_mem_file_data_128, 
                                               main_mem_file_data );

                      `ifdef AMBER_LOAD_MEM_DEBUG
                          $display ("Load RAM: PAddr: 0x%08x, Data 0x%08x", 
                                     main_mem_file_address, main_mem_file_data);
                      `endif   
                        
                        end
                    end  
                end
                
            $display("Read in %1d lines", main_mem_line_count);      
            end

	  $display("Load dec main memory from %s", `DEC_MAIN_MEM_FILE);

        main_mem_file   = $fopen(`DEC_MAIN_MEM_FILE, "r");
            
        // Read RAM File
        main_mem_line_count   = 0;
        
        if (main_mem_file == 0)
            begin
            $display("ERROR: Can't open input file %s", `DEC_MAIN_MEM_FILE);
            `ifndef NETLIST
			`TB_ERROR_MESSAGE
		`endif
            end

        if (main_mem_file != 0)
            begin  
            fgets_return = 1;
            while (fgets_return != 0)
                begin
                fgets_return        = $fgets(line, main_mem_file);
                main_mem_line_count = main_mem_line_count + 1;
                aligned_line        = align_line(line);
                
                // if the line does not start with a comment
                if (aligned_line[120*8-1:118*8] != 16'h2f2f)
                    begin
                    // check that line doesnt start with a '@' or a blank
                    if (aligned_line[120*8-1:119*8] != 8'h40 && aligned_line[120*8-1:119*8] != 8'h00)
                        begin
                        $display("Format ERROR in input file %s, line %1d. Line must start with a @, not %08x", 
                                 `DEC_MAIN_MEM_FILE, main_mem_line_count, aligned_line[118*8-1:117*8]);
                        `ifndef NETLIST
            			`TB_ERROR_MESSAGE
				`endif
                        end
                    
                    if (aligned_line[120*8-1:119*8] != 8'h00)
                        begin
                        main_mem_file_address =   hex_chars_to_32bits (aligned_line[119*8-1:111*8]);
                        main_mem_file_data    =   hex_chars_to_32bits (aligned_line[110*8-1:102*8]);

                      // Fast simulation model of main memory
                      // U_RAM - Can either point to simple or Xilinx DDR3 model. 
                      // Set in hierarchy_defines.v
                      
                      main_mem_file_data_128 = test.main_mem_d.ram[main_mem_file_address[31:4]];
                      test.main_mem_d.ram [main_mem_file_address[31:4]] = 
                          insert_32_into_128 ( main_mem_file_address[3:2], 
                                               main_mem_file_data_128, 
                                               main_mem_file_data );

                      `ifdef AMBER_LOAD_MEM_DEBUG
                          $display ("Load RAM: PAddr: 0x%08x, Data 0x%08x", 
                                     main_mem_file_address, main_mem_file_data);
                      `endif   
                        
                        end
                    end  
                end
                
            $display("Read in %1d lines", main_mem_line_count);      
            end
 end
`endif

dumpvcd u_dumpvcd();

// ======================================
// Terminate Test  
// ======================================
`include "../include/a25_localparams.vh"
`include "../include/a25_functions.vh"

reg             testfail;
wire            test_status_set;
wire [31:0]     test_status_reg;

initial
    begin
    testfail  = 1'd0;
    end

// ======================================
// Functions
// ======================================
function [127:0] insert_32_into_128;
input [1:0]   pos;
input [127:0] word128;
input [31:0]  word32;
begin
     case (pos)
         2'd0: insert_32_into_128 = {word128[127:32], word32};
         2'd1: insert_32_into_128 = {word128[127:64], word32, word128[31:0]};
         2'd2: insert_32_into_128 = {word128[127:96], word32, word128[63:0]};
         2'd3: insert_32_into_128 = {word32, word128[95:0]};
     endcase
end
endfunction



endmodule
