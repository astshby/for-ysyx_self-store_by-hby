`timescale 1ns/1ps
module reg_s (
    input wire clk,
    input wire rst,
    input wire en_w,
    input wire [1:0] rd,
    input wire [1:0] rs1,
    input wire [1:0] rs2,
    input wire [1:0] rs0,
    input wire [1:0] sp,
    input wire [7:0] need_write,
    output wire [7:0] rs1_ans,
    output wire [7:0] rs2_ans,
    output wire [7:0] rs0_ans,
    output wire [7:0] sp_ans
);
    reg [7:0] reg_main [3:0];

    integer i;
    always @(posedge clk or posedge rst) begin
        if(rst) begin
            for(i=0 ;i<4 ;i++) reg_main[i] <= 8'b0;
        end
        else begin
            if(en_w) begin
                reg_main[rd] <= need_write;
            end
        end
    end

    assign rs1_ans = reg_main[rs1];
    assign rs2_ans = reg_main[rs2];
    assign sp_ans = reg_main[sp];
    assign rs0_ans = reg_main[rs0];
    

endmodule
