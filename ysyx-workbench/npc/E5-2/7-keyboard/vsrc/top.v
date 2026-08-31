`timescale 1ns/1ps
module top (
    input wire clk,
    input wire rst,
    input wire ps2_clk,
    input wire ps2_data,
    output wire [13:0] seg_data_scan,
    output wire [13:0] seg_data_ascii,
    output wire [13:0] seg_data_count
);
    wire ready;
    wire overflow;
    wire en_count;
    wire en_data;
    wire n_n;
    wire [7:0] data;
    wire [7:0] scan_data;
    wire [7:0] ascii_data;
    reg [7:0] count;

    ps2_keyboard ps2_1(
        .clk(clk),
        .clrn(~rst),    //低电平复位
        .ps2_clk(ps2_clk),
        .ps2_data(ps2_data),
        .nextdata_n(n_n),
        .ready(ready),
        .overflow(overflow),
        .data(data)
    );

    keyboard_fsm fsm_1(
        .clk(clk),
        .rst(rst),
        .data(data),
        .ready(ready),
        .overflow(overflow),
        .seg_data(scan_data),
        .nextdata_n(n_n),
        .en_count(en_count),
        .is_pressed(en_data)
    );

    scan2ascii asc_1(
        .scan_code(scan_data),
        .ascii_code(ascii_data)
    );

    always @(posedge clk) begin
        if(rst) count <= 8'h0;
        else begin
            if(en_count) count <= count+8'h1;
            else count <= count;
        end
    end

    bcd7seg bcd_1(
        .en(en_data),
        .b(scan_data[3:0]),
        .h(seg_data_scan[6:0])
    );

    bcd7seg bcd_2(
        .en(en_data),
        .b(scan_data[7:4]),
        .h(seg_data_scan[13:7])
    );

    bcd7seg bcd_3(
        .en(en_data),
        .b(ascii_data[3:0]),
        .h(seg_data_ascii[6:0])
    );

    bcd7seg bcd_4(
        .en(en_data),
        .b(ascii_data[7:4]),
        .h(seg_data_ascii[13:7])
    );

    bcd7seg bcd_5(
        .en(1),
        .b(count[3:0]),
        .h(seg_data_count[6:0])
    );

    bcd7seg bcd_6(
        .en(1),
        .b(count[7:4]),
        .h(seg_data_count[13:7])
    );


endmodule
