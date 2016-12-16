/*

Descrition : Get sign of addition of quantized difference signal & partial
	signal estimate

Author : Siddharth Ramkrishnan

Revision History :
//----------------------------------------------------------------------------------
2/12/16	-	Siddharth	- Initial creation
2/15/16	-	Siddharth	- Removal of Variable Size of DQ & Changes to improve Test Coverage
2/17/16	-	Adam		- Added actual description of block
3/30/16	-	Siddharth	- Addition of comments to the (already easy to read) operations
//----------------------------------------------------------------------------------

*/

module ADDC (
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
           SEZ,
           PK0,
           SIGPK           
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
	DQ;

input [14:0]
	SEZ;
        
output wire
	PK0,
	SIGPK;

wire
	DQS,
	SEZS;

wire [15:0] 
	DQI,
	DQSEZ,
	SEZI;
	
assign	DQS	=	DQ[15];							// Read the sign of DQ and update to DQS

assign	DQI	=	(DQS == 0)?	DQ	:	(65536 - (DQ & 32767));	// Conversion of Sign Magnitude Number to 2's complement

assign	SEZS	=	SEZ[14];						// Read the sign of SEZ and update to SEZS
	
assign	SEZI	=	(SEZS == 0)?	SEZ	:	((32768) + SEZ);	// Conversion of 15 bit 2's complement to 16 bit representation
	
assign	DQSEZ	=	(DQI + SEZI);						// 2's complement addition of DQ and SEZ
	
assign	PK0	=	DQSEZ[15]; 						// Read the sign of DQSEZ and update to PK0
	
assign	SIGPK	=	(DQSEZ == 0);						// Zero value status of DQSEZ is indicated in SIGPK

endmodule // ADDC
