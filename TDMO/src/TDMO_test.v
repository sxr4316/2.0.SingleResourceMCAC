/*
*
* Module: TDMO testbench
*
* Author(s)		: David Malanga
*
* Description	: Tests TDMO by writing new, random data to each channel's register a
			: half cycle before it is scheduled to be seen on the serial output.
			: The constant NUM_TESTS controls how many times the testbench cycles
			: through all 32 channels.
*
* Revision History:
*----------------------------------------------------------------------------------
* 2/29/16 - by Dave Malanga - Initial creation
* 3/1/16  - by Dave Malanga - Added random frame sync testing
*----------------------------------------------------------------------------------
*/


//------------------------------------------------------------------------------------------------------------------
// Clock period definitions
`define IN_CLK_PERIOD 488.28125	// value is in ns
`define WB_CLK_PERIOD 12.207		// value is in ns
//------------------------------------------------------------------------------------------------------------------

`define NUM_TESTS 	1000
`define NUM_ADDRESSES	32
`define ERRORSTOP


module test;


`include "../include/register_addresses.vh"

//------------------------------------------------------------------------------------------------------------------
// Test scan i/o (not used)
wire scan_out0, scan_out1, scan_out2, scan_out3, scan_out4;
reg reset;
reg scan_in0, scan_in1, scan_in2, scan_in3, scan_in4, scan_enable, test_mode;
//------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------
// Input and Outputs

// Serial data input registers
reg			ser_clk;
reg			frame_sync_in;
wire			frame_sync_out;
reg	[7:0]		data_in_vec;
reg	[7:0]		data_out_reg;
wire			data_out;
wire			ser_clk_out;

// Wishbone interface (output) registers
reg			clk;
reg	[31:0]	i_wb_adr;	// Wishbone Address - The most significant 16 bits are the slave address (unused by the slave itself). The least significant 16-bits is a sub-address for registers within the slave.
reg	[3:0]		i_wb_sel;	// Wishbone Select - A register that indicates which part(s) of the data bus will be read by the master.
reg			i_wb_we;	// Wishbone Write Enable - is set (by master) high for a write, set low for a read.
wire	[31:0]	o_wb_dat;	// Wishbone Data Output - output data bus.
reg	[31:0]	i_wb_dat;	// Wishbone Data Input - input data bus.
reg			i_wb_cyc;	// Wishbone Cycle - is set high when there is a bus cycle in progress.
reg			i_wb_stb;	// Wishbone Strobe - is set high when the slave is selected.
wire			o_wb_ack;	// Wishbone Acknowledge - Set high by slave to indicate the end of a bus cycle
wire			o_wb_err;	// Wishbone Error - Set high when there is an abnormality in the bus cycle. This is not used by the TDMI/TDMO interface, so it is set low.

//------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------
// Variables for Test Generation

// for loop indeces
integer i, channel, bits;

// registers for generating random frame syncs
reg [2:0] rand_bit;
reg [4:0] rand_ch;
reg trigger;
reg [7:0] prev_ch0;

//------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------
// Instantiate the DUT
TDMO top (
  .reset(reset),

  .ser_clk(ser_clk),
  .frame_sync_in(frame_sync_in),
  .frame_sync_out(frame_sync_out),
  .data_out(data_out),
  .ser_clk_out(ser_clk_out),

  .scan_in0(scan_in0),
  .scan_in1(scan_in1),
  .scan_in2(scan_in2),
  .scan_in3(scan_in3),
  .scan_in4(scan_in4),
  .scan_enable(scan_enable),
  .test_mode(test_mode),

  .clk(clk),
  .i_wb_adr(i_wb_adr),
  .i_wb_sel(i_wb_sel),
  .i_wb_we(i_wb_we),
  .o_wb_dat(o_wb_dat),
  .i_wb_dat(i_wb_dat),
  .i_wb_cyc(i_wb_cyc),
  .i_wb_stb(i_wb_stb),
  .o_wb_ack(o_wb_ack),
  .o_wb_err(o_wb_err),

  .scan_out0(scan_out0),
  .scan_out1(scan_out1),
  .scan_out2(scan_out2),
  .scan_out3(scan_out3),
  .scan_out4(scan_out4)
);
//------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------
// create clocks
always #(`IN_CLK_PERIOD/2) ser_clk = ~ser_clk;
always #(`WB_CLK_PERIOD/2) clk = ~clk;
//------------------------------------------------------------------------------------------------------------------


initial begin
  $timeformat(-9,2,"ns", 16);
