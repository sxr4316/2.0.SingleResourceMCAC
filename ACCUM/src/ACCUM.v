/*

Descrition : The ACCUM block for our single resource FMULT/ACCUM structure
     is simply an adder and a register

Author : Adam Steenkamer

Revision History :
//----------------------------------------------------------------------------------
2/14/16 - by Adam Steenkamer - Initial creation
2/16/16 - by Adam Steenkamer - Added comment header. Made reset asynchronous
2/22/16 - by Adam Steenkamer - Switched to nonblocking
2/23/16 - by Adam Steenkamer - Added clear signal, since ACCUM needs to be cleared 
                                   for each channel
//----------------------------------------------------------------------------------
*/

module ACCUM (
  input             reset,       // system reset
  input             clk,         // system clock
  input             clear,       // control signal to clear ACCUM for each channel
  input             scan_in0,    // test scan mode data input
  input             scan_in1,    // test scan mode data input
  input             scan_in2,    // test scan mode data input
  input             scan_in3,    // test scan mode data input
  input             scan_in4,    // test scan mode data input
  input             scan_enable, // test scan mode enable
  input             test_mode,   // test mode
  input      [15:0] W,
  output reg [15:0] S,
  output            scan_out0,   // test scan mode data output
  output            scan_out1,   // test scan mode data output
  output            scan_out2,   // test scan mode data output
  output            scan_out3,   // test scan mode data output
  output            scan_out4    // test scan mode data output
);

  always@(posedge clk, posedge reset)begin
    if(reset)
      S <= 0;
    else if(clear)
      S <= 0;
    else
      S <= W + S;
  end

endmodule // ACCUM
