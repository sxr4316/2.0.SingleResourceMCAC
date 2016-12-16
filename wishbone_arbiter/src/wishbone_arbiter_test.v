/*
*
* Module: wishbone_arbiter testbench
*
* Author(s)		: David Malanga
*
* Description	: Tests the wishbone arbiter by changing the address and multiplexing
			: between the 10 slave devices. Random data is placed on the sub-address,
			: select, and data buses. The cyc, stb, we, err, and ack signals are toggled.
*
* Revision History:
*----------------------------------------------------------------------------------
* 3/8/16 - by Dave Malanga - Initial creation
*----------------------------------------------------------------------------------
*/


// Clock period definition - the arbiter is asynchronous, but the clk is used to add delay between setting and checking signals
`define CLK_PERIOD 12.207

`define ERRORSTOP

module test;

wire  scan_out0, scan_out1, scan_out2, scan_out3, scan_out4;

reg  clk, reset;
reg  scan_in0, scan_in1, scan_in2, scan_in3, scan_in4, scan_enable, test_mode;

parameter WB_DWIDTH = 32;
parameter WB_SWIDTH = 4;
parameter SLAVE_NUM = 10;

`include "../include/register_addresses.vh"
`include "../include/memory_configuration.vh"

reg	[31:0]		i_m0_wb_adr;
wire	[31:0]		o_s0_wb_adr,
				o_s1_wb_adr,
				o_s2_wb_adr,
				o_s3_wb_adr,
				o_s4_wb_adr,
				o_s5_wb_adr,
				o_s6_wb_adr,
				o_s7_wb_adr,
				o_s8_wb_adr,
				o_s9_wb_adr;

reg	[WB_SWIDTH-1:0]	i_m0_wb_sel;
wire	[WB_SWIDTH-1:0]	o_s0_wb_sel,
				o_s1_wb_sel,
				o_s2_wb_sel,
				o_s3_wb_sel,
				o_s4_wb_sel,
				o_s5_wb_sel,
				o_s6_wb_sel,
				o_s7_wb_sel,
				o_s8_wb_sel,
				o_s9_wb_sel;

reg				i_m0_wb_we;
wire				o_s0_wb_we,
				o_s1_wb_we,
				o_s2_wb_we,
				o_s3_wb_we,
				o_s4_wb_we,
				o_s5_wb_we,
				o_s6_wb_we,
				o_s7_wb_we,
				o_s8_wb_we,
				o_s9_wb_we;


reg	[WB_DWIDTH-1:0]	i_m0_wb_dat;
wire	[WB_DWIDTH-1:0]	o_s0_wb_dat,
				o_s1_wb_dat,
				o_s2_wb_dat,
				o_s3_wb_dat,
				o_s4_wb_dat,
				o_s5_wb_dat,
				o_s6_wb_dat,
				o_s7_wb_dat,
				o_s8_wb_dat,
				o_s9_wb_dat;

wire	[WB_DWIDTH-1:0]	o_m0_wb_dat;
reg	[WB_DWIDTH-1:0]	i_s0_wb_dat,
				i_s1_wb_dat,
				i_s2_wb_dat,
				i_s3_wb_dat,
				i_s4_wb_dat,
				i_s5_wb_dat,
				i_s6_wb_dat,
				i_s7_wb_dat,
				i_s8_wb_dat,
				i_s9_wb_dat;

reg				i_m0_wb_cyc;
wire				o_s0_wb_cyc,
				o_s1_wb_cyc,
				o_s2_wb_cyc,
				o_s3_wb_cyc,
				o_s4_wb_cyc,
				o_s5_wb_cyc,
				o_s6_wb_cyc,
				o_s7_wb_cyc,
				o_s8_wb_cyc,
				o_s9_wb_cyc;

reg				i_m0_wb_stb;
wire				o_s0_wb_stb,
				o_s1_wb_stb,
				o_s2_wb_stb,
				o_s3_wb_stb,
				o_s4_wb_stb,
				o_s5_wb_stb,
				o_s6_wb_stb,
				o_s7_wb_stb,
				o_s8_wb_stb,
				o_s9_wb_stb;

