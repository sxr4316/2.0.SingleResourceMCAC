`define CLK_PERIOD 20

`define ERRORSTOP

module test;

wire  scan_out0, scan_out1, scan_out2, scan_out3, scan_out4;

reg  clk, reset;
reg  scan_in0, scan_in1, scan_in2, scan_in3, scan_in4, scan_enable, test_mode;

reg [2*8-1:0]  rates [0:3];      // 4 rates (16kbps, 24kbps, 32kbps, 40kbps)
reg [4*8-1:0]  laws [0:3];       // 2 laws (a-law, u-law)
reg [6*8-1:0]  operations [0:3]; // 3 operations (normal or overload, or I for decoder)
reg [12*8-1:0] types [0:1];      // (reset, homing)
reg [6*8-1:0]  models [0:1];     // dec or enc
integer        rate, law, operation, type, model, j, loop;
reg [23:0] vectorSizes [0:19879];


//inputs and outputs
reg  [15:0] b1,b2,b3,b4,b5,b6,a1,a2;
reg  [23:0] b1s[0:19879],b2s[0:19879],b3s[0:19879],b4s[0:19879],b5s[0:19879],b6s[0:19879],a1s[0:19879],a2s [0:19879];

reg  [10:0] dq1,dq2,dq3,dq4,dq5,dq6,sr1,sr2;
reg  [23:0] dq1s[0:19879],dq2s[0:19879],dq3s[0:19879],dq4s[0:19879],dq5s[0:19879],dq6s[0:19879],sr1s[0:19879],sr2s[0:19879];
       
reg  [23:0] ses [0:19879];
wire [14:0] se;
wire [14:0] seExpected = ses[j];
reg  [23:0] sezs [0:19879];
wire [14:0] sez;
wire [14:0] sezExpected = sezs[j];
reg start;
wire done;

FMULT_ACCUM top(
        .reset(reset),
        .clk(clk),   
        .start(start),
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
        .b1(b1),
        .b2(b2),
        .b3(b3),
        .b4(b4),
	.b5(b5),
	.b6(b6),
	.a1(a1),
	.a2(a2),
        .dq1(dq1),
	.dq2(dq2),
	.dq3(dq3),
	.dq4(dq4),
	.dq5(dq5),
	.dq6(dq6),
	.sr1(sr1),
	.sr2(sr2),
        .se(se),
	.sez(sez),
        .done(done)
    );

//initialize path parameters
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

initial
begin
    $timeformat(-9,2,"ns", 16);
`ifdef SDFSCAN
    $sdf_annotate("sdf/FMULT_ACCUM_tsmc18_scan.sdf", test.top);
`endif
    clk = 1'b0;
    reset = 1'b0;
    scan_in0 = 1'b0;
    scan_in1 = 1'b0;
    scan_in2 = 1'b0;
    scan_in3 = 1'b0;
    scan_in4 = 1'b0;
    scan_enable = 1'b0;
    test_mode = 1'b0;
    start =1'b0;

 repeat(2)
    @(posedge clk);
  reset = 1'b1;
   @(posedge clk);
  reset = 1'b0;// make sure reset is off

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

                $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","b1.t"}, b1s);

                $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","b2.t"}, b2s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","b3.t"}, b3s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","b4.t"}, b4s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","b5.t"}, b5s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","b6.t"}, b6s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","a1.t"}, a1s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","a2.t"}, a2s);

                $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","dq1.t"}, dq1s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","dq2.t"}, dq2s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","dq3.t"}, dq3s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","dq4.t"}, dq4s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","dq5.t"}, dq5s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","dq6.t"}, dq6s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","sr1.t"}, sr1s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","sr2.t"}, sr2s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","sez.t"}, sezs);

                $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","se.t"}, ses);

		@(posedge clk);

		for (j=0; j<loop; j=j+1)  begin 
			b1=b1s[j];
			b2=b2s[j];
			b3=b3s[j];
			b4=b4s[j];
			b5=b5s[j];
			b6=b6s[j];
			a1=a1s[j];
			a2=a2s[j];
			dq1=dq1s[j];
			dq2=dq2s[j];
			dq3=dq3s[j];
			dq4=dq4s[j];
			dq5=dq5s[j];
			dq6=dq6s[j];
			sr1=sr1s[j];
			sr2=sr2s[j];
			start = 1'b1;
			@(negedge clk);
			@(posedge done);
			start =1'b0;

			if(sez !== sezs[j]) begin
				$display("%t ERROR: Test #%7d failed for %s, rate = %s, law = %s, %s, %s, b1 = %h , b2 = %h, b3 = %h, b4 = %h,b5 = %h, b6 = %h, a1 = %h,a2 = %h, dq1= %h, dq2 = %h, dq3= %h, dq4 = %h, dq5 = %h, dq6 = %h, sr1= %h, sr2= %h . sez = %h instead of %h", $time, j, models[model], rates[rate], laws[law], types[type], operations[operation],b1,b2,b3,b4,b5,b6,a1,a2,dq1,dq2,dq3,dq4,dq5,dq6,sr1,sr2,sez,sezs[j]);
