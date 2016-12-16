

module mcac;

enc ENC ();
dec DEC ();

initial
begin
	$display("mcac: enc: A = %d, B = %d", ENC.A, ENC.B);
	$display("mcac: dec: A = %d, B = %d", DEC.A, DEC.B);
end

endmodule
