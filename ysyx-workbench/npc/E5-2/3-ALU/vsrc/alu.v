//用case写当然方便，但是必须要优化
//位运算直接case运算，其余四种用一个加法器，通过判断对b进行操作,并进行模块化
//进位与溢出不是同种运算，进位直接添加1bit进行共享，溢出用判断
//大小比较和zero可以共用，比较结果的符号位与是否为0就可以
//先写中间信号，最后写输出信号
module alu (
    input wire [3:0] a,
    input wire [3:0] b,
    input wire [2:0] sel,
    output wire [3:0] out,   //由于最后直接赋值，所以都是wire
    output wire zero,
    output wire overflow,
    output wire carry
);

    //is_sub可以用于后续的初始进位
    wire is_sub;
    wire [3:0] b_op;
    wire [3:0] mid_out;
    wire mid_carry;
    wire mid_zero;
    wire mid_overflow;
    wire is_ocz;

    assign is_sub = sel==3'b001 || sel==3'b110 || sel==3'b111;
    assign b_op = is_sub ? ~b: b;
    assign is_ocz = (sel == 3'b000 || sel == 3'b001);
    //可优化加法
    op kks(.a(a),.b(b_op),.sel(sel),.cin(is_sub),.carry(mid_carry),.out(mid_out));
    assign mid_zero = (mid_out == 4'b0000);
    assign mid_overflow = (a[3]==b_op[3]) && (a[3]!=mid_out[3]);

    assign carry = is_ocz ? mid_carry : 1'b0;
    assign overflow = is_ocz ? ((a[3]==b_op[3]) && (a[3]!=mid_out[3])) : 1'b0;
    assign zero = is_ocz ? mid_zero : 1'b0;
    assign out = (sel == 3'b110) ? {3'b000, mid_out[3] ^ mid_overflow} :       //通过溢出修正小于(由于是小于即1开头就可以判断，所以无需进一步改)
                 (sel == 3'b111) ? {3'b000, mid_zero} : 
                 mid_out; 
endmodule
