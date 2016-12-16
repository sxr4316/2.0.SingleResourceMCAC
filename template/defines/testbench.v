

module test;

mcac MCAC();

initial
begin

`undef ENC
`undef DEC

`define ENC
`include "global_defines.vh"
	$display("test: enc: A = %d, B = %d", `A, `B);

`undef ENC
`undef DEC
`define DEC
`include "global_defines.vh"
	$display("test: dec: A = %d, B = %d", `A, `B);

`undef ENC
`undef DEC
`include "global_defines.vh"
	$display("test: undef: A = %d, B = %d", `A, `B);
end

endmodule
