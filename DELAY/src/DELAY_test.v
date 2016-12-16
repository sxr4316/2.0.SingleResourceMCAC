/* DELAY_test.v
*
* Module:  DELAY testbench
*
* Author(s): Adam Steenkamer
*
* Description:
* Using random data to test the DELAY module. It acts as shift registers for channels
*
* Revision History:
*----------------------------------------------------------------------------------
* 2/14/16 - by Adam Steenkamer - Initial creation
*
* 2/15/16 - by Adam Steenkamer - Made custom testbench
*
* 2/21/16 - by Adam Steenkamer - Modified it to loop through more data
*
* 2/22/16 - by Adam Steenkamer - Modified to get Netlist sim working with parameters
*
* 3/21/16 - by Adam Steenkamer - Modified after fixing DELAY. Should only need 32 shifts
                                   for a value to go from input to output
*----------------------------------------------------------------------------------
*
*/

`define CLK_PERIOD 20 // value is in ns

module test;

parameter WIDTH = 16;

wire scan_out0, scan_out1, scan_out2, scan_out3, scan_out4;

reg clk, reset;
reg scan_in0, scan_in1, scan_in2, scan_in3, scan_in4, scan_enable, test_mode;

integer j;

// input and outputs
reg  [WIDTH-1:0] X;
wire [WIDTH-1:0] Y;

// RTL Simulation
`ifndef NETLIST
  parameter RESET = 0;
  parameter RESET2 = 1;
  defparam top.SIZE = 16;
  defparam top.RSTVAL = RESET;
  defparam top2.SIZE = 11;
  defparam top2.RSTVAL = RESET2;
  reg  [10:0] X2;
  wire [10:0] Y2;
`endif

// Netlist Simulation
`ifdef NETLIST
  parameter RESET = 0;
  parameter RESET2 = 0;
  reg  [WIDTH-1:0] X2;
  wire [WIDTH-1:0] Y2;
`endif

DELAY top (
  .reset(reset),
  .clk(clk),
  .scan_in0(scan_in0),
  .scan_in1(scan_in1),
  .scan_in2(scan_in2),
  .scan_in3(scan_in3),
  .scan_in4(scan_in4),
  .scan_enable(scan_enable),
  .test_mode(test_mode),
  .X(X),
  .scan_out0(scan_out0),
  .scan_out1(scan_out1),
  .scan_out2(scan_out2),
  .scan_out3(scan_out3),
  .scan_out4(scan_out4),
  .Y(Y)
);

DELAY top2 (
  .reset(reset),
  .clk(clk),
  .scan_in0(scan_in0),
  .scan_in1(scan_in1),
  .scan_in2(scan_in2),
  .scan_in3(scan_in3),
  .scan_in4(scan_in4),
  .scan_enable(scan_enable),
  .test_mode(test_mode),
  .X(X2),
  .scan_out0(scan_out0),
  .scan_out1(scan_out1),
  .scan_out2(scan_out2),
  .scan_out3(scan_out3),
  .scan_out4(scan_out4),
  .Y(Y2)
);

// create clock
always #(`CLK_PERIOD/2) clk = ~clk;

initial begin
  $timeformat(-9,2,"ns", 16);
`ifdef SDFSCAN
  $sdf_annotate("sdf/DELAY_tsmc18_scan.sdf", test.top);
`endif

  clk         = 1'b0;
  reset       = 1'b1; // start with reset
  scan_in0    = 1'b0;
  scan_in1    = 1'b0;
  scan_in2    = 1'b0;
  scan_in3    = 1'b0;
  scan_in4    = 1'b0;
  scan_enable = 1'b0;
  test_mode   = 1'b0;

  X  = 0;
  X2 = 0;
  // leave reset high for a little then turn it off
  repeat(2)
    @(posedge clk);
  reset = 1'b0;

  repeat(31) begin
    @(posedge clk);
    if(Y !== RESET) begin
      $display("%t ERROR: Output not reset properly for 1st instantiation(not %h). Instead has value %h", $time, RESET, Y);
    end
    if(Y2 !== RESET2) begin
      $display("%t ERROR: Output not reset properly for 2nd instantiation(not %h). Instead has value %h", $time, RESET2, Y);
    end
  end
  for(j=1;j<2048;j=j+1) begin
   @(negedge clk);
    X  = j;
    X2 = j;
    // wait 32 clock cycles for new input to shift out
    repeat(32) begin
      @(posedge clk);
`ifndef NETLIST // RTL simulation
      if(Y !== j-1) begin
        $display("%t ERROR: Output not correct for 1st instantiation (not %h). Instead has value %h", $time, j-1, Y);
      end
      if(Y2 !== j-1) begin
        $display("%t ERROR: Output not correct for 2nd instantiation (not %h). Instead has value %h", $time, j-1, Y2);
      end
    end
`endif
`ifdef NETLIST // Netlist simulation
      if(Y !== (j-1)) begin
        $display("%t ERROR: Output not correct for 1st instantiation (not %h). Instead has value %h", $time, j-1, Y);
      end
      if(Y2 !== (j-1)) begin
        $display("%t ERROR: Output not correct for 2nd instantiation (not %h). Instead has value %h", $time, j-1, Y2);
      end
    end
`endif
    @(posedge clk);
`ifndef NETLIST // RTL simulation
    if(Y !== X) begin
      $display("%t ERROR: Output not correct for 1st instantiation (not %h). Instead has value %h", $time, X, Y);
    end
    if(Y2 !== X2) begin
      $display("%t ERROR: Output not correct for 2nd instantiation (not %h). Instead has value %h", $time, X2, Y2);
    end
`endif
`ifdef NETLIST // Netlist simulation
    if(Y !== X) begin
      $display("%t ERROR: Output not correct for 1st instantiation (not %h). Instead has value %h", $time, X, Y);
    end
    if(Y2 !== X2) begin
      $display("%t ERROR: Output not correct for 2nd instantiation (not %h). Instead has value %h", $time, X2, Y2);
    end
`endif

  end // for

  #10 $display("%t TEST COMPLETE", $time);
  $finish;
end // initial begin

endmodule

