/*

Description : CLK_GEN block

Author : Adam Steenkamer / Ghassan Dharb

Revision History :
//----------------------------------------------------------------------------------
3/5/16 - by Ghassan Dharb - Initial creation
3/8/16 - by Ghassan Dharb -made some spacing uniform
3/8/16 - by Ghassan Dharb - All og sys_clk, serial_clk, frame_syn, and co_proc_clk are buffered
3/14/16 - by Adam Steenkamer - Serial clock now re-generated from system clk for TDMO
3/14/16 - by Adam Steenkamer - Added re-generation of frame sync
4/7/16  - by Adam Steenkamer - Fixed an issue where the output frame sync was not starting correctly with first input frame sync
4/8/16 - by Adam Steenkamer - Changed elseif logic in always loops to be in an else statement if not reset.
4/20/16 - by Adam Steenkamer - Fixed the frame sync regeneration
//----------------------------------------------------------------------------------

*/

module CLK_GEN (
           reset,
           clk,
           scan_in0,
           scan_in1,
           scan_in2,
           scan_in3,
           scan_in4,
           scan_enable,
           test_mode,
           scan_out0,
           scan_out1,
           scan_out2,
           scan_out3,
           scan_out4,
           i_frame_syn,
           o_sys_clk,
           o_serial_clk,
           o_frame_syn,
           o_co_proc_clk
);

input
    reset,                      // system reset input
    clk;                        // system clock input

input
    scan_in0,                   // test scan mode data input
    scan_in1,                   // test scan mode data input
    scan_in2,                   // test scan mode data input
    scan_in3,                   // test scan mode data input
    scan_in4,                   // test scan mode data input
    scan_enable,                // test scan mode enable
    test_mode;                  // test mode 

input
    i_frame_syn;		// frame sync input

output
    scan_out0,                  // test scan mode data output
    scan_out1,                  // test scan mode data output
    scan_out2,                  // test scan mode data output
    scan_out3,                  // test scan mode data output
    scan_out4;                  // test scan mode data output

output
    o_sys_clk,			// system clock output
    o_serial_clk,		// serial clock output
    o_frame_syn,		// regenerated frame sync output
    o_co_proc_clk;		// co-processor clock output

// Wires for outputs and buffers
wire o_sys_clk;
wire o_serial_clk; 
wire o_frame_syn;
wire o_co_proc_clk;
wire mux_serial_clk;
wire mux_co_proc_clk;

// Signals for generating the co-processor clock
reg [3:0] co_count;
reg co_proc_clk;

// Signals for generating the serial clock
reg [7:0] ser_count;
reg ser_clk;

// Signals for re-generating the frame sync
reg [7:0] fs_count;
reg first_pass;
reg frame_syn;
reg last_fs;
reg first_fs;

// Buffer the clock lines
CLKBUFX4 sys_clk_buf( // buffer the system clock
	.Y(o_sys_clk),
	.A(clk)
);

CLKBUFX4 frame_syn_buf( // buffer the regenerated frame sync
	.Y(o_frame_syn),
	.A(frame_syn)
);

CLKBUFX4 serial_clk_buf( // buffer output of serial clock mux
	.Y(o_serial_clk),
	.A(mux_serial_clk)
);

CLKBUFX4 co_proc_clk_buf( // buffer output of co-processor clock mux
	.Y(o_co_proc_clk),
	.A(mux_co_proc_clk)
);

// Re-generate the serial clock from the system clock for TDMO
always @ (posedge clk, posedge reset) begin
	if(reset) begin
		ser_clk   <= 1'b0;
		ser_count <= 8'h00;
	end 
	else begin
		if (ser_count != 8'h3B) begin
			ser_count <= ser_count + 1'b1;
			ser_clk   <= ser_clk;
		end
		else begin
			ser_count <= 8'h00;
			ser_clk   <= ~ ser_clk;
		end
	end
end

assign mux_serial_clk = test_mode ? clk : ser_clk;

// Generate the co-processor clock from the system clock
always @ (posedge clk, posedge reset) begin
	if(reset) begin
		co_proc_clk <= 1'b0;
		co_count    <= 4'h0;
	end 
	else begin
		if (co_count != 4'h2) begin
			co_count    <= co_count + 1'b1 ;
			co_proc_clk <= co_proc_clk;
		end
		else begin
			co_count    <= 4'h0;
			co_proc_clk <= ~ co_proc_clk;
		end
	end
end

assign mux_co_proc_clk = test_mode ? clk : co_proc_clk;

always @(posedge mux_serial_clk, posedge reset) begin
	if(reset) begin
		fs_count <= 8'h00;
		frame_syn <= 1'b0;
		first_fs <= 1'b0;
		last_fs <= 1'b0;
		first_pass <= 1'b0;
	end
	else begin
		if(first_fs == 0) begin
			frame_syn <= i_frame_syn;
			fs_count <= 8'h01;
			if(i_frame_syn == 1) begin
				first_fs <= 1'b1;
			end
			else begin
				first_fs <= 1'b0;
			end
			last_fs <= 1'b0;
			first_pass <= 1'b1;
		end
		else if (last_fs == 0 && i_frame_syn == 1 && first_pass == 0) begin
			first_fs <= first_fs;
			frame_syn <= 1'b1;
			fs_count <= 8'h01;
			last_fs <= i_frame_syn;
			first_pass <= first_pass;
		end
		else if (fs_count < 8'h07 || fs_count == 8'hFF) begin
			first_fs <= first_fs;
			frame_syn <= 1'b1;
			fs_count <= fs_count + 1'b1;
			last_fs <= i_frame_syn;
			first_pass <= first_pass;
		end
		else if (first_pass == 1 && fs_count == 8'h07) begin
			first_fs <= first_fs;
			frame_syn <= 1'b1;
			fs_count <= fs_count + 1'b1;
			last_fs <= i_frame_syn;
			first_pass <= first_pass;
		end
		else begin
			first_fs <= first_fs;
			frame_syn <= 1'b0;
			fs_count <= fs_count + 1'b1;
			last_fs <= i_frame_syn;
			if(first_pass == 1) first_pass <= 1'b0;
			else first_pass <= first_pass;
		end
	end
end

endmodule // CLK_GEN
























