//////////////////////////////////////////////////////////////////
//                                                              //
//  8KBytes SRAM configured with boot software                  //
//                                                              //
//  This file is part of the Amber project                      //
//  http://www.opencores.org/project,amber                      //
//                                                              //
//  Description                                                 //
//  Holds just enough software to get the system going.         //
//  The boot loader fits into this 8KB embedded SRAM on the     //
//  FPGA and enables it to load large applications via the      //
//  serial port (UART) into the DDR3 memory                     //
//                                                              //
//  Author(s):                                                  //
//      - Conor Santifort, csantifort.amber@gmail.com           //
//                                                              //
//////////////////////////////////////////////////////////////////
//                                                              //
// Copyright (C) 2010 Authors and OPENCORES.ORG                 //
//                                                              //
// This source file may be used and distributed without         //
// restriction provided that this copyright statement is not    //
// removed from the file and that any derivative work contains  //
// the original copyright notice and the associated disclaimer. //
//                                                              //
// This source file is free software; you can redistribute it   //
// and/or modify it under the terms of the GNU Lesser General   //
// Public License as published by the Free Software Foundation; //
// either version 2.1 of the License, or (at your option) any   //
// later version.                                               //
//                                                              //
// This source is distributed in the hope that it will be       //
// useful, but WITHOUT ANY WARRANTY; without even the implied   //
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      //
// PURPOSE.  See the GNU Lesser General Public License for more //
// details.                                                     //
//                                                              //
// You should have received a copy of the GNU Lesser General    //
// Public License along with this source; if not, download it   //
// from http://www.opencores.org/lgpl.shtml                     //
//                                                              //
//////////////////////////////////////////////////////////////////


module boot_mem128 #(
parameter WB_DWIDTH   = 128,
parameter WB_SWIDTH   = 16,
parameter MADDR_WIDTH = 10
)(

input clk,
input reset,
input scan_in0,
input scan_in1,
input scan_in2,
input scan_in3,
input scan_in4,
input scan_enable,
input test_mode,
output scan_out0,
output scan_out1,
output scan_out2,
output scan_out3,
output scan_out4,

`ifdef verilog
input scan_in,
input test_se,
output scan_out,
`endif

input       [31:0]          i_wb_adr,
input       [WB_SWIDTH-1:0] i_wb_sel,
input                       i_wb_we,
output      [WB_DWIDTH-1:0] o_wb_dat,
input       [WB_DWIDTH-1:0] i_wb_dat,
input                       i_wb_cyc,
input                       i_wb_stb,
output                      o_wb_ack,
output                      o_wb_err
);


wire                    start_write;
wire                    start_read;
wire [WB_DWIDTH-1:0]    read_data;
wire [WB_DWIDTH-1:0]    write_data;
wire [WB_SWIDTH-1:0]    byte_enable;
wire [MADDR_WIDTH-1:0]  address;

`ifdef AMBER_WISHBONE_DEBUG
    reg  [7:0]              jitter_r;
    reg  [1:0]              start_read_r;
`else
    reg                     start_read_r;
`endif

// Can't start a write while a read is completing. The ack for the read cycle
// needs to be sent first
`ifdef AMBER_WISHBONE_DEBUG
    assign start_write = i_wb_stb &&  i_wb_we && !(|start_read_r) && jitter_r[0];
`else
    assign start_write = i_wb_stb &&  i_wb_we && !(|start_read_r);
`endif
assign start_read  = i_wb_stb && !i_wb_we && !(|start_read_r);

`ifdef AMBER_WISHBONE_DEBUG
	always @( posedge reset, posedge clk ) begin
		if (reset) begin
			jitter_r <= 8'h0f;
		end else begin
			jitter_r <= {jitter_r[6:0], jitter_r[7] ^ jitter_r[4] ^ jitter_r[1]};
		end
	end

	always @( posedge reset, posedge clk ) begin
		if (reset) begin
			start_read_r <= 'd0;
		end else begin
			if (start_read)
				start_read_r <= {3'd0, start_read};
			else if (o_wb_ack)
				start_read_r <= 'd0;
			else
				start_read_r <= {start_read_r[2:0], start_read};
		end
	end

`else
	always @( posedge reset, posedge clk ) begin
		if (reset) begin
			start_read_r <= 'd0;
		end else begin
			start_read_r <= start_read;
		end
	end
`endif

assign o_wb_err = 1'd0;

assign write_data  = i_wb_dat;
assign byte_enable = i_wb_sel;
assign o_wb_dat    = read_data;
assign address     = i_wb_adr[MADDR_WIDTH+3:4];

`ifdef AMBER_WISHBONE_DEBUG
    assign o_wb_ack    = i_wb_stb && ( start_write || start_read_r[jitter_r[1]] );
`else
    assign o_wb_ack    = i_wb_stb && ( start_write || start_read_r );
`endif


// ------------------------------------------------------
// Instantiate SRAMs
// ------------------------------------------------------
//

`ifndef verilog
	genvar n;
	generate
		for (n=0; n<16; n=n+1) begin : mem_gen
			generic_sram_byte_en_1024x8
			u_mem (
				.A(address),
				.CE(clk),
				.WEB(~(byte_enable[n] & start_write)),
				.OEB(1'b0),
				.CSB(1'b0),
				.I(write_data[8*(n+1)-1:8*n]),
				.O(read_data[8*(n+1)-1:8*n])
			);
		end
	endgenerate
`else
	generic_sram_byte_en #(
		.DATA_WIDTH     ( WB_DWIDTH    ),
		.ADDRESS_WIDTH  ( MADDR_WIDTH  )
	)
	u_mem (
		.clk            ( clk          ),
		.reset          ( reset        ),
		.scan_enable    ( scan_enable  ),
		.test_mode      ( test_mode    ),
		.i_write_enable ( start_write  ),
		.i_byte_enable  ( byte_enable  ),
		.i_address      ( address      ),  // 1024 words, 128 bits
		.o_read_data    ( read_data    ),
		.i_write_data   ( write_data   )
	);
`endif

// =======================================================================================
// =======================================================================================
// =======================================================================================
// Non-synthesizable debug code
// =======================================================================================


//synopsys translate_off
`ifdef XILINX_SPARTAN6_FPGA
    `ifdef BOOT_MEM128_PARAMS_FILE
        initial
            $display("Boot mem file is %s", `BOOT_MEM128_PARAMS_FILE );
    `endif
`endif
//synopsys translate_on
    
endmodule



