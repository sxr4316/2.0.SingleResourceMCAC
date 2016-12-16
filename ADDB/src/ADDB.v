/*

Description : Adds quantized difference signal & signal estimate to get
    reconstructed signal

Author : Siddharth Ramkrishnan

Revision History :
//----------------------------------------------------------------------------------
2/12/16	-	Siddharth	- Initial creation
2/15/16	-	Siddharth	- Removal of Variable Size of DQ & Changes to improve Test Coverage
2/17/16	-	Adam		- Created an actual description, removed unncessary comma 
3/30/16	-	Siddharth	- Addition of comments to the (already easy to read) operations
//----------------------------------------------------------------------------------

*/

module ADDB (
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
	   DQ,
	   SE,
	   SR
	   );

input
    reset,                      // system reSEt
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
	DQ;

input [14:0]	
	SE;

output wire [15:0]
	SR;

wire
	DQS,
	SES;

wire [15:0] 
	DQI,
	SEI;

assign DQS	= DQ[15];									// Read the sign of DQ and update to DQS

assign DQI	= (DQS == 1'b0)	?	DQ	:	(65536 - ({1'b0,DQ[14:0]}))	;	// Convert DQ from Sign Magnitude to 2's Complement

assign SES	= SE[14];									// Read the sign of SE and update to SES
	
assign SEI	= (SES == 1'b0)	?	SE	:	(32768 + SE)	;			// Convert SE from 15 bit 2's complement to 16bits  

assign SR	= (DQI + SEI);									// 2's complement addition of DQ and SE

endmodule // ADDB