wire				o_m0_wb_ack;
reg				i_s0_wb_ack,
				i_s1_wb_ack,
				i_s2_wb_ack,
				i_s3_wb_ack,
				i_s4_wb_ack,
				i_s5_wb_ack,
				i_s6_wb_ack,
				i_s7_wb_ack,
				i_s8_wb_ack,
				i_s9_wb_ack;

wire				o_m0_wb_err;
reg				i_s0_wb_err,
				i_s1_wb_err,
				i_s2_wb_err,
				i_s3_wb_err,
				i_s4_wb_err,
				i_s5_wb_err,
				i_s6_wb_err,
				i_s7_wb_err,
				i_s8_wb_err,
				i_s9_wb_err;

integer i;
reg	[31:0]		adr;
reg	[WB_SWIDTH-1:0] 	sel;
reg				we;
reg				cyc;
reg				stb;
reg	[WB_DWIDTH-1:0]	dat;
reg	[WB_DWIDTH-1:0]	slave_data;

/* Does not synthesize with this inside
#(
	.WB_DWIDTH(WB_DWIDTH),
	.WB_SWIDTH(WB_SWIDTH)
)*/


wishbone_arbiter top (
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

	// WISHBONE master 0 - Amber
	.i_m0_wb_adr(i_m0_wb_adr),
	.i_m0_wb_sel(i_m0_wb_sel),
	.i_m0_wb_we (i_m0_wb_we ),
	.o_m0_wb_dat(o_m0_wb_dat),
	.i_m0_wb_dat(i_m0_wb_dat),
	.i_m0_wb_cyc(i_m0_wb_cyc),
	.i_m0_wb_stb(i_m0_wb_stb),
	.o_m0_wb_ack(o_m0_wb_ack),
	.o_m0_wb_err(o_m0_wb_err),

	// WISHBONE slave 0 - Boot Memory
	.o_s0_wb_adr(o_s0_wb_adr),
	.o_s0_wb_sel(o_s0_wb_sel),
	.o_s0_wb_we (o_s0_wb_we ),
	.i_s0_wb_dat(i_s0_wb_dat),
	.o_s0_wb_dat(o_s0_wb_dat),
	.o_s0_wb_cyc(o_s0_wb_cyc),
	.o_s0_wb_stb(o_s0_wb_stb),
	.i_s0_wb_ack(i_s0_wb_ack),
	.i_s0_wb_err(i_s0_wb_err),

	// WISHBONE slave 1 - Main Memory
	.o_s1_wb_adr(o_s1_wb_adr),
	.o_s1_wb_sel(o_s1_wb_sel),
	.o_s1_wb_we (o_s1_wb_we ),
	.i_s1_wb_dat(i_s1_wb_dat),
	.o_s1_wb_dat(o_s1_wb_dat),
	.o_s1_wb_cyc(o_s1_wb_cyc),
	.o_s1_wb_stb(o_s1_wb_stb),
	.i_s1_wb_ack(i_s1_wb_ack),
	.i_s1_wb_err(i_s1_wb_err),

	// WISHBONE slave 2 - APRSC
	.o_s2_wb_adr(o_s2_wb_adr),
	.o_s2_wb_sel(o_s2_wb_sel),
	.o_s2_wb_we (o_s2_wb_we ),
	.i_s2_wb_dat(i_s2_wb_dat),
	.o_s2_wb_dat(o_s2_wb_dat),
	.o_s2_wb_cyc(o_s2_wb_cyc),
	.o_s2_wb_stb(o_s2_wb_stb),
	.i_s2_wb_ack(i_s2_wb_ack),
	.i_s2_wb_err(i_s2_wb_err),

	// WISHBONE slave 3 - UART 0
	.o_s3_wb_adr(o_s3_wb_adr),
	.o_s3_wb_sel(o_s3_wb_sel),
	.o_s3_wb_we (o_s3_wb_we ),
	.i_s3_wb_dat(i_s3_wb_dat),
	.o_s3_wb_dat(o_s3_wb_dat),
	.o_s3_wb_cyc(o_s3_wb_cyc),
	.o_s3_wb_stb(o_s3_wb_stb),
	.i_s3_wb_ack(i_s3_wb_ack),
	.i_s3_wb_err(i_s3_wb_err),

	// WISHBONE slave 4 - TDMI
	.o_s4_wb_adr(o_s4_wb_adr),
	.o_s4_wb_sel(o_s4_wb_sel),
	.o_s4_wb_we (o_s4_wb_we ),
	.i_s4_wb_dat(i_s4_wb_dat),
	.o_s4_wb_dat(o_s4_wb_dat),
	.o_s4_wb_cyc(o_s4_wb_cyc),
	.o_s4_wb_stb(o_s4_wb_stb),
	.i_s4_wb_ack(i_s4_wb_ack),
	.i_s4_wb_err(i_s4_wb_err),

	// WISHBONE slave 5 - Test Module
	.o_s5_wb_adr(o_s5_wb_adr),
	.o_s5_wb_sel(o_s5_wb_sel),
	.o_s5_wb_we (o_s5_wb_we ),
	.i_s5_wb_dat(i_s5_wb_dat),
	.o_s5_wb_dat(o_s5_wb_dat),
	.o_s5_wb_cyc(o_s5_wb_cyc),
	.o_s5_wb_stb(o_s5_wb_stb),
	.i_s5_wb_ack(i_s5_wb_ack),
	.i_s5_wb_err(i_s5_wb_err),

	// WISHBONE slave 6 - Timer Module
	.o_s6_wb_adr(o_s6_wb_adr),
	.o_s6_wb_sel(o_s6_wb_sel),
	.o_s6_wb_we (o_s6_wb_we ),
	.i_s6_wb_dat(i_s6_wb_dat),
	.o_s6_wb_dat(o_s6_wb_dat),
	.o_s6_wb_cyc(o_s6_wb_cyc),
	.o_s6_wb_stb(o_s6_wb_stb),
	.i_s6_wb_ack(i_s6_wb_ack),
	.i_s6_wb_err(i_s6_wb_err),

	// WISHBONE slave 7 - Interrupt Controller
	.o_s7_wb_adr(o_s7_wb_adr),
	.o_s7_wb_sel(o_s7_wb_sel),
	.o_s7_wb_we (o_s7_wb_we ),
	.i_s7_wb_dat(i_s7_wb_dat),
	.o_s7_wb_dat(o_s7_wb_dat),
	.o_s7_wb_cyc(o_s7_wb_cyc),
	.o_s7_wb_stb(o_s7_wb_stb),
	.i_s7_wb_ack(i_s7_wb_ack),
	.i_s7_wb_err(i_s7_wb_err),

	// WISHBONE slave 8 - TDMO
	.o_s8_wb_adr(o_s8_wb_adr),
	.o_s8_wb_sel(o_s8_wb_sel),
	.o_s8_wb_we (o_s8_wb_we ),
	.i_s8_wb_dat(i_s8_wb_dat),
	.o_s8_wb_dat(o_s8_wb_dat),
	.o_s8_wb_cyc(o_s8_wb_cyc),
	.o_s8_wb_stb(o_s8_wb_stb),
	.i_s8_wb_ack(i_s8_wb_ack),
	.i_s8_wb_err(i_s8_wb_err),

	// WISHBONE slave 9 - Config Module
	.o_s9_wb_adr(o_s9_wb_adr),
	.o_s9_wb_sel(o_s9_wb_sel),
	.o_s9_wb_we (o_s9_wb_we ),
	.i_s9_wb_dat(i_s9_wb_dat),
	.o_s9_wb_dat(o_s9_wb_dat),
	.o_s9_wb_cyc(o_s9_wb_cyc),
	.o_s9_wb_stb(o_s9_wb_stb),
	.i_s9_wb_ack(i_s9_wb_ack),
	.i_s9_wb_err(i_s9_wb_err)
);

