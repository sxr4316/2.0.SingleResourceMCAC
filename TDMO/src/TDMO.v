/*
Description	: time division multiplexing output (TDMO). There are 32 registers (one
			: per channel) that can be written to through the wishbone interface.
			: The rest of the code loops through each register, outputting the value
			: on the serial data_out line, MSB first. A frame sync resets the output
			: to channel 0.

Author		: David Malanga

Revision History	:
//----------------------------------------------------------------------------------
2/29/16	-	by Dave Malanga	-	Initial creation
3/1/16	-	by Dave Malanga	-	Added wishbone read capability, frame sync input
3/3/15	-	by Dave Malanga	-	Added buffer to ser_clk_out, fixed ack signal.
3/7/16	-	by Dave Malanga	-	Changed data out line to wire, and added grey code counting.
//----------------------------------------------------------------------------------
*/

module TDMO (
		input 		reset,
		input 		ser_clk,
		output 		data_out,
		input		frame_sync_in,
		output		frame_sync_out,
		output		ser_clk_out,

		input 		scan_in0,
		input 		scan_in1,
		input 		scan_in2,
		input 		scan_in3,
		input 		scan_in4,
		input 		scan_enable,
		input 		test_mode,
		output 		scan_out0,
		output 		scan_out1,
		output 		scan_out2,
		output 		scan_out3,
		output 		scan_out4,

		// The Wishbone Interface - Names are organized as follows:
		//  i_ or o_	= input or output
		//  wb_			= wishbone
		//  [sig]		= signal name
		input				clk,
		input       [31:0]	i_wb_adr,	// Wishbone Address - The ms 16 bits are the slave address (unused by the slave - can be ignored). The ls 16 bits is a sub-address for registers within the slave.
		input       [3:0]	i_wb_sel,	// Wishbone Select - A register that indicates which part(s) of the data bus should contain valid data.
		input            	i_wb_we,	// Wishbone Write Enable - set high for a write, set low for a read.
		output reg  [31:0]	o_wb_dat,	// Wishbone Data Output - output data bus.
		input       [31:0]	i_wb_dat,	// Wishbone Data Input - input data bus.
		input             	i_wb_cyc,	// Wishbone Cycle - Set high when there is a bus cycle in progress.
		input           	i_wb_stb,	// Wishbone Strobe - set high when the slave is selected. Only responds to the rest of the signals when this is high.
		output	      		o_wb_ack,	// Wishbone Acknowledge - Set high to indicate the end of a bus cycle
		output            	o_wb_err	// Wishbone Error - Set high when there is an abnormality in the bus cycle. This is not used by the TDMI/TDMO interface, so it is set low.
);

