module concat_emission(input [2:0] a, input b, output [3:0] y);
  assign y = {a, b};
endmodule
