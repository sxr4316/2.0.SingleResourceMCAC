/* template_vectors_tb.v
*
* Module: (module_name) testbench
*
* Author(s): (name here)
*
* Description:
* (description here)
*
* Revision History:
*----------------------------------------------------------------------------------
* 2/19/16 - by Adam Steenkamer - template created
*
* 2/21/16 - Siddharth Ramkrishnan - Addition of necessary bracket to match syntax
*
* 2/22/16 - by Adam Steenkamer - added note to changed name of .sdf file
*
*(date here) - (DESCRIPTIVE comment here of changes made)
*----------------------------------------------------------------------------------
*
*/

`define CLK_PERIOD 20 // value is in ns

module test;

wire scan_out0, scan_out1, scan_out2, scan_out3, scan_out4;

reg clk, reset;
reg scan_in0, scan_in1, scan_in2, scan_in3, scan_in4, scan_enable, test_mode;

// parameterize the paths
reg [2*8-1:0]  rates [0:3];      // 4 rates (16kbps, 24kbps, 32kbps, 40kbps)
reg [4*8-1:0]  laws [0:3];       // 2 laws (a-law, u-law)
reg [6*8-1:0]  operations [0:3]; // 3 operations (normal or overload, or I for decoder)
reg [12*8-1:0] types [0:1];      // (reset, homing)
reg [6*8-1:0]  models [0:1];     // dec or enc
integer        rate, law, operation, type, model, j, loop;
reg [23:0] vectorSizes [0:19879];

// input and outputs ************************************************************************
reg  [15:0] In1;             // modify for input name. Repeat these 2 lines for each input
reg  [23:0] In1s [0:19879];  // modify for input name. Repeat these 2 lines for each input
wire [15:0] Out1;            // modify for output name. Repeat these 2 lines for each output
reg  [23:0] Out1s [0:19879]; // modify for output name. Repeat these 2 lines for each output


MODULE_NAME top ( // modify for module name
  .reset(reset),
  .clk(clk),
  .scan_in0(scan_in0),
  .scan_in1(scan_in1),
  .scan_in2(scan_in2),
  .scan_in3(scan_in3),
  .scan_in4(scan_in4),
  .scan_enable(scan_enable),
  .test_mode(test_mode),
  .In1(In1),                 // modify for input name. Repeat for each input
  .scan_out0(scan_out0),
  .scan_out1(scan_out1),
  .scan_out2(scan_out2),
  .scan_out3(scan_out3),
  .scan_out4(scan_out4),
  .Out1(Out1)                // modify for output name. Repeat for each output. Make sure last doesn't have a comma
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

// create clock
always #(`CLK_PERIOD/2) clk = ~clk;

initial begin
  $timeformat(-9,2,"ns", 16);
`ifdef SDFSCAN
  $sdf_annotate("sdf/DELAY_tsmc18_scan.sdf", test.top);       // replace DELAY with name of your module*********
`endif

  clk         = 1'b0;
  reset       = 1'b0;
  scan_in0    = 1'b0;
  scan_in1    = 1'b0;
  scan_in2    = 1'b0;
  scan_in3    = 1'b0;
  scan_in4    = 1'b0;
  scan_enable = 1'b0;
  test_mode   = 1'b0;

  // exhaustively test for a-law -> a-law and u-law -> u-law
  for (model=0; model<2; model=model+1) begin
    for (rate=0; rate<4; rate=rate+1) begin
      for (operation=0; operation<3; operation = operation+1) begin
        if(!(model==0 && operation==2)) begin // I test only for decoder
          for (type=0; type<2; type=type+1) begin
            for (law=0; law<2; law=law+1) begin
              $display("%s %s %s %s %s law", models[model], rates[rate], operations[operation], types[type], laws[law]);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","vectorSize.t"}, vectorSizes);
              loop = vectorSizes[0];
              // repeat readmem lines for all inputs and outputs
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","In1.t"}, In1s); // modify last 2 parts for input file name
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","Out1.t"}, Out1s); // modify last 2 parts for output file name

              for (j=0; j<loop; j=j+1) begin // go through the elements of each vector
                #1 In1 = In1s[j]; // add assignments for each input. only leave one #1 at start
                #10;
`ifdef VERBOSE
                $display("In1 = %h", In1);   // modify for input and repeat for all inputs
                $display("Out1 = %h", Out1); // modify for output and repeat for all outputs
`endif
                // repeat this if statement for each output
                if (Out1 !== Out1s[j]) begin
                  // modify variables in string & at end for inputs and outputs for DUT. Also inside the string to print your inputs and outputs
                  $display("%t ERROR: Test #%7d failed for %s, rate = %s, law = %s, %s, %s, In1 = %h. Out1 = %h instead of %h",
                    $time, j, models[model], rates[rate], laws[law], types[type], operations[operation], In1, Out1, Out1s[j]); 
`ifdef ERRORSTOP
                  $stop;
`endif
                end // if
              end // element loop
            end // law
          end // type
        end // if for I test 
      end // operation
    end // rate
  end // model

  // exhaustively test for a-law -> u-law and u-law -> a-law
  for (model=0; model<2; model=model+1) begin
    for (rate=0; rate<4; rate=rate+1) begin
      for (operation=0; operation<2; operation = operation+1) begin
        for (type=0; type<2; type=type+1) begin
          if(!(model==0 && type==1)) begin // homing test only for decoder
            for (law=2; law<4; law=law+1) begin
              $display("%s %s %s %s %s law", models[model], rates[rate], operations[operation], types[type], laws[law]);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","vectorSize.t"}, vectorSizes);
              loop = vectorSizes[0];
              // repeat readmem lines for all inputs and outputs
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","In1.t"}, In1s); // modify last 2 parts for input file name
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","Out1.t"}, Out1s); // modify last 2 parts for output file name

              for (j=0; j<loop; j=j+1) begin // go through the elements of each vector
                #1 In1 = In1s[j]; // add assignments for each input. only leave one #1 at start
                #10;
`ifdef VERBOSE
                $display("In1 = %h", In1);   // modify for input and repeat for all inputs
                $display("Out1 = %h", Out1); // modify for output and repeat for all outputs
`endif
                // repeat this if statement for each output
                if (Out1 !== Out1s[j]) begin
                  // modify variables in string & at end for inputs and outputs for DUT. Also inside the string to print your inputs and outputs
                  $display("%t ERROR: Test #%7d failed for %s, rate = %s, law = %s, %s, %s, In1 = %h. Out1 = %h instead of %h",
                    $time, j, models[model], rates[rate], laws[law], types[type], operations[operation], In1, Out1, Out1s[j]); 
`ifdef ERRORSTOP
                  $stop;
`endif
                end // if
              end // element loop
            end // law
          end // if for homing
        end // type
      end // operation
    end // rate
  end // model

  #10 $display("%t TEST COMPLETE", $time);
  $finish;
end // initial begin

endmodule
