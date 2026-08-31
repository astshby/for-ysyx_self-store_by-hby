// 总体：clk输入，得到pclk，然后vga告诉位置，rom(存储图片)给data，然后由vga负责输出信号
module top(
    input  wire clk,   
    input  wire rst,   
    // VGA 物理接口 (看看N4与nvboard的实例)
    output VGA_CLK,
    output wire VGA_HSYNC,   // 行同步
    output wire VGA_VSYNC,   // 场同步
    output wire VGA_BLANK_N, // 图像有效
    output wire [7:0] VGA_R, // 红色
    output wire [7:0] VGA_G, // 绿色
    output wire [7:0] VGA_B  // 蓝色
);

    wire pclk; // 内部的 25MHz 时钟线
    wire [9:0] h_addr;   
    wire [9:0] v_addr;     // 640 × 480 最后一位不用了
    wire [23:0] vga_data;
    assign pclk = clk;      
    assign VGA_CLK = pclk & ~rst;   //为了解决复位问题，在rst永远不输入外界时序

    // 时钟生成器 (生成 25MHz)
    // 根据nvboard，这个东西上实际板子很必要，上nvboard不必要
    /*
    clkgen #(25000000) my_clkgen (
        .clkin(clk), 
        .rst(rst), 
        .clken(1'b1),  // clk_en让它一直跑
        .clkout(pclk)
    );
    */

    // 图片ROM数据得出
    vmem my_rom (
        .clk(clk),
        .h_addr(h_addr),      
        .v_addr(v_addr[8:0]), 
        .vga_data(vga_data)
    );

    // 实例化 VGA 控制器，有进有出，并不是永远单项(要得到信号地址，然后得到信号后输出给外设)
    vga_ctrl my_vga (
        .pclk(pclk),
        .reset(1'b0),          //为了结局复位问题，不复位了
        .vga_data(vga_data), 
        .h_addr(h_addr),     
        .v_addr(v_addr),
        .hsync(VGA_HSYNC),   
        .vsync(VGA_VSYNC),
        .valid(VGA_BLANK_N),
        .vga_r(VGA_R),
        .vga_g(VGA_G),
        .vga_b(VGA_B)
    );

endmodule
