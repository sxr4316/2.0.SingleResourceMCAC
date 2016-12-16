/*

Description		: Time Division Multiplexer In. Takes in a 32-channel 8-bit serial input, MSb first, and 
			: outputs it onto a wishbone data bus. The output runs through channels 0-31, and repeats.
			: A frame sync resets it to channel 0, bit 7.

Author		: David Malanga

Revision History	:
//----------------------------------------------------------------------------------
2/15/16	-	by Dave Malanga	-	Initial creation
2/29/16	-	by Dave Malanga	-	Corrected timing errors, made reset asynchronous, added wishbone interface
3/3/16	-	by Dave Malanga	-	Fixed ack signal output (only high when data is ready to be read).
//----------------------------------------------------------------------------------

*/
module TDMI (
		input reset,
		input ser_clk,
		input data_in,
		input frame_sync,
		input scan_in0,
		input scan_in1,
		input scan_in2,
		input scan_in3,
		input scan_in4,
		input scan_enable,
		input test_mode,
		output reg new_data_int,
		output scan_out0,
		output scan_out1,
		output scan_out2,
		output scan_out3,
		output scan_out4,

		// The Wishbone Interface - Names are organized as follows:
		//  i_ or o_	= input or output
		//  wb_		= wishbone
		//  [sig]		= signal name
		input		clk,
		input       [31:0]          i_wb_adr,	// Wishbone Address - The ms 16 bits are the slave address (unused by the slave - can be ignored). The ls 16 bits is a sub-address for registers within the slave.
		input       [3:0] 	    i_wb_sel,	// Wishbone Select - A register that indicates which part(s) of the data bus should contain valid data.
		input                       i_wb_we,	// Wishbone Write Enable - set high for a write, set low for a read.
		output reg  [31:0] 	    o_wb_dat,	// Wishbone Data Output - output data bus.
		input       [31:0] 	    i_wb_dat,	// Wishbone Data Input - input data bus.
		input                       i_wb_cyc,	// Wishbone Cycle - Set high when there is a bus cycle in progress.
		input                       i_wb_stb,	// Wishbone Strobe - set high when the slave is selected. Only responds to the rest of the signals when this is high.
		output	                o_wb_ack,	// Wishbone Acknowledge - Set high to indicate the end of a bus cycle
		output                      o_wb_err	// Wishbone Error - Set high when there is an abnormality in the bus cycle. This is not used by the TDMI/TDMO interface, so it is set low.
);

