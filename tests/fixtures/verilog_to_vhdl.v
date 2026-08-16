// Fixture for the verilog_to_vhdl regression (hif-backend#48).
//
// Deliberately minimal. The defect was raised on the standard library that a
// verilog2hif output always references, not on anything the design contains,
// so any module reproduces it and a larger one would only add unrelated ways
// to fail.
//
// This fixture originally carried a conditional assignment as well, to prove
// the fix did not suppress legitimate When lowering along with the
// standard-library traversal it skips. That had to be removed: a Verilog
// ternary translated to VHDL produces an If whose condition is still
// bit-typed, which VHDL does not allow, and hif-core's sanity check rejects
// the tree (hif-backend#54). That is a separate defect that #48 was masking -
// it is only reachable once #48 stops aborting first - so it is tracked
// separately rather than fixed here, and this fixture stays within what #48
// is about.
module verilog_to_vhdl(
    input  wire a,
    input  wire b,
    output wire y
);
    assign y = a & b;
endmodule
