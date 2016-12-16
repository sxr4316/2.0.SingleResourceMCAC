
`undef ENC
`undef DEC

`define DEC

`include "global_defines.vh"

module dec;

reg [1:0] A, B;

initial
begin
	A = `A;
	B = `B;
	$display("dec: A = %d, B = %d", A, B);
end

endmodule
