/*
*
* Module: Config Module testbench
*
* Author(s):Jason Zhong
*
* Description: Tests the functionality of the CFG_INT block. Assign random values to 
	       the 64 channel registers and then read them back. 
*
* Revision History:
*----------------------------------------------------------------------------------
* 
3/11/16 - by Jason Zhong - Adjusted where the ws signal changes in the write section.
4/4/16 - by Adam Steenkamer - Changed test bench to wait for wb ack signal
4/23/16 - by Adam Steenkamer - Changed test bench to check and test the new registers
				for handshaking between the core and test bench
*----------------------------------------------------------------------------------
*/
//-------------------------------------------------------------------------------------------
// Clock period definitions
`define WB_CLK_PERIOD 4.069		// value is in ns
//-------------------------------------------------------------------------------------------
`define ERRORSTOP

module test;

`include "../include/register_addresses.vh"
//--------------------- Test scan i/o (not used)-----------------------
wire  scan_out0, scan_out1, scan_out2, scan_out3, scan_out4;

reg  clk, reset;
reg  scan_in0, scan_in1, scan_in2, scan_in3, scan_in4, scan_enable, test_mode;


//--------------Config Module Input Registers-------------------------
reg  cs, ws, rs;
reg [7:0] w_data;
wire [7:0] r_data;
reg [6:0] addrs;
reg [31:0] wb_address;

//----------------------------------------------------------------------------------
// Wishbone interface (output) registers

//-------------------Encoder Wishbone ----------------------------------------
reg	[31:0]	i_wb_adr_e;	// Wishbone Address - The most significant 16 bits are the slave address (unused by the slave itself). The least significant 16-bits is a sub-address for registers within the slave.
reg	[3:0]	i_wb_sel_e;	// Wishbone Select - A register that indicates which part(s) of the data bus will be read by the master.
reg		i_wb_we_e;	// Wishbone Write Enable - is set (by master) high for a write, set low for a read.
wire	[31:0]	o_wb_dat_e;	// Wishbone Data Output - output data bus.
reg	[31:0]	i_wb_dat_e;	// Wishbone Data Input - input data bus.
reg		i_wb_cyc_e;	// Wishbone Cycle - is set high when there is a bus cycle in progress.
reg		i_wb_stb_e;	// Wishbone Strobe - is set high when the slave is selected.
wire		o_wb_ack_e;	// Wishbone Acknowledge - Set high by slave to indicate the end of a bus cycle
wire		o_wb_err_e;	// Wishbone Error - Set high when there is an abnormality in the bus cycle. This is not used by the TDMI/TDMO interface, so it is set low.

//-------------------Decoder Wishbone ----------------------------------------
reg	[31:0]	i_wb_adr_d;	
reg	[3:0]	i_wb_sel_d;	
reg		i_wb_we_d;	
wire	[31:0]	o_wb_dat_d;	
reg	[31:0]	i_wb_dat_d;	
reg		i_wb_cyc_d;	
reg		i_wb_stb_d;	
wire		o_wb_ack_d;	
wire		o_wb_err_d;	
//-------------------------For loop values for testbench-----------------------------
integer i;
integer j;
integer k;
integer l;
reg [2:0] data_check;

reg [2:0] enc_ch [31:0];	//top level 32 3-bit encoder channels
reg [2:0] dec_ch [31:0];	//top level 32 3-bit decoder channels
reg [4:0] rand_ch, rand_ch2;

CFG_INT top(
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
	.rs(rs),
	.ws(ws),
	.cs(cs),
	.w_data(w_data),
	.addrs(addrs),
	.r_data(r_data),
	.i_wb_adr_e(i_wb_adr_e),
    	.i_wb_sel_e(i_wb_sel_e),
	.i_wb_we_e(i_wb_we_e),
	.o_wb_dat_e(o_wb_dat_e),
	.i_wb_dat_e(i_wb_dat_e),
	.i_wb_cyc_e(i_wb_cyc_e),
	.i_wb_stb_e(i_wb_stb_e),
	.o_wb_ack_e(o_wb_ack_e),
	.o_wb_err_e(o_wb_err_e),
	.i_wb_adr_d(i_wb_adr_d),
    	.i_wb_sel_d(i_wb_sel_d),
	.i_wb_we_d(i_wb_we_d),
	.o_wb_dat_d(o_wb_dat_d),
	.i_wb_dat_d(i_wb_dat_d),
	.i_wb_cyc_d(i_wb_cyc_d),
	.i_wb_stb_d(i_wb_stb_d),
	.o_wb_ack_d(o_wb_ack_d),
	.o_wb_err_d(o_wb_err_d)
    );

//------------------------create clocks---------------------------
always #(`WB_CLK_PERIOD/2) clk = ~clk;

initial
begin
    $timeformat(-9,2,"ns", 16);
