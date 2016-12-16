/*

Description : Compute the predicted signal estimate


Author(s)   : Jason,Siddharth,Kartik

Revision History :
//----------------------------------------------------------------------------------
*
*	2/27/2016	Jason			Updated APRSC with all the blocks called except the FMULT controller
*
*	2/28/2016	Jason			Fixed errors locating the files
*
*	2/29/2016	Jason			Included path to ACCUM for now
*
*	3/05/2016	Siddharth & Kartik	Updation of the Delay Block Sizes and reset value
*
*	3/05/2016	Siddharth		Intermediate Data commit for APRSC wiring
*
*	3/08/2016	Siddharth		Updation of the APRSC CoProcessor Module, to synchronize the Amber Request and FMULTACCUM state
*
*	3/13/2016	Adam			Working on APRSC control state machine at the bottom. Created temp signal for control flag always loop
*
*	3/13/2016	Adam			Redesigned APRSC control state machine
*
*	3/15/2016	Dave			Added wishbone functionality (can be condensed to write to more than one variable within on register address)
*
*	3/29/2016	Adam			Added wishbone functionality for reading SR (needed for decoder)
*	
*	4/11/2016	Dave			Added registers to buffer the wishbone acknowledge
//----------------------------------------------------------------------------------
*/

module APRSC (

	reset,
	clk,
	CoPrclk,
	scan_in0,
	scan_in1,
	scan_in2,
	scan_in3,
	scan_in4,
	scan_enable,
	test_mode,
	scan_out0,
	scan_out1,
	scan_out2,
	scan_out3,
	scan_out4,

	i_wb_adr,
	i_wb_sel,
	i_wb_we,
	o_wb_dat,
	i_wb_dat,
	i_wb_cyc,
	i_wb_stb,
	o_wb_ack,
	o_wb_err

       );

input
    	reset,                  // system reset
    	CoPrclk,		// CoProcessor clock
    	clk;                    // wishbone clock (system clock)

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


// The Wishbone Interface - Names are organized as follows:
//  i_ or o_	= input or output
//  wb_		= wishbone
//  [sig]		= signal name
input       [31:0]          i_wb_adr;	// Wishbone Address - The ms 16 bits are the slave address (unused by the slave - can be ignored). The ls 16 bits is a sub-address for registers within the slave.
input       [3:0] 	    i_wb_sel;	// Wishbone Select - A register that indicates which part(s) of the data bus should contain valid data.
input                       i_wb_we;	// Wishbone Write Enable - set high for a write, set low for a read.
output reg  [31:0] 	    o_wb_dat;	// Wishbone Data Output - output data bus.
input       [31:0] 	    i_wb_dat;	// Wishbone Data Input - input data bus.
input                       i_wb_cyc;	// Wishbone Cycle - Set high when there is a bus cycle in progress.
input                       i_wb_stb;	// Wishbone Strobe - set high when the slave is selected. Only responds to the rest of the signals when this is high.
output	                o_wb_ack;	// Wishbone Acknowledge - Set high to indicate the end of a bus cycle
output                      o_wb_err;	// Wishbone Error - Set high when there is an abnormality in the bus cycle. This is not used by the TDMI/TDMO interface, so it is set low.

