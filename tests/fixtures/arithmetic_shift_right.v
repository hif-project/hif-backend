// Fixture for the arithmetic right shift regression.
//
// hif2verilog printed HIF's op_sra as the literal token "sra" instead of
// Verilog's ">>>", so the regenerated file parsed as neither Verilog nor
// input to verilog2hif.
//
// `>>` is present as a control: the logical shifts were never affected, and
// having both operators in one module keeps the test honest about which one
// changed.
//
// The assertions on this fixture are on the emitted syntax and on the
// regenerated file reparsing, not on simulated values. That is deliberate:
// the design is also affected by #81 (the `signed` qualifier is dropped from
// ports and signed operands are zero-extended), so a behavioural check on
// sign extension would fail for that defect rather than this one.
module arithmetic_shift_right(
    input signed [7:0] a,
    input [2:0] amt,
    output signed [7:0] arith,
    output [7:0] logical
);
    assign arith   = a >>> amt;
    assign logical = a >> amt;
endmodule
