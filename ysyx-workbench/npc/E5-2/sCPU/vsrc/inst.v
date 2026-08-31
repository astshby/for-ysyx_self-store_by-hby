`timescale 1ns/1ps
module inst (
    input [7:0] inst_s,
    output reg en_write,
    output reg en_imm,
    output reg en_addr,
    output reg [1:0] rd,
    output reg [1:0] rs1,
    output reg [1:0] rs2,
    output reg [3:0] addr,
    output reg [3:0] imm
);
    wire [1:0] kks;
    assign kks = inst_s[7:6];

    always @(*) begin
        en_write = 1'b0;
        en_imm = 1'b0;
        en_addr = 1'b0;
        rd = inst_s[5:4];
        rs1 = inst_s[3:2];
        rs2 = inst_s[1:0];
        addr = inst_s[5:2];
        imm = inst_s[3:0];

        case(kks)
            2'b00:begin
                en_write = 1'b1;
                en_imm = 1'b0;
                en_addr = 1'b0;
            end

            2'b10:begin
                en_write = 1'b1;
                en_imm = 1'b1;
                en_addr = 1'b0;
            end

            2'b11:begin
                en_write = 1'b0;
                en_imm = 1'b0;
                en_addr = 1'b1;
            end

            default:begin
                en_write = 1'b0;
                en_imm = 1'b0;
                en_addr = 1'b0;
            end
        endcase
    end
endmodule
