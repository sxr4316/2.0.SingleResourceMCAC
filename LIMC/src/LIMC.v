/*

Description : Limits on a2 coefficient of 2nd order predictor

Author : Kartik Patel

Revision History :
//----------------------------------------------------------------------------------
2/15/16 - by Kartik Patel - Initial creation
2/17/16 - by Adam Steenkamer - Updated header & added actual description
//----------------------------------------------------------------------------------

*/

module LIMC (
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
           A2T,
           A2P
       );

input
    reset,                      // system reset
    clk;                        // system clock

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

input [15:0] A2T;

output reg [15:0] A2P;

 reg [15:0] A2UL;
 reg [15:0] A2LL;
always@(A2T)begin
                A2UL  = 16'h3000;                    //Sets the upper thresold
                A2LL  = 16'hD000;                    // Sets the lower thresold
		if ((16'h8000 <= A2T) && (A2T <= A2LL)) begin       //Compares with the lower thresold and assigns the values to the output
                A2P = A2LL;
                end
                else if ((A2UL <= A2T) && (A2T <= 16'h7FFF)) begin       //Compares with the upper thresold and assigns the values to the output
                A2P = A2UL;
                end 
                else begin
                A2P = A2T;
                end
end
endmodule               // LIMC
