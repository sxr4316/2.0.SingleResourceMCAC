/*

Description : Updates a1 coefficient of 2nd order predictor

Author : Anurag Reddy Gankat

Revision History :
//----------------------------------------------------------------------------------
2/16/16 - by Anurag Reddy Gankat - Initial creation
2/17/16 - by Adam Steenkamer - Created actual module description, fixed always block
	spacing, fixed ULA1 logic, removed unnecessary masking (&)
2/20/16 - by Anurag Reddy Gankat - Repalced shift with concatenation
3/30/16 - by Anurag Reddy Gankat - Changed blocking statements to non-blocking statements and added comments.
//----------------------------------------------------------------------------------

*/
module UPA1 (
	input     reset,
	input     clk,
	input     scan_in0,
	input     scan_in1,
	input     scan_in2,
	input     scan_in3,
	input     scan_in4,
	input     scan_enable,
	input     test_mode,
	output    scan_out0,
	output    scan_out1,
	output    scan_out2,
	output    scan_out3,
	output    scan_out4,
//I/O
	input     PK0,
	input     PK1,
	input     [15:0]A1,
	input     SIGPK,
	output    [15:0]A1T
);

   reg           [15:0] UGA1;
   wire          PKS,A1S;
   wire          [15:0] ULA1;
   wire          [15:0] UA1;

//1 bit exclusive or
assign PKS = PK0^PK1;

//SIGPK=0
always @ (SIGPK or PK0 or PK1) begin
	if ( (!PKS) && (!SIGPK))

		UGA1<= 192;

	else if ( (PKS) && (!SIGPK))

		UGA1<= 65344;

	else 
      
		UGA1<=0;

end

assign A1S=A1[15];

assign ULA1= A1S ? (65536-({8'h00, A1[15:8]} + 65280)) : (65536-{8'h00, A1[15:8]});

assign UA1=(UGA1+ULA1);

assign A1T=(A1+UA1);

endmodule // UPA1
