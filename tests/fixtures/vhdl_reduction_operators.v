// Fixture for hif-backend#92.
//
// hif2vhdl aborted on every Verilog reduction operator - exit 134 on the
// Log.cpp assertion, "This operator should be managed in refinement steps",
// with no VHDL written. VHDL before 2008 has no reduction operator, so the
// three of them need a lowering rather than a token.
//
// All three are here because all three hit the same refusal, and each lowers
// to a different bit operator, so a lowering that picked one of them for
// everything would be caught.
//
// `bitwise` is a control: it uses the same three operators in their ordinary
// binary form, which hif2vhdl has always printed correctly and which the
// lowering must not disturb.
module vhdl_reduction_operators(
    input  [3:0] a,
    input  [3:0] b,
    output or_red,
    output and_red,
    output xor_red,
    output [3:0] bitwise
);
    assign or_red  = |a;
    assign and_red = &a;
    assign xor_red = ^a;
    assign bitwise = (a | b) & (a ^ b);
endmodule
