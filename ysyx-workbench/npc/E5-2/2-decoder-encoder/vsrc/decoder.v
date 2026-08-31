module decoder (
    input wire [7:0] in_data,
    input wire en,
    output reg [2:0]led,
    output reg [6:0] out_segments,
    output reg flag
);
    reg [3:0] bcd_digit;
    reg [6:0] k_seg;
    reg [2:0] k_led;
    reg k_flag;
    always @(*) begin
        casez (in_data)
            8'b1???????: begin
                bcd_digit = 4'h7;
                k_flag = 1;
                k_led = 3'b111;
            end
            8'b01??????: begin
                bcd_digit = 4'h6;
                k_flag = 1;
                k_led = 3'b110;
            end
            8'b001?????: begin
                bcd_digit = 4'h5;
                k_flag = 1;
                k_led = 3'b101;
            end
            8'b0001????: begin
                bcd_digit = 4'h4;
                k_flag = 1;
                k_led = 3'b100;
            end
            8'b00001???: begin
                bcd_digit = 4'h3;
                k_flag = 1;
                k_led = 3'b011;
            end
            8'b000001??: begin
                bcd_digit = 4'h2;
                k_flag = 1;
                k_led = 3'b010;
            end
            8'b0000001?: begin
                bcd_digit = 4'h1;
                k_flag = 1;
                k_led = 3'b001;
            end
            8'b00000001: begin
                bcd_digit = 4'h0;
                k_flag = 1;
                k_led = 3'b000;
            end
            default:begin
                bcd_digit = 4'h0;
                k_flag = 0;
                k_led = 3'b000;
            end 
        endcase
    end

    bcd7seg seg_decoder (
        .b(bcd_digit),
        .h(k_seg)
    );

    assign out_segments = en ? k_seg : 7'b1111111; // 使能时输出解码结果，否则全灭
    assign flag = en ? k_flag : 0; // 使能时输出标志，否则为0
    assign led = en ? k_led : 3'b000; // 使能时输出LED状态，否则全灭

endmodule