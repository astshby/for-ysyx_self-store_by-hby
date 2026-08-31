`timescale 1ns/1ps
module alu (
    input wire [7:0] rs0_d,
    input wire [7:0] rs1_d,
    input wire [7:0] rs2_d,
    input wire [3:0] imm,
    input wire if_addr,
    input wire en_imm,
    output reg [7:0] need_write,
    output reg en_addr 
);
    always @(*) begin
        en_addr = 1'b0;
        need_write = 8'b0;

        if(en_imm) need_write = {4'b0000,imm};
        else need_write = rs1_d + rs2_d;
        
        if(if_addr) begin
            if(rs0_d != rs2_d) en_addr = 1'b1;
            else en_addr = 1'b0;
        end
    end
endmodule
