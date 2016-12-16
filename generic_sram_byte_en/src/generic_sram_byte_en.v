//////////////////////////////////////////////////////////////////
//                                                              //
//  Generic Library SRAM with per byte write enable             //
//                                                              //
//  This file is part of the Amber project                      //
//  http://www.opencores.org/project,amber                      //
//                                                              //
//  Description                                                 //
//  Configurable depth and width. The DATA_WIDTH must be a      //
//  multiple of 8.                                              //
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


module generic_sram_byte_en
#(
    parameter DATA_WIDTH    = 128,
    parameter ADDRESS_WIDTH = 10,
    parameter INITIALIZE_TO_ZERO = 2 // 0 = skip; 1 = 0; 2 = random
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
    input      [DATA_WIDTH/8-1:0]   i_byte_enable,
    output reg [DATA_WIDTH-1:0]     o_read_data

    );                                                     

//synopsys translate_off 

reg [DATA_WIDTH-1:0]   mem  [0:2**ADDRESS_WIDTH-1];
integer i;
integer j;

always @(posedge clk or posedge reset) begin
    if (reset) begin
      if (INITIALIZE_TO_ZERO) begin
        o_read_data <= {DATA_WIDTH{1'b0}};
        for (j = 0; j < (2**ADDRESS_WIDTH); j = j + 1) begin
            mem[j] <= (INITIALIZE_TO_ZERO == 2) ? {DATA_WIDTH{1'b1}} : {DATA_WIDTH{1'b0}};
        end
      end
    end
    else begin
        // read
        o_read_data <= i_write_enable ? {DATA_WIDTH{1'd0}} : mem[i_address];

        // write
        if (i_write_enable) begin
            for (i=0;i<DATA_WIDTH/8;i=i+1) begin
                mem[i_address][i*8+0] <= i_byte_enable[i] ? i_write_data[i*8+0] : mem[i_address][i*8+0] ;
                mem[i_address][i*8+1] <= i_byte_enable[i] ? i_write_data[i*8+1] : mem[i_address][i*8+1] ;
                mem[i_address][i*8+2] <= i_byte_enable[i] ? i_write_data[i*8+2] : mem[i_address][i*8+2] ;
                mem[i_address][i*8+3] <= i_byte_enable[i] ? i_write_data[i*8+3] : mem[i_address][i*8+3] ;
                mem[i_address][i*8+4] <= i_byte_enable[i] ? i_write_data[i*8+4] : mem[i_address][i*8+4] ;
                mem[i_address][i*8+5] <= i_byte_enable[i] ? i_write_data[i*8+5] : mem[i_address][i*8+5] ;
                mem[i_address][i*8+6] <= i_byte_enable[i] ? i_write_data[i*8+6] : mem[i_address][i*8+6] ;
                mem[i_address][i*8+7] <= i_byte_enable[i] ? i_write_data[i*8+7] : mem[i_address][i*8+7] ;
            end
        end
    end                                                
end
   
//synopsys translate_on

endmodule

