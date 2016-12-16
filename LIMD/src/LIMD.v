/*

Description : Limits on a1 coefficient of 2nd order predictor

Author : Kartik Patel

Revision History :
//----------------------------------------------------------------------------------
2/15/16 - by Kartik Patel - Initial creation
2/17/16 - by Adam Steenkamer - Updated header & added actual description
2/19/16 - by Kartik Patel - Removed the masking and updated.
//----------------------------------------------------------------------------------

*/

module LIMD (
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
           A1T,
           A2P,
           A1P
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

input [15:0]
	A1T;

input [15:0]
	A2P;

output reg [15:0]
       A1P;

 reg [15:0] A1UL;
 reg [15:0] A1LL;
 reg [15:0] OME;

always @(A1T,A2P)begin
                OME   = 16'd15360;
                A1UL  = (OME + 65536 - A2P);                    //Sets the lower thresold
                A1LL  = (A2P + 65536 - OME); 
                if ( 32768 <= A1T && A1T <= A1LL) begin       //Compares with the lower thresold and assigns the values
                A1P = A1LL;
                end 
                else if ( A1UL <= A1T && A1T <= 32767) begin       //Compares with the upper thresold and assigns the values
                A1P = A1UL;
                end 
                else begin
                A1P = A1T;
                end
end 
endmodule // LIMD
