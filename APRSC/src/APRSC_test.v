
/*

Description : APRSC single vector test bench. It only tests one case based on a file
		created by the single_run_aprsc.pl script

Author : Adam

Revision History :
//----------------------------------------------------------------------------------
3/3/16 - Adam - Initial Creation
//----------------------------------------------------------------------------------

*/
`define SYS_CLK_PERIOD 12.207
`define CO_PROC_PERIOD 48.828

`define ERRORSTOP

module test;

`include "../include/register_addresses.vh"

wire  scan_out0, scan_out1, scan_out2, scan_out3, scan_out4;

reg  clk,CoPrclk,reset;
reg  scan_in0, scan_in1, scan_in2, scan_in3, scan_in4, scan_enable, test_mode;

reg [2*8-1:0]  rates [0:3];      // 4 rates (16kbps, 24kbps, 32kbps, 40kbps)
reg [4*8-1:0]  laws [0:3];       // 2 laws (a-law, u-law)
reg [6*8-1:0]  operations [0:3]; // 3 operations (normal or overload, or I for decoder)
reg [12*8-1:0] types [0:1];      // (reset, homing)
reg [6*8-1:0]  models [0:1];     // dec or enc
integer        rate, law, operation, type, model, j, loop;
reg [23:0] vectorSizes [0:19879];
reg [23:0] testCases [0:19879];

integer k;
integer test_channel;
reg [23:0] channels [0:19879];

//inputs and outputs
reg [15:0] dq;
reg [23:0] dqs[0:636160];
reg tr;
reg [23:0] trs[0:636160];
reg req;
reg [1:0] rate_in;
wire ack;
reg wait_for_ack;

reg  [23:0] ses[0:636160];
reg [14:0] se;
wire [14:0] seExpected = ses[j];

reg [23:0] a2ps[0:636160];
reg [15:0] a2p;
wire [15:0] a2pExpected = a2ps[j];

reg [23:0] srs[0:636160];
reg [15:0] sr;
wire [15:0] srExpected = srs[j];

// The Wishbone Interface - Names are organized as follows:
//  i_ or o_	= input or output
//  wb_		= wishbone
//  [sig]		= signal name
reg	[31:0]	i_wb_adr;	// Wishbone Address - The ms 16 bits are the slave address (unused by the slave - can be ignored). The ls 16 bits is a sub-address for registers within the slave.
reg	[3:0]		i_wb_sel;	// Wishbone Select - A register that indicates which part(s) of the data bus should contain valid data.
reg			i_wb_we;	// Wishbone Write Enable - set high for a write, set low for a read.
wire	[31:0]	o_wb_dat;	// Wishbone Data Output - output data bus.
reg	[31:0]	i_wb_dat;	// Wishbone Data Input - input data bus.
reg			i_wb_cyc;	// Wishbone Cycle - Set high when there is a bus cycle in progress.
reg			i_wb_stb;	// Wishbone Strobe - set high when the slave is selected. Only responds to the rest of the signals when this is high.
wire			o_wb_ack;	// Wishbone Acknowledge - Set high to indicate the end of a bus cycle
wire			o_wb_err;	// Wishbone Error - Set high when there is an abnormality in the bus cycle. This is not used by the TDMI/TDMO interface, so it is set low.


APRSC top(

	.reset(reset),
	.clk(clk),
	.CoPrclk(CoPrclk),
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

// initialize path parameters
initial begin
  rates[0]      = "40";
  rates[1]      = "32";
  rates[2]      = "24";
  rates[3]      = "16";
  laws[0]       = "AA";
  laws[1]       = "MM";  // m for u-Law
  laws[2]       = "AM";
  laws[3]       = "MA";
  operations[0] = "nrm";
  operations[1] = "ovr";
  operations[2] = "i";
  types[0]      = "reset";
  types[1]      = "homing";
  models[0]     = "enc";
  models[1]     = "dec";
end

// create system clock
always #(`SYS_CLK_PERIOD/2) clk = ~clk;

//create Co-processor clock
always #(`CO_PROC_PERIOD/2) CoPrclk = ~CoPrclk;

initial
begin
    $timeformat(-9,2,"ns", 16);
`ifdef SDFSCAN
    $sdf_annotate("sdf/APRSC_saed32nm_scan.sdf", test.top);
`endif
    clk = 1'b0;
    CoPrclk = 1'b0;
    reset = 1'b0;
    scan_in0 = 1'b0;
    scan_in1 = 1'b0;
    scan_in2 = 1'b0;
    scan_in3 = 1'b0;
    scan_in4 = 1'b0;
    scan_enable = 1'b0;
    test_mode = 1'b0;
    wait_for_ack = 1'b1;

    i_wb_adr <= 32'h0000_0000;
    i_wb_sel <= 4'h0;
    i_wb_we <= 1'b0;
    i_wb_dat <= 32'h0000_0000;
    i_wb_cyc <= 1'b0;
    i_wb_stb <= 1'b0;

    j = 0;
    k = 0;
    test_channel = 0;
    dq = 0;
    tr = 0;
    rate_in = 0;
    req = 1'b0;

//reset logic
repeat(4) begin
    @(posedge CoPrclk)
   reset <= 1'b1;
end
   @(posedge CoPrclk)
   reset <= 1'b0;// make sure reset is off

repeat(2) begin
    @(posedge CoPrclk)
   test_mode <= 1'b1;
end
   @(posedge CoPrclk)
   test_mode <= 1'b0;// make sure test mode is off

//reset logic
repeat(4) begin
    @(posedge CoPrclk)
    reset <= 1'b1;
end
   @(posedge CoPrclk)
   reset <= 1'b0;// make sure reset is off

// get the test case from the file
$readmemh({"./APRSCTests.t"},testCases);
model = testCases[0];
type = testCases[1];
operation=testCases[2];
law = testCases[3];
rate = testCases[4];

$display("%s %s %s %s %s law", models[model], rates[rate], operations[operation], types[type], laws[law]);

// load vectors for the specific case
$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[model], "/", types[type],"/", operations[operation],"/",rates[rate],"/","vectorSize.t"}, vectorSizes);
loop = vectorSizes[0];
$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[model], "/", types[type],"/", operations[operation],"/",rates[rate],"/","channel.t"}, channels);
test_channel = channels[0];
$display("Current channel being tested with main vectors is: %d", test_channel);
$display("Other channels being tested with null vectors");           
$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[model], "/", types[type],"/", operations[operation],"/",rates[rate],"/","dq.t"}, dqs);
$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[model], "/", types[type],"/", operations[operation],"/",rates[rate],"/","tr.t"}, trs);
$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[model], "/", types[type],"/", operations[operation],"/",rates[rate],"/","se.t"}, ses);
$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[model], "/", types[type],"/", operations[operation],"/",rates[rate],"/","a2p.t"}, a2ps);
$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[model], "/", types[type],"/", operations[operation],"/",rates[rate],"/","sr.t"}, srs);

