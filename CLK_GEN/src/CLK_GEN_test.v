/*

Description: Test bench for clock generator module

Author: Adam Steenkamer

Revision History:
//---------------------------------------------------------------------------------------
3/13/16 - by Adam Steenkamer - Initial creation
3/14/16 - by Adam Steenkamer - Added defines for all clock periods & removed serial clk input
				port mapping
3/14/16 - by Adam Steenkamer - Test bench now automatically checks outputs as well
4/8/16 - by Adam Steenkamer - Test bench now generates serial clock and frame sync off
		of system clock
4/20/16 - by Adam Steenkamer - Modified clock frequencies and changed test bench
		to not check the frame sync output the first time since it will be slightly
		off, which doesn't matter since we aren't actually outputting data then
//---------------------------------------------------------------------------------------
*/

`define SYS_CLK_PERIOD 4.069 // 245.761 MHz 3.255 // 307.2 MHz 2.441
`define SER_CLK_PERIOD 488.281 // 2.048 MHz
`define CO_CLK_PERIOD 24.414 // 40.96 MHz 12.207 // 81.92 MHz 48.82 // 20.48 MHz
// sys_clk * 6 = co proc clock
// sys_clk * 120 = serial clock

module test;

parameter PROPDLY = 0;

wire  scan_out0, scan_out1, scan_out2, scan_out3, scan_out4;

reg  clk, reset;
reg  scan_in0, scan_in1, scan_in2, scan_in3, scan_in4, scan_enable, test_mode;

wire i_serial_clk;
reg [7:0] ser_count;
reg ser_clk;

// I/O
reg i_frame_syn;
wire o_sys_clk, o_serial_clk, o_frame_syn, o_co_proc_clk;

// for loops
integer channel, bits, k;
integer exp_channel, exp_bits, exp_k;
reg fs_enabled;

// expected frame sync to compare o_frame_syn to
reg exp_frame_syn;
reg start_fs_check;

// storing times for clocks
integer sys_clk_start, sys_clk_end, co_proc_clk_start, co_proc_clk_end, ser_clk_start, ser_clk_end;

CLK_GEN top(
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
	.i_frame_syn(i_frame_syn),
	.o_sys_clk(o_sys_clk),
	.o_serial_clk(o_serial_clk),
	.o_frame_syn(o_frame_syn),
	.o_co_proc_clk(o_co_proc_clk)
    );

// create system clock
always #(`SYS_CLK_PERIOD*1000/2) clk <= ~clk;

// create serial clock for generating frame sync in test bench
//always #(`SER_CLK_PERIOD*1000/2) i_serial_clk <= ~i_serial_clk;
always @ (posedge clk, posedge reset) begin // generate serial clock from system clock
	if(reset) begin
		ser_clk   <= 1'b0;
		ser_count <= 0;
	end
	else begin
		if (ser_count != 8'h3B) begin // 8'h63) begin
			ser_count <= ser_count + 1'b1;
			ser_clk   <= ser_clk;
		end
		else begin
			ser_count <= 0;
			ser_clk   <= ~ ser_clk;
		end
	end
end

assign i_serial_clk = ser_clk;

initial
begin
    $timeformat(-9,3,"ns",16);
