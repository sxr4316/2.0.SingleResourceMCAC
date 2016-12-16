`include "../include/global_timescale.vh"
`include "../include/system_config_defines.vh"
`include "../include/global_defines.vh"

`define TB_LOAD_MEM
`define RESET_MEM_TICKS 2
`define RESET_TICKS 2+`RESET_MEM_TICKS
`define SER_CLK_PERIOD 488.281
`define CLK_PERIOD (1000/`AMBER_CLK_FREQ)
`define AMBER_LOAD_MAIN_MEM

`define AMBER_SIM_CTRL 4

//`define SINGLECHANNEL
`define ERRORCHECK
`define STANDALONE

module test();

`include "../include/debug_functions.vh"
`include "../include/system_functions.vh"
`include "../include/memory_configuration.vh"

reg                     sysrst;
reg [31:0]              clk_count = 'd0;

integer                 log_file;

`ifdef AMBER_LOAD_MAIN_MEM
integer                 main_mem_file;
reg     [31:0]          main_mem_file_address;
reg     [31:0]          main_mem_file_data;
reg     [127:0]         main_mem_file_data_128;
integer                 main_mem_line_count;
reg     [22:0]          mm_ddr3_addr;
`endif

integer                 boot_mem_file;
reg     [31:0]          boot_mem_file_address;
reg     [31:0]          boot_mem_file_data;
reg     [127:0]         boot_mem_file_data_128;
integer                 boot_mem_line_count;
integer                 fgets_return;
reg     [120*8-1:0]     line;
reg     [120*8-1:0]     aligned_line;
integer                 timeout = 0;

// signals for the test uart
wire                    uart0_cts;
wire                    uart0_rx;
wire                    uart0_rts;
wire                    uart0_tx;

reg  [7*8-1:0]          core_str;

wire  scan_out0, scan_out1, scan_out2, scan_out3, scan_out4;

reg  clk, uart_clk, reset;
reg  scan_in0, scan_in1, scan_in2, scan_in3, scan_in4, scan_enable, test_mode;

reg dec_s, dec_s_clk, dec_s_fs;

wire dec_o, dec_o_clk, dec_o_fs;

wire	[31:0]	o_mm_wb_adr;
wire	[15:0]	o_mm_wb_sel;
wire			o_mm_wb_we;       
wire	[127:0]	i_mm_wb_dat;
wire	[127:0]	o_mm_wb_dat;
wire			o_mm_wb_cyc;
wire			o_mm_wb_stb;
wire			i_mm_wb_ack;
wire			i_mm_wb_err;

reg rs, ws, cs;
reg [7:0] w_data;
reg [6:0] addrs;
wire [7:0] r_data;

wire [31:0]	i_wb_adr_e;
wire [15:0]	i_wb_sel_e;
wire		i_wb_we_e;
wire [127:0] o_wb_dat_e;
wire [127:0] i_wb_dat_e;
wire		i_wb_cyc_e;
wire		i_wb_stb_e;
wire		o_wb_ack_e;
wire		o_wb_err_e;
wire [31:0] i_wb_adr_d;
wire [15:0] i_wb_sel_d;
wire		i_wb_we_d;
wire [127:0] o_wb_dat_d;
wire [127:0] i_wb_dat_d;
wire		i_wb_cyc_d;
wire		i_wb_stb_d;
wire		o_wb_ack_d;
wire		o_wb_err_d;

// parameterize the paths
reg [2*8-1:0]  rates [0:3];      // 4 rates (16kbps, 24kbps, 32kbps, 40kbps)
reg [4*8-1:0]  laws [0:3];       // 2 laws (a-law, u-law)
reg [6*8-1:0]  operations [0:3]; // 3 operations (normal or overload, or I for decoder)
reg [12*8-1:0] types [0:1];      // (reset, homing)
reg [6*8-1:0]  models [0:1];     // dec or enc
integer        rate, law, operation, type, model, j;
reg program_start;
integer delay_start;

reg [4:0] ch_count, tdmo_ch_count;
reg [2:0] bit_count, tdmo_bit_count;
reg [23:0] data_in_vec  [0:636160];
reg [23:0] data_out_vec [0:636160];
reg [23:0] vectorSizes  [0:19384];
reg [23:0] testCases [0:19879];
integer test_channel;
reg [23:0] channels [0:19879];
reg [7:0] data_out [31:0];
integer next_channel;
integer i, test_count, tdmo_test_count, loop;

localparam integer UART_BIT_PERIOD = (1000000000 / `AMBER_UART_BAUD);
localparam integer SYS_CLK_PER = `CLK_PERIOD;
localparam integer CLKS_PER_BIT =   (UART_BIT_PERIOD / SYS_CLK_PER);
`define UART_CLK_PERIOD (CLKS_PER_BIT*`CLK_PERIOD)

reg config_done;

dec 
`ifndef NETLIST
#(
	.WB_DWIDTH(128),
	.WB_SWIDTH(16),
	.WB_SLAVES(10)
)
`endif
top (
	.reset(reset),
	.clk(clk),
	.dec_s(dec_s),
	.dec_s_clk(dec_s_clk),
	.dec_s_fs(dec_s_fs),
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
	
	.i_uart0_tx(uart0_tx),
	.o_uart0_cts(uart0_cts),
	.i_uart0_rts(uart0_rts),
	.o_uart0_rx(uart0_rx),
	
	.dec_o(dec_o),
	.dec_o_clk(dec_o_clk),
	.dec_o_fs(dec_o_fs),

	.o_mm_wb_adr(o_mm_wb_adr),
	.o_mm_wb_sel(o_mm_wb_sel),
	.o_mm_wb_we(o_mm_wb_we),
	.i_mm_wb_dat(i_mm_wb_dat),
	.o_mm_wb_dat(o_mm_wb_dat),
	.o_mm_wb_cyc(o_mm_wb_cyc),
	.o_mm_wb_stb(o_mm_wb_stb),
	.i_mm_wb_ack(i_mm_wb_ack),
	.i_mm_wb_err(i_mm_wb_err),

	.o_cfg_wb_adr(i_wb_adr_d),
	.o_cfg_wb_sel(i_wb_sel_d),
	.o_cfg_wb_we(i_wb_we_d),
	.i_cfg_wb_dat(o_wb_dat_d),
	.o_cfg_wb_dat(i_wb_dat_d),
	.o_cfg_wb_cyc(i_wb_cyc_d),
	.o_cfg_wb_stb(i_wb_stb_d),
	.i_cfg_wb_ack(o_wb_ack_d),
	.i_cfg_wb_err(o_wb_err_d)
);

// ======================================
// Instantiate Testbench UART
// ======================================
tb_uart u_tb_uart (
    .clk            ( uart_clk  ),
    .reset          ( reset     ),
    .scan_enable    (scan_enable),
    .test_mode      ( test_mode ),
    .i_uart_cts_n   ( uart0_cts ),          // Clear To Send
    .i_uart_rxd     ( uart0_rx  ),
    .o_uart_rts_n   ( uart0_rts ),          // Request to Send
    .o_uart_txd     ( uart0_tx  )
);

main_mem #(
	.WB_DWIDTH(128),
	.WB_SWIDTH(16)
)
u_main_mem (
	.clk(clk),
	.reset(reset),
	.scan_enable(scan_enable),
	.test_mode(test_mode),
	.i_mem_ctrl(1'b0),  // 0=128MB, 1=32MB
	.i_wb_adr(o_mm_wb_adr),
	.i_wb_sel(o_mm_wb_sel),
	.i_wb_we(o_mm_wb_we),
	.o_wb_dat(i_mm_wb_dat),
	.i_wb_dat(o_mm_wb_dat),
	.i_wb_cyc(o_mm_wb_cyc),
	.i_wb_stb(o_mm_wb_stb),
	.o_wb_ack(i_mm_wb_ack),
	.o_wb_err(i_mm_wb_err)
);


assign o_wb_dat_e[127:96] = o_wb_dat_e[31:0];
assign o_wb_dat_e[95:64]  = o_wb_dat_e[31:0];
assign o_wb_dat_e[63:32]  = o_wb_dat_e[31:0];

assign o_wb_dat_d[127:96] = o_wb_dat_d[31:0];
assign o_wb_dat_d[95:64]  = o_wb_dat_d[31:0];
assign o_wb_dat_d[63:32]  = o_wb_dat_d[31:0];

CFG_INT config_mod (
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
	.i_wb_sel_e(i_wb_sel_e[3:0]),
	.i_wb_we_e(i_wb_we_e),
	.o_wb_dat_e(o_wb_dat_e[31:0]),
	.i_wb_dat_e(i_wb_dat_e[31:0]),
	.i_wb_cyc_e(i_wb_cyc_e),
	.i_wb_stb_e(i_wb_stb_e),
	.o_wb_ack_e(o_wb_ack_e),
	.o_wb_err_e(o_wb_err_e),
	.i_wb_adr_d(i_wb_adr_d),
	.i_wb_sel_d(i_wb_sel_d[3:0]),
	.i_wb_we_d(i_wb_we_d),
	.o_wb_dat_d(o_wb_dat_d[31:0]),
	.i_wb_dat_d(i_wb_dat_d[31:0]),
	.i_wb_cyc_d(i_wb_cyc_d),
	.i_wb_stb_d(i_wb_stb_d),
	.o_wb_ack_d(o_wb_ack_d),
	.o_wb_err_d(o_wb_err_d)
);


always #(`CLK_PERIOD/2) clk = ~clk;

