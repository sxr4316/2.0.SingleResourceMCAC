/*
*
* Module: TDMI testbench
*
* Author(s): David Malanga
*
* Description: Tests the functionality of the TDMI block. Emulates the serial input clock and sends a frame sync. Then requests the data by emulating the wishbone interface.
*
* Revision History:
*----------------------------------------------------------------------------------
* 2/23/16 - by Dave Malanga - Initial creation
  2/27/16 - by Dave Malanga - Added wishbone interface functionality
  2/29/16 - by Dave Malanga - Added multiple register testing and random frame sync testing
*----------------------------------------------------------------------------------
*/


//------------------------------------------------------------------------------------------------------------------
// Clock period definitions
`define IN_CLK_PERIOD 488.28125	// value is in ns
`define WB_CLK_PERIOD 12.207		// value is in ns
//------------------------------------------------------------------------------------------------------------------

`define NUM_TESTS 	10
`define NUM_ADDRESSES	3
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
reg			frame_sync;
reg			data_in;
reg	[7:0]		data_in_vec;
reg	[7:0]		data_in_prev;

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

// Interrupt output pin (optional)
wire 			new_data_int;
//------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------
// Variables for Test Generation

// for loop indeces
integer i, channel, bits, adrs;

// registers for random frame_sync 
reg [2:0] rand_bit;
reg [4:0] rand_ch;
reg trigger;

//------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------
// Instantiate the DUT
TDMI top (
  .reset(reset),

  .ser_clk(ser_clk),
  .frame_sync(frame_sync),
  .data_in(data_in),
  .new_data_int(new_data_int),
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
  $sdf_annotate("sdf/TDMI_saed32nm_scan.sdf", test.top);
`endif

  frame_sync  = 1'b0;
  ser_clk     = 1'b0;
  clk	  	  = 1'b0;
  reset       = 1'b0;
  scan_in0    = 1'b0;
  scan_in1    = 1'b0;
  scan_in2    = 1'b0;
  scan_in3    = 1'b0;
  scan_in4    = 1'b0;
  scan_enable = 1'b0;
  test_mode   = 1'b0;
  
  i_wb_cyc = 1'b0;
  i_wb_stb = 1'b0;

	@(posedge ser_clk);
	@(posedge ser_clk) begin reset <= 1'b1; end
	@(negedge ser_clk) begin reset <= 1'b0; end

	repeat (20)	@(posedge ser_clk);

	@(posedge clk) begin
		i_wb_adr <= TDMI_START;		// write the desired address to the wishbone bus
		i_wb_dat <= {31'h00000000, 1'b1};
		i_wb_cyc <= 1'b1;			// set cycle high to indicate the start of a read
		i_wb_stb <= 1'b1;			// set strobe high to indicate the start of a read
		i_wb_we  <= 1'b1;			// set write enable high
	end

	@(posedge clk) begin
		i_wb_cyc <= 1'b0;
		i_wb_stb <= 1'b0;
		i_wb_we  <= 1'b0;
	end

	@(posedge ser_clk);
	
	for (adrs=0; adrs<`NUM_ADDRESSES; adrs=adrs+1) begin
		for (i=0; i<`NUM_TESTS; i=i+1) begin
			for (channel=0; channel<32; channel=channel+1) begin
				data_in_vec <= $random;
				for (bits=7; bits>=0; bits=bits-1) begin
					@(posedge ser_clk) begin
						if ((channel == 0)&&(i==0)) begin
							frame_sync <= 1'b1;
						end else begin
							frame_sync <= 1'b0;
						end					
						data_in <= data_in_vec[bits];
					end
				end
				@(negedge ser_clk);
				case (4*adrs)
					TDMI_ALL:		wb_read_strobe({16'h0000, 4*adrs[15:0]}, {19'h00000, channel[4:0], data_in_vec});
					TDMI_DATA:		wb_read_strobe({16'h0000, 4*adrs[15:0]}, {24'h000000, data_in_vec});
					TDMI_CHANNEL:	wb_read_strobe({16'h0000, 4*adrs[15:0]}, {27'h000000, channel[4:0]});
				endcase
			end
		end
	end

	// test an intermittent frame_sync
	for (i=0; i<`NUM_TESTS; i=i+1) begin
		rand_ch  <= $random;
		rand_bit <= $random;
		trigger <= 1'b0;
		for (channel=0; channel<32; channel=channel+1) begin
			data_in_vec <= $random;
			for (bits=7; bits>=0; bits=bits-1) begin
				@(posedge ser_clk) begin
					if ((channel == rand_ch) && (bits == rand_bit) && (trigger == 1'b0)) begin
						bits = 7;
						channel = 0;
						trigger <= 1'b1;
						frame_sync <= 1'b1;
					end else if ((channel == 1) && (trigger == 1'b1)) begin
						frame_sync <= 1'b0;
					end
					data_in <= data_in_vec[bits];
				end
			end
			@(negedge ser_clk);
			wb_read_strobe(32'h00000000, {24'h000000, data_in_vec});
		end
	end

  #10 $display("%t TEST COMPLETE", $time);
  $finish;
end // initial begin


task wb_read_strobe;
	
	input [31:0] wb_address_test;				// The address being read from
	input [31:0] truth_data_test;

	begin

		@(posedge new_data_int);

		@(posedge clk);

		@(posedge clk) begin
			i_wb_adr <= wb_address_test;	// write the desired address to the wishbone bus
			i_wb_cyc <= 1'b1;			// set cycle high to indicate the start of a read
			i_wb_stb <= 1'b1;			// set strobe high to indicate the start of a read
			i_wb_we  <= 1'b0;			// set write enable low (read operation)
		end

		@(posedge o_wb_ack);

		// Need at least 3 clock cycles for the data to propagate through the buffer registers
		@(posedge clk) begin
			i_wb_cyc <= 1'b0;
			i_wb_stb <= 1'b0;
		end

		if (o_wb_dat !== truth_data_test) begin
			$display("%t ERROR: failure found when truth_data = %h, channel = %d. Is %h but should be %h",
		  	$time, truth_data_test, channel, o_wb_dat, truth_data_test); 
			`ifdef ERRORSTOP
				$stop;
			`endif
		end

	end
endtask

endmodule
