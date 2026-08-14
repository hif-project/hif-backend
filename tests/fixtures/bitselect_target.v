// Fixture for the bitselect_target regression (hif-backend#23).
//
// Assigning to a bit-select used to abort hif2verilog: VerilogPrinter's
// visitAssign handed the left-hand side straight to getDeclaration, and a
// bit-select is a Member wrapping the identifier rather than a symbol, so
// the lookup asserted inside hif-core. The process died after the output
// file had been created, leaving it zero bytes.
//
// Both forms are covered, because the abort was in the shared assignment
// printer rather than in anything specific to continuous assignment:
//
//   y  - continuous assignment to a bit-select
//   z  - procedural assignment to a bit-select
//
// The chained form is the one that makes this ordinary rather than exotic:
// a Gray-to-binary converter is written exactly this way, and so is any
// design that builds a vector one bit at a time.

module bitselect_target(input clk, input a, input [3:0] g, output [1:0] y, output reg [1:0] z, output [3:0] b);
  assign y[0] = a;
  assign y[1] = ~a;

  always @(posedge clk) begin
    z[0] <= a;
    z[1] <= ~a;
  end

  // Gray to binary: each bit-select target reads the one above it.
  assign b[3] = g[3];
  assign b[2] = b[3] ^ g[2];
  assign b[1] = b[2] ^ g[1];
  assign b[0] = b[1] ^ g[0];
endmodule