always #(`UART_CLK_PERIOD/2) uart_clk = ~uart_clk;


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

initial begin
    $timeformat(-9,2,"ns", 16);
`ifdef SDFSCAN
    $sdf_annotate("sdf/dec_tsmc065_scan.sdf", test.top);
`endif
	clk = 1'b0;
	dec_s = 1'b0;
	dec_s_fs = 1'b0;
	uart_clk = 1'b0;
	scan_in0 = 1'b0;
	scan_in1 = 1'b0;
	scan_in2 = 1'b0;
	scan_in3 = 1'b0;
	scan_in4 = 1'b0;
	scan_enable = 1'b0;
	test_mode = 1'b0;
	ch_count = 5'd0;
	bit_count = 3'd7;
	test_count = 'd0;
	next_channel = 'd0;


	tdmo_ch_count = 5'd0;
	tdmo_bit_count = 3'd7;
	tdmo_test_count = 'd0;

	reset = 1'd0;
	@(posedge clk)
		reset = #1 1'd1;
	repeat(`RESET_MEM_TICKS)
		@(negedge clk);
	reset = #1 1'd0;

	config_done = 1'b0;
	addrs = 7'h00;
	cs = 1'b1;
	rs = 1'b0;
	ws = 1'b0;
	w_data = 8'h00;


`ifdef STANDALONE
	model = 1;//testCases[0];
	type = 0;//testCases[1];
	operation = 0;//testCases[2];
	law = 0;//testCases[3];
	rate = 0;//testCases[4];
`else
	// get the test case from the file
	$readmemh({"./DECTests.t"},testCases);
	model = testCases[0];
	type = testCases[1];
	operation = testCases[2];
	law = testCases[3];
	rate = testCases[4];
`endif
	$display("%s %s %s %s %s law", models[model], rates[rate], operations[operation], types[type], laws[law]);

	// load vectors for the specific case
	$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[model], "/", types[type],"/", operations[operation],"/",rates[rate],"/","vectorSize.t"}, vectorSizes);
	loop = vectorSizes[0];
