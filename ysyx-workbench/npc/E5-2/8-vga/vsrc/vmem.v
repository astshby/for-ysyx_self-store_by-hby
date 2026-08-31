// 读取要上板的图片的hex,相当与一个存储卡
// 建议加入时钟
module vmem(
    input [9:0] h_addr,
    input [8:0] v_addr,
    input clk,
    output reg [23:0] vga_data
);

    // 不更改10位，10位，所以就用其中10位，9位，多余的地方（nvboard也不显示）就在python补充黑色
    reg [23:0] vga_mem [524287:0];  

    initial begin
        $readmemh("build/picture.hex", vga_mem);
    end

    //由于是python原因,调整顺序
    always @(posedge clk) begin
        vga_data <= vga_mem[{v_addr, h_addr}];
    end 

endmodule

