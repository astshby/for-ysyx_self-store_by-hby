//注意！！！：carry只是进行进位判断的，一定要0扩展,进位是给下一位看的，与溢出不同(进位针对无符号数)
//溢出针对有符号数
module op (
    input wire [3:0] a,
    input wire [3:0] b,
    input wire [2:0] sel,
    input wire cin,
    output reg carry,
    output reg [3:0] out 
);
    reg  [4:0] mid;

    always @(*) begin
        case(sel)
            3'b010:begin
               out = ~a;
               carry = 1'b0;
               mid = 5'b00000; 
            end
            3'b011:begin
                out = a & b;
                carry = 1'b0;
                mid = 5'b00000;
            end
            3'b100:begin
                out = a | b;
                carry = 1'b0;
                mid = 5'b00000;
            end
            3'b101:begin
                out = a ^ b;
                carry = 1'b0;
                mid = 5'b00000;
            end
            default:begin
                mid = {1'b0,a} + {1'b0,b} +{4'b0000,cin};
                carry = mid[4];
                out = mid[3:0];
            end 
        endcase
    end
endmodule
