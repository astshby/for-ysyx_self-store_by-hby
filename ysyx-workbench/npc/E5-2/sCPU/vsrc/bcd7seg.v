`timescale 1ns/1ps
module bcd7seg (
    input wire [3:0] b,
    input wire en,
    output reg [6:0] h
);
    always @(*) begin
        if(en) begin
        case(b)
            4'h0: h = 7'b0000001; // 0
            4'h1: h = 7'b1001111; // 1
            4'h2: h = 7'b0010010; // 2
            4'h3: h = 7'b0000110; // 3
            4'h4: h = 7'b1001100; // 4
            4'h5: h = 7'b0100100; // 5
            4'h6: h = 7'b0100000; // 6
            4'h7: h = 7'b0001111; // 7
            4'h8: h = 7'b0000000; // 8
            4'h9: h = 7'b0000100; // 9
            4'hA: h = 7'b0001000; // A
            4'hB: h = 7'b1100000; // B (小写b)
            4'hC: h = 7'b0110001; // C
            4'hD: h = 7'b1000010; // D (小写d，防止和0混淆)
            4'hE: h = 7'b0110000; // E
            4'hF: h = 7'b0111000; // F
            default: h = 7'b1111111; // 默认全灭
        endcase
        end
        else h = 7'b1111111;
    end

endmodule