`ifdef SDFSCAN
    $sdf_annotate("sdf/CLK_GEN_saed32nm_scan.sdf", test.top);
`endif
    clk = 1'b0;
    reset = 1'b0;
    i_frame_syn = 1'b0;
    exp_frame_syn = 1'b0;
    scan_in0 = 1'b0;
    scan_in1 = 1'b0;
    scan_in2 = 1'b0;
    scan_in3 = 1'b0;
    scan_in4 = 1'b0;
    scan_enable = 1'b0;
    test_mode = 1'b0;
    fs_enabled = 1'b1; // frame sync enabled to start
    k = 0;
    exp_k = 0;
    bits = 0;
    exp_bits = 0;
    channel = 0;
    exp_channel = 0;

    // reset logic
    @(posedge clk);
    reset <= 1'b1;
    repeat(2)
        @(posedge clk);
    reset <= 1'b0;
    @(posedge clk);

    // time the system clock
    @(posedge o_sys_clk);
    sys_clk_start = $time;
    @(posedge o_sys_clk);
    sys_clk_end = $time;
    @(posedge clk);
    $display("Start of clk period at: %t", sys_clk_start);
    $display("End of clk period at : %t", sys_clk_end);
    $display("Clk period. Expected: %t. Obtained: %t", `SYS_CLK_PERIOD*1000, (sys_clk_end-sys_clk_start));
    if( ((sys_clk_end-sys_clk_start) >  (`SYS_CLK_PERIOD*1000)+5) ||  ((sys_clk_end-sys_clk_start) <  (`SYS_CLK_PERIOD*1000)-5)) begin
        $display("%t ERROR: System clock period not as expected", $time);
`ifdef ERRORSTOP
        $stop;
`endif
    end
    $display("-----------------------------------");

    // time the co-processor clock
    @(posedge o_co_proc_clk);
    co_proc_clk_start = $time;
    @(posedge o_co_proc_clk);
    co_proc_clk_end = $time;
    @(posedge clk);
    $display("Start of co-proc clk period at: %t", co_proc_clk_start);
    $display("End of co-proc clk period at: %t", co_proc_clk_end);
    $display("Co-proc Clk period. Expected: %t. Obtained: %t", `CO_CLK_PERIOD*1000, (co_proc_clk_end-co_proc_clk_start));
    if( ((co_proc_clk_end-co_proc_clk_start) >  (`CO_CLK_PERIOD*1000)+10) ||  ((co_proc_clk_end-co_proc_clk_start) <  (`CO_CLK_PERIOD*1000)-10)) begin
        $display("%t ERROR: Co-processor clock period not as expected", $time);
`ifdef ERRORSTOP
        $stop;
`endif
    end
    $display("-----------------------------------");

    // time the serial clock
    @(posedge o_serial_clk);
    ser_clk_start = $time;
    @(posedge o_serial_clk);
    ser_clk_end = $time;
    @(posedge clk);
    $display("Start of serial clk period at: %t", ser_clk_start);
    $display("End of serial clk period at: %t", ser_clk_end);
    $display("Serial Clk period. Expected: %t. Obtained: %t", `SER_CLK_PERIOD*1000, (ser_clk_end-ser_clk_start));
    if( ((ser_clk_end-ser_clk_start) >  (`SER_CLK_PERIOD*1000)+125) ||  ((ser_clk_end-ser_clk_start) <  (`SER_CLK_PERIOD*1000)-125)) begin
        $display("%t ERROR: Serial clock period not as expected", $time);
`ifdef ERRORSTOP
        $stop;
`endif
    end
    $display("-----------------------------------");

    // let the module run to see frame syncs and such
    repeat(1024)
        @(posedge o_serial_clk);
    
    repeat(35530)//65536)
        @(posedge o_serial_clk);

    // disable frame sync input to make sure output is still being regenerated properly
    fs_enabled = 1'b0;
    repeat(30016)
        @(posedge o_serial_clk);

    // wait a little at end of test so waves are visible
    repeat(4)
        @(posedge clk);
    #10 $display("%t TEST COMPLETE", $time);
    $finish;
end // initial begin

// GENERATE INPUT FRAME SYNC from generated serial clock
initial begin
    // wait for reset logic to finish
    repeat(2)
        @(posedge clk);
    @(posedge clk);

    // generate input frame sync over and over again
    for (k=0;k<1048576;k=k+1) begin
        if(fs_enabled) begin // only generate input frame sync if enabled in test bench
            for (channel = 0; channel<32; channel = channel+1) begin
                for (bits=7; bits>=0; bits = bits-1) begin
                  //  @(posedge i_serial_clk) begin
if(k>10) begin
		            if(channel == 0 && bits == 7) begin
		                i_frame_syn <= #PROPDLY 1'b1;
		            end
		            else if(channel == 0 && bits != 7) begin
		                    i_frame_syn <= #PROPDLY 1'b1;
		            end
		            else if(channel != 0 && bits == 7) begin
		                i_frame_syn <= #PROPDLY 1'b0;
		            end
		            else begin
		                    i_frame_syn <= #PROPDLY 1'b0;
		            end
end
			@(posedge i_serial_clk);
                    //end
                end // for bits
            end // for channel
        end // if frame sync enabled
    end // for k
end // initial begin

// generate expected frame sync based on output serial clock
// it's one serial clock cycle off of the input frame sync
initial begin
    // wait for reset logic to finish
    start_fs_check = 1'b0;
    repeat(2)
        @(posedge clk);
    @(posedge clk);
    // generate frame sync over and over again
    for (exp_k=0;exp_k<1048576;exp_k=exp_k+1) begin
        for (exp_channel = 0; exp_channel<32; exp_channel = exp_channel+1) begin
            for (exp_bits=7; exp_bits>=0; exp_bits = exp_bits-1) begin
              //  @(posedge o_serial_clk) begin
if(k==12) start_fs_check = 1'b1;
if(exp_k>10) begin
                if(exp_channel == 0 && exp_bits == 7) begin
                    exp_frame_syn <= #PROPDLY 1'b1;
                end
                else if(exp_channel == 0 && exp_bits != 7) begin
                        exp_frame_syn <= #PROPDLY 1'b1;
                end
                else if(exp_channel != 0 && exp_bits == 7) begin
                    exp_frame_syn <= #PROPDLY 1'b0;
                end
                else begin
                        exp_frame_syn <= #PROPDLY 1'b0;
                end

end
		@(posedge o_serial_clk);
               // end
            end // for bits
        end // for channel
    end // for k
end // initial begin

// Constantly check output frame sync
initial begin
    // wait for reset logic to finish
    repeat(2)
        @(posedge clk);
    @(posedge clk);
    while(1) begin
        @(posedge clk);
	if(start_fs_check) begin
    	    if(o_frame_syn !== exp_frame_syn) begin
       	     $display("%t ERROR: Frame sync not as expected", $time);
`ifdef ERRORSTOP
           	 $stop;
`endif
            end // frame sync if
	end // fs check if
    end // while
end // initial begin

endmodule