`ifdef ERRORSTOP
                    		$stop;

`endif
                	end
			if(se !== ses[j]) begin
                   		$display("%t ERROR: Test #%7d failed for %s, rate = %s, law = %s, %s, %s, b1 = %h , b2 = %h, b3 = %h, b4 = %h,b5 = %h, b6 = %h, a1 = %h,a2 = %h, dq1= %h, dq2 = %h, dq3= %h, dq4 = %h, dq5 = %h, dq6 = %h, sr1= %h, sr2= %h . se = %h instead of %h", $time, j, models[model], rates[rate], laws[law], types[type], operations[operation],b1,b2,b3,b4,b5,b6,a1,a2,dq1,dq2,dq3,dq4,dq5,dq6,sr1,sr2,se,ses[j]);
`ifdef ERRORSTOP
                    		$stop;
`endif
                  	end
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
      for (operation=0; operation<3; operation = operation+1) begin
        if(!(model==0 && operation==2)) begin // I test only for decoder
          for (type=0; type<2; type=type+1) begin
            for (law=0; law<2; law=law+1) begin
              $display("%s %s %s %s %s law", models[model], rates[rate], operations[operation], types[type], laws[law]);
              $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","vectorSize.t"}, vectorSizes);
              	loop = vectorSizes[0];

                $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","b1.t"}, b1s);

                $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","b2.t"}, b2s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","b3.t"}, b3s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","b4.t"}, b4s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","b5.t"}, b5s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","b6.t"}, b6s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","a1.t"}, a1s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","a2.t"}, a2s);

                $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","dq1.t"}, dq1s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","dq2.t"}, dq2s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","dq3.t"}, dq3s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","dq4.t"}, dq4s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","dq5.t"}, dq5s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","dq6.t"}, dq6s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","sr1.t"}, sr1s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/", models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","sr2.t"}, sr2s);

		$readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","sez.t"}, sezs);

                $readmemh({"../model/adpcm-itu/vector_out/",laws[law],"/",models[model], "/", types[type],"/",
                operations[operation],"/",rates[rate],"/","se.t"}, ses);

		@(posedge clk);

		for (j=0; j<loop; j=j+1)  begin 
			b1=b1s[j];
			b2=b2s[j];
			b3=b3s[j];
			b4=b4s[j];
			b5=b5s[j];
			b6=b6s[j];
			a1=a1s[j];
			a2=a2s[j];
			dq1=dq1s[j];
			dq2=dq2s[j];
			dq3=dq3s[j];
			dq4=dq4s[j];
			dq5=dq5s[j];
			dq6=dq6s[j];
			sr1=sr1s[j];
			sr2=sr2s[j];
			start = 1'b1;
			@(negedge clk);	
			@(posedge done);
			start =1'b0;

			if(sez !== sezs[j]) begin
				$display("%t ERROR: Test #%7d failed for %s, rate = %s, law = %s, %s, %s, b1 = %h , b2 = %h, b3 = %h, b4 = %h,b5 = %h, b6 = %h, a1 = %h,a2 = %h, dq1= %h, dq2 = %h, dq3= %h, dq4 = %h, dq5 = %h, dq6 = %h, sr1= %h, sr2= %h . sez = %h instead of %h", $time, j, models[model], rates[rate], laws[law], types[type], operations[operation],b1,b2,b3,b4,b5,b6,a1,a2,dq1,dq2,dq3,dq4,dq5,dq6,sr1,sr2,sez,sezs[j]);
`ifdef ERRORSTOP
                    		$stop;

`endif
                	end
			if(se !== ses[j]) begin
                   		$display("%t ERROR: Test #%7d failed for %s, rate = %s, law = %s, %s, %s, b1 = %h , b2 = %h, b3 = %h, b4 = %h,b5 = %h, b6 = %h, a1 = %h,a2 = %h, dq1= %h, dq2 = %h, dq3= %h, dq4 = %h, dq5 = %h, dq6 = %h, sr1= %h, sr2= %h . se = %h instead of %h", $time, j, models[model], rates[rate], laws[law], types[type], operations[operation],b1,b2,b3,b4,b5,b6,a1,a2,dq1,dq2,dq3,dq4,dq5,dq6,sr1,sr2,se,ses[j]);
`ifdef ERRORSTOP
                    		$stop;
`endif
                  	end
			@(posedge clk);
              end // element loop
            end // law
          end // type
        end // if for I test 
      end // operation
    end // rate
  end // model

#10 $display("%t TEST COMPLETE", $time);
  $finish;
end // initial begin

endmodule