`include "../include/register_addresses.vh"	// Include directory for register address definitions

wire 		serial_clk;		// buffered input serial clk
wire 		serial_clk_inv;	// inverted buffered serial clk
wire 		sys_clk;		// buffered input system/wishbone clk
wire 		selected_clk;	// output of test mode mux

reg	[7:0] data_reg [31:0];	// registers to hold 8 bits data for each channel

reg	[4:0] ch_count;		// register to hold the current channel only
reg	[4:0] next_ch;		// counter to keep the channel

reg	[2:0] bit_count;		// register to hold the current bit only
reg	[2:0] next_bit;		// counter for keeping track of the current bit being transmitted - is in grey code.

reg	[2:0] ch0_bit_count;	// register to hold current bit for frame sync (channel 0) only.
reg	[2:0] next_ch0_bit;	// counter for keeping track of the current bit being transmitted for frame sync (channel 0) only - is in grey code.

reg	[2:0] ch0_grey_count;	// counter for frame syncing (channel 0) only - is used to convert to grey code. This is done to prevent glitches on the data output
reg	[2:0] grey_counter;	// counter for normal bit count - is used to convert to grey code. This is done to prevent glitches on the data output


reg 	[7:0] frame_reg;		// register to hold the 8-bit frame sync (channel 0) data, so that it's synchronous with serial_clk
reg 	[7:0] data_out_reg;	// register to hold the 8-bit data, so that it's synchronous with serial_clk


reg	[7:0] data_buf_1;		// data buffer registers needed for clock domain crossing
reg	[7:0] data_buf_2;		// data buffer registers needed for clock domain crossing 
reg	[7:0] data_out_buf;	// data buffer registers needed for clock domain crossing

reg 	[7:0] ch0_reg_1;		// channel 0 data buffer registers needed for clock domain crossing
reg	[7:0] ch0_reg_2;		// channel 0 data buffer registers needed for clock domain crossing

reg		start_read;		// registers to signal that the data on the wishbone bus is valid



assign o_wb_ack = (i_wb_we) ? (i_wb_stb & i_wb_cyc) : (i_wb_stb & i_wb_cyc & start_read);	// ack for the wishbone - need to wait for data to be put on the output bus to set ack high for a read

assign o_wb_err = 1'b0;				// this module does not use the error signal, so it is tied to ground

assign frame_sync_out = frame_sync_in;	// transfer the frame sync input to the output

assign selected_clk = (test_mode) ? (clk) : (ser_clk);

CLKBUFX4 clk_buf_1 (
	.A(clk),
	.Y(sys_clk)
);

CLKBUFX4 clk_buf_2 (
	.A(selected_clk),
	.Y(serial_clk)
);

CLKBUFX4 clk_buf_3 (
	.A(selected_clk),
	.Y(ser_clk_out)
);

CLKINVX4 clk_buf_4 (
	.A(selected_clk),
	.Y(serial_clk_inv)
);

assign data_out = (reset)		?	(1'b0) :
			((frame_sync_in)	?	(frame_reg[ch0_bit_count])	: (data_out_reg[bit_count])); // switch to channel 0 immediately upon a frame sync

always@ (posedge serial_clk_inv, posedge reset) begin
	if (reset) begin
		next_ch0_bit <= 3'b000;		// everything becomes 0 upon a reset
		next_bit <= 3'b000;
		next_ch <= 5'b00000;
		ch0_grey_count <= 3'b001;	// this is a 1 because bit 0 will already be transmitted by the time this always block is executed (when frame_sync is high)
		grey_counter <= 3'b000;
	end else begin
		if (frame_sync_in) begin	// is frame sync is high --> update channel 0 counter, and reset normal counters
			
			next_ch0_bit[0] <= ch0_grey_count[0] ^ ch0_grey_count[1];	// grey code conversion
			next_ch0_bit[1] <= ch0_grey_count[1] ^ ch0_grey_count[2];
			next_ch0_bit[2] <= ch0_grey_count[2];

			ch0_grey_count <= ch0_grey_count + 3'b001;	// increment the counter for the next clock cycle

			next_ch <= 5'b00001;	// initialize the normal counters
			next_bit <= 3'b000;
			grey_counter <= 3'b001;
		end else begin

			next_bit[0] <= grey_counter[0] ^ grey_counter[1];	// grey code conversion
			next_bit[1] <= grey_counter[1] ^ grey_counter[2];
			next_bit[2] <= grey_counter[2];

			grey_counter <= grey_counter + 3'b001;	// increment the counter for the next clock cycle

			ch0_grey_count <= 3'b001;	// initialize frame sync counter

			if (next_bit == 3'b100) begin
				next_ch <= next_ch + 5'b00001; // increment the channel counter if bit counter resets
			end
		end
	end
end


// TDM portion of the module. Transfers an 8-bit register onto a (serial) data output line, bit-by-bit.
always@ (posedge serial_clk, posedge reset) begin
	if (reset) begin					// if reset --> initialize the channel and bit counters and set the data output line low
		ch_count <= 5'b00000;
		bit_count <= 3'b000;
		ch0_bit_count <= 3'b000;
		data_out_reg <= 8'h00;
		frame_reg <= 8'h00;
	end else begin
		ch0_bit_count <= next_ch0_bit;	// transfer the next counter values into the appropriate registers
		bit_count <= next_bit;
		ch_count <= next_ch;
		frame_reg <= ch0_reg_2;
		data_out_reg <= data_out_buf;
	end
end

// Wishbone Interface portion of the module.
always@ (posedge sys_clk, posedge reset) begin

	if (reset) begin
		data_reg[0]  <= 8'h00; data_reg[1]  <= 8'h00; data_reg[2]  <= 8'h00; data_reg[3]  <= 8'h00;	// initialize all internal channel registers to 0
		data_reg[4]  <= 8'h00; data_reg[5]  <= 8'h00; data_reg[6]  <= 8'h00; data_reg[7]  <= 8'h00;
		data_reg[8]  <= 8'h00; data_reg[9]  <= 8'h00; data_reg[10] <= 8'h00; data_reg[11] <= 8'h00;
		data_reg[12] <= 8'h00; data_reg[13] <= 8'h00; data_reg[14] <= 8'h00; data_reg[15] <= 8'h00;
		data_reg[16] <= 8'h00; data_reg[17] <= 8'h00; data_reg[18] <= 8'h00; data_reg[19] <= 8'h00;
		data_reg[20] <= 8'h00; data_reg[21] <= 8'h00; data_reg[22] <= 8'h00; data_reg[23] <= 8'h00;
		data_reg[24] <= 8'h00; data_reg[25] <= 8'h00; data_reg[26] <= 8'h00; data_reg[27] <= 8'h00;
		data_reg[28] <= 8'h00; data_reg[29] <= 8'h00; data_reg[30] <= 8'h00; data_reg[31] <= 8'h00;
		data_buf_1 <= 8'h00;	// initialize the buffer registers to 0
		data_buf_2 <= 8'h00;
		data_out_buf <= 8'h00;
		ch0_reg_1 <= 8'h00;
		ch0_reg_2 <= 8'h00;
		o_wb_dat <= 32'h00000000;
		start_read <= 1'b0;
	end else begin
		data_out_buf <= data_buf_2;		// shift the data through the buffer registers @ every clock cycle

		data_buf_2[0] <= data_buf_1[7];	// grey code conversion
		data_buf_2[1] <= data_buf_1[6];
		data_buf_2[3] <= data_buf_1[5];
		data_buf_2[2] <= data_buf_1[4];
		data_buf_2[6] <= data_buf_1[3];
		data_buf_2[7] <= data_buf_1[2];
		data_buf_2[5] <= data_buf_1[1];
		data_buf_2[4] <= data_buf_1[0];

		data_buf_1 <= data_reg[next_ch];	// transfer channel data to metastability buffers

		ch0_reg_2[0] <= ch0_reg_1[7];		// grey code conversion
		ch0_reg_2[1] <= ch0_reg_1[6];
		ch0_reg_2[3] <= ch0_reg_1[5];
		ch0_reg_2[2] <= ch0_reg_1[4];
		ch0_reg_2[6] <= ch0_reg_1[3];
		ch0_reg_2[7] <= ch0_reg_1[2];
		ch0_reg_2[5] <= ch0_reg_1[1];
		ch0_reg_2[4] <= ch0_reg_1[0];

		ch0_reg_1 <= data_reg[0];		// transfer channel 0 data to metastability buffers

		if (i_wb_stb & i_wb_cyc & i_wb_we) begin	// a write request
			case (i_wb_adr[15:0])			// depending on the address selected, transfer the data on the wishbone bus to the correct internal register
				TDMO_CH0:	data_reg[0]  <= i_wb_dat[7:0];
				TDMO_CH1:	data_reg[1]  <= i_wb_dat[7:0];
				TDMO_CH2:	data_reg[2]  <= i_wb_dat[7:0];
				TDMO_CH3:	data_reg[3]  <= i_wb_dat[7:0];
				TDMO_CH4:	data_reg[4]  <= i_wb_dat[7:0];
				TDMO_CH5:	data_reg[5]  <= i_wb_dat[7:0];
				TDMO_CH6:	data_reg[6]  <= i_wb_dat[7:0];
				TDMO_CH7:	data_reg[7]  <= i_wb_dat[7:0];
				TDMO_CH8:	data_reg[8]  <= i_wb_dat[7:0];
				TDMO_CH9:	data_reg[9]  <= i_wb_dat[7:0];
				TDMO_CH10:	data_reg[10] <= i_wb_dat[7:0];
				TDMO_CH11:	data_reg[11] <= i_wb_dat[7:0];
				TDMO_CH12:	data_reg[12] <= i_wb_dat[7:0];
				TDMO_CH13:	data_reg[13] <= i_wb_dat[7:0];
				TDMO_CH14:	data_reg[14] <= i_wb_dat[7:0];
				TDMO_CH15:	data_reg[15] <= i_wb_dat[7:0];
				TDMO_CH16:	data_reg[16] <= i_wb_dat[7:0];
				TDMO_CH17:	data_reg[17] <= i_wb_dat[7:0];
				TDMO_CH18:	data_reg[18] <= i_wb_dat[7:0];
				TDMO_CH19:	data_reg[19] <= i_wb_dat[7:0];
				TDMO_CH20:	data_reg[20] <= i_wb_dat[7:0];
				TDMO_CH21:	data_reg[21] <= i_wb_dat[7:0];
				TDMO_CH22:	data_reg[22] <= i_wb_dat[7:0];
				TDMO_CH23:	data_reg[23] <= i_wb_dat[7:0];
				TDMO_CH24:	data_reg[24] <= i_wb_dat[7:0];
				TDMO_CH25:	data_reg[25] <= i_wb_dat[7:0];
				TDMO_CH26:	data_reg[26] <= i_wb_dat[7:0];
				TDMO_CH27:	data_reg[27] <= i_wb_dat[7:0];
				TDMO_CH28:	data_reg[28] <= i_wb_dat[7:0];
				TDMO_CH29:	data_reg[29] <= i_wb_dat[7:0];
				TDMO_CH30:	data_reg[30] <= i_wb_dat[7:0];
				TDMO_CH31:	data_reg[31] <= i_wb_dat[7:0];
				default:	data_reg[0]  <= i_wb_dat[7:0];
			endcase
		end else if (i_wb_stb & i_wb_cyc & (~i_wb_we)) begin // a read request (likely will not be used)
			start_read <= 1'b1;
			case (i_wb_adr[15:0])
				TDMO_CH0:	o_wb_dat[7:0] <= data_reg[0];
				TDMO_CH1:	o_wb_dat[7:0] <= data_reg[1];
				TDMO_CH2:	o_wb_dat[7:0] <= data_reg[2];
				TDMO_CH3:	o_wb_dat[7:0] <= data_reg[3];
				TDMO_CH4:	o_wb_dat[7:0] <= data_reg[4];
				TDMO_CH5:	o_wb_dat[7:0] <= data_reg[5];
				TDMO_CH6:	o_wb_dat[7:0] <= data_reg[6];
				TDMO_CH7:	o_wb_dat[7:0] <= data_reg[7];
				TDMO_CH8:	o_wb_dat[7:0] <= data_reg[8];
				TDMO_CH9:	o_wb_dat[7:0] <= data_reg[9];
				TDMO_CH10:	o_wb_dat[7:0] <= data_reg[10];
				TDMO_CH11:	o_wb_dat[7:0] <= data_reg[11];
				TDMO_CH12:	o_wb_dat[7:0] <= data_reg[12];
				TDMO_CH13:	o_wb_dat[7:0] <= data_reg[13];
				TDMO_CH14:	o_wb_dat[7:0] <= data_reg[14];
				TDMO_CH15:	o_wb_dat[7:0] <= data_reg[15];
				TDMO_CH16:	o_wb_dat[7:0] <= data_reg[16];
				TDMO_CH17:	o_wb_dat[7:0] <= data_reg[17];
				TDMO_CH18:	o_wb_dat[7:0] <= data_reg[18];
				TDMO_CH19:	o_wb_dat[7:0] <= data_reg[19];
				TDMO_CH20:	o_wb_dat[7:0] <= data_reg[20];
				TDMO_CH21:	o_wb_dat[7:0] <= data_reg[21];
				TDMO_CH22:	o_wb_dat[7:0] <= data_reg[22];
				TDMO_CH23:	o_wb_dat[7:0] <= data_reg[23];
				TDMO_CH24:	o_wb_dat[7:0] <= data_reg[24];
				TDMO_CH25:	o_wb_dat[7:0] <= data_reg[25];
				TDMO_CH26:	o_wb_dat[7:0] <= data_reg[26];
				TDMO_CH27:	o_wb_dat[7:0] <= data_reg[27];
				TDMO_CH28:	o_wb_dat[7:0] <= data_reg[28];
				TDMO_CH29:	o_wb_dat[7:0] <= data_reg[29];
				TDMO_CH30:	o_wb_dat[7:0] <= data_reg[30];
				TDMO_CH31:	o_wb_dat[7:0] <= data_reg[31];
				default:	o_wb_dat[7:0] <= data_reg[0];
			endcase
		end else begin
			start_read <= 1'b0;
		end
	end
end



endmodule // TDMO
