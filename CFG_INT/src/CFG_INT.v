/*

Descrition	: This is the configuration module that writes to or reads from 64 channel memories, 
		  depending on the write/read strobe. Half of the channels is for encoder and the other 
		  half is for decoder. 	  

Author		:  Jason Zhong

Revision History	: Rev 1 2/20/16	Jason Zhong
			: Rev 2 3/11/16 Jason Zhong
			: Rev 3 4/4/16 Adam Steenkamer: Fixed wb_ack to only go high when data is ready
			: Rev 4 4/23/16 Adam Steenkamer: Added registers for core to know when config
				is done and for test bench to know when core is ready for data
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------

*/
module CFG_INT (
          reset,
	  clk,
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
	  rs,
	  ws,
	  cs,
	  w_data,
	  addrs,
	  r_data,
	  i_wb_adr_e,
    	  i_wb_sel_e,
	  i_wb_we_e,
	  o_wb_dat_e,
	  i_wb_dat_e,
	  i_wb_cyc_e,
	  i_wb_stb_e,
	  o_wb_ack_e,
	  o_wb_err_e,
	  i_wb_adr_d,
    	  i_wb_sel_d,
	  i_wb_we_d,
	  o_wb_dat_d,
	  i_wb_dat_d,
	  i_wb_cyc_d,
	  i_wb_stb_d,
	  o_wb_ack_d,
	  o_wb_err_d
       );