`ifdef SDFSCAN
    $sdf_annotate("sdf/CFG_INT_saed32nm_scan.sdf", test.top);
`endif
    clk = 1'b0;
    scan_in0 = 1'b0;
    scan_in1 = 1'b0;
    scan_in2 = 1'b0;
    scan_in3 = 1'b0;
    scan_in4 = 1'b0;
    scan_enable = 1'b0;
    test_mode = 1'b0;

    cs = 1'b1;
    rs = 1'b0;
    ws = 1'b0;

    w_data = 8'b00000000;
    addrs = 7'b0000000;
    i_wb_cyc_e = 1'b0;
    i_wb_stb_e = 1'b0;
    i_wb_cyc_d = 1'b0;
    i_wb_stb_d = 1'b0;
    i_wb_sel_e = 1'b0;
    i_wb_sel_d = 1'b0;
    wb_address = 32'h00000000;
    data_check = 3'b000;

   // reset config module
   reset = 1'b0;
   @(posedge clk);
   reset = 1'b1;
   repeat(2)
       @(posedge clk);
   reset = 1'b0;
   for (i = 0; i<32; i=i+1) begin
  	enc_ch[i] <= 3'b000;
   	dec_ch[i] <= 3'b000;
   end

//-------------------write to decoder memory----------------
// store random values into the channel memories

	rs = 1'b0;
	for (i=0; i<1000; i = i+1) begin
		ws <= 1'b1;
		repeat(2)
		@(posedge clk);
		w_data[2:0] = $random;
		rand_ch[4:0] = $random;
		addrs = {2'b00, rand_ch[4:0]};
		
		@(posedge clk);
		//$display("WS: w_data is %h", w_data);
		ws <= 1'b0;
		repeat (3)
			@(posedge clk);
		dec_ch[addrs[4:0]] = w_data[2:0];

		@(posedge clk);
		if(i==15) begin // make sure register for config being done hasn't been set
			wb_read_strobe_d (CONFIG_DONE_REG,{29'h00000000, 3'b000});
		end	
	end

	// make sure all decoder registers were configured
	@(posedge clk);
	wb_read_strobe_d (CONFIG_DONE_REG,{29'h00000000, 3'b001});

	
//--------------------write to encoder memory-----------------------
	@(posedge clk);
	for (j=0; j<1000; j=j+1) begin
		ws <= 1'b1;
		repeat (2)
		@(posedge clk);
		rand_ch[4:0] = $random;
		addrs <= {2'b01, rand_ch[4:0]};
		w_data[2:0] <= $random;
		@(posedge clk);
		ws = 1'b0;
		repeat (3)
			@(posedge clk);
		enc_ch[addrs[4:0]] <= w_data[2:0];

		@(posedge clk);
		if(i==15) begin // make sure register for config being done hasn't been set
			wb_read_strobe_e (CONFIG_DONE_REG,{29'h00000000, 3'b000});
		end	
	end

	// make sure all encoder registers were configured
	@(posedge clk);
	wb_read_strobe_e (CONFIG_DONE_REG,{29'h00000000, 3'b001});
	@(posedge clk);

//------------------------Read Decoder--------------------------
	ws = 1'b0;
	addrs[5] = 1'b0;
	$display("THIS HAPPENED: dec_ch0 is %h", dec_ch[0]);
	for(k=0; k<1000; k = k+1) begin
		rand_ch[4:0] <= $random;
		data_check <= dec_ch[rand_ch[4:0]];
		addrs[4:0] <= {2'b00, rand_ch[4:0]};
		@(posedge clk);
		rs = 1'b1;
		@(posedge clk);
		wb_address = 4*addrs[4:0];
		wb_read_strobe_d (wb_address,{29'h00000000, data_check[2:0]});
		rs = 1'b0;
		if (r_data[2:0] !== data_check) begin
			$display("%t DECODER ERROR: failure found. r_data should be %h , but is %h. Address is %h", $time, data_check, r_data, addrs); 
			`ifdef ERRORSTOP
				$stop;
			`endif
		end		
		
	end

//-----------------------Read Encoder--------------------------
	addrs[5] = 1'b1;
	for(l=0; l<1000; l = l+1) begin
		rand_ch[4:0] <= $random;
		data_check <= enc_ch[rand_ch[4:0]];
		addrs[4:0] <= rand_ch[4:0];
		@(posedge clk);
		rs = 1'b1;
		@(posedge clk);
		wb_address = 4*addrs[4:0];
		wb_read_strobe_e (wb_address,{29'h00000000, data_check[2:0]});
		rs = 1'b0;
		if (r_data[2:0] !== data_check) begin
			$display("%t ENCODER ERROR: failure found. r_data should be %h , but is %h", $time, data_check, r_data); 
			`ifdef ERRORSTOP
				$stop;
			`endif
		end
	end


//--------------------Checking core ready register--------------------
	@(posedge clk);
	wb_write_strobe_e(CONFIG_CORE_READY,32'h00000001); // encoder ready for data
	@(posedge clk);
	addrs = 7'b1100000;
	@(posedge clk);
	rs = 1'b1;
	@(posedge clk);
	rs = 1'b0;
	if (r_data !== 1) begin
		$display("%t ENCODER ERROR: Encoder core ready register not correct",$time);
	end
	else begin
		$display("%t Encoder core and config module handshake correct",$time);
	end

	@(posedge clk);
	wb_write_strobe_d(CONFIG_CORE_READY,32'h00000001); // decoder ready for data
	@(posedge clk);
	addrs = 7'b1000000;
	@(posedge clk);
	rs = 1'b1;
	@(posedge clk);
	rs = 1'b0;
	if (r_data !== 1) begin
		$display("%t DECODER ERROR: Decoder core ready register not correct",$time);
	end
	else begin
		$display("%t Decoder core and config module handshake correct",$time);
	end
//-------------------------------------------------------------------
	@(posedge clk);
	@(posedge clk);
	$display("TEST COMPLETE!"); 
   	$finish;   
end //Initial End

//------------------------------------------------------------------------------
//				Wishbone Tasks
//------------------------------------------------------------------------------

//---------------------------Encoder-------------------------------------------
task wb_read_strobe_e;
	
	input [31:0] wb_address_test_e;				// The address being read from
	input [31:0] truth_data_test_e;

	begin

		i_wb_adr_e <= wb_address_test_e;	// write the desired address to the wishbone bus
		i_wb_cyc_e <= 1'b1;			// set cycle high to indicate the start of a read
		i_wb_stb_e <= 1'b1;			// set strobe high to indicate the start of a read
		i_wb_we_e  <= 1'b0;			// set write enable low (read operation)

		@(posedge o_wb_ack_e); // wait for wishbone ack signal from module
		@(posedge clk) begin
			i_wb_cyc_e <= 1'b0;
			i_wb_stb_e <= 1'b0;
		end

		if (o_wb_dat_e !== truth_data_test_e) begin
			$display("%t ENCODER ERROR: failure found when truth_data_e = %h", $time, truth_data_test_e, o_wb_dat_e, truth_data_test_e); 
			`ifdef ERRORSTOP
				$stop;
			`endif
		end

	end
endtask
task wb_write_strobe_e;
	
	input [31:0] wb_waddress_test_e;	// the register address to write to
	input [31:0] wb_data_test_e;		// the data to write to that register

	begin
		@(posedge clk) begin			// wait for a falling edge (a known state)
			i_wb_adr_e <= wb_waddress_test_e;	// write the desired address to the wishbone bus
			i_wb_cyc_e <= 1'b1;			// set cycle high to indicate the start of a write
			i_wb_stb_e <= 1'b1;			// set strobe high to indicate the start of a write
			i_wb_we_e  <= 1'b1;			// set write enable high (write operation)
			i_wb_dat_e <= wb_data_test_e;	// transfer the data to the wishbone bus
		end


		@(posedge clk) begin
			i_wb_we_e  <= 1'b0;			// set write enable back to 0
			i_wb_cyc_e <= 1'b0;			// set cycle low to indicate the end of the write
			i_wb_stb_e <= 1'b0;			// set strobe low to indicate the end of the write
		end
	end
endtask
//---------------------------Decoder-------------------------------------------
task wb_read_strobe_d;
	
	input [31:0] wb_address_test_d;				// The address being read from
	input [31:0] truth_data_test_d;

	begin

		i_wb_adr_d <= wb_address_test_d;	// write the desired address to the wishbone bus
		i_wb_cyc_d <= 1'b1;			// set cycle high to indicate the start of a read
		i_wb_stb_d <= 1'b1;			// set strobe high to indicate the start of a read
		i_wb_we_d  <= 1'b0;			// set write enable low (read operation)

		@(posedge o_wb_ack_d); // wait for wishbone ack signal from module
		@(posedge clk) begin
			i_wb_cyc_d <= 1'b0;
			i_wb_stb_d <= 1'b0;
		end

		if (o_wb_dat_d !== truth_data_test_d) begin
			$display("%t DECODER ERROR: failure found when truth_data_d = %h, and wishbone has %h", $time, truth_data_test_d, o_wb_dat_d); 
			`ifdef ERRORSTOP
				$stop;
			`endif
		end

	end
endtask
task wb_write_strobe_d;
	
	input [31:0] wb_waddress_test_d;	// the register address to write to
	input [31:0] wb_data_test_d;		// the data to write to that register

	begin
		@(posedge clk) begin			// wait for a falling edge (a known state)
			i_wb_adr_d <= wb_waddress_test_d;	// write the desired address to the wishbone bus
			i_wb_cyc_d <= 1'b1;			// set cycle high to indicate the start of a write
			i_wb_stb_d <= 1'b1;			// set strobe high to indicate the start of a write
			i_wb_we_d  <= 1'b1;			// set write enable high (write operation)
			i_wb_dat_d <= wb_data_test_d;	// transfer the data to the wishbone bus
		end


		@(posedge clk) begin
			i_wb_we_d  <= 1'b0;			// set write enable back to 0
			i_wb_cyc_d <= 1'b0;			// set cycle low to indicate the end of the write
			i_wb_stb_d <= 1'b0;			// set strobe low to indicate the end of the write
		end
	end
endtask
endmodule
