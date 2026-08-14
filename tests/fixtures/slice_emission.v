// Fixture for the slice_emission regression (hif-backend#18).
//
// Two distinct inputs that both made hif2verilog emit a part-select that is
// not legal Verilog. hif2verilog exited 0 in both cases; the damage only
// showed up on reparse.
//
//   * y    - a slice applied to an expression. The unsized `2` widens the
//            shift to 32 bits, so verilog2hif models the assignment as a
//            narrowing Cast back to 8. That was emitted as a part-select
//            straight after the shift amount, with no parentheses, so it
//            read as a part-select of the constant - and even parenthesised,
//            `(expr)[7:0]` is not legal Verilog-2001.
//
//   * dout - a part-select whose bounds are parameter expressions. Both
//            bounds were printed before the identifier with the brackets
//            left empty:
//                dout <= DEPTH * WIDTH - 1(DEPTH - 1) * WIDTHchain[:];
//            Every piece present, in the wrong order.
//
// The sized-constant and variable-amount shifts are here to keep the
// narrow trigger honest: they round-tripped cleanly before the fix and
// must keep doing so.

module slice_emission #(parameter WIDTH = 4, parameter DEPTH = 3) (
    input clk,
    input [7:0] d,
    input [2:0] amt,
    input [WIDTH-1:0] din,
    output [7:0] y,
    output [7:0] y_sized,
    output [7:0] y_var,
    output [WIDTH-1:0] dout
);
  reg [WIDTH*DEPTH-1:0] chain;

  // Case 1: unsized constant widens the expression, forcing a narrowing
  // cast of an expression.
  assign y = d << 2;

  // Same shape, but already 8-bit wide - no narrowing cast.
  assign y_sized = d << 3'd2;
  assign y_var   = d << amt;

  // Case 2: part-select with parameter-expression bounds.
  always @(posedge clk) chain <= {chain, din};
  assign dout = chain[WIDTH*DEPTH-1:WIDTH*(DEPTH-1)];
endmodule