`include "../include/register_addresses.vh"

input
    reset;                      // system reset
input clk;
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

input
	rs,
	ws,
	cs;

input [7:0] w_data;
input [6:0] addrs;	
output reg [7:0] r_data;

reg [31:0] encoder_configured, decoder_configured; // registers for keeping track of which registers were configured
reg enc_ready_for_data, dec_ready_for_data; // registers for keeping track of when the cores are ready for data

// The Wishbone Interface - Names are organized as follows:
// i_/o_	= input or output
// wb		= wishbone
// [sig]	= signal name

//----------------Wishbone - Encoder------------------
input	[31:0]	i_wb_adr_e;		// Wishbone Address - The most significant 16 bits are the slave address (unused by the slave). The least significant 16-bits is a sub-address for registers within the slave.
input	[3:0] i_wb_sel_e;	// Wishbone Select - A register that indicates which part(s) of the 
input	i_wb_we_e;			// Wishbone Write Enable - set high for a write, set low for a read.
output reg  [31:0] o_wb_dat_e;	// Wishbone Data Output - output data bus.
input	[31:0] i_wb_dat_e;	// Wishbone Data Input - input data bus.
input	i_wb_cyc_e;			// Wishbone Cycle - Set high when there is a bus cycle in progress.
input	i_wb_stb_e;			// Wishbone Strobe - set high when the slave is selected. Only responds to the rest of the signals when this is high.
output 	reg o_wb_ack_e;		// Wishbone Acknowledge - Set high to indicate the end of a bus cycle
output 	o_wb_err_e;		// Wishbone Error - Set high when there is an abnormality in the bus cycle.

//-----------------Wishbone - Decoder-----------------
input	[31:0]	i_wb_adr_d;	
input	[3:0] i_wb_sel_d;	
input	i_wb_we_d;			
output reg  [31:0] o_wb_dat_d;	
input	[31:0] i_wb_dat_d;	
input	i_wb_cyc_d;			
input	i_wb_stb_d;			
output 	reg o_wb_ack_d;		
output 	o_wb_err_d;		

//----------------------------------------------
reg [2:0] enc_ch [31:0];	//top level 32 3-bit encoder channels
reg [2:0] dec_ch [31:0];	//top level 32 3-bit decoder channels
reg [2:0] buffer_1;
reg [2:0] buffer_2;
reg [2:0] buffer_3;
reg [2:0] buffer_4;

reg       wb_ack2_e;
reg       wb_ack2_d;
wire      wb_ack1_e;
wire      wb_ack1_d;


integer i;	//top level channel counter
integer j;	//encoder channel write counter
integer k;	//decoder channel write counter
reg start_read_e;	//Check wishbone bus data for encoder; 
reg start_read_d;	//Check wishbone bus data for decoder; 

wire read_clk = test_mode ? clk : (cs & rs);

always@ (posedge read_clk or posedge reset) begin
        if (reset) begin
                r_data <= 8'h00;
       end
       else begin
		if(addrs[6]) begin
			if(addrs[5]) begin
				r_data <= {7'b0000000,enc_ready_for_data};
			end
			else begin
				r_data <= {7'b0000000,dec_ready_for_data};
			end
		end
                else if (addrs[5]) begin
                        r_data <= enc_ch[addrs[4:0]];
                end
		else begin
                        r_data <= dec_ch[addrs[4:0]];
                end
        end
end

wire write_clk = test_mode ? clk : ~(cs & ws);

always@ (posedge write_clk or posedge reset) begin
        if (reset) begin
                for (i = 0; i<32; i=i+1) begin
                        enc_ch[i] <= 3'b000;
                        dec_ch[i] <= 3'b000;
                end
		encoder_configured <= 32'h00000000;
		decoder_configured <= 32'h00000000;
        end
        else begin
                if (addrs[5]) begin
                        enc_ch[addrs[4:0]]             <= w_data[2:0];
			encoder_configured[addrs[4:0]] <= 1'b1;
                end else begin
                        dec_ch[addrs[4:0]]             <= w_data[2:0];
			decoder_configured[addrs[4:0]] <= 1'b1;
                end

        end
end

// Wishbone interface - Data is transferred through a set of buffer registers, as the clock domain changes.
//----------------------------------
// Encoder
//----------------------------------
//assign o_wb_ack_e = i_wb_stb_e & i_wb_cyc_e & start_read_e;	// as per the wishbone spec, as long as the error and retry signals are not used, ack can be the and of the cycle and strobe inputs
//assign wb_ack1_e = i_wb_stb_e & i_wb_cyc_e & start_read_e;
assign wb_ack1_e = (i_wb_we_e) ? (i_wb_stb_e & i_wb_cyc_e) : (i_wb_stb_e & i_wb_cyc_e & start_read_e);
assign o_wb_err_e = 1'b0;			// this module does not use the error signal, so it is tied to ground

always@ (posedge clk, posedge reset) begin	
	if (reset) begin
		o_wb_dat_e         <= 32'h00000000;
		start_read_e	   <= 1'b0;
		o_wb_ack_e         <= 1'b0;
		wb_ack2_e          <= 1'b0;
		enc_ready_for_data <= 1'b0;
	end else begin
		if(i_wb_adr_e[15:0] == CONFIG_DONE_REG) begin
			buffer_3 <= {2'b00,&(encoder_configured)}; //reading if all encoder registers are configured
			buffer_4 <= buffer_3;
		end
		else begin 
			buffer_3   <= enc_ch[(i_wb_adr_e[6:0])/4];
			buffer_4   <= buffer_3;
		end		
		wb_ack2_e  <= wb_ack1_e;
		o_wb_ack_e <= wb_ack2_e;
		
		if(i_wb_stb_e & i_wb_cyc_e & i_wb_we_e) begin // if write request
			if(i_wb_adr_e[15:0] == CONFIG_CORE_READY) begin
				enc_ready_for_data <= 1'b1;
			end
		end

		if (i_wb_stb_e & i_wb_cyc_e & (~i_wb_we_e)) begin	// If the strobe is high, this is the slave device that is selected. Otherwise, the outputs for the wishbone are untouched.
			o_wb_dat_e <= {29'h00000000, buffer_4};
			start_read_e <= 1'b1;
		end else begin
			start_read_e <= 1'b0;
		end
	end
end


//----------------------------------
// Decoder
//----------------------------------
//assign o_wb_ack_d = i_wb_stb_d & i_wb_cyc_d & start_read_d;	// as per the wishbone spec, as long as the error and retry signals are not used, ack can be the and of the cycle and strobe inputs
//assign wb_ack1_d = i_wb_stb_d & i_wb_cyc_d & start_read_d;
assign wb_ack1_d = (i_wb_we_d) ? (i_wb_stb_d & i_wb_cyc_d) : (i_wb_stb_d & i_wb_cyc_d & start_read_d);
assign o_wb_err_d = 1'b0;			// this module does not use the error signal, so it is tied to ground
always@ (posedge clk, posedge reset) begin
	if (reset) begin
		o_wb_dat_d         <= 32'h00000000;
		start_read_d	   <= 1'b0;
		o_wb_ack_d         <= 1'b0;
		wb_ack2_d          <= 1'b0;
		dec_ready_for_data <= 1'b0;
	end else begin
		if(i_wb_adr_d[15:0] == CONFIG_DONE_REG) begin
			buffer_1 <= {2'b00,&(decoder_configured)}; //reading if all decoder registers are configured
			buffer_2 <= buffer_1;
		end
		else begin
			buffer_1   <= dec_ch[(i_wb_adr_d[6:0])/4];
			buffer_2   <= buffer_1;
		end
		wb_ack2_d  <= wb_ack1_d;
		o_wb_ack_d <= wb_ack2_d;

		if(i_wb_stb_d & i_wb_cyc_d & i_wb_we_d) begin // if write request
			if(i_wb_adr_d[15:0] == CONFIG_CORE_READY) begin
				dec_ready_for_data <= 1'b1;
			end
		end

		if (i_wb_stb_d & i_wb_cyc_d & (~i_wb_we_d)) begin	// If the strobe is high, this is the slave device that is selected. Otherwise, the outputs for the wishbone are untouched.	
			o_wb_dat_d <= {29'h00000000, buffer_2};
			start_read_d <= 1'b1;
		end else begin
			start_read_d <= 1'b0;
		end		
	end
end

endmodule // CFG_INT
