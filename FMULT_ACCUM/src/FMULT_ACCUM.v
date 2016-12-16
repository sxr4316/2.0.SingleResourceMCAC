/*

Description : The Fmult_accum integration according to the single resource logic

Author : Suryanarayanan Subramaniam, Anurag Reddy and Swathika Ramakrishnan

Revision History :
//----------------------------------------------------------------------------------
2/29/16	-	Surya	-   Initial creation
3/7/16 -        Surya, Anurag & Swathika  -  File synthesizes and works
3/7/16 -  by Adam Steenkamer - Removed content that got in port mapping by mistake.
				Removed port mapping of scan signals.
3/15/16	-	Dave	-	Removed `include's and added the directories to the etc folder
3/30/16 - Connor - Changed if else block to case statements
//----------------------------------------------------------------------------------

*/

module FMULT_ACCUM (
           reset,
           clk,
           start,
           scan_in0,
           scan_in1,
           scan_in2,
           scan_in3,
           scan_in4,
           scan_enable,
           test_mode,
           scan_out0,
           scan_out1,
           scan_out2,
           scan_out3,
           scan_out4,
           b1,b2,b3,b4,b5,b6,a1,a2,
           dq1,dq2,dq3,dq4,dq5,dq6,sr1,sr2,
           se,sez,
           done
           );

input
    reset,                      // system reset
    start,
    clk;
                           // system clock

input[15:0] b1,b2,b3,b4,b5,b6,a1,a2;             // defining the inputs
input[10:0] dq1,dq2,dq3,dq4,dq5,dq6,sr1,sr2;     // defining the inputs

                            
input
    scan_in0,                   // test scan mode data input
    scan_in1,                   // test scan mode data input
    scan_in2,                   // test scan mode data input
    scan_in3,                   // test scan mode data input
    scan_in4,                   // test scan mode data input
    scan_enable,                // test scan mode enable
    test_mode;                  // test mode 

output
    scan_out0,                  // test scan mode data output
    scan_out1,                  // test scan mode data output
    scan_out2,                  // test scan mode data output
    scan_out3,                  // test scan mode data output
    scan_out4;                  // test scan mode data output

output
    done;
 


reg [15:0] f1;                //defining the internal signals
reg [10:0] f2;
reg [3:0]  in_sel;
wire[15:0] S;
wire[15:0] W;
output reg[14:0] sez,se;
reg clear,done;

FMULT FMULT_INST0(
	   .reset(reset),
           .clk(clk),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
           .scan_out4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .AnBn(f1),
           .SRnDQn(f2),
           .WAnWBn(W) 
);

ACCUM ACCUM_INST0(
	   .reset(reset),
           .clk(clk),
           .clear(clear),
           .scan_in0(),
           .scan_in1(),
           .scan_in2(),
           .scan_in3(),
           .scan_in4(),
           .scan_enable(scan_enable),
           .test_mode(test_mode),
           .W(W),
           .S(S),
           .scan_out0(),
           .scan_out1(),
           .scan_out2(),
           .scan_out3(),
           .scan_out4()
           
);

always @(posedge reset, posedge clk)
begin
	if(reset)
	begin
		f1<=16'h0000;
		f2<=11'h000;
		in_sel <= 4'h0;
		clear<=1'b1;
		done<=1'b0;
		sez<=15'h0000;
		se <=15'h0000;
	end
	else begin
		if (start) begin
			case (in_sel)
				4'h0: begin
					f1<=16'h0000;
					f2<=11'h000;
					in_sel<= in_sel + 1'b1;
					clear<=1'b0;
					done<=1'b0;
					sez<=15'h0000;
					se <=15'h0000;
				end

				4'h1: begin
					f1 <= b1;
					f2 <= dq1;
					in_sel<= in_sel + 1'b1;
					clear<=1'b0;
					done<=1'b0;
					sez<=15'h0000;
					se <=15'h0000;
				end 

				4'h2: begin
					f1 <= b2;
					f2 <= dq2;
					in_sel<= in_sel + 1'b1;
					clear<=1'b0;
					done<=1'b0;
					sez<=15'h0000;
					se <=15'h0000;
				end

				4'h3: begin
					f1 <= b3;
					f2 <= dq3;
					in_sel<= in_sel + 1'b1;
					clear<=1'b0;
					done<=1'b0;
					sez<=15'h0000;
					se <=15'h0000;
				end
				
				4'h4: begin
					f1 <= b4;
					f2 <= dq4;
					in_sel<= in_sel + 1'b1;
					clear<=1'b0;
					done<=1'b0;
					sez<=15'h0000;
					se <=15'h0000;
				end
		
				4'h5: begin
					f1 <= b5;
					f2 <= dq5;
					in_sel<= in_sel + 1'b1;
					clear<=1'b0;
					done<=1'b0;
					sez<=15'h0000;
					se <=15'h0000;
				end
		
				4'h6: begin
					f1 <= b6;
					f2 <= dq6;
					in_sel<= in_sel + 1'b1;
					clear<=1'b0;
					done<=1'b0;
					sez<=15'h0000;
					se <=15'h0000;
				end
		
				4'h7: begin
					f1 <= a1;
					f2 <= sr1;
					sez <= 15'h0000;
					se <=15'h0000;
					in_sel<= in_sel + 1'b1;
					clear<=1'b0;
					done<=1'b0;
				end
				 
				4'h8: begin
					f1 <= a2;
					f2 <= sr2;
					in_sel<= in_sel + 1'b1;
					clear<=1'b0;
					done<=1'b0;
					sez[14:0]<=S[15:1];
					se <=15'h0000;
				end
		
				4'h9: begin
					f1 <= 16'h0000;
					f2 <= 11'h000;
					in_sel<= in_sel + 1'b1;
					clear<=1'b0;
					done<=1'b0;
					sez<=sez;
					se <=15'h0000;
				end
		
				default: begin
					f1<=16'h0000;
					f2<=11'h000;
					se[14:0]<=S[15:1];
					sez<=sez;
					in_sel<=1'b0;
					clear<=1'b1;
					done<=1'b1;
				end
			endcase // in_sel
		end // if (start)
	end // not reset
end // always

endmodule
