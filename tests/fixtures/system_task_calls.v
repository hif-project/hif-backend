// Fixture for hif-backend#29: hif2verilog emitted nothing at all for a Verilog
// system *task* call. hif-core declares these as subprograms with no return
// type and no body, so the call resolved to a Procedure whose StateTable is
// null and visitProcedureCall's "no body to inline" early return dropped it
// without a word. Exit 0, and the output parsed and reparsed cleanly.
//
// Deliberately carries logic cones as well. visitProcedureCall is also what
// inlines frontend-synthesized cones at their call sites, and that path
// carries the hif-backend#16 contract, so the two have to be shown coexisting
// in one design rather than one being fixed at the other's expense. The gate
// primitives below are what make the frontend synthesize a cone;
// gate_primitives.v exists for exactly that reason.
//
// The printing process reads only module inputs, on purpose. A process that
// both triggers on a and b and reads a gate-driven output of those same
// inputs is a race in plain Verilog - whether it observes the settled value
// depends on scheduling - so a trace built on one would compare two
// well-formed designs and call the difference a regression.
module system_task_calls(input a, input b, output sum, output cout);

  wire axb, ab, axb_cin;

  // Cone territory: primitive gate instances, which hif-frontend turns into
  // cone Procedures that visitProcedureCall expands at the call site.
  xor x1(axb, a, b);
  xor x2(sum, axb, 1'b0);
  and a1(ab, a, b);
  and a2(axb_cin, axb, 1'b0);
  or  o1(cout, ab, axb_cin);

  // System tasks, in three shapes the fix has to get right independently.
  always @(a or b) begin
    // Arguments, the first of which is a string literal - the format string
    // that carries the message. Emitting the call without it restores nothing.
    $display("a=%b b=%b", a, b);
    // A string containing characters that must survive quoting. Verilog
    // stores these escapes in source form, so re-escaping them would change
    // what the literal says.
    $display("quote=\" backslash=\\ done");
    // A single argument, and a task that is not $display.
    $write("w\n");
  end

endmodule
