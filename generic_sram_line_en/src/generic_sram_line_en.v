//////////////////////////////////////////////////////////////////
//                                                              //
//  Generic Library SRAM with single write enable               //
//                                                              //
//  This file is part of the Amber project                      //
//  http://www.opencores.org/project,amber                      //
//                                                              //
//  Description                                                 //
//  Configurable depth and width.                               //
//                                                              //
//  Author(s):                                                  //
//      - Conor Santifort, csantifort.amber@gmail.com           //
//      - Chris Schwab (modified)                               //
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


module generic_sram_line_en
#(
    parameter DATA_WIDTH            = 21,
    parameter ADDRESS_WIDTH         = 8,
    parameter INITIALIZE_TO_ZERO    = 1 // 0 = skip; 1 = 0; 2 = 1
)

(

    ///////////////////////////////////////////
    // Reset, Clk, and DFT Interface
    //
    input                       reset,          // system reset (active high)
    input                       clk,            // system clock
    
    input                       test_mode,      // DFT test mode control signal
    input                       scan_enable,    // DFT scan enable signal
    input                       scan_in0,       // DFT scan chain input 0
    input                       scan_in1,       // DFT scan chain input 1
    input                       scan_in2,       // DFT scan chain input 2
    input                       scan_in3,       // DFT scan chain input 3
    input                       scan_in4,       // DFT scan chain input 4
    output                      scan_out0,      // DFT scan chain output 0
    output                      scan_out1,      // DFT scan chain output 1
    output                      scan_out2,      // DFT scan chain output 2
    output                      scan_out3,      // DFT scan chain output 3
    output                      scan_out4,      // DFT scan chain output 4

    ///////////////////////////////////////////
    // Inputs and Outputs
    //
    input      [DATA_WIDTH-1:0]     i_write_data,
    input                           i_write_enable,
    input      [ADDRESS_WIDTH-1:0]  i_address,
    output reg [DATA_WIDTH-1:0]     o_read_data

);    

//synopsys translate_off

reg [DATA_WIDTH-1:0]   mem  [0:2**ADDRESS_WIDTH-1];
integer i;
    
always @(posedge clk or posedge reset) begin
    if (reset) begin
      if (INITIALIZE_TO_ZERO) begin
        o_read_data <= {DATA_WIDTH{1'b0}};
        for (i = 0; i < (2**ADDRESS_WIDTH); i = i + 1) begin
            mem[i] <= (INITIALIZE_TO_ZERO == 2) ? {DATA_WIDTH{1'b1}} : {DATA_WIDTH{1'b0}};
        end
      end
    end
    else begin
        // read
        o_read_data <= i_write_enable ? {DATA_WIDTH{1'd0}} : mem[i_address];

        // write
        if (i_write_enable)
            mem[i_address] <= i_write_data;
    end
end
    
//synopsys translate_on

endmodule

