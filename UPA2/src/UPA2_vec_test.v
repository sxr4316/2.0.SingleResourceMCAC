/* UPA2_tb.v
*
* Module: UPA2 testbench
*
* Author(s): Anurag Reddy Gankat
*
* Description:
* (description here)
*
* Revision History:
*----------------------------------------------------------------------------------
* 2/19/16 - by Adam Steenkamer - template created
*
* 2/23/16 - Anurag Reddy Gankat - template used to test UPA2 module
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
reg [23:0]     vectorSizes [0:19879];

// input and outputs ************************************************************************
reg  PK0;             
reg  [23:0] PK0s [0:19879];  
reg  PK1;             
reg  [23:0] PK1s [0:19879];  
reg  PK2;             
reg  [23:0] PK2s [0:19879];  
reg  [15:0] A1;             
reg  [23:0] A1s [0:19879];  
reg  [15:0] A2;             
reg  [23:0] A2s [0:19879];  
reg  SIGPK;             
reg  [23:0] SIGPKs [0:19879];  
wire [15:0] A2T;            
reg  [23:0] A2Ts [0:19879]; 


UPA2 top ( 
  .reset(reset),
  .clk(clk),
  .scan_in0(scan_in0),
  .scan_in1(scan_in1),
  .scan_in2(scan_in2),
  .scan_in3(scan_in3),
  .scan_in4(scan_in4),
  .scan_enable(scan_enable),
  .test_mode(test_mode),
  .PK0(PK0),
  .PK1(PK1),
  .PK2(PK2),
  .A1(A1),
  .A2(A2),
  .SIGPK(SIGPK),	               
  .scan_out0(scan_out0),
  .scan_out1(scan_out1),
  .scan_out2(scan_out2),
  .scan_out3(scan_out3),
  .scan_out4(scan_out4),
  .A2T(A2T)                
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
  $sdf_annotate("sdf/UPA2_tsmc18_scan.sdf", test.top);
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
              
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","pk0.t"}, PK0s); 

              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","pk1.t"}, PK1s); 

              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","pk2.t"}, PK2s); 

              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","a1.t"}, A1s); 

              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","a2.t"}, A2s); 

              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","sigpk.t"}, SIGPKs); 

              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","a2t.t"}, A2Ts); 

              loop = vectorSizes[0];
     
              @(posedge clk);   

              for (j=0; j<loop; j=j+1) begin 

                 PK0 = PK0s[j]; 
                   PK1 = PK1s[j];
                   PK2 = PK2s[j];
                   A1 = A1s[j];
                   A2 = A2s[j];
                   SIGPK = SIGPKs[j];

              @(negedge clk);
                
`ifdef VERBOSE
                $display("PK0 = %h", PK0); 
                $display("PK1 = %h", PK1);
                $display("PK2 = %h", PK2);
                $display("A1 = %h", A1);
                $display("A2 = %h", A2);
                $display("SIGPK = %h", SIGPK);
                $display("A2T = %h", A2T); 
`endif
                
                if (A2T !== A2Ts[j]) begin
                  
                  $display("%t ERROR: Test #%7d failed for %s, rate = %s, law = %s, %s, %s, PK0 = %h PK1=%h PK2=%h A1=%h A2=%h SIGPK=%h A2T = %h instead of %h",
                    $time, j, models[model], rates[rate], laws[law], types[type], operations[operation], PK0,PK1,PK2,A1,A2,SIGPK,A2T, A2Ts[j]); 
`ifdef ERRORSTOP
                  $stop;
`endif
                end // if

               @(posedge clk);

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

              // repeat readmem lines for all inputs and outputs
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","pk0.t"}, PK0s);

              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","pk1.t"}, PK1s); 

              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","pk2.t"}, PK2s); 

              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","a1.t"}, A1s); 

              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","a2.t"}, A2s); 

              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","sigpk.t"}, SIGPKs);   
                        
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","a2t.t"}, A2Ts); 

             loop = vectorSizes[0];
               
              @(posedge clk);

              for (j=0; j<loop; j=j+1) begin 

                   PK0 = PK0s[j]; 
                   PK1 = PK1s[j];
                   PK2 = PK2s[j];
                   A1 = A1s[j];
                   A2 = A2s[j];
                   SIGPK = SIGPKs[j];

               @(negedge clk);
                
`ifdef VERBOSE
                $display("PK0 = %h", PK0);   
                $display("PK1 = %h", PK1);
                $display("PK2 = %h", PK2);
                $display("A1 = %h", A1);
                $display("A2 = %h", A2);
                $display("SIGPK = %h", SIGPK);
                $display("A2T = %h", A2T); 
`endif
                
                if (A2T !== A2Ts[j]) begin
                  
                  $display("%t ERROR: Test #%7d failed for %s, rate = %s, law = %s, %s, %s, PK0 = %h PK1 = %h PK2 = %h A1 = %h A2 = %h SIGPK = %h A2T = %h instead of %h",
                    $time, j, models[model], rates[rate], laws[law], types[type], operations[operation], PK0,PK1,PK2,A1,A2,SIGPK, A2T, A2Ts[j]); 
`ifdef ERRORSTOP
                  $stop;
`endif
                end // if

               @(posedge clk);

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
