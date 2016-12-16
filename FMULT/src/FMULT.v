/*
Description		: floating point multiplier block in the FMULT/ACCUM. First does
			: floating point conversion for the An/Bn coefficient. Then multiplies
			: An/Bn with SRn/DQn, and converts the product back to two's complement
			: before outputting it as WAn/WBn.

Author		: David Malanga

Revision History	:
//----------------------------------------------------------------------------------
2/12/16	-	by Dave Malanga	-	Initial creation
2/15/16	-	by Dave Malanga	-	Changed sensitivity list to operate on the inputs, and not the clk
2/20/16	-	by Dave Malanga	-	Improved structure of the code, and got rid of unnecessary shifting and masking
2/25/16	-	by Dave Malanga	-	Changed the exponent calculation to a casez statement (reduces worst case timing)
//----------------------------------------------------------------------------------
*/
module FMULT (
input		reset,
input		clk,
input		scan_in0,
input		scan_in1,
input		scan_in2,
input		scan_in3,
input		scan_in4,
input		scan_enable,
input		test_mode,
output		scan_out0,
output		scan_out1,
output		scan_out2,
output		scan_out3,
output		scan_out4,

input  [15:0]	AnBn,
input  [10:0]	SRnDQn,
output [15:0]	WAnWBn
);

// The coefficient input is two's complement, so it needs to be converted into a floating point format prior to the multiplication
wire		AnBnS;
wire [12:0]	AnBnMAG;			// Magnitude of the coefficient
wire [5:0]	AnBnMANT;			// Mantissa of the coefficient (after floating point conversion)
reg  [3:0]	AnBnEXP;			// Exponent of the coefficient

// The signal input is broken down into pieces (DQ/SR is already a floating point number)
wire 		SRnDQnS;
wire [3:0]	SRnDQnEXP;
wire [5:0]	SRnDQnMANT;

// The output product is computed in pieces, and then assembled into the final output
wire 		WAnWBnS;
wire [4:0] 	WAnWBnEXP;
wire [11:0] WAnWBnMANT;
wire [14:0]	WAnWBnMAG;

assign AnBnS = AnBn[15];		// Grabs the sign bits of the inputs
assign SRnDQnS = SRnDQn[10];

assign WAnWBnS = AnBnS ^ SRnDQnS;	// Compute the sign of the output

assign SRnDQnEXP = SRnDQn[9:6];	// Grab the exponent of the floating point input
assign SRnDQnMANT = SRnDQn[5:0];	// Grab the mantissa of the floating point input

assign AnBnMAG = (AnBnS) ? (15'h4000 - {2'b00, AnBn[15:2]}) : (AnBn[15:2]); // Get the absolute value (magnitude) of the coefficient

// Determine the exponent value for the coefficient (floating point conversion)
always@ (AnBnMAG) begin

	casez (AnBnMAG)	// synopsys parallel_case
		13'b1????????????:
			begin
				AnBnEXP <= 4'hD;
			end
		13'b01???????????:
			begin
				AnBnEXP <= 4'hC;
			end
		13'b001??????????:
			begin
				AnBnEXP <= 4'hB;
			end
		13'b0001?????????:
			begin
				AnBnEXP <= 4'hA;
			end
		13'b00001????????:
			begin
				AnBnEXP <= 4'h9;
			end
		13'b000001???????:
			begin
				AnBnEXP <= 4'h8;
			end
		13'b0000001??????:
			begin
				AnBnEXP <= 4'h7;
			end
		13'b00000001?????:
			begin
				AnBnEXP <= 4'h6;
			end
		13'b000000001????:
			begin
				AnBnEXP <= 4'h5;
			end
		13'b0000000001???:
			begin
				AnBnEXP <= 4'h4;
			end
		13'b00000000001??:
			begin
				AnBnEXP <= 4'h3;
			end
		13'b000000000001?:
			begin
				AnBnEXP <= 4'h2;
			end
		13'b0000000000001:
			begin
				AnBnEXP <= 4'h1;
			end
		default:
			begin
				AnBnEXP <= 4'h0;
			end
	endcase
end

// Add the two exponents
assign WAnWBnEXP = AnBnEXP + SRnDQnEXP;

// Computes the mantissa of the product
assign WAnWBnMANT = (AnBnMAG)	? ((({AnBnMAG, 6'b000000} >> AnBnEXP) * SRnDQnMANT) + 12'h030) : (((6'h20) * SRnDQnMANT) + 12'h030);

// Compute the magnitude
assign WAnWBnMAG = (WAnWBnEXP < 27) ? ({WAnWBnMANT[11:4], 7'b0000000} >> (26 - WAnWBnEXP)) : ({WAnWBnMANT[11:4], 7'b0000000} << (WAnWBnEXP - 26));

// Convert output to two's complement
assign WAnWBn = (WAnWBnS) ? (17'h10000 - {2'b00, WAnWBnMAG}) : (WAnWBnMAG);

endmodule // FMULT
