module genlfsr (
    input wire [7:0] num_in,
    output wire [7:0] num_out
);
    wire x;
    wire [7:0] mid_out;
    assign x = num_in[4]^num_in[3]^num_in[2]^num_in[0];
    assign mid_out[7] = x;
    assign mid_out[6:0] = num_in[7:1];
    assign num_out = (num_in == 8'b00000000)? 8'b00000001 : mid_out;

endmodule

//工业界往往不在这里加判断，此处加上也没事