`include "../include/register_addresses.vh"

wire sigpk, pk0, pk1, pk2;

wire u1, u2, u3, u4, u5, u6;

wire [15:0] an_bn, dqn_srn, wan_wbn;

wire [15:0] sr_int;

wire [10:0] sr0, sr1, sr2;

wire [15:0] a1t, a2t, a1p, a1r, a2r, a1, a2;

wire [15:0] b1, b2, b3, b4, b5, b6;

wire [15:0] b1p, b2p, b3p, b4p, b5p, b6p;

wire [15:0] b1r, b2r, b3r, b4r, b5r, b6r;

wire [10:0] dq0, dq1, dq2, dq3, dq4, dq5, dq6;

wire [14:0] se_int, sez_int;
wire [15:0] a2p_int;

reg [14:0] se, se1, se2; 

reg [14:0] sez;

reg [15:0] a2p, a2p1, a2p2;
reg a2p_read;

reg [15:0] sr_out, sr_out1, sr_out2;

reg req, req1, req2; // control flag to request evaluation of APRSC (start) and corresponding regs for clock domain crossing

reg [15:0] dq;

reg tr;

reg [1:0] rate_in;


// APRSC state machine variables
reg [1:0] state;
reg MACstart;	// FMULT/ACCUM start
wire MACdone;	// FMULT/ACCUM done
reg pulseDelay, pulseDelay1;
reg  ack1, ack2;	// for double registering ack output for clock domain crossing
wire negCoPrclk_int; // inverted version of co-processor clock

// Clocks
wire delayTrig_int = test_mode ? clk : pulseDelay; // signal to pulse delay blocks
wire CoPrclk_int_int = test_mode ? clk : CoPrclk;

assign o_wb_err = 1'b0;

reg start_read, read_done, start_read_buf_1, start_read_buf_2;

assign o_wb_ack = (i_wb_we) ? (i_wb_stb & i_wb_cyc) : (i_wb_stb & i_wb_cyc & start_read & (~read_done));


CLKINVX4 clk_buf_1 (
	.A(CoPrclk_int_int),
	.Y(negCoPrclk_int)
);

CLKBUFX4 clk_buf_2 (
	.A(delayTrig_int),
	.Y(delayTrig)
);

CLKBUFX4 clk_buf_3 (
	.A(CoPrclk_int_int),
	.Y(CoPrclk_int)
);

CLKINVX4 clk_buf_4 (
	.A(clk),
	.Y(clk_inv)
);

ADDC ADDC_INST0 (           
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .DQ(dq),
           .SEZ(sez_int),
           .PK0(pk0),
           .SIGPK(sigpk)    	
	);

DELAY #(.SIZE(1), .RSTVAL(0)) DELAY_PK0(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(pk0),
           .Y(pk1)  	
	);

DELAY #(.SIZE(1), .RSTVAL(0)) DELAY_PK1(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(pk1),
           .Y(pk2)  	
	);

//--------------------------------------------

ADDB ADDB_INST0 (           
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .DQ(dq),
           .SE(se_int),
           .SR(sr_int)    	
	);

FLOATB FLOATB_INST0(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .SR(sr_int),
           .SR0(sr0) 	
	);


DELAY #(.SIZE(11), .RSTVAL(32))DELAY_SR0(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(sr0),
           .Y(sr1)  	
	);

DELAY #(.SIZE(11), .RSTVAL(32)) DELAY_SR1(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(sr1),
           .Y(sr2)  	
	);

//--------------------------------------------------

FLOATA FLOATA_INST0(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .DQ(dq),
           .DQ0(dq0) 	
	);


DELAY #(.SIZE(11), .RSTVAL(32)) DELAY_DQ0(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(dq0),
           .Y(dq1)  	
	);

DELAY #(.SIZE(11), .RSTVAL(32)) DELAY_DQ1(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(dq1),
           .Y(dq2)  	
	);

DELAY #(.SIZE(11), .RSTVAL(32)) DELAY_DQ2(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(dq2),
           .Y(dq3)  	
	);

DELAY #(.SIZE(11), .RSTVAL(32)) DELAY_DQ3(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(dq3),
           .Y(dq4)  	
	);

DELAY #(.SIZE(11), .RSTVAL(32)) DELAY_DQ4(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(dq4),
           .Y(dq5)  	
	);

DELAY #(.SIZE(11), .RSTVAL(32)) DELAY_DQ5(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(dq5),
           .Y(dq6)  	
	);
	
//---------------------------------------------

UPA1 UPA1_INST0(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .SIGPK(sigpk),
           .PK0(pk0),
           .PK1(pk1),
           .A1(a1),
           .A1T(a1t)
	);

LIMD LIMD_INST0(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .A1T(a1t),
           .A2P(a2p_int),
           .A1P(a1p)
	);

TRIGB TRIGB_A1P(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .AnP(a1p),
           .TR(tr),
           .AnR(a1r)
	);

DELAY #(.SIZE(16), .RSTVAL(0)) DELAY_A1R(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(a1r),
           .Y(a1)  	
	);

//-----------------------------------------------

UPA2 UPA2_INST0(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .SIGPK(sigpk),
           .PK0(pk0),
           .PK1(pk1),
           .PK2(pk2),
           .A1(a1),
           .A2(a2),
           .A2T(a2t)
	);

LIMC LIMC_INST0(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .A2T(a2t),
           .A2P(a2p_int) 
	);

TRIGB TRIGB_A2P(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .AnP(a2p_int), 
           .TR(tr),
           .AnR(a2r)
	);

DELAY #(.SIZE(16), .RSTVAL(0)) DELAY_A2R(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(a2r),
           .Y(a2)  	
	);

//----------------------------------------------------

XOR XOR_INST0(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .DQn(dq1),
           .DQ(dq),
           .Un(u1)
	);

UPB UPB_INST0(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
           .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .DQn(dq),
           .Un(u1),
	   .rate_in(rate_in),
           .Bn(b1),
           .BnP(b1p)
	);

TRIGB TRIGB_B1P(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
           .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .AnP(b1p),
           .TR(tr),
           .AnR(b1r)
	);

DELAY #(.SIZE(16), .RSTVAL(0)) DELAY_B1R(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
           .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(b1r),
           .Y(b1)  	
	);

//------------------------------------------------

XOR XOR_INST1(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
           .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .DQn(dq2),
           .DQ(dq),
           .Un(u2)
	);

UPB UPB_INST1(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
           .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .DQn(dq),
           .Un(u2),
	   .rate_in(rate_in),
           .Bn(b2),
           .BnP(b2p)
	);

TRIGB TRIGB_B2P(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .AnP(b2p),
           .TR(tr),
           .AnR(b2r)
	);

DELAY #(.SIZE(16), .RSTVAL(0)) DELAY_B2R(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(b2r),
           .Y(b2)  	
	);

//------------------------------------------------

XOR XOR_INST2(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .DQn(dq3),
           .DQ(dq),
           .Un(u3)
	);

UPB UPB_INST2(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .DQn(dq),
           .Un(u3),
	   .rate_in(rate_in),
           .Bn(b3),
           .BnP(b3p)
	);

TRIGB TRIGB_B3P(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .AnP(b3p),
           .TR(tr),
           .AnR(b3r)
	);

DELAY #(.SIZE(16), .RSTVAL(0)) DELAY_B3R(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(b3r),
           .Y(b3)  	
	);

//--------------------------------------

XOR XOR_INST3(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .DQn(dq4),
           .DQ(dq),
           .Un(u4)
	);

UPB UPB_INST3(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .DQn(dq),
           .Un(u4),
	   .rate_in(rate_in),
           .Bn(b4),
           .BnP(b4p)
	);

TRIGB TRIGB_B4P(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .AnP(b4p),
           .TR(tr),
           .AnR(b4r)
	);

DELAY #(.SIZE(16), .RSTVAL(0)) DELAY_B4R(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(b4r),
           .Y(b4)  	
	);

//--------------------------------------

XOR XOR_INST4(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .DQn(dq5),
           .DQ(dq),
           .Un(u5)
	);

UPB UPB_INST4(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .DQn(dq),
           .Un(u5),
	   .rate_in(rate_in),
           .Bn(b5),
           .BnP(b5p)
	);

TRIGB TRIGB_B5P(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .AnP(b5p),
           .TR(tr),
           .AnR(b5r)
	);

DELAY #(.SIZE(16), .RSTVAL(0)) DELAY_B5R(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(b5r),
           .Y(b5)  	
	);

//--------------------------------------

XOR XOR_INST5(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .DQn(dq6),
           .DQ(dq),
           .Un(u6)
	);

UPB UPB_INST5(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .DQn(dq),
           .Un(u6),
	   .rate_in(rate_in),
           .Bn(b6),
           .BnP(b6p)
	);

TRIGB TRIGB_B6P(
	   .reset(reset),
           .clk(CoPrclk_int),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .AnP(b6p),
           .TR(tr),
           .AnR(b6r)
	);

DELAY #(.SIZE(16), .RSTVAL(0)) DELAY_B6R(
	   .reset(reset),
           .clk(delayTrig),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .X(b6r),
           .Y(b6)  	
	);

FMULT_ACCUM FMULT_ACCUM_INST0 (
           .reset(reset),
           .clk(CoPrclk_int),
           .start(MACstart),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
	   .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .b1(b1),
           .b2(b2),
           .b3(b3),
           .b4(b4),
           .b5(b5),
           .b6(b6),
           .a1(a1),
           .a2(a2),
           .dq1(dq1),
           .dq2(dq2),
           .dq3(dq3),
           .dq4(dq4),
           .dq5(dq5),
           .dq6(dq6),
           .sr1(sr1),
           .sr2(sr2),
           .se(se_int),
           .sez(sez_int),
           .done(MACdone)
           );

// double registering req input for clock domain crossing (from clk to CoPrclk)
always @(posedge CoPrclk_int or posedge reset) begin
	if(reset) begin
		req2		<=	1'b0;
		req1		<=	1'b0;
	end
	else begin
		req2		<=	req1;	// req2 is after clock domain crossing
		req1		<=	req;
	end
end

// Wait 2 clock cycles after FMULT/ACCUM is done before pulsing delay blocks
always @(posedge CoPrclk_int, posedge reset) begin
	if(reset) begin
		pulseDelay1 	<= 1'b0;
		pulseDelay	<= 1'b0;
	end
	else if(a2p_read) begin
		pulseDelay1	<= 1'b1;
		pulseDelay	<= pulseDelay1;
	end
	else begin
		pulseDelay1	<= 1'b0;
		pulseDelay	<= pulseDelay1;
	end
end


// Create negative version of clock to use @posedge instead of @negedge
//assign negCoPrclk_int = ~CoPrclk_int;

// deal with FMULT_ACCUM control
always @(posedge negCoPrclk_int or posedge reset) begin
	if(reset) begin
		MACstart	<=	1'b0;
		se		<=	15'h0000;
		sez		<=	15'h0000;
		a2p		<=	16'h0000;
		state		<=	2'b00;
		sr_out		<=	16'h0000;
	end
	else begin
		case(state)
			2'b00: begin
				se     <= se;
				sez    <= sez;
				a2p    <= a2p;
				sr_out <= sr_out;

				if(req2) begin
					state    <= 2'b01;
					MACstart <= 1'b1;
				end
				else begin
					state    <= 2'b00;
					MACstart <= 1'b0;
				end
			end
			2'b01: begin
				if(!MACdone) begin
					state    <= 2'b01;
					se       <= se;
					sez      <= sez;
					a2p      <= a2p;
					sr_out   <= sr_out;
					MACstart <= 1'b1;
				end
				else begin
					state    <= 2'b10;
					MACstart <= 1'b0;
					se       <= se_int;
					sez      <= sez_int;
					a2p      <= a2p_int;
					sr_out   <= sr_int;
				end
			end
			2'b10: begin
				MACstart <= 1'b0;
				se       <= se;
				sez      <= sez;
				a2p      <= a2p;
				sr_out   <= sr_out;
				if(!req2) begin
					state <= 2'b00;
				end
				else begin
					state <= 2'b10;
				end
			end
			default: begin
				state    <= 2'b00;
				MACstart <= 1'b0;
				se       <= se;
				sez      <= sez;
				a2p      <= a2p;
				sr_out   <= sr_out;
			end
		endcase
	end
end // FMULT_ACCUM control always

always @(posedge reset, posedge clk) begin
	if (reset) begin
		start_read	<= 1'b0;
		start_read_buf_1 <= 1'b0;
		start_read_buf_2 <= 1'b0;

		o_wb_dat	<= 32'h0000_0000;

		ack1 <= 1'b0;
		ack2 <= 1'b0;

		se1  <= 15'h0000;
		se2  <= 15'h0000;

		a2p1 <= 16'h0000;
		a2p2 <= 16'h0000;

		sr_out1  <= 16'h0000;
		sr_out2  <= 16'h0000;

		req     <= 1'b0;
		tr      <= 1'b0;
		dq      <= 16'h0000;
		rate_in <= 2'b00;

		a2p_read <= 1'b0;

	end else begin

		ack1  <= MACdone;
		ack2  <= ack1;

		se1 <= se;
		se2 <= se1;

		a2p1 <= a2p_int;
		a2p2 <= a2p1;

		sr_out1  <= sr_int;
		sr_out2  <= sr_out1;

		if (pulseDelay1) begin
			a2p_read <= 1'b0;
		end

		// Wishbone write handling
		if (i_wb_stb & i_wb_cyc & i_wb_we) begin
			case (i_wb_adr[15:0])
				APRSC_REQ:	req <= i_wb_dat[0];
				APRSC_TR:	tr  <= i_wb_dat[0];
				APRSC_DQ:	dq  <= i_wb_dat[15:0];
				APRSC_RATE: rate_in <= i_wb_dat[1:0];
				default:	req <= req;
			endcase
			start_read <= 1'b0;
		end

		// Wishbone read handling
		if (i_wb_stb & i_wb_cyc & (~i_wb_we)) begin
			case (i_wb_adr[15:0])
				APRSC_ACK:	o_wb_dat    <= {31'h0000_0000, ack2};
				APRSC_A2P:	begin
					o_wb_dat    <= {16'h0000, a2p2};
					a2p_read	<= 1'b1;
				end
				APRSC_SE:	o_wb_dat    <= {17'h00000, se2};
				APRSC_SR:	o_wb_dat    <= {16'h0000, sr_out2};
				default:	o_wb_dat    <= 32'hdead_beef;
			endcase
			start_read_buf_1 <= 1'b1;
			start_read_buf_2 <= start_read_buf_1;
			start_read       <= start_read_buf_2;
		end else begin
			start_read_buf_1 <= 1'b0;
			start_read_buf_2 <= 1'b0;
			start_read     <= 1'b0;
		end
	end
end

// This loop ensures that the acknowledge is pulled low in the case of consecutive reads (i.e., cyc & stb are only low for 1 clock cycle).
// A glitch was occuring due to start_read not being low yet before cyc & stb are set high for the next read.
always @( posedge reset, posedge clk_inv) begin
	if (reset) begin
		read_done <= 1'b1;
	end else begin
		if (i_wb_stb & i_wb_cyc) begin
			read_done <= 1'b0;
		end else begin
			read_done <= 1'b1;
		end
	end
end

endmodule // APRSC


