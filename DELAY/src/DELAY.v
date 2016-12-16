/*

Descrition : Delay function operates as a shift register, storing data for channels

Author : Adam Steenkamer

Revision History :
//----------------------------------------------------------------------------------
2/14/16 - by Adam Steenkamer - Initial creation
2/15/16 - by Adam Steenkamer - Fixed how the reset works
2/16/16 - by Adam Steenkamer - Changed the reset to be asynchronous
2/17/16 - by Adam Steenkamer - Changed the comments with the parameters
3/21/16 - by Adam Steenkamer - Modified module to have 32nd register be the output reg.
                                 Actually had 33 registers.
//----------------------------------------------------------------------------------
*/
module DELAY #(
  // parameters for size of signals and for what the reset value is
  parameter SIZE   = 16, // default size is 16bits
  parameter RSTVAL = 0   // default value is 0
)(
  input                 reset,       // system reset
  input                 clk,         // system clock
  input                 scan_in0,    // test scan mode data input
  input                 scan_in1,    // test scan mode data input
  input                 scan_in2,    // test scan mode data input
  input                 scan_in3,    // test scan mode data input
  input                 scan_in4,    // test scan mode data input
  input                 scan_enable, // test scan mode enable
  input                 test_mode,   // test mode
  input  [SIZE-1:0]     X,           // input of delay block
  output                scan_out0,   // test scan mode data output
  output                scan_out1,   // test scan mode data output
  output                scan_out2,   // test scan mode data output
  output                scan_out3,   // test scan mode data output
  output                scan_out4,   // test scan mode data output
  output reg [SIZE-1:0] Y            // output of delay block
);

  // Internal declarations
  reg [SIZE-1:0] R [0:30]; // 31 shift registers

  always@(posedge clk, posedge reset) begin
    if(reset) begin // reset to specified value
      Y     <= RSTVAL; // output is reset to given value according to spec
      R[30] <= RSTVAL;
      R[29] <= RSTVAL;
      R[28] <= RSTVAL;
      R[27] <= RSTVAL;
      R[26] <= RSTVAL;
      R[25] <= RSTVAL;
      R[24] <= RSTVAL;
      R[23] <= RSTVAL;
      R[22] <= RSTVAL;
      R[21] <= RSTVAL;
      R[20] <= RSTVAL;
      R[19] <= RSTVAL;
      R[18] <= RSTVAL;
      R[17] <= RSTVAL;
      R[16] <= RSTVAL;
      R[15] <= RSTVAL;
      R[14] <= RSTVAL;
      R[13] <= RSTVAL;
      R[12] <= RSTVAL;
      R[11] <= RSTVAL;
      R[10] <= RSTVAL;
      R[9]  <= RSTVAL;
      R[8]  <= RSTVAL;
      R[7]  <= RSTVAL;
      R[6]  <= RSTVAL;
      R[5]  <= RSTVAL;
      R[4]  <= RSTVAL;
      R[3]  <= RSTVAL;
      R[2]  <= RSTVAL;
      R[1]  <= RSTVAL;
      R[0]  <= RSTVAL;
    end
    else begin // shift
      Y     <= R[30];
      R[30] <= R[29];
      R[29] <= R[28];
      R[28] <= R[27];
      R[27] <= R[26];
      R[26] <= R[25];
      R[25] <= R[24];
      R[24] <= R[23];
      R[23] <= R[22];
      R[22] <= R[21];
      R[21] <= R[20];
      R[20] <= R[19];
      R[19] <= R[18];
      R[18] <= R[17];
      R[17] <= R[16];
      R[16] <= R[15];
      R[15] <= R[14];
      R[14] <= R[13];
      R[13] <= R[12];
      R[12] <= R[11];
      R[11] <= R[10];
      R[10] <= R[9];
      R[9]  <= R[8];
      R[8]  <= R[7];
      R[7]  <= R[6];
      R[6]  <= R[5];
      R[5]  <= R[4];
      R[4]  <= R[3];
      R[3]  <= R[2];
      R[2]  <= R[1];
      R[1]  <= R[0];
      R[0]  <= X;
    end
  end
endmodule











