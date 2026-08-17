// Fixture for the verilog_to_vhdl regression (hif-backend#48).
//
// Deliberately minimal. The defect was raised on the standard library that a
// verilog2hif output always references, not on anything the design contains,
// so any module reproduces it and a larger one would only add unrelated ways
// to fail.
//
// The conditional assignment proves the fix did not suppress legitimate When
// lowering along with the standard-library traversal it skips: without it,
// disabling When lowering outright would pass this test. It was removed for a
// time because a Verilog ternary translated to VHDL produced an If whose
// condition was still bit-typed, which VHDL does not allow and hif-core's
// sanity check rejected (hif-backend#54). That is now fixed, so the ternary is
// back where it belongs.
module verilog_to_vhdl(
    input  wire a,
    input  wire b,
    input  wire sel,
    output wire y,
    output wire z
);
    assign y = a & b;
    assign z = sel ? a : b;
endmodule
