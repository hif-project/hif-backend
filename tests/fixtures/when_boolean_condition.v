// ----------------------------------------------------------------------------
// Fixture for hif-backend#54.
//
// A Verilog conditional (ternary) lowered for VHDL produced an If whose
// condition kept the Verilog one-bit type. VHDL's IF accepts only boolean, so
// hif-core's checker rejected the tree and hif2vhdl aborted at exit 134 with
// assertions enabled - which is how CI builds - and, in a Release build,
// emitted questionable VHDL at exit 0 instead.
//
// Three shapes, because they do not share a code path:
//
//   z  ternary whose condition is a bare bit, driven concurrently. Reaches
//      the when_function lowering (case 4 of PreRefine_misc::visitWhen).
//   w  ternary whose condition is a comparison. Verilog `==` also yields a
//      one-bit value rather than a boolean, so this fails identically before
//      the fix - and must come out as a plain VHDL boolean comparison rather
//      than a redundant `(a = b) = '1'`.
//   q  ternary inside a process, so the When is the source of an Assign and
//      reaches the If lowering (case 3) instead. Both call sites need the
//      coercion; a fix applied to only one passes for the other's shape.
// ----------------------------------------------------------------------------
module when_boolean_condition(
    input  wire a,
    input  wire b,
    input  wire sel,
    input  wire gate,
    output wire z,
    output wire w,
    output reg  q
);
    assign z = sel ? a : b;
    assign w = (a == b) ? a : b;

    always @(a or b or sel or gate) begin
        q = sel ? a : b;
        if (gate) q = 1'b0;
    end
endmodule
