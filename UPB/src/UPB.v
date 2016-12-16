/*

Descrition		: Block for updating "B" coefficients in the 6th order predictor

Author		: David Malanga

Revision History	:
//----------------------------------------------------------------------------------
2/13/16	-	Initial creation (not sure what the rate input looks like at this point, may need to be edited).
2/20/16	-	Improved the structure of the code, and got rid of unnecessary shifting and masking.
2/22/16	-	Changed variable name from DQ to DQn
//----------------------------------------------------------------------------------

*/
module UPB (
input			reset,
input			clk,
input	[1:0]		rate_in,
input			Un,
input [15:0]	DQn,
input [15:0]	Bn,
input			scan_in0,
input			scan_in1,
input			scan_in2,
input			scan_in3,
input			scan_in4,
input			scan_enable,
input			test_mode,
output		scan_out0,
output		scan_out1,
output		scan_out2,
output		scan_out3,
output		scan_out4,
output reg [15:0] BnP
	);

wire [15:0] DQnMAG;	// Just the magnitude of DQ (strips the sign bit)

wire 		BnS;		// sign bit of input B

wire [15:0] UBn;

reg [15:0] UGBn;		// Gain factor
reg [15:0] ULBn;		// Leak Factor (feed back input)

assign DQnMAG = {1'b0, DQn[14:0]};	// Assign to the magnitude of DQ

assign BnS = Bn[15];	// Grab the sign bit of the coefficient input

// Updates UGB
always@ (DQnMAG, Un) begin
	if (DQnMAG == 16'h0000) begin
		UGBn <= 16'h0000;		// If DQnMAG is 0, gain factor is 0
	end else if (Un) begin
		UGBn <= 16'hFF80;		// If Un is 1, gain is -1/128.
	end else begin
		UGBn <= 16'h0080;		// If Un is 0, gain is +1/128.
	end
end

// Updates ULB
always@ (Bn, BnS, rate_in) begin
	if (rate_in == 2'b00) begin
		if (BnS) begin
			ULBn <= 17'h10000 - ({9'h1FF, Bn[15:9]});
		end else begin
			ULBn <= 17'h10000 - ({9'h000, Bn[15:9]});
		end
	end else begin
		if (BnS) begin
			ULBn <= 17'h10000 - ({8'hFF, Bn[15:8]});
		end else begin
			ULBn <= 17'h10000 - ({8'h00, Bn[15:8]});
		end
	end
end

assign UBn = UGBn + ULBn;

// Adds the gain to Bn, and updates the output coefficient
always@ (UBn, Bn) begin
	
	BnP <= Bn + UBn;
end

endmodule // UPB
