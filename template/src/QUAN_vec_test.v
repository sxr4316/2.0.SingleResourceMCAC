/* QUAN_vec_test.v
*
* Module: test
*
* Authors:
* 
*
* Description:
* Testbench for QUAN.
* Runs a directed test.
*
* Revision History:
* _Rev #_	_Author(s)_	_Date_		_Changes_
* 
*/
module test ();

//parameterize the paths
reg [2*8-1:0] rates [0:3];
//reg [3*8-1:0] places [1:2]; //QUAN exists only in enc
reg [4*8-1:0] laws [0:1];
integer rate, law, j, i;//,place

//call the appropriate inputs and outputs
reg [11:0] DLN;
reg [23:0] DLNs [0:19879]; 
reg DS;
reg [23:0] DSs [0:19879];
reg [1:0] RATE;
wire [4:0] I;
reg [23:0] Is [0:19879];

//instantiate the dut
QUAN top (
  .DLN	(DLN),
  .DS	(DS),
  .RATE	(RATE),
  .I	(I)
);

initial //initialize path parameters
begin
	rates[0] = "40";
	rates[1] = "32";
	rates[2] = "24";
	rates[3] = "16";
	//places[0] = "enc";
	//places[1] = "dec";
	laws[0] = "aa";
	laws[1] = "uu"; 
end

initial
begin
  $timeformat(-9,2,"ns", 16);
`ifdef SDFSCAN
  $sdf_annotate("sdf/QUAN_tsmc18_scan.sdf", test.top);
`endif
  #10;
//$display("%s",{rates[0],rates[1]});
//$finish;
//exhaustively test all rates, laws, and places for dut (assuming path below
//is correct and vectors are available)
for(rate=0;rate<4;rate=rate+1) begin
//for(place=1;place<2;place=place+1) begin
for(law=0;law<2;law=law+1) begin
	$display("%s %s", rates[rate], laws[law]);
	$readmemh({"../model/adpcm-itu/vector_out/",laws[law],/*places[place]*/"/enc/",rates[rate],"/nrm/homing/dln.t"}, DLNs);
	$readmemh({"../model/adpcm-itu/vector_out/",laws[law],/*places[place]*/"/enc/",rates[rate],"/nrm/homing/ds.t"}, DSs);
	$readmemh({"../model/adpcm-itu/vector_out/",laws[law],/*places[place]*/"/enc/",rates[rate],"/nrm/homing/I.t"}, Is);
	RATE = rate;
	for (j = 0; j < 19879; j=j+1) begin
		#1 DLN = DLNs[j];
		DS = DSs[j];
		#10;
`ifdef VERBOSE
		$display("DLN = %h",DLN);
		$display("DS = %b",DS);
		$display("I = %h",I);
`endif
		if (I!=Is[j]) begin
			$display("TEST #%7d FAIL for RATE = %b, DS = %b, DLN = %d = 0x%h. I = %h instead of %h", j, RATE, DS, DLN, DLN, I, Is[j]);
`ifdef BREAKONERR
			$stop;
`endif
		end else begin
			//$display("TEST #%7d  OK  for RATE = %b, DS = %b, DLN = %d. I = %h", j, RATE, DS, DLN, I);
		end
	end
end
//end
end

  #10 $display("** Test Complete!");
  $finish;
end

endmodule
