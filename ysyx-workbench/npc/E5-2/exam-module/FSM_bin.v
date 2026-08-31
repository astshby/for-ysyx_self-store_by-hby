// ! 学会带参模块
module FSM_bin(
  input  wire clk, in, reset,
  output reg out
);

//参数可以带上[]
parameter[3:0] S0 = 0, S1 = 1, S2 = 2, S3 = 3,
          S4 = 4, S5 = 5, S6 = 6, S7 = 7, S8 = 8;

wire [3:0] state_din, state_dout;
wire state_wen;

//下方使用带数据的模块，格式：模块类 #(参数) 模块名字 (引脚)
//Reg:#(位宽,重置值)(clk,rst,din,dout ,wen)
//MuxKeyInternal:#(查找表数量,键宽,值宽,是否有默认值)(out ,key,default ,lut({查找表内容}))
//状态保存模块
Reg #(4,0) state(clk, reset, state_din, state_dout, state_wen);

assign state_wen = 1;

//输出模块
MuxKeyInternal #(9, 4, 1) outMux(.out(out), .key(state_dout), .default_out(0), .lut({
  S0, 1'b0,
  S1, 1'b0,
  S2, 1'b0,
  S3, 1'b0,
  S4, 1'b1,
  S5, 1'b0,
  S6, 1'b0,
  S7, 1'b0,
  S8, 1'b1
}));

//状态转移模块
MuxKeyInternal #(9, 4, 4) stateMux(.out(state_din), .key(state_dout), .default_out(S0), .lut({
  S0, in ? S5 : S1,
  S1, in ? S5 : S2,
  S2, in ? S5 : S3,
  S3, in ? S5 : S4,
  S4, in ? S5 : S4,
  S5, in ? S6 : S1,
  S6, in ? S7 : S1,
  S7, in ? S8 : S1,
  S8, in ? S8 : S1
}));

endmodule

//状态机分类:(1)in:同步/异步 (2)状态:one hot,gray,顺序 (3)输出:moore，mealy(前者输入不影响输出，后者影响)(AXI总线会用mealy)
//         (4)状态机本身:FSM，EFSM:基于数据+基于变量转移
//如何写mealy:
/*
MuxKeyInternal #(9, 4, 1) outMux(.out(out), .key(state_dout), .default_out(0), .lut({
  S0, 1'b0,
  S1, 1'b0,
  S2, 1'b0,
  S3, in ? 1'b1:1'b0,
  S4, 1'b1,
  S5, 1'b0,
  S6, 1'b0,
  S7, in ? 1'b1:1'b0,
  S8, 1'b1
}));
*/