always #(`CLK_PERIOD/2) clk = ~clk;

initial begin
	$timeformat(-9,2,"ns", 16);
	`ifdef SDFSCAN
		$sdf_annotate("sdf/wishbone_arbiter_tsmc18_scan.sdf", test.top);
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

	// initialize master bus registers
	i_m0_wb_adr <= 32'h00000000;
	i_m0_wb_sel <= {WB_SWIDTH{1'b0}};
	i_m0_wb_dat <= {WB_DWIDTH{1'b0}};
	i_m0_wb_we  <= 1'b0;
	i_m0_wb_cyc <= 1'b0;
	i_m0_wb_stb <= 1'b0;

	i_s0_wb_ack <= 1'b0;
	i_s1_wb_ack <= 1'b0;
	i_s2_wb_ack <= 1'b0;
	i_s3_wb_ack <= 1'b0;
	i_s4_wb_ack <= 1'b0;
	i_s5_wb_ack <= 1'b0;
	i_s6_wb_ack <= 1'b0;
	i_s7_wb_ack <= 1'b0;
	i_s8_wb_ack <= 1'b0;
	i_s9_wb_ack <= 1'b0;

	i_s0_wb_err <= 1'b0;
	i_s1_wb_err <= 1'b0;
	i_s2_wb_err <= 1'b0;
	i_s3_wb_err <= 1'b0;
	i_s4_wb_err <= 1'b0;
	i_s5_wb_err <= 1'b0;
	i_s6_wb_err <= 1'b0;
	i_s7_wb_err <= 1'b0;
	i_s8_wb_err <= 1'b0;
	i_s9_wb_err <= 1'b0;

	i_s0_wb_dat <= {WB_DWIDTH{1'b0}};
	i_s1_wb_dat <= {WB_DWIDTH{1'b0}};
	i_s2_wb_dat <= {WB_DWIDTH{1'b0}};
	i_s3_wb_dat <= {WB_DWIDTH{1'b0}};
	i_s4_wb_dat <= {WB_DWIDTH{1'b0}};
	i_s5_wb_dat <= {WB_DWIDTH{1'b0}};
	i_s6_wb_dat <= {WB_DWIDTH{1'b0}};
	i_s7_wb_dat <= {WB_DWIDTH{1'b0}};
	i_s8_wb_dat <= {WB_DWIDTH{1'b0}};
	i_s9_wb_dat <= {WB_DWIDTH{1'b0}};

	@(posedge clk) begin reset <= 1'b1; end
	@(posedge clk) begin reset <= 1'b0; end

	for (i=0; i<SLAVE_NUM; i=i+1) begin

		i_m0_wb_adr[15:0] <= $random;
		i_m0_wb_sel		<= $random;		
		i_m0_wb_dat		<= $random;

		case (i)
			0:	i_m0_wb_adr 	 <= BOOT_BASE;			 // address = 0-3FFF, address[31:14] = 28000h
			1:	i_m0_wb_adr 	 <= MAIN_BASE + 32'h03FF_FFFF; // address = 0-7FFFFFF, and not in boot mem
			2:	i_m0_wb_adr[31:16] <= APRSC_BASE;
			3:	i_m0_wb_adr[31:16] <= AMBER_UART0_BASE;
			4:	i_m0_wb_adr[31:16] <= TDMI_BASE;
			5:	i_m0_wb_adr[31:16] <= TEST_BASE;
			6:	i_m0_wb_adr[31:16] <= AMBER_TM_BASE;
			7:	i_m0_wb_adr[31:16] <= AMBER_IC_BASE;
			8:	i_m0_wb_adr[31:16] <= TDMO_BASE;
			9:	i_m0_wb_adr[31:16] <= CONFIG_BASE;
		endcase

		@(posedge clk) begin
			i_m0_wb_cyc <= 1'b1;			// set cycle high to indicate the start of a write
			i_m0_wb_stb <= 1'b1;			// set strobe high to indicate the start of a write
			i_m0_wb_we  <= 1'b1;			// set write enable high (write operation)
		end

		@(negedge clk) begin
			case (i)
				0:
					begin
					adr <= o_s0_wb_adr;
					sel <= o_s0_wb_sel;
					we  <= o_s0_wb_we;
					cyc <= o_s0_wb_cyc;
					stb <= o_s0_wb_stb;
					dat <= o_s0_wb_dat;
					i_s0_wb_dat = $random;
					slave_data <= i_s0_wb_dat;
					i_s0_wb_ack <= 1'b1;
					i_s0_wb_err <= 1'b1;
					end
				1:
					begin
					adr <= o_s1_wb_adr;
					sel <= o_s1_wb_sel;
					we  <= o_s1_wb_we;
					cyc <= o_s1_wb_cyc;
					stb <= o_s1_wb_stb;
					dat <= o_s1_wb_dat;
					i_s1_wb_dat = $random;
					slave_data <= i_s1_wb_dat;
					i_s1_wb_ack <= 1'b1;
					i_s1_wb_err <= 1'b1;
					end
				2:
					begin
					adr <= o_s2_wb_adr;
					sel <= o_s2_wb_sel;
					we  <= o_s2_wb_we;
					cyc <= o_s2_wb_cyc;
					stb <= o_s2_wb_stb;
					dat <= o_s2_wb_dat;
					i_s2_wb_dat = $random;
					slave_data <= i_s2_wb_dat;
					i_s2_wb_ack <= 1'b1;
					i_s2_wb_err <= 1'b1;
					end
				3:
					begin
					adr <= o_s3_wb_adr;
					sel <= o_s3_wb_sel;
					we  <= o_s3_wb_we;
					cyc <= o_s3_wb_cyc;
					stb <= o_s3_wb_stb;
					dat <= o_s3_wb_dat;
					i_s3_wb_dat = $random;
					slave_data <= i_s3_wb_dat;
					i_s3_wb_ack <= 1'b1;
					i_s3_wb_err <= 1'b1;
					end
				4:
					begin
					adr <= o_s4_wb_adr;
					sel <= o_s4_wb_sel;
					we  <= o_s4_wb_we;
					cyc <= o_s4_wb_cyc;
					stb <= o_s4_wb_stb;
					dat <= o_s4_wb_dat;
					i_s4_wb_dat = $random;
					slave_data <= i_s4_wb_dat;
					i_s4_wb_ack <= 1'b1;
					i_s4_wb_err <= 1'b1;
					end
				5:
					begin
					adr <= o_s5_wb_adr;
					sel <= o_s5_wb_sel;
					we  <= o_s5_wb_we;
					cyc <= o_s5_wb_cyc;
					stb <= o_s5_wb_stb;
					dat <= o_s5_wb_dat;
					i_s5_wb_dat = $random;
					slave_data <= i_s5_wb_dat;
					i_s5_wb_ack <= 1'b1;
					i_s5_wb_err <= 1'b1;
					end
				6:
					begin
					adr <= o_s6_wb_adr;
					sel <= o_s6_wb_sel;
					we  <= o_s6_wb_we;
					cyc <= o_s6_wb_cyc;
					stb <= o_s6_wb_stb;
					dat <= o_s6_wb_dat;
					i_s6_wb_dat = $random;
					slave_data <= i_s6_wb_dat;
					i_s6_wb_ack <= 1'b1;
					i_s6_wb_err <= 1'b1;
					end
				7:
					begin
					adr <= o_s7_wb_adr;
					sel <= o_s7_wb_sel;
					we  <= o_s7_wb_we;
					cyc <= o_s7_wb_cyc;
					stb <= o_s7_wb_stb;
					dat <= o_s7_wb_dat;
					i_s7_wb_dat = $random;
					slave_data <= i_s7_wb_dat;
					i_s7_wb_ack <= 1'b1;
					i_s7_wb_err <= 1'b1;
					end
				8:
					begin
					adr <= o_s8_wb_adr;
					sel <= o_s8_wb_sel;
					we  <= o_s8_wb_we;
					cyc <= o_s8_wb_cyc;
					stb <= o_s8_wb_stb;
					dat <= o_s8_wb_dat;
					i_s8_wb_dat = $random;
					slave_data <= i_s8_wb_dat;
					i_s8_wb_ack <= 1'b1;
					i_s8_wb_err <= 1'b1;
					end
				9:
					begin
					adr <= o_s9_wb_adr;
					sel <= o_s9_wb_sel;
					we  <= o_s9_wb_we;
					cyc <= o_s9_wb_cyc;
					stb <= o_s9_wb_stb;
					dat <= o_s9_wb_dat;
					i_s9_wb_dat = $random;
					slave_data <= i_s9_wb_dat;
					i_s9_wb_ack <= 1'b1;
					i_s9_wb_err <= 1'b1;
					end
			endcase
		end

		@(posedge clk) begin
			if (adr !== i_m0_wb_adr) begin
				$display("%t ERROR: adr is %h, but should be %h", $time, adr, i_m0_wb_adr);
				`ifdef ERRORSTOP
					$stop;
				`endif
			end
			if (sel !== i_m0_wb_sel) begin
				$display("%t ERROR: sel is %h, but should be %h", $time, sel, i_m0_wb_sel);
				`ifdef ERRORSTOP
					$stop;
				`endif
			end
			if (we !== i_m0_wb_we) begin
				$display("%t ERROR: we is %h, but should be %h", $time, we, i_m0_wb_we);
				`ifdef ERRORSTOP
					$stop;
				`endif
			end
			if (cyc !== i_m0_wb_cyc) begin
				$display("%t ERROR: cyc is %h, but should be %h", $time, cyc, i_m0_wb_cyc);
				`ifdef ERRORSTOP
					$stop;
				`endif
			end
			if (stb !== i_m0_wb_stb) begin
				$display("%t ERROR: stb is %h, but should be %h", $time, stb, i_m0_wb_stb);
				`ifdef ERRORSTOP
					$stop;
				`endif
			end
			if (dat !== i_m0_wb_dat) begin
				$display("%t ERROR: dat is %h, but should be %h", $time, dat, i_m0_wb_dat);
				`ifdef ERRORSTOP
					$stop;
				`endif
			end
			if (slave_data !== o_m0_wb_dat) begin
				$display("%t ERROR: master data out is %h, but should be %h", $time, o_m0_wb_dat, slave_data);
				`ifdef ERRORSTOP
					$stop;
				`endif
			end
			if (o_m0_wb_ack !== 1'b1) begin
				$display("%t ERROR: dat is %h, but should be 1", $time, o_m0_wb_ack);
				`ifdef ERRORSTOP
					$stop;
				`endif
			end
			if (o_m0_wb_err !== 1'b1) begin
				$display("%t ERROR: err is %h, but should be 1", $time, o_m0_wb_err);
				`ifdef ERRORSTOP
					$stop;
				`endif
			end
		end

		case (i)
			0:
				begin
				i_s0_wb_dat <= {WB_DWIDTH{1'b0}};
				i_s0_wb_ack <= 1'b0;
				i_s0_wb_err <= 1'b0;
				end
			1:
				begin
				i_s1_wb_dat <= {WB_DWIDTH{1'b0}};
				i_s1_wb_ack <= 1'b0;
				i_s1_wb_err <= 1'b0;
				end
			2:
				begin
				i_s2_wb_dat <= {WB_DWIDTH{1'b0}};
				i_s2_wb_ack <= 1'b0;
				i_s2_wb_err <= 1'b0;
				end
			3:
				begin
				i_s3_wb_dat <= {WB_DWIDTH{1'b0}};
				i_s3_wb_ack <= 1'b0;
				i_s3_wb_err <= 1'b0;
				end
			4:
				begin
				i_s4_wb_dat <= {WB_DWIDTH{1'b0}};
				i_s4_wb_ack <= 1'b0;
				i_s4_wb_err <= 1'b0;
				end
			5:
				begin
				i_s5_wb_dat <= {WB_DWIDTH{1'b0}};
				i_s5_wb_ack <= 1'b0;
				i_s5_wb_err <= 1'b0;
				end
			6:
				begin
				i_s6_wb_dat <= {WB_DWIDTH{1'b0}};
				i_s6_wb_ack <= 1'b0;
				i_s6_wb_err <= 1'b0;
				end
			7:
				begin
				i_s7_wb_dat <= {WB_DWIDTH{1'b0}};
				i_s7_wb_ack <= 1'b0;
				i_s7_wb_err <= 1'b0;
				end
			8:
				begin
				i_s8_wb_dat <= {WB_DWIDTH{1'b0}};
				i_s8_wb_ack <= 1'b0;
				i_s8_wb_err <= 1'b0;
				end
			9:
				begin
				i_s9_wb_dat <= {WB_DWIDTH{1'b0}};
				i_s9_wb_ack <= 1'b0;
				i_s9_wb_err <= 1'b0;
				end
		endcase

		@(posedge clk) begin
			i_m0_wb_cyc <= 1'b0;			// set cycle high to indicate the start of a write
			i_m0_wb_stb <= 1'b0;			// set strobe high to indicate the start of a write
			i_m0_wb_we  <= 1'b0;			// set write enable high (write operation)
		end

	end

	$display("TEST COMPLETE!");	
	$finish;
end

endmodule
