/*

Description: Top level encoder module. Includes mid-level blocks:
		TDMI, TDMO, APRSC, CLKGEN, and the AMBER modules

Author: Adam Steenkamer

Revision History
//----------------------------------------------------------------------------
3/7/16	-	by Adam Steenkamer -	Initial Creation
3/14/16	-	by Dave Malanga	 -	Added the rest of the necessary files
//----------------------------------------------------------------------------
*/

`undef ENC
`undef DEC

`define ENC

module enc #(
	parameter WB_DWIDTH = 128,
	parameter WB_SWIDTH = 16,
	parameter WB_SLAVES = 10
)(
		reset,
		clk,
		enc_s,
		enc_s_clk,
		enc_s_fs,
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
		i_uart0_tx,
		o_uart0_cts,
		i_uart0_rts,
		o_uart0_rx,
		enc_o,
		enc_o_clk,
		enc_o_fs,

		o_mm_wb_adr,
		o_mm_wb_sel,
		o_mm_wb_we,
		i_mm_wb_dat,
		o_mm_wb_dat,
		o_mm_wb_cyc,
		o_mm_wb_stb,
		i_mm_wb_ack,
		i_mm_wb_err,

		o_cfg_wb_adr,
		o_cfg_wb_sel,
		o_cfg_wb_we,
		i_cfg_wb_dat,
		o_cfg_wb_dat,
		o_cfg_wb_cyc,
		o_cfg_wb_stb,
		i_cfg_wb_ack,
		i_cfg_wb_err

       );

//-------------------------------------------------------------------------------------------------
// I/O Declarations


// Global Reset & Clock
input
	reset,		// global system reset
	clk;			// system clock


// Encoder Interface Ports
input
	enc_s,		// serial data in (PCM)
	enc_s_clk,		// serial clock input
	enc_s_fs;		// frame sync input
output
	enc_o,		// serial data out (ADPCM)
	enc_o_clk,		// serial clock
	enc_o_fs;		// frame sync output



// Scan & Test Input Ports
input
    scan_in0,		// test scan mode data input
    scan_in1,		// test scan mode data input
    scan_in2,		// test scan mode data input
    scan_in3,		// test scan mode data input
    scan_in4,		// test scan mode data input
    scan_enable,		// test scan mode enable
    test_mode;		// test mode 


// Scan Output Ports
output
    scan_out0,		// test scan mode data output
    scan_out1,		// test scan mode data output
    scan_out2,		// test scan mode data output
    scan_out3,		// test scan mode data output
    scan_out4;		// test scan mode data output


// Uart Interface I/O
input
	i_uart0_tx,
	i_uart0_rts;
output
	o_uart0_rx,
	o_uart0_cts;


// Main Memory Wishbone Interface
input
[WB_DWIDTH-1:0]	i_mm_wb_dat;
input
			i_mm_wb_ack,
			i_mm_wb_err;
output
[31:0]		o_mm_wb_adr;
output
[WB_SWIDTH-1:0]	o_mm_wb_sel;
output
[WB_DWIDTH-1:0]	o_mm_wb_dat;
output
			o_mm_wb_we,
			o_mm_wb_cyc,
			o_mm_wb_stb;


// Config Module Wishbone Interface
input
[WB_DWIDTH-1:0]	i_cfg_wb_dat;
input
			i_cfg_wb_ack,
			i_cfg_wb_err;
output
[31:0]		o_cfg_wb_adr;
output
[WB_SWIDTH-1:0]	o_cfg_wb_sel;
output
[WB_DWIDTH-1:0]	o_cfg_wb_dat;
output
			o_cfg_wb_we,
			o_cfg_wb_cyc,
			o_cfg_wb_stb;

//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
// Internal Signals


// Clock Generation Signals
wire		tdmo_ser_clk_in;		// serial clock input to tdmo
wire		tdmo_fs_in;			// frame sync input to tdmo
wire		sys_clk_in;			// system clock (output of clock generator) = 81.92 MHz
wire		aprsc_clk_in;		// APRSC (co-processor) clock = 20.48 MHz


// Wishbone Slave Interfaces
wire	[31:0]		s_wb_adr	[WB_SLAVES-1:0];	// address bus
wire	[WB_SWIDTH-1:0]	s_wb_sel	[WB_SLAVES-1:0];	// select bus
wire	[WB_SLAVES-1:0]	s_wb_we;				// write enable signal
wire	[WB_DWIDTH-1:0]	s_wb_dat_w	[WB_SLAVES-1:0];	// write data bus (w.r.t. master)
wire	[WB_DWIDTH-1:0]	s_wb_dat_r	[WB_SLAVES-1:0];	// read data bus	(w.r.t. master)
wire	[WB_SLAVES-1:0]	s_wb_cyc;				// cycle signal
wire	[WB_SLAVES-1:0]	s_wb_stb;				// strobe signal
wire	[WB_SLAVES-1:0]	s_wb_ack;				// acknowledge signal
wire	[WB_SLAVES-1:0]	s_wb_err;				// error signal


// Wishbone Master Interface
wire	[31:0]		m_wb_adr;	// address bus
wire	[WB_SWIDTH-1:0]	m_wb_sel;	// select bus
wire				m_wb_we;	// write enable signal
wire	[WB_DWIDTH-1:0]	m_wb_dat_w;	// write data bus
wire	[WB_DWIDTH-1:0]	m_wb_dat_r;	// read data bus
wire				m_wb_cyc;	// cycle signal
wire				m_wb_stb;	// strobe signal
wire				m_wb_ack;	// acknowledge signal
wire				m_wb_err;	// error signal


// Test Module Signals
wire		test_reg_irq;		// test module interrupt request
wire		test_reg_firq;		// test module fast interrupt request
wire		test_mem_ctrl;		// test module memory control signal (goes to ddr)


// Interrupt Signals
wire		amber_irq;			// interrupt request from the interrupt controller to amber
wire		amber_firq;			// fast interrupt request from the interrupt controller to amber
wire		tdmi_int;			// TDMI new data available interrupt
wire		uart0_int;			// uart interrupt
wire [2:0]	timer_int;			// timer interrupts


// Other Internal Signals
wire	system_rdy;

//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
// Assign Statements

// Halt core until system is ready
assign system_rdy = !reset;

assign test_mem_ctrl = 1'b0;

assign s_wb_dat_r[2][WB_DWIDTH-1:96]	= s_wb_dat_r[2][31:0];
assign s_wb_dat_r[2][95:64]			= s_wb_dat_r[2][31:0];
assign s_wb_dat_r[2][63:32]			= s_wb_dat_r[2][31:0];

assign s_wb_dat_r[4][WB_DWIDTH-1:96]	= s_wb_dat_r[4][31:0];
assign s_wb_dat_r[4][95:64]			= s_wb_dat_r[4][31:0];
assign s_wb_dat_r[4][63:32]			= s_wb_dat_r[4][31:0];

assign s_wb_dat_r[8][WB_DWIDTH-1:96]	= s_wb_dat_r[8][31:0];
assign s_wb_dat_r[8][95:64]			= s_wb_dat_r[8][31:0];
assign s_wb_dat_r[8][63:32]			= s_wb_dat_r[8][31:0];

assign s_wb_adr[1] = 'd0;
assign s_wb_sel[1] = 'd0;
assign s_wb_we[1] = 'd0;
assign s_wb_dat_w[1] = 'd0;
assign s_wb_dat_r[1] = 'd0;
assign s_wb_cyc[1] = 'd0;
assign s_wb_stb[1] = 'd0;
assign s_wb_ack[1] = 'd0;
assign s_wb_err[1] = 'd0;

assign s_wb_adr[9] = 'd0;
assign s_wb_sel[9] = 'd0;
assign s_wb_we[9] = 'd0;
assign s_wb_dat_w[9] = 'd0;
assign s_wb_dat_r[9] = 'd0;
assign s_wb_cyc[9] = 'd0;
assign s_wb_stb[9] = 'd0;
assign s_wb_ack[9] = 'd0;
assign s_wb_err[9] = 'd0;

//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
// Instantiation


// Clock Generator Instantiation
CLK_GEN clkgen_inst0 (

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
	.i_frame_syn(enc_s_fs),
	.o_sys_clk(sys_clk_in),
	.o_serial_clk(tdmo_ser_clk_in),
	.o_frame_syn(tdmo_fs_in),
	.o_co_proc_clk(aprsc_clk_in)
);


// Wishbone Arbiter Instantiation
wishbone_arbiter
wb_arb_inst0 (
	.reset(reset),
	.clk(sys_clk_in),
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

	// WISHBONE master 0 - Amber
	.i_m0_wb_adr(m_wb_adr),
	.i_m0_wb_sel(m_wb_sel),
	.i_m0_wb_we(m_wb_we),
	.o_m0_wb_dat(m_wb_dat_r),
	.i_m0_wb_dat(m_wb_dat_w),
	.i_m0_wb_cyc(m_wb_cyc),
	.i_m0_wb_stb(m_wb_stb),
	.o_m0_wb_ack(m_wb_ack),
	.o_m0_wb_err(m_wb_err),

	// WISHBONE slave 0 - Boot Memory
	.o_s0_wb_adr(s_wb_adr[0]),
	.o_s0_wb_sel(s_wb_sel[0]),
	.o_s0_wb_we(s_wb_we[0]),
	.i_s0_wb_dat(s_wb_dat_r[0]),
	.o_s0_wb_dat(s_wb_dat_w[0]),
	.o_s0_wb_cyc(s_wb_cyc[0]),
	.o_s0_wb_stb(s_wb_stb[0]),
	.i_s0_wb_ack(s_wb_ack[0]),
	.i_s0_wb_err(s_wb_err[0]),

	// WISHBONE slave 1 - Main Memory
	.o_s1_wb_adr(o_mm_wb_adr),
	.o_s1_wb_sel(o_mm_wb_sel),
	.o_s1_wb_we(o_mm_wb_we),
	.i_s1_wb_dat(i_mm_wb_dat),
	.o_s1_wb_dat(o_mm_wb_dat),
	.o_s1_wb_cyc(o_mm_wb_cyc),
	.o_s1_wb_stb(o_mm_wb_stb),
	.i_s1_wb_ack(i_mm_wb_ack),
	.i_s1_wb_err(i_mm_wb_err),

	// WISHBONE slave 2 - APRSC
	.o_s2_wb_adr(s_wb_adr[2]),
	.o_s2_wb_sel(s_wb_sel[2]),
	.o_s2_wb_we(s_wb_we[2]),
	.i_s2_wb_dat(s_wb_dat_r[2]),
	.o_s2_wb_dat(s_wb_dat_w[2]),
	.o_s2_wb_cyc(s_wb_cyc[2]),
	.o_s2_wb_stb(s_wb_stb[2]),
	.i_s2_wb_ack(s_wb_ack[2]),
	.i_s2_wb_err(s_wb_err[2]),

	// WISHBONE slave 3 - UART 0
	.o_s3_wb_adr(s_wb_adr[3]),
	.o_s3_wb_sel(s_wb_sel[3]),
	.o_s3_wb_we(s_wb_we[3]),
	.i_s3_wb_dat(s_wb_dat_r[3]),
	.o_s3_wb_dat(s_wb_dat_w[3]),
	.o_s3_wb_cyc(s_wb_cyc[3]),
	.o_s3_wb_stb(s_wb_stb[3]),
	.i_s3_wb_ack(s_wb_ack[3]),
	.i_s3_wb_err(s_wb_err[3]),

	// WISHBONE slave 4 - TDMI
	.o_s4_wb_adr(s_wb_adr[4]),
	.o_s4_wb_sel(s_wb_sel[4]),
	.o_s4_wb_we(s_wb_we[4]),
	.i_s4_wb_dat(s_wb_dat_r[4]),
	.o_s4_wb_dat(s_wb_dat_w[4]),
	.o_s4_wb_cyc(s_wb_cyc[4]),
	.o_s4_wb_stb(s_wb_stb[4]),
	.i_s4_wb_ack(s_wb_ack[4]),
	.i_s4_wb_err(s_wb_err[4]),

	// WISHBONE slave 5 - Test Module
	.o_s5_wb_adr(s_wb_adr[5]),
	.o_s5_wb_sel(s_wb_sel[5]),
	.o_s5_wb_we(s_wb_we[5]),
	.i_s5_wb_dat(s_wb_dat_r[5]),
	.o_s5_wb_dat(s_wb_dat_w[5]),
	.o_s5_wb_cyc(s_wb_cyc[5]),
	.o_s5_wb_stb(s_wb_stb[5]),
	.i_s5_wb_ack(s_wb_ack[5]),
	.i_s5_wb_err(s_wb_err[5]),

	// WISHBONE slave 6 - Timer Module
	.o_s6_wb_adr(s_wb_adr[6]),
	.o_s6_wb_sel(s_wb_sel[6]),
	.o_s6_wb_we(s_wb_we[6]),
	.i_s6_wb_dat(s_wb_dat_r[6]),
	.o_s6_wb_dat(s_wb_dat_w[6]),
	.o_s6_wb_cyc(s_wb_cyc[6]),
	.o_s6_wb_stb(s_wb_stb[6]),
	.i_s6_wb_ack(s_wb_ack[6]),
	.i_s6_wb_err(s_wb_err[6]),

	// WISHBONE slave 7 - Interrupt Controller
	.o_s7_wb_adr(s_wb_adr[7]),
	.o_s7_wb_sel(s_wb_sel[7]),
	.o_s7_wb_we(s_wb_we[7]),
	.i_s7_wb_dat(s_wb_dat_r[7]),
	.o_s7_wb_dat(s_wb_dat_w[7]),
	.o_s7_wb_cyc(s_wb_cyc[7]),
	.o_s7_wb_stb(s_wb_stb[7]),
	.i_s7_wb_ack(s_wb_ack[7]),
	.i_s7_wb_err(s_wb_err[7]),

	// WISHBONE slave 8 - TDMO
	.o_s8_wb_adr(s_wb_adr[8]),
	.o_s8_wb_sel(s_wb_sel[8]),
	.o_s8_wb_we(s_wb_we[8]),
	.i_s8_wb_dat(s_wb_dat_r[8]),
	.o_s8_wb_dat(s_wb_dat_w[8]),
	.o_s8_wb_cyc(s_wb_cyc[8]),
	.o_s8_wb_stb(s_wb_stb[8]),
	.i_s8_wb_ack(s_wb_ack[8]),
	.i_s8_wb_err(s_wb_err[8]),

	// WISHBONE slave 9 - Config Module
	.o_s9_wb_adr(o_cfg_wb_adr),
	.o_s9_wb_sel(o_cfg_wb_sel),
	.o_s9_wb_we(o_cfg_wb_we),
	.i_s9_wb_dat(i_cfg_wb_dat),
	.o_s9_wb_dat(o_cfg_wb_dat),
	.o_s9_wb_cyc(o_cfg_wb_cyc),
	.o_s9_wb_stb(o_cfg_wb_stb),
	.i_s9_wb_ack(i_cfg_wb_ack),
	.i_s9_wb_err(i_cfg_wb_err)
);


// Boot Memory Instantiation (slave 0)
boot_mem128 boot_mem_inst0 (
	.clk(sys_clk_in),
	.reset(reset),
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
	.i_wb_adr(s_wb_adr[0]),
	.i_wb_sel(s_wb_sel[0]),
	.i_wb_we(s_wb_we[0]),
	.o_wb_dat(s_wb_dat_r[0]),
	.i_wb_dat(s_wb_dat_w[0]),
	.i_wb_cyc(s_wb_cyc[0]),
	.i_wb_stb(s_wb_stb[0]),
	.o_wb_ack(s_wb_ack[0]),
	.o_wb_err(s_wb_err[0])
);


// APRSC Instantiation (slave 2)
APRSC copro_inst0 (
	.clk(sys_clk_in),
	.CoPrclk(aprsc_clk_in),
	.reset(reset),
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
	.i_wb_adr(s_wb_adr[2]),
	.i_wb_sel(s_wb_sel[2][3:0]),
	.i_wb_we(s_wb_we[2]),
	.o_wb_dat(s_wb_dat_r[2][31:0]),
	.i_wb_dat(s_wb_dat_w[2][31:0]),
	.i_wb_cyc(s_wb_cyc[2]),
	.i_wb_stb(s_wb_stb[2]),
	.o_wb_ack(s_wb_ack[2]),
	.o_wb_err(s_wb_err[2])
);


// UART0 Instantiation (slave 3)
uart
uart_inst0(
	.clk(sys_clk_in),
	.reset(reset),
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
	.i_wb_adr(s_wb_adr[3]),
	.i_wb_sel(s_wb_sel[3]),
	.i_wb_we(s_wb_we[3]),
	.o_wb_dat(s_wb_dat_r[3]),
	.i_wb_dat(s_wb_dat_w[3]),
	.i_wb_cyc(s_wb_cyc[3]),
	.i_wb_stb(s_wb_stb[3]),
	.o_wb_ack(s_wb_ack[3]),
	.o_wb_err(s_wb_err[3]),
	.o_uart_int(uart0_int),
	.i_uart_cts_n(i_uart0_rts),	// Clear To Send
	.o_uart_txd(o_uart0_rx),    	// Transmit data
	.o_uart_rts_n(o_uart0_cts),  	// Request to Send
	.i_uart_rxd(i_uart0_tx)    	// Receive data
);


// TDMI Instantiation (slave 4)
TDMI tdmi_inst0(
	.reset(reset),
	.ser_clk(enc_s_clk),
	.data_in(enc_s),
	.frame_sync(enc_s_fs),
	.scan_in0(),
	.scan_in1(),
	.scan_in2(),
	.scan_in3(),
	.scan_in4(),
	.scan_enable(scan_enable),
	.test_mode(test_mode),
	.new_data_int(tdmi_int),
	.scan_out0(),
	.scan_out1(),
	.scan_out2(),
	.scan_out3(),
	.scan_out4(),
	.clk(sys_clk_in),
	.i_wb_adr(s_wb_adr[4]),
	.i_wb_sel(s_wb_sel[4][3:0]),
	.i_wb_we(s_wb_we[4]),
	.o_wb_dat(s_wb_dat_r[4][31:0]),
	.i_wb_dat(s_wb_dat_w[4][31:0]),
	.i_wb_cyc(s_wb_cyc[4]),
	.i_wb_stb(s_wb_stb[4]),
	.o_wb_ack(s_wb_ack[4]),
	.o_wb_err(s_wb_err[4])
);


// Test Module Instantiation (slave 5)
test_module
test_inst0 (
	.clk(sys_clk_in),
	.reset(reset),
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
	.o_irq(test_reg_irq),
	.o_firq(test_reg_firq),
	.o_mem_ctrl(test_mem_ctrl), // 0=128MB, 1=32MB (goes to ddr controller)
	.i_wb_adr(s_wb_adr[5]),
	.i_wb_sel(s_wb_sel[5]),
	.i_wb_we(s_wb_we[5]),
	.o_wb_dat(s_wb_dat_r[5]),
	.i_wb_dat(s_wb_dat_w[5]),
	.i_wb_cyc(s_wb_cyc[5]),
	.i_wb_stb(s_wb_stb[5]),
	.o_wb_ack(s_wb_ack[5]),
	.o_wb_err(s_wb_err[5]),
	.o_led(),		// don't need this (led output)
	.o_phy_rst_n()	// don't need this (for ethernet)
);


// Timer Module Instantiation (slave 6)
timer_module
timer_inst0 (
	.clk(sys_clk_in),
	.reset(reset),
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
	.i_wb_adr(s_wb_adr[6]),
	.i_wb_sel(s_wb_sel[6]),
	.i_wb_we(s_wb_we[6]),
	.o_wb_dat(s_wb_dat_r[6]),
	.i_wb_dat(s_wb_dat_w[6]),
	.i_wb_cyc(s_wb_cyc[6]),
	.i_wb_stb(s_wb_stb[6]),
	.o_wb_ack(s_wb_ack[6]),
	.o_wb_err(s_wb_err[6]),
	.o_timer_int(timer_int)
);


// Interrupt Controller Instantiation (slave 7)
interrupt_controller
interrupt_controller_inst0 (
	.clk			(sys_clk_in),
	.reset		(reset),
	.scan_in0		(),
	.scan_in1		(),
	.scan_in2		(),
	.scan_in3		(),
	.scan_in4		(),
	.scan_enable	(scan_enable),
	.test_mode		(test_mode),
	.scan_out0		(),
	.scan_out1		(),
	.scan_out2		(),
	.scan_out3		(),
	.scan_out4		(),
	.o_irq		(amber_irq),
	.o_firq		(amber_firq),
	.i_uart0_int	(uart0_int),
	.i_test_reg_irq	(test_reg_irq),
	.i_test_reg_firq	(test_reg_firq),
	.i_tm_timer_int	(timer_int),
	.i_tdmi_int		(tdmi_int),
	.i_wb_adr		(s_wb_adr[7]),
	.i_wb_sel		(s_wb_sel[7]),
	.i_wb_we		(s_wb_we[7]),
	.o_wb_dat		(s_wb_dat_r[7]),
	.i_wb_dat		(s_wb_dat_w[7]),
	.i_wb_cyc		(s_wb_cyc[7]),
	.i_wb_stb		(s_wb_stb[7]),
	.o_wb_ack		(s_wb_ack[7]),
	.o_wb_err		(s_wb_err[7])
);


// TDMO Instantiation (slave 8)
TDMO tdmo_inst0(
	.reset(reset),
	.ser_clk(tdmo_ser_clk_in),
	.data_out(enc_o),
	.frame_sync_in(tdmo_fs_in),
	.frame_sync_out(enc_o_fs),
	.ser_clk_out(enc_o_clk),
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
	.clk(sys_clk_in),
	.i_wb_adr(s_wb_adr[8]),
	.i_wb_sel(s_wb_sel[8][3:0]),
	.i_wb_we(s_wb_we[8]),
	.o_wb_dat(s_wb_dat_r[8][31:0]),
	.i_wb_dat(s_wb_dat_w[8][31:0]),
	.i_wb_cyc(s_wb_cyc[8]),
	.i_wb_stb(s_wb_stb[8]),
	.o_wb_ack(s_wb_ack[8]),
	.o_wb_err(s_wb_err[8])
);


// Instantiate AMBER Processor Core
a25_core amber_inst0(
	.clk(sys_clk_in),
	.reset(reset),
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
	.i_irq(amber_irq),
	.i_firq(amber_firq),
	.i_system_rdy(system_rdy),
	.o_wb_adr(m_wb_adr),
	.o_wb_sel(m_wb_sel),
	.o_wb_we(m_wb_we),
	.i_wb_dat(m_wb_dat_r),
	.o_wb_dat(m_wb_dat_w),
	.o_wb_cyc(m_wb_cyc),
	.o_wb_stb(m_wb_stb),
	.i_wb_ack(m_wb_ack),
	.i_wb_err(m_wb_err)
);

//-------------------------------------------------------------------------------------------------


endmodule // enc


