/*

Description : Predictor trigger block

Author : Ghassan Dharb

Revision History :
//----------------------------------------------------------------------------------
2/16/16 - by Ghassan Dharb - Initial creation
2/17/16 - by Adam Steenkamer - Created an actual description, made some spacing uniform
//----------------------------------------------------------------------------------

*/

module TRIGB (
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
           TR,
           AnP,
           AnR
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
    test_mode,                  // test mode 
    TR;

input [15:0] AnP;

output
    scan_out0,                  // test scan mode data output
    scan_out1,                  // test scan mode data output
    scan_out2,                  // test scan mode data output
    scan_out3,                  // test scan mode data output
    scan_out4;                  // test scan mode data output

output [15:0] AnR;
reg [15:0] AnR;

always @ (AnP, TR) begin
	if ( TR == 0) begin
		AnR = AnP;                   // equating AnR and AnP when TR = 0
	end else begin
		AnR = 0;                     // AnR will be 0 when TR = 1             
	end
end

endmodule // TRIGB
