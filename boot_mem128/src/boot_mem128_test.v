


`define CLK_PERIOD 20

module test;

wire  scan_out0, scan_out1, scan_out2, scan_out3, scan_out4;

reg  clk, reset;
reg  scan_in0, scan_in1, scan_in2, scan_in3, scan_in4, scan_enable, test_mode;

reg       [31:0]          i_wb_adr;
reg       [15:0] i_wb_sel;
reg                       i_wb_we;
wire      [127:0] o_wb_dat;
reg       [127:0] i_wb_dat;
reg                       i_wb_cyc;
reg                       i_wb_stb;
wire                      o_wb_ack;
wire                      o_wb_err;

integer i;

boot_mem128 top(
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
	.scan_out4(scan_out4),

	.i_wb_adr(i_wb_adr),
	.i_wb_sel(i_wb_sel),
	.i_wb_we(i_wb_we),
	.o_wb_dat(o_wb_dat),
	.i_wb_dat(i_wb_dat),
	.i_wb_cyc(i_wb_cyc),
	.i_wb_stb(i_wb_stb),
	.o_wb_ack(o_wb_ack),
	.o_wb_err(o_wb_err)
    );

always #(`CLK_PERIOD/2) clk = ~clk;

initial begin
	$timeformat(-9,2,"ns", 16);
	`ifdef SDFSCAN
		$sdf_annotate("sdf/boot_mem128_tsmc18_scan.sdf", test.top);
	`endif
	clk <= 1'b0;
	reset <= 1'b0;
	scan_in0 <= 1'b0;
	scan_in1 <= 1'b0;
	scan_in2 <= 1'b0;
	scan_in3 <= 1'b0;
	scan_in4 <= 1'b0;
	scan_enable <= 1'b0;
	test_mode <= 1'b0;

	i_wb_adr <= 'd0;
	i_wb_sel <= 16'h0000;
	i_wb_we  <= 1'b0;
	i_wb_dat <= 'd0;
	i_wb_cyc <= 1'b0;
	i_wb_stb <= 1'b0;

	@(posedge clk) begin
		reset <= 1'b1;
	end

	@(posedge clk);
	
	@(negedge clk) begin
		reset <= 1'b0;
	end

	for (i=0; i<1024; i=i+1) begin
		i_wb_sel <= 16'hFFFF;
		wb_write_strobe({18'h00000, i[9:0], 4'h0}, i);
	end
	for (i=0; i<1024; i=i+1) begin
		i_wb_sel <= 16'hFFFF;
		wb_read_strobe({18'h00000, i[9:0], 4'h0}, 0, 0);
	end

    $finish;
end

task wb_write_strobe;
	
	input [31:0] wb_waddress_test;	// the register address to write to
	input [127:0] wb_data_test;		// the data to write to that register

	begin
		@(posedge clk) begin			// wait for a falling edge (a known state)
			i_wb_adr <= wb_waddress_test;	// write the desired address to the wishbone bus
			i_wb_cyc <= 1'b1;			// set cycle high to indicate the start of a write
			i_wb_stb <= 1'b1;			// set strobe high to indicate the start of a write
			i_wb_we  <= 1'b1;			// set write enable high (write operation)
			i_wb_dat <= wb_data_test;	// transfer the data to the wishbone bus
		end


		@(posedge clk) begin
			i_wb_we  <= 1'b0;			// set write enable back to 0
			i_wb_cyc <= 1'b0;			// set cycle low to indicate the end of the write
			i_wb_stb <= 1'b0;			// set strobe low to indicate the end of the write
		end
	end
endtask

task wb_read_strobe;
	
	input [31:0] wb_address_test;				// The address being read from
	input		 check_for_truth;
	input [31:0] truth_data_test;

	begin
		@(posedge clk) begin
			i_wb_adr <= wb_address_test;	// write the desired address to the wishbone bus
			i_wb_cyc <= 1'b1;			// set cycle high to indicate the start of a read
			i_wb_stb <= 1'b1;			// set strobe high to indicate the start of a read
			i_wb_we  <= 1'b0;			// set write enable low (read operation)
		end

		@(posedge clk);

		@(posedge clk) begin
			i_wb_cyc <= 1'b0;
			i_wb_stb <= 1'b0;
		end

		if (check_for_truth) begin
			if (o_wb_dat !== truth_data_test) begin
				$display("%t ERROR: failure found when truth_data = %h. Is %h but should be %h",
			  	$time, truth_data_test, o_wb_dat, truth_data_test); 
				`ifdef ERRORSTOP
					$stop;
				`endif
			end
		end
	end
endtask


endmodule




