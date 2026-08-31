//补充：寄存器写 Q = {Q[7],Q[7:1]} 会生成一个缓慢的移位寄存器，与普遍的寄存器不一样！！！，不可用在cpu中(太慢)
//普通寄存器就是提供一个输入输出的接口，移位逻辑由alu完成组合(桶形移位器),在组合逻辑写为：assign out = in << shift_num,且要注意才能综合
//何以实现？：4位选择器版：对shift_num解码为移动1,2,4...位，每个选择器接：本位+两个可能移动位+可能数位，由不同的信号选择，对sn的每位进行级联

//此处加上复位信号，可以异步复位,但是也要在上升沿，否则会上升下降都触发瞬间变换
module lfsr (
    input wire clk,
    input wire rst,
    input wire load,
    input wire [7:0] in,
    output reg [13:0] seg 
);
    reg [7:0] out;
    reg [7:0] num;

    always @(posedge clk or posedge rst) begin
        if(rst) num <= 8'b00000001;
        else begin
            if(load) num <= in;
            else num <= out;
        end
    end

    genlfsr kks(
        .num_in(num),
        .num_out(out)
    );

    bcd7seg a1(
        .b(num[3:0]),
        .h(seg[6:0])
    );

    bcd7seg a2(
        .b(num[7:4]),
        .h(seg[13:7])
    );

endmodule
