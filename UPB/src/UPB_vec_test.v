/*
*
* Module: UPB testbench
*
* Author(s): David Malanga
*
* Description: Test bench for the coefficient updater block in the 6th order predictot in the co-processor. Scans through all 6 I/O combinations.
*
* Revision History:
*----------------------------------------------------------------------------------
* 2/22/16 - by David Malanga - Initial creation
*----------------------------------------------------------------------------------
*/

`define CLK_PERIOD 20 // value is in ns
`define ERRORSTOP
//`define VERBOSE

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
reg [3:0]	   in_select;
integer        rate, law, operation, type, model, j, loop;
reg [23:0] vectorSizes [0:19879];

// input and outputs
reg		Un;
reg  [23:0] Uns [0:19879];

reg  [15:0] DQn;
reg  [23:0] DQns [0:19879];

reg  [15:0] Bn;
reg  [23:0] Bns [0:19879];

reg  [1:0] rate_in;

wire [15:0] BnP;
reg  [23:0] BnPs [0:19879];


UPB top (
  .reset(reset),
  .clk(clk),
  .scan_in0(scan_in0),
  .scan_in1(scan_in1),
  .scan_in2(scan_in2),
  .scan_in3(scan_in3),
  .scan_in4(scan_in4),
  .scan_enable(scan_enable),
  .test_mode(test_mode),
  .Un(Un),
  .DQn(DQn),
  .Bn(Bn),
  .rate_in(rate_in),
  .scan_out0(scan_out0),
  .scan_out1(scan_out1),
  .scan_out2(scan_out2),
  .scan_out3(scan_out3),
  .scan_out4(scan_out4),
  .BnP(BnP)
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
  $sdf_annotate("sdf/UPB_tsmc18_scan.sdf", test.top);
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
			for (in_select=0; in_select<6; in_select=in_select+1) begin
				case (in_select)
				
					4'b0000:
						begin
						  	$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","u1.t"}, Uns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","dq.t"}, DQns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b1.t"}, Bns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b1p.t"}, BnPs);
						end
					4'b0001:
						begin
							$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","u2.t"}, Uns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","dq.t"}, DQns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b2.t"}, Bns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b2p.t"}, BnPs);
						end
					4'b0010:
						begin
							$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","u3.t"}, Uns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","dq.t"}, DQns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b3.t"}, Bns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b3p.t"}, BnPs);
						end
					4'b0011:
						begin
							$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","u4.t"}, Uns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","dq.t"}, DQns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b4.t"}, Bns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b4p.t"}, BnPs);
						end
					4'b0100:
						begin
							$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","u5.t"}, Uns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","dq.t"}, DQns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b5.t"}, Bns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b5p.t"}, BnPs);
						end
					4'b0101:
						begin
							$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","u6.t"}, Uns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","dq.t"}, DQns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b6.t"}, Bns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b6p.t"}, BnPs);
						end
				endcase
				@(posedge clk);
				for (j=0; j<loop; j=j+1) begin // go through the elements of each vector
					Un = Uns[j];
					DQn = DQns[j];
					Bn = Bns[j];
					rate_in = rate;
					@(posedge clk);
					`ifdef VERBOSE
						$display("Un = %h", Un);
						$display("DQn = %h", DQn);
						$display("Bn = %h", Bn);
						$display("BnP = %h", BnP);
					`endif
					if (BnP !== BnPs[j]) begin
					// modify variables in string & at end for inputs and outputs for DUT. Also inside the string to print your inputs and outputs
					$display("%t ERROR: Test #%7d failed for %s, rate = %s, law = %s, %s, %s, Un = %h, DQn = %h, Bn = %h. BnP = %h instead of %h",
                    				$time, j, models[model], rates[rate], laws[law], types[type], operations[operation], Un, DQn, Bn, BnP, BnPs[j]); 
					`ifdef ERRORSTOP
                  			$stop;
					`endif
                			end // if
              		end // element loop
			end // input select loop
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
			for (in_select=0; in_select<6; in_select=in_select+1) begin
				case (in_select)
				
					4'b0000:
						begin
						  	$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","u1.t"}, Uns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","dq.t"}, DQns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b1.t"}, Bns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b1p.t"}, BnPs);
						end
					4'b0001:
						begin
							$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","u2.t"}, Uns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","dq.t"}, DQns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b2.t"}, Bns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b2p.t"}, BnPs);
						end
					4'b0010:
						begin
							$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","u3.t"}, Uns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","dq.t"}, DQns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b3.t"}, Bns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b3p.t"}, BnPs);
						end
					4'b0011:
						begin
							$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","u4.t"}, Uns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","dq.t"}, DQns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b4.t"}, Bns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b4p.t"}, BnPs);
						end
					4'b0100:
						begin
							$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","u5.t"}, Uns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","dq.t"}, DQns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b5.t"}, Bns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b5p.t"}, BnPs);
						end
					4'b0101:
						begin
							$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","u6.t"}, Uns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","dq.t"}, DQns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b6.t"}, Bns);
						  $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
						    operations[operation],"/",rates[rate],"/","b6p.t"}, BnPs);
						end
				endcase
				@(posedge clk);
				for (j=0; j<loop; j=j+1) begin // go through the elements of each vector
					Un = Uns[j];
					DQn = DQns[j];
					Bn = Bns[j];
					rate_in = rate;
					@(posedge clk);
					`ifdef VERBOSE
						$display("Un = %h", Un);
						$display("DQn = %h", DQn);
						$display("Bn = %h", Bn);
						$display("BnP = %h", BnP);
					`endif
					if (BnP !== BnPs[j]) begin
					// modify variables in string & at end for inputs and outputs for DUT. Also inside the string to print your inputs and outputs
					$display("%t ERROR: Test #%7d failed for %s, rate = %s, law = %s, %s, %s, Un = %h, DQn = %h, Bn = %h. BnP = %h instead of %h",
                    				$time, j, models[model], rates[rate], laws[law], types[type], operations[operation], Un, DQn, Bn, BnP, BnPs[j]); 
					`ifdef ERRORSTOP
                  			$stop;
					`endif
                			end // if
              		end // element loop
			end // input select loop
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
