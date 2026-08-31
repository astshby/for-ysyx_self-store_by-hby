`timescale 1ns/1ps
module top(
    input clk,
    input rst,
    output wire [13:0] ans
  );

    wire [7:0] ans_kks;
    
    wire [7:0] inst_s;
    wire en_w;
    wire if_addr;
    wire en_addr;
    wire en_imm;
    wire [1:0] rd; 
    wire [1:0] rs1; 
    wire [1:0] rs2; 
    wire [7:0] n_write;
    wire [7:0] rs1_ans;
    wire [7:0] rs2_ans;
    wire [7:0] r0;
    wire [3:0] imm;
    wire [3:0] addr;

    reg  [3:0] PC;

    rom rom_kks(
        .PC(PC),
        .inst_s(inst_s)
    );

    inst inst_kks(
        .inst_s(inst_s),
        .en_write(en_w),
        .en_addr(if_addr),
        .en_imm(en_imm),
        .rd(rd),
        .rs1(rs1),
        .rs2(rs2),
        .addr(addr),
        .imm(imm)
    );

    reg_s reg_kks(
        .clk(clk),
        .rst(rst),
        .en_w(en_w),
        .rd(rd),
        .rs1(rs1),
        .rs2(rs2),
        .rs0(2'b00),
        .rs1_ans(rs1_ans),
        .rs2_ans(rs2_ans),
        .rs0_ans(r0),
        .need_write(n_write),
        .sp(2'b10),
        .sp_ans(ans_kks)
    );
    
    alu alu_kks(
        .rs0_d(r0),
        .rs1_d(rs1_ans),
        .rs2_d(rs2_ans),
        .if_addr(if_addr),
        .en_imm(en_imm),
        .imm(imm),
        .en_addr(en_addr),
        .need_write(n_write)
    );
    
    always @(posedge clk or posedge rst) begin
        if(rst) begin
            PC <= 4'b0;
        end
        else begin
            if(en_addr) PC <= addr;
            else PC <= PC + 4'b1; 
        end
    end


    bcd7seg bcd1(
        .en(1'b1),
        .b(ans_kks[3:0]),
        .h(ans[6:0])
    );

    bcd7seg bcd2(
        .en(1'b1),
        .b(ans_kks[7:4]),
        .h(ans[13:7])
    );

endmodule
