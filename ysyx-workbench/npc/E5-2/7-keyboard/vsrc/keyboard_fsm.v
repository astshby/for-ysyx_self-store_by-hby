//永远分为三部分：时序次态赋值，时序状态输出，组合状态转移(内部仍是组合)
//由于FIFO存在，扩展ACT为nextdata_n输出0,多两个时序
`timescale 1ns/1ps
module keyboard_fsm (
    input  wire       clk,
    input  wire       rst,        // 假设高电平复位
    input  wire [7:0] data,       // 来自 FIFO 的数据
    input  wire       ready,      // FIFO 有数据标志
    input  wire       overflow,   // FIFO 溢出标志

    output reg        nextdata_n, // 给 FIFO 的读应答 (低电平有效)
    output reg        en_count,   // 脉冲信号：通知外部计数器 +1
    output reg  [7:0] seg_data,   // 输出给数码管的有效扫描码
    output reg        is_pressed  // 持续信号：1代表按下(亮)，0代表松开(灭)
);

    // 状态编码 (独热码或二进制码)
    localparam IDLE      = 3'd0; // S0: 等待按键按下
    localparam ACK_MAKE  = 3'd1; // S1: 确认读取按下码
    localparam HOLD      = 3'd2; // S2: 按住状态，过滤连发
    localparam ACK_REP   = 3'd3; // S3: 确认读取连发码 (直接丢弃)
    localparam ACK_F0    = 3'd4; // S4: 确认读取断码 F0
    localparam WAIT_REL  = 3'd5; // S5: 等待松开时的最后一个扫描码
    localparam ACK_REL   = 3'd6; // S6: 确认读取松开码

    reg [2:0] state, next_state;

    // 第一段：状态转移 (同步时序)
    // 把 overflow 当作硬复位，实现系统自愈！
    always @(posedge clk) begin
        if (rst || overflow) begin 
            state <= IDLE;
        end else begin
            state <= next_state;
        end
    end

    // 第二段：次态逻辑 (纯组合逻辑，用 = 赋值)
    always @(*) begin
        next_state = state; // 默认保持当前状态，防止锁存器,可以节省下面再写！！！
        case (state)
            IDLE: 
                if (ready) begin
                    if (data == 8'hF0) next_state = ACK_F0; // 过滤单独的F0毛刺
                    else               next_state = ACK_MAKE; // 正常按下！
                end
            
            ACK_MAKE: 
                next_state = HOLD; // 读完马上进入按住状态
            
            HOLD: 
                if (ready) begin
                    if (data == 8'hF0) next_state = ACK_F0;  // 终于松手了，发来了 F0
                    else               next_state = ACK_REP; // 一直按着，发来了连发码
                end
                
            ACK_REP: 
                next_state = HOLD; // 丢掉连发码，继续等松手
                
            ACK_F0: 
                next_state = WAIT_REL; // 读掉 F0，去等最后一个扫描码
                
            WAIT_REL: 
                if (ready) next_state = ACK_REL;
                
            ACK_REL: 
                next_state = IDLE; // 完整结束，回到初始状态等下一个键
                
            default: next_state = IDLE;
        endcase
    end

    // 第三段：输出逻辑,data在IDLE进入就定下了，可以一直press，最后清零，en也是，只需要控制nextdata_n
    always @(posedge clk) begin
        if (rst || overflow) begin
            // 复位/溢出时的绝对安全初始状态
            nextdata_n <= 1'b1;  
            en_count   <= 1'b0;  
            is_pressed <= 1'b0;  
            seg_data   <= 8'h00;
        end else begin
            case (state)
                IDLE: begin
                    nextdata_n <= 1'b1; // FIFO信号一直要注意
                    en_count <= 1'b0;
                    is_pressed <= 1'b0;
                    // data仅仅需要赋值一次！！！
                    // 只有检测到有效按下，才更新数据并点亮。
                    if (ready && data != 8'hF0) begin
                        seg_data   <= data;
                        en_count   <= 1'b1; // 仅在此周期产生计数脉冲，其余无所谓
                        is_pressed <= 1'b1; // 点亮标志
                    end
                end

                ACK_MAKE: begin
                    nextdata_n <= 1'b0; // 发出读取应答 (低电平)
                    en_count   <= 1'b0; // 停止计数
                end

                HOLD: nextdata_n <= 1'b1; // 恢复不读状态，静静等待
                
                ACK_REP: nextdata_n <= 1'b0; // 把讨厌的连发码读走丢掉
                
                ACK_F0: nextdata_n <= 1'b0; // 把断码 F0 读走丢掉
                
                WAIT_REL: nextdata_n <= 1'b1; // 恢复不读状态，等最后一个码
                
                ACK_REL: begin
                    nextdata_n <= 1'b0; // 读走最后一个扫描码
                    is_pressed <= 1'b0; // 向外发出熄灭信号，seg_data无需清零
                end
                
                default: begin
                    nextdata_n <= 1'b1;
                    en_count   <= 1'b0;
                    seg_data   <= 8'h00;
                    is_pressed <= 1'b0;
                end
            endcase
        end
    end

endmodule
