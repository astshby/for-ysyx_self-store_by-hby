`timescale 1ns/1ps
module rom (
    input wire [3:0] PC,
    output reg [7:0] inst_s
);
   always @(*) begin
        case(PC)
            4'd0: inst_s=8'b10001010;    // 0: li r0, 10
            4'd1: inst_s=8'b10010000;    // 1: li r1, 0
            4'd2: inst_s=8'b10100000;    // 2: li r2, 0
            4'd3: inst_s=8'b10110001;    // 3: li r3, 1
            4'd4: inst_s=8'b00010111;    // 4: add r1, r1, r3
            4'd5: inst_s=8'b00101001;    // 5: add r2, r2, r1
            4'd6: inst_s=8'b11010001;    // 6: bner0 r1, 4
            4'd7: inst_s=8'b11011111;    // 7: bner0 r3, 7
            default: inst_s=8'b00000000;
        endcase
   end
endmodule
