
`undef ENC
`undef DEC

`define ENC

`include "global_defines.vh"

module enc;

reg [1:0] A, B;

initial
begin
	A = `A;
	B = `B;
	$display("enc: A = %d, B = %d", A, B);
end

endmodule
