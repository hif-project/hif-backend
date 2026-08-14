// Fixture for the sensitivity_equivalence regression.
//
// Covers, in one design, every shape in which hif-frontend's logic-cone
// factoring (FixDescription_3.cpp's generateConeFunctions/addConesPCalls)
// interacts with process emission:
//
//   * t          - a cone read by TWO combinational processes (shared cone,
//                  one Procedure with two call sites).
//   * s1, c1, c2 - cones produced by flattening submodule instances, one of
//                  which (c2) reads another cone's target (s1).
//   * lat        - a LEVEL-sensitive clock process that reads a cone target.
//                  This one is the guard rail: `always @(clk)` must keep
//                  updating only when clk changes. Emitting `always @(*)`
//                  for it, or adding t to its sensitivity list, turns it
//                  into a transparent latch - a different bug traded for
//                  the one being fixed.
//
// The test drives all 8 input vectors, toggling a/b/cin while clk is held
// stable so that a latch-vs-register difference on `lat` is observable, and
// compares the simulation trace of this file against the trace of the
// Verilog hif2verilog regenerates from it.

module half_adder(input a, input b, output s, output c);
  assign s = a ^ b;
  assign c = a & b;
endmodule

module sensitivity_equivalence(
    input a,
    input b,
    input cin,
    input clk,
    output sum,
    output cout,
    output shared_y,
    output shared_z,
    output reg lat
);
  wire s1, c1, c2;
  wire t;

  // Chained continuous assigns: shared_y/shared_z both read t, which is
  // itself driven by a continuous assign. Reading a cone target from a
  // process that is not sensitive to it is the defect under test.
  assign t = a & b;
  assign shared_y = t ^ a;
  assign shared_z = t | b;

  // Full adder from two half adders - the hierarchical shape reported in
  // hif-backend#16 as differing on 3 of 8 vectors.
  half_adder h0(.a(a),  .b(b),   .s(s1),  .c(c1));
  half_adder h1(.a(s1), .b(cin), .s(sum), .c(c2));
  assign cout = c1 | c2;

  // Level-sensitive clock reading a cone target. Must stay clk-triggered.
  always @(clk) lat <= t;
endmodule
