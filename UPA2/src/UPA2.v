/*

Description :Updates a2 coefficient of the 2nd order predictor

Author : Anurag Reddy Gankat

Revision History :
//----------------------------------------------------------------------------------
2/16/16 - by Anurag Reddy Gankat - Initial creation
2/17/16 - Modified
2/20/16-  by Anurag Reddy Gankat - Replaced shifts with concatenation's
3/30/16 - by Anurag Reddy Gankat - Fixed spacing, used non-blocking statements in the always loop and added comments.
3/31/16 - by Adam Steenkamer - Fixed always loop. A1S needed to be assigned outside
//----------------------------------------------------------------------------------

*/
module UPA2 (
input               reset,          
input               clk,            
input               test_mode,      
input               scan_enable,    
input               scan_in0,      
input               scan_in1,       
input               scan_in2,      
input               scan_in3,      
input               scan_in4,       
output              scan_out0,      
output              scan_out1,      
output              scan_out2,      
output              scan_out3,      
output              scan_out4,      

//IO
input          PK0,            
input          PK1,            
input          PK2,           
input [15:0]   A1,                
input [15:0]   A2,                
input          SIGPK,            
output [15:0]  A2T                
       );
//Internal Signals      
wire        PKS1;
wire        PKS2;
wire [16:0] UGA2A;
wire        A1S;
reg  [16:0] FA1;
wire [16:0] FA;
wire [16:0] UGA2B;
wire        UGA2S;
wire [15:0] UGA2;
wire        A2S;
wire [15:0] ULA2;
wire [15:0] UA2;
wire [15:0] a;
 
//1 bit exclusive or
assign PKS1 = PK0 ^ PK1;

//1 bit exclusive or
assign PKS2 = PK0 ^ PK2; 

assign UGA2A = PKS2 ? 114688 : 16384;


assign A1S = A1[15]; // sign of Aaa1

always @(A1,A1S) begin
	if (!A1S)
		FA1 <= (A1<=8191)?{A1[14:0], 2'b00}:(32764);
	else
		FA1 <= (A1>=57345)? {A1[14:0], 2'b00}: (98308);
end//always block

assign FA= PKS1 ? FA1 : (131072-FA1);

assign UGA2B =(UGA2A+FA);

assign UGA2S =UGA2B[16];

assign a = UGA2S ? {6'b000000, UGA2B[16:7]}+64512 : {6'b000000, UGA2B[16:7]} ;

assign UGA2 = SIGPK ? 0 : a ;         

assign A2S=A2[15];//sign of A2

assign ULA2= A2S ? (65536-({7'b0000000,A2[15:7]}+65024)) : (65536-{7'b0000000,A2[15:7]});

//Final Assignments
assign UA2 =(UGA2+ULA2);

assign A2T = (A2 + UA2);

endmodule // UPA2