`ifndef SINGLECHANNEL
	loop = loop / 32;
`endif
	$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[model], "/", types[type],"/", operations[operation],"/",rates[rate],"/","channel.t"}, channels);
	test_channel = channels[0];
	$display("Current channel being tested with main vectors is: %d", test_channel);
	$display("Other channels being tested with null vectors");           
	$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[model], "/", types[type],"/", operations[operation],"/",rates[rate],"/","input.t"}, data_in_vec);
	$readmemh({"../sw/mc_model/vector_out/",laws[law],"/", models[model], "/", types[type],"/", operations[operation],"/",rates[rate],"/","output.t"}, data_out_vec);

//$fflush(); // flush console to log file

end

reg [7:0] ser_count;
always @ (posedge clk, posedge reset) begin
	if(reset) begin
		dec_s_clk <= 1'b0;
		ser_count <= 8'h00;
	end else if (ser_count != 8'h3B) begin
		ser_count <= ser_count + 1'b1;
		dec_s_clk <= dec_s_clk;
	end else if (ser_count == 8'h3B) begin
		ser_count <= 8'h00;
		dec_s_clk <= ~ dec_s_clk;
	end
end

always @(rate, law) begin
	case (rates[rate])
		"40": w_data[2:1] = 2'b00;
		"32": w_data[2:1] = 2'b01;
		"24": w_data[2:1] = 2'b10;
		"16": w_data[2:1] = 2'b11;
	endcase
	case (laws[law])
		"AA": w_data[0] = 1'b1;
		"MM": w_data[0] = 1'b0;
		"AM": w_data[0] = 1'b0;
		"MA": w_data[0] = 1'b1;
	endcase
	for (next_channel = 0; next_channel<32; next_channel=next_channel+1) begin
		addrs[4:0] = next_channel[4:0];
		ws = 1'b1;
		@(posedge clk);
		ws = 1'b0;
		@(posedge clk);
	end
	config_done = 1'b1;
end

always @(posedge reset, posedge clk) begin
	if (reset) begin
		program_start <= 1'b0;
	end else begin
		if((config_done == 1'b1) && (program_start == 0)) begin
			addrs = 7'b1000000;
			@(posedge clk);
			rs = 1'b1;
			@(posedge clk);
			rs = 1'b0;
			if (r_data == 1) begin
				program_start <= 1'b1;
			end
		end
	end
end


// ======================================
// Counter of system clock ticks        
// ======================================
always @ ( posedge `U_SYSTEM.clk )
    clk_count <= clk_count + 1'd1;


// ======================================
// Data feed into TDMI     
// ======================================
always @(posedge dec_s_clk) begin
	if (reset | ~program_start) begin
		delay_start <= 'd0;
	end else begin
		if (test_count < loop)  begin
			if ((ch_count == 5'd0) /*&& (test_count == 0)*/) begin
				dec_s_fs <= 1'b1;
			end else begin
				dec_s_fs <= 1'b0;
			end
	`ifdef SINGLECHANNEL
			if (ch_count == 0) begin
				dec_s <= data_in_vec[test_count][bit_count];
			end else begin
				dec_s <= 1'b0;
			end
	`else
			dec_s <= data_in_vec[32*test_count + ch_count][bit_count];
	`endif
			bit_count <= bit_count - 1'b1;
			if (bit_count == 3'd0) begin
				ch_count <= ch_count + 1'b1;
				if (ch_count == 5'b11111) begin
					test_count <= test_count + 1'b1;
				end
			end
		end
	end
end



// ======================================
// Capture data from TDMO
// ======================================
reg tdmo_fs_prev;
reg captured_data;
integer tdmo_delay_start;
always @(posedge reset, negedge dec_o_clk) begin
	if (reset | ~program_start) begin
		tdmo_fs_prev <= 1'b0;
		captured_data <= 1'b0;
		tdmo_delay_start <= 'd0;
	end else begin
		if (tdmo_test_count <= loop) begin
			if (dec_o_fs & ~tdmo_fs_prev) begin
				data_out[0][7] <= dec_o;
				tdmo_bit_count <= 3'b110;
				tdmo_ch_count <= 5'b00000;
			end else begin
				data_out[tdmo_ch_count][tdmo_bit_count] <= dec_o;
				tdmo_bit_count <= tdmo_bit_count - 1'b1;
				if (tdmo_test_count == 'd0 && tdmo_ch_count == 5'b11111 && tdmo_bit_count == 3'b001) begin
					tdmo_bit_count <= 3'd7;
					tdmo_test_count <= tdmo_test_count + 1'b1;
					tdmo_ch_count <= 5'b00000;
				end else if (tdmo_bit_count == 3'd0) begin
						captured_data <= 1'b1;
						tdmo_bit_count <= 3'd7;
						tdmo_ch_count <= tdmo_ch_count + 1'b1;
						if (tdmo_ch_count == 5'b11111) begin
							tdmo_test_count <= tdmo_test_count + 1'b1;
							tdmo_ch_count <= 5'b00000;
						end
				end else begin
`ifdef ERRORCHECK
					if (captured_data && (tdmo_bit_count == 3'd7)) begin
`ifdef SINGLECHANNEL
					if ((tdmo_ch_count == 1) && (tdmo_test_count >= 1)) begin
						if (data_out[0] != data_out_vec[tdmo_test_count-1][7:0]) begin
							$display("%t TESTBENCHERROR: failure found for channel 0 and test number %d, when output data = %h but should be %h",$time,tdmo_test_count-1'b1,data_out[0],data_out_vec[tdmo_test_count-1][7:0]); 
							$finish;
						end
					end
`else
					if (tdmo_test_count >= 1) begin
						if (tdmo_ch_count == 0) begin
							if (data_out[31] != data_out_vec[32*tdmo_test_count-33][7:0]) begin
								$display("%t TESTBENCHERROR: failure found for channel %h and test number %d, when output data = %h but should be %h",$time,tdmo_ch_count-1'b1,tdmo_test_count,data_out[31],data_out_vec[32*tdmo_test_count-33][7:0]); 
								$finish;
							end
						end else begin
							if (data_out[tdmo_ch_count-1] != data_out_vec[32*tdmo_test_count+tdmo_ch_count-33][7:0]) begin
								$display("%t TESTBENCHERROR: failure found for channel %h and test number %d, when output data = %h but should be %h",$time,tdmo_ch_count-1'b1,tdmo_test_count,data_out[tdmo_ch_count-1],data_out_vec[32*tdmo_test_count+tdmo_ch_count-33][7:0]); 
								$finish;
							end
							else if ((tdmo_test_count % 100)==0 && tdmo_ch_count == 1) begin // print passing once in a while
								$display("%t Passing tests so far. Currently on test number %d", $time, tdmo_test_count);
								//$fflush(); // flush console to log file
							end
						end
					end
`endif
					end
`endif
					captured_data <= 1'b0;
				end
			end
			tdmo_fs_prev <= dec_o_fs;
		end else begin
			$display("%t TESTBENCHFINISHED: simulation complete",$time);
			$finish;
		end
	end
end


// ======================================
// Initialize Boot Memory
// ======================================
always @ ( posedge `U_SYSTEM.reset ) begin
		repeat(`RESET_MEM_TICKS)
			@ ( negedge `U_SYSTEM.clk );
        $display("Load boot memory from %s", `BOOT_MEM_FILE);
        boot_mem_line_count   = 0;
        boot_mem_file         = $fopen(`BOOT_MEM_FILE,    "r");
        if (boot_mem_file == 0)
            begin
`ifndef NETLIST
            `TB_ERROR_MESSAGE
`endif
            $display("TESTBENCHERROR: Can't open input file %s", `BOOT_MEM_FILE);
            $finish;
            end
        
        if (boot_mem_file != 0)
            begin  
            fgets_return = 1;
            while (fgets_return != 0)
                begin
                fgets_return        = $fgets(line, boot_mem_file);
                boot_mem_line_count = boot_mem_line_count + 1;
                aligned_line        = align_line(line);
                
                // if the line does not start with a comment
                if (aligned_line[120*8-1:118*8] != 16'h2f2f)
                    begin
                    // check that line doesnt start with a '@' or a blank
                    if (aligned_line[120*8-1:119*8] != 8'h40 && aligned_line[120*8-1:119*8] != 8'h00)
                        begin
                        $display("Format ERROR in input file %s, line %1d. Line must start with a @, not %08x", 
                                 `BOOT_MEM_FILE, boot_mem_line_count, aligned_line[118*8-1:117*8]);
`ifndef NETLIST
                        `TB_ERROR_MESSAGE
`endif
                        end
                    
                    if (aligned_line[120*8-1:119*8] != 8'h00)
                        begin
                        boot_mem_file_address  =   hex_chars_to_32bits (aligned_line[119*8-1:111*8]);
                        boot_mem_file_data     =   hex_chars_to_32bits (aligned_line[110*8-1:102*8]);
                        
                        boot_mem_file_data_128 = `U_BOOT_MEM.u_mem.mem[boot_mem_file_address[BOOT_MSB:4]];
                        `U_BOOT_MEM.u_mem.mem[boot_mem_file_address[BOOT_MSB:4]] = 
                              insert_32_into_128 ( boot_mem_file_address[3:2], 
                                                   boot_mem_file_data_128, 
                                                   boot_mem_file_data );
                        
                        `ifdef AMBER_LOAD_MEM_DEBUG
                            $display ("Load Boot Mem: PAddr: 0x%08x, Data 0x%08x", 
                                        boot_mem_file_address, boot_mem_file_data);
                        `endif   
                        end
                    end  
                end
                
            $display("Read in %1d lines", boot_mem_line_count);      
            end

        // Grab the test name from memory    
        timeout   = `AMBER_TIMEOUT   ;  
        core_str = "amber25";      
        $display("Core %s, log file %s, timeout %0d, test name %0s ", core_str, `AMBER_LOG_FILE, timeout, `DEC_TEST_NAME );          
        log_file = $fopen(`AMBER_LOG_FILE, "a");                               
end


// ======================================
// Initialize Main Memory
// ======================================
`ifdef AMBER_LOAD_MAIN_MEM
always @ ( posedge `U_SYSTEM.reset ) begin
	repeat(`RESET_MEM_TICKS)
		@ ( negedge `U_SYSTEM.clk );
        $display("Load main memory from %s", `DEC_MAIN_MEM_FILE);

        main_mem_file   = $fopen(`DEC_MAIN_MEM_FILE, "r");
            
        // Read RAM File
        main_mem_line_count   = 0;
        
        if (main_mem_file == 0)
            begin
            $display("ERROR: Can't open input file %s", `DEC_MAIN_MEM_FILE);
`ifndef NETLIST
            `TB_ERROR_MESSAGE
`endif
            end

        if (main_mem_file != 0)
            begin  
            fgets_return = 1;
            while (fgets_return != 0)
                begin
                fgets_return        = $fgets(line, main_mem_file);
                main_mem_line_count = main_mem_line_count + 1;
                aligned_line        = align_line(line);
                
                // if the line does not start with a comment
                if (aligned_line[120*8-1:118*8] != 16'h2f2f)
                    begin
                    // check that line doesnt start with a '@' or a blank
                    if (aligned_line[120*8-1:119*8] != 8'h40 && aligned_line[120*8-1:119*8] != 8'h00)
                        begin
                        $display("Format ERROR in input file %s, line %1d. Line must start with a @, not %08x", 
                                 `DEC_MAIN_MEM_FILE, main_mem_line_count, aligned_line[118*8-1:117*8]);
`ifndef NETLIST
                        `TB_ERROR_MESSAGE
`endif
                        end
                    
                    if (aligned_line[120*8-1:119*8] != 8'h00)
                        begin
                        main_mem_file_address =   hex_chars_to_32bits (aligned_line[119*8-1:111*8]);
                        main_mem_file_data    =   hex_chars_to_32bits (aligned_line[110*8-1:102*8]);

                      // Fast simulation model of main memory
                      // U_RAM - Can either point to simple or Xilinx DDR3 model. 
                      // Set in hierarchy_defines.v
                      
                      main_mem_file_data_128 = `U_RAM [main_mem_file_address[31:4]];
                      `U_RAM [main_mem_file_address[31:4]] = 
                          insert_32_into_128 ( main_mem_file_address[3:2], 
                                               main_mem_file_data_128, 
                                               main_mem_file_data );

                      `ifdef AMBER_LOAD_MEM_DEBUG
                          $display ("Load RAM: PAddr: 0x%08x, Data 0x%08x", 
                                     main_mem_file_address, main_mem_file_data);
                      `endif   
                        
                        end
                    end  
                end
                
            $display("Read in %1d lines", main_mem_line_count);      
            end
 end
`endif

dumpvcd u_dumpvcd();


`ifndef NETLIST
// ======================================
// Terminate Test  
// ======================================
`include "../include/a25_localparams.vh"
`include "../include/a25_functions.vh"

reg             testfail;
wire            test_status_set;
wire [31:0]     test_status_reg;

initial
    begin
    testfail  = 1'd0;
    end
    
assign test_status_set = `U_TEST_MODULE.test_status_set;
assign test_status_reg = `U_TEST_MODULE.test_status_reg;

always @*
        begin
        if ( test_status_set || testfail )
            begin
            if ( test_status_reg == 32'd17 && !testfail )
                begin
                display_registers;
                $display("++++++++++++++++++++");
                $write("Passed %s %0d ticks\n", `AMBER_TEST_NAME, `U_TB.clk_count);
                $display("++++++++++++++++++++");
                $fwrite(`U_TB.log_file,"Passed %s %0d ticks\n", `AMBER_TEST_NAME, `U_TB.clk_count);
                $finish;
                end
            else
                begin
                display_registers;
                if ( testfail )
                    begin
                    $display("++++++++++++++++++++");
                    $write("Failed %s\n", `AMBER_TEST_NAME);
                    $display("++++++++++++++++++++");
                    $fwrite(`U_TB.log_file,"Failed %s\n", `AMBER_TEST_NAME);
                    $finish;
                    end
                else
                    begin
                    $display("++++++++++++++++++++");
                    if (test_status_reg >= 32'h8000)
                        $write("Failed %s - with error 0x%08x\n", `AMBER_TEST_NAME, test_status_reg);
                    else
                        $write("Failed %s - with error on line %1d\n", `AMBER_TEST_NAME, test_status_reg);
                    $display("++++++++++++++++++++");
                    if (test_status_reg >= 32'h8000)
                        $fwrite(`U_TB.log_file,"Failed %s - with error 0x%08h\n", `AMBER_TEST_NAME, test_status_reg);
                    else
                        $fwrite(`U_TB.log_file,"Failed %s - with error on line %1d\n", `AMBER_TEST_NAME, test_status_reg);
                    $finish;
                    end
                end
            end
        end


// ======================================
// Timeout
// ======================================
always @ ( posedge `U_SYSTEM.clk )
    if ( timeout != 0 )
        if (`U_TB.clk_count >= timeout)
            begin
            `TB_ERROR_MESSAGE
            $display("Timeout Error. Edit $AMBER_BASE/hw/tests/timeouts.txt to change the timeout");
            end
            
// ======================================
// Tasks
// ======================================
task display_registers;
begin
    $display("");
    $display("----------------------------------------------------------------------------");
    $display("Amber Core");

    case (`U_EXECUTE.status_bits_mode)
        FIRQ:    $display("         User       > FIRQ         IRQ          SVC"); 
        IRQ:     $display("         User         FIRQ       > IRQ          SVC"); 
        SVC:     $display("         User         FIRQ         IRQ        > SVC"); 
        default: $display("       > User         FIRQ         IRQ          SVC"); 
    endcase

    $display("r0       0x%08x", `U_REGISTER_BANK.r0);
    $display("r1       0x%08x", `U_REGISTER_BANK.r1);
    $display("r2       0x%08x", `U_REGISTER_BANK.r2);
    $display("r3       0x%08x", `U_REGISTER_BANK.r3);
    $display("r4       0x%08x", `U_REGISTER_BANK.r4);
    $display("r5       0x%08x", `U_REGISTER_BANK.r5);
    $display("r6       0x%08x", `U_REGISTER_BANK.r6);
    $display("r7       0x%08x", `U_REGISTER_BANK.r7);
    $display("r8       0x%08x   0x%08x ", `U_REGISTER_BANK.r8,  `U_REGISTER_BANK.r8_firq);
    $display("r9       0x%08x   0x%08x ", `U_REGISTER_BANK.r9,  `U_REGISTER_BANK.r9_firq);
    $display("r10      0x%08x   0x%08x ", `U_REGISTER_BANK.r10, `U_REGISTER_BANK.r10_firq);
    $display("r11      0x%08x   0x%08x ", `U_REGISTER_BANK.r11, `U_REGISTER_BANK.r11_firq);
    $display("r12      0x%08x   0x%08x ", `U_REGISTER_BANK.r12, `U_REGISTER_BANK.r12_firq);
    
    $display("r13      0x%08x   0x%08x   0x%08x   0x%08x", 
                                               `U_REGISTER_BANK.r13, 
                                               `U_REGISTER_BANK.r13_firq, 
                                               `U_REGISTER_BANK.r13_irq,
                                               `U_REGISTER_BANK.r13_svc);
    $display("r14 (lr) 0x%08x   0x%08x   0x%08x   0x%08x", 
                                               `U_REGISTER_BANK.r14, 
                                               `U_REGISTER_BANK.r14_firq, 
                                               `U_REGISTER_BANK.r14_irq,
                                               `U_REGISTER_BANK.r14_svc);


    $display("r15 (pc) 0x%08x", {6'd0,`U_REGISTER_BANK.r15,2'd0});
    $display("");
    $display("Status Bits: N=%d, Z=%d, C=%d, V=%d, IRQ Mask %d, FIRQ Mask %d, Mode = %s",  
       `U_EXECUTE.status_bits_flags[3],
       `U_EXECUTE.status_bits_flags[2],
       `U_EXECUTE.status_bits_flags[1],
       `U_EXECUTE.status_bits_flags[0],
       `U_EXECUTE.status_bits_irq_mask,
       `U_EXECUTE.status_bits_firq_mask,
       mode_name (`U_EXECUTE.status_bits_mode) );
    $display("----------------------------------------------------------------------------");
    $display("");       

end
endtask
`endif

// ======================================
// Functions
// ======================================
function [127:0] insert_32_into_128;
input [1:0]   pos;
input [127:0] word128;
input [31:0]  word32;
begin
     case (pos)
         2'd0: insert_32_into_128 = {word128[127:32], word32};
         2'd1: insert_32_into_128 = {word128[127:64], word32, word128[31:0]};
         2'd2: insert_32_into_128 = {word128[127:96], word32, word128[63:0]};
         2'd3: insert_32_into_128 = {word32, word128[95:0]};
     endcase
end
endfunction



endmodule
