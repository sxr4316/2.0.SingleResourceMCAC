/*

Descrition : Testbench for ACCUM module. The design is single resource, so it just a simple adder where the output gets fed back. 

Author : Adam Steenkamer

Revision History :
//----------------------------------------------------------------------------------
2/14/16 - by Adam Steenkamer - Initial creation
2/15/16 - by Adam Steenkamer - Moved unnecessary time scale and commented code
2/20/16 - by Adam Steenkamer - Modified testbench to use vectors.
2/21/16 - by Adam Steenkamer - Added in loops for test vector file structure
2/22/16 - by Adam Steenkamer - Modified to have expected values & more
2/23/16 - by Adam Steenkamer - Changed input to use non blocking assignments to fix 
                                    RTL vs Netlist issues with output timing
2/23/16 - by Adam Steenkamer - Migrated vector TB to _vec_test & added clear to 
                                    match implementation
//----------------------------------------------------------------------------------

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

// input and outputs
reg         clear;
reg  [15:0] W;
reg  [23:0] WA1s [0:19879];
wire [23:0] WA1exp = WA1s[j];
reg  [23:0] WA2s [0:19879];
wire [23:0] WA2exp = WA2s[j]; 
reg  [23:0] WB1s [0:19879]; 
wire [23:0] WB1exp = WB1s[j];
reg  [23:0] WB2s [0:19879]; 
wire [23:0] WB2exp = WB2s[j];
reg  [23:0] WB3s [0:19879]; 
wire [23:0] WB3exp = WB3s[j];
reg  [23:0] WB4s [0:19879]; 
wire [23:0] WB4exp = WB4s[j];
reg  [23:0] WB5s [0:19879]; 
wire [23:0] WB5exp = WB5s[j];
reg  [23:0] WB6s [0:19879]; 
wire [23:0] WB6exp = WB6s[j];
wire [15:0] S;       
reg  [23:0] SEs [0:19879];
wire [14:0] SEexp = SEs[j];
reg  [23:0] SEZs [0:19879];
wire [14:0] SEZexp = SEZs[j];

// other internal TB signals
integer   k;

ACCUM top (
  .reset(reset),
  .clk(clk),
  .clear(clear),
  .scan_in0(scan_in0),
  .scan_in1(scan_in1),
  .scan_in2(scan_in2),
  .scan_in3(scan_in3),
  .scan_in4(scan_in4),
  .scan_enable(scan_enable),
  .test_mode(test_mode),
  .W(W),
  .S(S),
  .scan_out0(scan_out0),
  .scan_out1(scan_out1),
  .scan_out2(scan_out2),
  .scan_out3(scan_out3),
  .scan_out4(scan_out4)
);

// initialize path parameters
initial begin
  rates[0]      = "40";
  rates[1]      = "32";
  rates[2]      = "24";
  rates[3]      = "16";
  laws[0]       = "AA";
  laws[1]       = "MM";
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
  $sdf_annotate("sdf/ACCUM_tsmc18_scan.sdf", test.top);
`endif

  clk         = 1'b0;
  reset       = 1'b1;  // start with reset high to clear reg
  scan_in0    = 1'b0;
  scan_in1    = 1'b0;
  scan_in2    = 1'b0;
  scan_in3    = 1'b0;
  scan_in4    = 1'b0;
  scan_enable = 1'b0;
  test_mode   = 1'b0;

  clear = 1'b0;
  W <= 0;
 
  repeat(2)
    @(posedge clk);
  reset = 1'b0; // make sure reset is off


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
                operations[operation],"/",rates[rate],"/","wa1.t"}, WA1s);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","wa2.t"}, WA2s);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","wb1.t"}, WB1s);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","wb2.t"}, WB2s);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","wb3.t"}, WB3s);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","wb4.t"}, WB4s);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","wb5.t"}, WB5s);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","wb6.t"}, WB6s);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","sez.t"}, SEZs);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","se.t"}, SEs);

              @(posedge clk);

              for (j=0; j<loop; j=j+1) begin // go through the elements of each vector

                // ACCUM clears based on count (so for beginning of each new set of inputs)
                clear = 1'b1;
                @(posedge clk);
                clear = 1'b0; // make sure clear is off
                @(posedge clk);

                for (k=0; k<9; k=k+1) begin
                  case(k)
                    0: begin W <= WB1s[j]; end
                    1: begin W <= WB2s[j]; end
                    2: begin W <= WB3s[j]; end
                    3: begin W <= WB4s[j]; end
                    4: begin W <= WB5s[j]; end
                    5: begin W <= WB6s[j]; end
                    6: begin W <= WA1s[j]; end
                    7: begin W <= WA2s[j]; end
                    8: begin W <= 0; end
                  endcase

                  @(negedge clk);

                  if(k === 6 && S[15:1] !== SEZs[j]) begin
                    $display("%t ERROR: Test #%7d failed for %s, rate = %s, law = %s, %s, %s, W = %h. S = %h instead of %h", $time, j, models[model], rates[rate], laws[law], types[type], operations[operation], W, S[15:1], SEZs[j]);
`ifdef ERRORSTOP
                    $stop;
`endif
                  end
                  else if(k === 8 && S[15:1] !== SEs[j]) begin
                    $display("%t ERROR: Test #%7d failed for %s, rate = %s, law = %s, %s, %s, W = %h. S = %h instead of %h", $time, j, models[model], rates[rate], laws[law], types[type], operations[operation], W, S[15:1], SEs[j]);
`ifdef ERRORSTOP
                    $stop;
`endif
                  end // if
                  @(posedge clk);
                end // k loop
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
                operations[operation],"/",rates[rate],"/","wa1.t"}, WA1s);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","wa2.t"}, WA2s);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","wb1.t"}, WB1s);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","wb2.t"}, WB2s);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","wb3.t"}, WB3s);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","wb4.t"}, WB4s);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","wb5.t"}, WB5s);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","wb6.t"}, WB6s);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","sez.t"}, SEZs);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","se.t"}, SEs);

              @(posedge clk);

              for (j=0; j<loop; j=j+1) begin // go through the elements of each vector

                // ACCUM clears based on count (so for beginning of each new set of inputs)
                clear = 1'b1;
                @(posedge clk);
                clear = 1'b0; // make sure clear is off
                @(posedge clk);

                for (k=0; k<9; k=k+1) begin
                  case(k)
                    0: begin W <= WB1s[j]; end
                    1: begin W <= WB2s[j]; end
                    2: begin W <= WB3s[j]; end
                    3: begin W <= WB4s[j]; end
                    4: begin W <= WB5s[j]; end
                    5: begin W <= WB6s[j]; end
                    6: begin W <= WA1s[j]; end
                    7: begin W <= WA2s[j]; end
                    8: begin W <= 0; end
                  endcase

                  @(negedge clk);

                  if(k === 6 && S[15:1] !== SEZs[j]) begin
                    $display("%t ERROR: Test #%7d failed for %s, rate = %s, law = %s, %s, %s, W = %h. S = %h instead of %h", $time, j, models[model], rates[rate], laws[law], types[type], operations[operation], W, S[15:1], SEZs[j]);
`ifdef ERRORSTOP
                    $stop;
`endif
                  end
                  else if(k === 8 && S[15:1] !== SEs[j]) begin
                    $display("%t ERROR: Test #%7d failed for %s, rate = %s, law = %s, %s, %s, W = %h. S = %h instead of %h", $time, j, models[model], rates[rate], laws[law], types[type], operations[operation], W, S[15:1], SEs[j]);
`ifdef ERRORSTOP
                    $stop;
`endif
                  end // if
                  @(posedge clk);
                end // k loop
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