@(posedge CoPrclk);

//reset logic
repeat(4)
begin
	@(posedge CoPrclk)
	reset <= 1'b1;
end
@(posedge CoPrclk)
reset <= 1'b0;// make sure reset is off
@(posedge CoPrclk);

// loop through vectors
for (j=0; j<loop; j=j+1) begin
	wb_write_strobe({16'h0000, APRSC_DQ}, {8'h00, dqs[j]});
	wb_write_strobe({16'h0000, APRSC_TR}, {8'h00, trs[j]});
	wb_write_strobe({16'h0000, APRSC_RATE}, rate);
	wb_write_strobe({16'h0000, APRSC_REQ}, 32'h0000_0001);
	repeat(4)
		@(negedge CoPrclk);
	while(wait_for_ack) begin
		wb_read_strobe({16'h0000, APRSC_ACK}, 1'b0, 32'h0000_0000);
		if (o_wb_dat[0] == 1'b1) begin
			wait_for_ack = 1'b0;
		end
		@(posedge clk);
	end
	wait_for_ack = 1'b1;
	wb_write_strobe({16'h0000, APRSC_REQ}, 32'h0000_0000);
	wb_read_se({16'h0000, APRSC_SE}, 1'b1, {8'h00, ses[j]});
	wb_read_sr({16'h0000, APRSC_SR}, 1'b1, {8'h00, srs[j]});
	wb_read_a2p({16'h0000, APRSC_A2P}, 1'b1, {8'h00, a2ps[j]});
	@(posedge CoPrclk);
	wb_write_strobe({16'h0000, APRSC_REQ}, 32'h0000_0000);	
	@(posedge CoPrclk);
end // element loop

#10 $display("%t TEST COMPLETE", $time);
  $finish;
end // initial begin

task wb_write_strobe;
	
	input [31:0] wb_waddress_test;	// the register address to write to
	input [31:0] wb_data_test;		// the data to write to that register

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
				$display("%t TESTBENCHERROR: failure found when truth_data = %h. Is %h but should be %h",
			  	$time, truth_data_test, o_wb_dat, truth_data_test); 
				`ifdef ERRORSTOP
					$finish;
				`endif
			end
		end
	end
endtask

task wb_read_se;
	
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
			se = o_wb_dat;
			if (o_wb_dat !== truth_data_test) begin
				$display("%t TESTBENCHERROR: failure found when SE = %h. Is %h but should be %h",
			  	$time, truth_data_test, o_wb_dat, truth_data_test); 
				`ifdef ERRORSTOP
					$finish;
				`endif
			end
		end
	end
endtask

task wb_read_a2p;
	
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
			a2p = o_wb_dat;
			if (o_wb_dat !== truth_data_test) begin
				$display("%t TESTBENCHERROR: failure found when A2P = %h. Is %h but should be %h",
			  	$time, truth_data_test, o_wb_dat, truth_data_test); 
				`ifdef ERRORSTOP
					$finish;
				`endif
			end
		end
	end
endtask

task wb_read_sr;
	
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
			sr = o_wb_dat;
			if (o_wb_dat !== truth_data_test) begin
				$display("%t TESTBENCHERROR: failure found when SR = %h. Is %h but should be %h",
			  	$time, truth_data_test, o_wb_dat, truth_data_test); 
				`ifdef ERRORSTOP
					$finish;
				`endif
			end
		end
	end
endtask


endmodule


