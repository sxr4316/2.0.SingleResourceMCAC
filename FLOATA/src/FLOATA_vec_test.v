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
*----------------------------------------------------------------------------------------------------
*2/19/16 - by Adam Steenkamer - template created
*2/20/16 - by swathika Ramakrishnan - modifies the inputs and outputs for this module
*2/21/16 - by swathika Ramakrishnan - made few corrections like closing the bracket and module name
*2/23/16 - by swathika Ramakrishnan - change the delay with posedge clk, delay file to FLOATA file name and changed the testbench from test.v file to vec_test.v file
*2/24/16 - by Swathika Ramakrishnan - removed the unnecessary comments
*----------------------------------------------------------------------------------------------------
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
reg  [15:0] DQ;             
reg  [23:0] DQs [0:19879];  
wire [10:0] DQ0;            
reg  [23:0] DQ0s [0:19879]; 


FLOATA top ( 
  .reset(reset),
  .clk(clk),
  .scan_in0(scan_in0),
  .scan_in1(scan_in1),
  .scan_in2(scan_in2),
  .scan_in3(scan_in3),
  .scan_in4(scan_in4),
  .scan_enable(scan_enable),
  .test_mode(test_mode),
  .DQ(DQ[15:0]),                
  .scan_out0(scan_out0),
  .scan_out1(scan_out1),
  .scan_out2(scan_out2),
  .scan_out3(scan_out3),
  .scan_out4(scan_out4),
  .DQ0(DQ0[10:0])              
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
  $sdf_annotate("sdf/FLOATA_tsmc18_scan.sdf", test.top);
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
              
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","dq.t"}, DQs); 
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","dq0.t"}, DQ0s); 

              for (j=0; j<loop; j=j+1) begin 
                #1 DQ = DQs[j]; 
                @(posedge clk);
`ifdef VERBOSE
                $display("DQ = %h", DQ);   
                $display("DQ0 = %h", DQ0); 
`endif              
                if (DQ0 !== DQ0s[j]) begin
                  
                  $display("%t ERROR: Test #%7d failed for %s, rate = %s, law = %s, %s, %s, DQ = %h. DQ0 = %h instead of %h",
                    $time, j, models[model], rates[rate], laws[law], types[type], operations[operation], DQ, DQ0, DQ0s[j]); 
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
             
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","dq.t"}, DQs); 
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","dq0.t"}, DQ0s); 

              for (j=0; j<loop; j=j+1) begin 
                #1 DQ = DQs[j]; 
                @(posedge clk);
`ifdef VERBOSE
                $display("DQ = %h", DQ);   
                $display("DQ0 = %h", DQ0); 
`endif
                
                if (DQ0 !== DQ0s[j]) begin
                  
                  $display("%t ERROR: Test #%7d failed for %s, rate = %s, law = %s, %s, %s, DQ = %h. DQ0 = %h instead of %h",
                    $time, j, models[model], rates[rate], laws[law], types[type], operations[operation], DQ, DQ0, DQ0s[j]); 
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

  @(posedge clk) $display("%t TEST COMPLETE", $time);
  $finish;
end // initial begin

endmodule
