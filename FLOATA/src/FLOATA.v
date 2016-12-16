/*
Description: Low level FLOATA block to convert 15-bit or 16-bit signed magnitude to floating point

Author: Swathika Ramakriahnan

Revision History:

//-----------------------------------------------------------------------------------------------------------------------------
2/13/16 - Swathika - Initial creation
2/16/15 - Swathika - modified
2/17/15 - Adam - added a missing parenthesis. Also added back in the full conditionings for EXP from the spec
2/20/16 - swathika - concatenation instead of shift 
3/30/16 - swathika - replaced the blocking statements with the non-blocking statements and added comments
3/31/16 - swathika - replaced the If statements with case statements
//-----------------------------------------------------------------------------------------------------------------------------

*/

module FLOATA (
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
	   DQ0
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

output wire [10:0] 
	DQ0;

wire
	DQS;

wire [14:0]
	MAG;

reg [3:0]
	EXP;

wire [5:0]
	MANT;

// convert 16-bit signed magnitude to floating point	
assign	DQS = DQ[15];
	
// compute magnitude
assign	MAG = (DQ & 32767);
	
//compute exponent
always@ (MAG) begin

	casez (MAG)	
		15'b1??????????????:
			begin
				EXP <= 15;
			end
		15'b01?????????????:
			begin
				EXP <= 14;
			end
		15'b001????????????:
			begin
				EXP <= 13;
			end
		15'b0001???????????:
			begin
				EXP <= 12;
			end
		15'b00001??????????:
			begin
				EXP <= 11;
			end
		15'b000001?????????:
			begin
				EXP <= 10;
			end
		15'b0000001????????:
			begin
				EXP <= 9;
			end
		15'b00000001???????:
			begin
				EXP <= 8;
			end
		15'b000000001??????:
			begin
				EXP <= 7;
			end
		15'b0000000001?????:
			begin
				EXP <= 6;
			end
		15'b00000000001????:
			begin
				EXP <= 5;
			end
		15'b000000000001???:
			begin
				EXP <= 4;
			end
		15'b0000000000001??:
			begin
				EXP <= 3;
			end
		15'b00000000000001?:
			begin
				EXP <= 2;
			end
		15'b000000000000001:
			begin
				EXP <= 1;
			end
		default:
			begin
				EXP <= 0;
			end
	endcase
end

//compute mantissa with a 1 in the most significant bit
assign MANT = (MAG == 0)?32:(({MAG,6'b000000})>>EXP);
	
//combine sign bit, 4 exponent bits and 6 mantissa bits into one 11-bit word
assign	DQ0 = ({DQS,EXP,MANT});

endmodule // FLOATA
