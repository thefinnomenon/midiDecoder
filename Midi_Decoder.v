/** 
 ** ECE353 LAB B: MIDI Decoder
 ** Fall 2011
 ** Authors: Chris Finn, Paulo Leal, & Shuwen Cao
 **/
module Midi_Decoder (Midi_in, Clock, Reset_n, LED_output);
//////////////////
// Inputs
////////////// 
input Midi_in;
input Clock;
input Reset_n;

//////////////////
// Output
//////////////
output reg [7:0] LED_output;

//////////////////
// States
//////////////  
parameter IDLE = 2'b00;
parameter STATUS = 2'b01;
parameter NOTE = 2'b10;
parameter VELOCITY = 2'b11;

//////////////////
// Registers
//////////////  
reg [1:0] state;
reg [1:0] state_nxt;
reg [7:0] LED_output_nxt;
reg [10:0] count;
reg [10:0] count_nxt;
reg in_1;
reg in_2;

//////////////////
// Constants
//////////////
parameter Const_1280 = 11'b10100000000;
parameter Const_1 = 11'b00000000001;

//////////////////
// Sequential
//////////////
always @(posedge Clock) 
begin
	in_1 <= Midi_in;
	in_2 <= in_1;
	
	count <= count_nxt;
	LED_output <= LED_output_nxt;
	state <= state_nxt;
	if(!Reset_n)
	begin
		LED_output <= LED_output_nxt;
		state <= IDLE;
		count <= 0;
	end
end	

//////////////////
// Combinational
//////////////
always@(Reset_n or in_2 or LED_output or state or count)
begin
	if(!Reset_n)
	begin
		LED_output_nxt = 8'b00000000;
		count_nxt = 0;
		state_nxt = IDLE;
	end
	else begin
	case (state)
		IDLE: 					      
		begin
			state_nxt = IDLE;
			LED_output_nxt = LED_output;
			if(count < Const_1280)
				count_nxt = count + Const_1;	
			if(!in_2 && count > 2)
			begin
				state_nxt = STATUS;
				count_nxt = 0;
			end
		end
		 
		STATUS: 					      
		begin
			count_nxt = count;
			if(count < Const_1280)
				count_nxt = count + Const_1;
			state_nxt = STATUS;
			LED_output_nxt = LED_output;
			if(count >= Const_1280 && !in_2)
			begin
				count_nxt = 0; 
				state_nxt = NOTE; 
			end
		end
		
		NOTE: 					      
		begin
			count_nxt = count;
			if(count < Const_1280)
			begin
				count_nxt = count + Const_1;
				LED_output_nxt = LED_output;
			end
			state_nxt = NOTE;
			if(count >= Const_1280 && !in_2)
			begin
				count_nxt = 0;
				state_nxt = VELOCITY;
			end
			case(count)
				192: LED_output_nxt[0] = in_2;
				320: LED_output_nxt[1] = in_2;
				448: LED_output_nxt[2] = in_2;
				576: LED_output_nxt[3] = in_2;
				704: LED_output_nxt[4] = in_2;
				832: LED_output_nxt[5] = in_2;
				960: LED_output_nxt[6] = in_2;
				1088: LED_output_nxt[7] = in_2;
				default: LED_output_nxt = LED_output;
			endcase 
		end
		
		VELOCITY: 					      
		begin
			count_nxt = count;
			if(count < Const_1280)
				count_nxt = count + Const_1;
			state_nxt = VELOCITY;
			LED_output_nxt = LED_output;	
			if(count >= Const_1280)
			begin
				count_nxt = 0; 
				state_nxt = IDLE;
			end
		end
	endcase
	end
end
endmodule