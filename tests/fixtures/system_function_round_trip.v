// Regression fixture (hif-backend#19): a Verilog system function must come
// back out spelled with its '$'.
//
// $clog2 appears twice on purpose, in the two places the printer reaches it
// from: a port's declared width, and an ordinary expression in the body.
module system_function_round_trip #(parameter DEPTH = 16) (
    input wire [$clog2(DEPTH)-1:0] addr,
    input wire [31:0] seed,
    output wire [$clog2(DEPTH)-1:0] next_addr,
    output wire [31:0] scaled);

  assign next_addr = addr + 1'b1;
  assign scaled    = seed * $clog2(DEPTH);
endmodule
