/*

Description : One bit XOR of difference signal sign & delayed difference signal sign

Author : Ghassan Dharb

Revision History :
//----------------------------------------------------------------------------------
2/16/16 - by Ghassan Dharb - Initial creation
2/17/16 - by Adam Steenkamer - Added description, made some of the spacing uniform
//----------------------------------------------------------------------------------

*/


module XOR (
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
           DQn,
           DQ,
           Un
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

input [15:0] DQ;
input [10:0] DQn;

output
    scan_out0,                  // test scan mode data output
    scan_out1,                  // test scan mode data output
    scan_out2,                  // test scan mode data output
    scan_out3,                  // test scan mode data output
    scan_out4,                  // test scan mode data output
    Un;

reg DQS;
reg DQnS;
reg Un;


always @ (DQn, DQ) begin
	DQS = DQ [15];       // DQS is the MSB of DQ
	DQnS = DQn [10];     // DQnS is the MSB of DQn
	Un = DQS ^ DQnS;     // 1 bit exclisive or
end

endmodule // XOR