`include "../include/register_addresses.vh"		// Include directory for register address definitions


wire 	 sys_clk, serial_clk;		// clock wires (outputs of the clk buffers)
wire	 selected_clk;			// output of teh test_mode mux

reg [4:0] 	ch_count;			// Counter to keep track of the current channel
reg [2:0] 	bit_count;			// Counter to keep track of the number of bits transferred in the current channel
reg		prev_frame_sync;		// Saves the state of the previous frame sync (for detecting the positive edge)
reg [7:0]	data_out_buf;		// Register to write the value of data_in to, bit-by-bit
reg [4:0] 	ch_buf_1, ch_buf_2;	// channel buffer registers for clock domain crossing
reg [7:0] 	data_buf_1, data_buf_2; // data buffer registers for clock domain crossing
reg		start_read;
reg		int_buf_1;
reg		int_set;
reg [2:0]	bit_count_buf;
reg		tdmi_begin;
reg		fs_start;

assign o_wb_ack = (i_wb_we) ? (i_wb_stb & i_wb_cyc) : (i_wb_stb & i_wb_cyc & start_read) ;	// as per the wishbone spec, as long as the error and retry signals are not used, ack can be the and of the cycle and strobe inputs

assign o_wb_err = 1'b0;				// this module does not use the error signal, so it is tied to ground

CLKBUFX4 clk_buf_1 (				// clock buffer for system/wishbone clock
	.A(clk),
	.Y(sys_clk)
);

assign selected_clk = (test_mode) ? (clk) : (ser_clk);	// When in test mode, set the serial clock input to the system clock.

CLKINVX4 clk_buf_2 (							// clock buffer for serial clock
	.A(selected_clk),
	.Y(serial_clk)
);


// TDM portion of the module. Grabs the bit on the data_in line and stores it upon every rising edge of the clock. Once 8 bits have been 
// registered, the bits are all moved simultaneously to the output register. A channel counter keeps track of which channel the data belongs to.
always@ (posedge serial_clk, posedge reset) begin
	if (reset) begin						// reset the channel and bit counters.
		ch_count		<= 5'b00000;
		bit_count		<= 3'b111;			// msb first
		prev_frame_sync 	<= 1'b0;
		data_out_buf 	<= 8'h00;
		fs_start		<= 1'b0;
	end else begin
		if (tdmi_begin) begin
			if ((~prev_frame_sync) & frame_sync) begin	// if previous state of frame sync = 0, and current state of frame sync = 1 --> positive edge is detected
				ch_count <= 5'b00000;			// set the channel count to 0
				data_out_buf[7] <= data_in;		// grab the first bit of data
				bit_count <= 3'b110;			// set the bit counter to the next position (msb-1)
				fs_start <= 1'b1;
			end else begin
				data_out_buf[bit_count] <= data_in;	// transfer the current input bit to the buffer
				bit_count <= bit_count - 3'b001;	// decrement the bit counter
				if (bit_count == 3'b111) begin	// if bit_count is 7 --> a new channel has begun being processed
					ch_count <= ch_count + 5'b00001;	// Increment the channel counter
				end
			end
			prev_frame_sync <= frame_sync;		// save the state of frame_sync for the next clock event
		end
	end
end

// Wishbone Interface portion of the module. Data is transferred through a set of buffer registers, as the clock domain changes.
always@ (posedge sys_clk, posedge reset) begin

	if (reset) begin
		data_buf_1	<= 8'h00;
		data_buf_2	<= 8'h00;
		ch_buf_1	<= 5'b00000;
		ch_buf_2	<= 5'b00000;
		o_wb_dat	<= 32'h00000000;
		int_buf_1   <= 1'b0;
		int_set	<= 1'b0;
		new_data_int <= 1'b0;
		start_read <= 1'b0;
		bit_count_buf <= 3'b000;
		tdmi_begin	<= 1'b0;
	end else begin
		if (tdmi_begin) begin
			bit_count_buf <= bit_count;
			if ((~prev_frame_sync) & frame_sync) begin
				int_buf_1 <= 1'b0;
				int_set <= int_set;
			end else if ((bit_count_buf != 3'b111) || (bit_count != 3'b111)) begin
				int_set <= 1'b0;
				int_buf_1 <= int_buf_1;
			end else if (~int_set & fs_start) begin
				int_buf_1 <= 1'b1;	// set the interrupt flag high
				int_set <= 1'b1;
			end
			new_data_int <= int_buf_1;
			if (bit_count == 3'b111) begin
				ch_buf_1 <= ch_count;	// Transfer the relevant data to the first set of buffer registers
				data_buf_1 <= data_out_buf; // Transfer the relevant data to the first set of buffer registers
			end
			data_buf_2 <= data_buf_1;	// Transfer the relevant data to the second set of buffer registers
			ch_buf_2 <= ch_buf_1;
		end
		if (i_wb_stb & i_wb_cyc & (~i_wb_we)) begin	// If the strobe is high, this is the slave device that is selected. Otherwise, the outputs for the wishbone are untouched.
			case (i_wb_adr[15:0])				// Check the sub-address, and transfer the corresponding registers to the wishbone data bus.
				TDMI_ALL:
					begin
						o_wb_dat <= {19'h00000, ch_buf_2, data_buf_2};	// Transfers both the channel and the data to the wishbone bus.
					end
				TDMI_CHANNEL:
					begin
						o_wb_dat <= {27'h0000000, ch_buf_2[4:0]};		// Transfers only the channel to the wishbone bus.
					end
				TDMI_DATA:
					begin
						o_wb_dat <= {24'h00000, data_buf_2};		// Transfers only the data to the wishbone bus.
					end
				default:
					begin
						o_wb_dat <= {19'h00000, ch_buf_2, data_buf_2};	// Defaults to transferring both the data and channel number.
					end
			endcase
			start_read <= 1'b1;
			int_buf_1 <= 1'b0;	// reset the interrupt pin
		end else if (i_wb_stb & i_wb_cyc & i_wb_we) begin
			case (i_wb_adr[15:0])
				TDMI_START:
					begin
						tdmi_begin <= i_wb_dat[0];
					end
				default:	tdmi_begin <= 1'b0;
			endcase
			start_read <= 1'b0;
		end else begin
			start_read <= 1'b0;
		end
	end
end


endmodule // TDMI