`ifdef SDFSCAN
  $sdf_annotate("sdf/TDMO_saed32nm_scan.sdf", test.top);
`endif

  ser_clk     = 1'b0;
  clk	  	  = 1'b0;
  reset       = 1'b0;
  frame_sync_in = 1'b0;

  scan_in0    = 1'b0;
  scan_in1    = 1'b0;
  scan_in2    = 1'b0;
  scan_in3    = 1'b0;
  scan_in4    = 1'b0;
  scan_enable = 1'b0;
  test_mode   = 1'b0;
  
  i_wb_cyc = 1'b0;
  i_wb_stb = 1'b0;
  i_wb_we  = 1'b0;
	


	@(posedge ser_clk) begin reset <= 1'b1; end // reset and wait a few clock cycles
	@(negedge ser_clk) begin reset <= 1'b0; end
	@(posedge ser_clk);
	@(negedge ser_clk);
	for (i=0; i<`NUM_TESTS; i=i+1) begin // number of tests is configured
		for (channel=0; channel<32; channel=channel+1) begin // loops through all 32 channels
			data_in_vec = $random; // data that is output is pseudorandomly generated
			wb_write_strobe({16'h0000,4*channel[15:0]}, {24'h000000, data_in_vec}); // write the data to the current channel's register
			if ((channel==0)&&(i==0)) begin
				@(posedge ser_clk) begin frame_sync_in <= 1'b1; end // set frame sync high for channel 0 initially
			end else begin
				@(posedge ser_clk) begin frame_sync_in <= 1'b0; end // otherwise, it's 0
			end
			for (bits=7; bits>=0; bits=bits-1) begin
				@(negedge ser_clk) begin data_out_reg[bits] = data_out; end // capture the output data on the serial line on the negative edge of the clock (similar to tdmi)
			end
			if (data_out_reg !== data_in_vec) begin // if the output doesn't match the data that was input, an error occured
				$display("%t ERROR: failure found when truth_data = %h, channel = %d. Is %h but should be %h",
			  	$time, data_in_vec, channel, data_out_reg, data_in_vec); 
				`ifdef ERRORSTOP
					$stop;
				`endif
			end
		end
	end

	// This loop generates pseudo random frame syncs while feeding input data in the same fashion as the loop above
	for (i=0; i<`NUM_TESTS; i=i+1) begin
		trigger <= 1'b1; // trigger ensures that the frame sync only happens once per test (i.e. once per i)
		rand_bit <= $random; // frame sync happens on a random bit (0-7)
		rand_ch <= $random; // frame sync happens on a random channel (0-31)
		frame_sync_in <= 1'b0; // set frame sync to 0 initially
		for (channel=0; channel<32; channel=channel+1) begin
			data_in_vec = $random; // random data is input
			if (channel == 0) begin
				prev_ch0 = data_in_vec; // keeps track of the last written value for channel 0, so that when frame sync occurs, a comparison can be made
			end
			wb_write_strobe({16'h0000,4*channel[15:0]}, {24'h000000, data_in_vec}); // write the data to the current channel's register
			if ((channel==1) && (frame_sync_in==1)) begin
				@(posedge ser_clk) begin frame_sync_in <= 1'b0; end // if frame sync happened previously, bring it back down to 0 since we're starting the next channel
			end
			for (bits=7; bits>=0; bits=bits-1) begin
				if ((channel == rand_ch)&&(bits == rand_bit)&&(trigger == 1'b1)) begin // if the current channel & bit count are equal to the random targets generated, set frame sync high
					@(posedge ser_clk) begin
						frame_sync_in <= 1'b1;
						bits <= 7; // reset the bit and channel counters, and change the input vector back to last written ch0 value
						channel <= 0;
						trigger <= 1'b0;
						data_in_vec <= prev_ch0;
					end
				end
				@(negedge ser_clk) begin data_out_reg[bits] = data_out; end // capture the data on falling edge of clk
			end
			if (data_out_reg !== data_in_vec) begin // data comparison
				$display("%t ERROR: failure found when truth_data = %h, channel = %d. Is %h but should be %h",
			  	$time, data_in_vec, channel, data_out_reg, data_in_vec); 
				`ifdef ERRORSTOP
					$stop;
				`endif
			end
		end
	end

  #10 $display("%t TEST COMPLETE", $time);
  $finish;
end // initial begin


task wb_write_strobe;
	
	input [31:0] wb_waddress_test;	// the register address to write to
	input [31:0] wb_data_test;		// the data to write to that register

	begin
		@(posedge clk) begin			// wait for a falling edge (a known state)
			i_wb_adr <= wb_waddress_test;	// write the desired address to the wishbone bus
			i_wb_cyc <= 1'b1;			// set cycle high to indicate the start of a write
			i_wb_stb <= 1'b1;			// set strobe high to indicate the start of a write
			i_wb_we  <= 1'b1;			// set write enable high (write operation)
			i_wb_dat <= wb_data_test;	// transfer the data to the wishbone bus
		end


		@(posedge clk) begin
			i_wb_we  <= 1'b0;			// set write enable back to 0
			i_wb_cyc <= 1'b0;			// set cycle low to indicate the end of the write
			i_wb_stb <= 1'b0;			// set strobe low to indicate the end of the write
		end
	end
endtask

task wb_read_strobe;
	
	input [31:0] wb_address_test;				// The address being read from

	begin
		@(posedge clk) begin
			i_wb_adr <= wb_address_test;	// write the desired address to the wishbone bus
			i_wb_cyc <= 1'b1;			// set cycle high to indicate the start of a read
			i_wb_stb <= 1'b1;			// set strobe high to indicate the start of a read
			i_wb_we  <= 1'b0;			// set write enable low (read operation)
		end

		@(posedge clk);

		@(posedge clk) begin
			i_wb_cyc <= 1'b0;
			i_wb_stb <= 1'b0;
		end
	end
endtask


endmodule
