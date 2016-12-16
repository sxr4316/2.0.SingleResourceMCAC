
module test;

wire  scan_out0, scan_out1, scan_out2, scan_out3, scan_out4;

reg  clk, reset;
reg  scan_in0, scan_in1, scan_in2, scan_in3, scan_in4, scan_enable, test_mode;

reg clk_count, testfail;

dec top(
        .reset(reset),
        .clk(clk),
        .scan_in0(scan_in0),
        .scan_in1(scan_in1),
        .scan_in2(scan_in2),
        .scan_in3(scan_in3),
        .scan_in4(scan_in4),
        .scan_enable(scan_enable),
        .test_mode(test_mode),
        .scan_out0(scan_out0),
        .scan_out1(scan_out1),
        .scan_out2(scan_out2),
        .scan_out3(scan_out3),
        .scan_out4(scan_out4)
    );


initial
begin
    $timeformat(-9,2,"ns", 16);
`ifdef SDFSCAN
    $sdf_annotate("sdf/dec_tsmc18_scan.sdf", test.top);
`endif
    clk = 1'b0;
    reset = 1'b0;
    scan_in0 = 1'b0;
    scan_in1 = 1'b0;
    scan_in2 = 1'b0;
    scan_in3 = 1'b0;
    scan_in4 = 1'b0;
    scan_enable = 1'b0;
    test_mode = 1'b0;
    $finish;
end

endmodule
