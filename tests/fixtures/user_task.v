// Fixture for hif-backend#38: a user-defined Verilog task was inlined as if it
// were a frontend-synthesized logic cone, which then tripped the
// hif-backend#16 assert and left a zero-byte output file.
//
// The design deliberately holds both kinds of subprogram at once, because the
// fix is a distinction between them rather than a change to either:
//
//   doit     a task whose body writes a module-level target. This is what
//            aborted: a cone assigns to a "_sig_var" Variable, a task assigns
//            to whatever the source told it to.
//
//   chain    a task read by a *later statement in the same block*, which is
//            the staleness hif-backend#16 is about. The frontend represents
//            `carry` as a Variable, so the task body keeps blocking "=" and
//            the read that follows sees the value just written. If that ever
//            stopped holding, this would simulate differently rather than
//            merely look different.
//
//   the gate a pair of chained primitive gate instances. The frontend lowers
//   chain    the shared intermediate into a real cone procedure, which must
//            still be inlined at its call site - so the fix cannot simply stop
//            inlining every Procedure. Chained rather than single because a
//            gate whose output nothing reads needs no cone at all.
//
// The always block reads only module inputs, for the reason system_task_calls.v
// gives: a process that both triggers on a and b and reads a gate-driven output
// of those same inputs is a race in plain Verilog, and a trace built on one
// would compare two well-formed designs and call the difference a regression.

module user_task (
    input  wire a,
    input  wire b,
    output reg  y,
    output reg  z,
    output wire g
);

    reg  carry;
    wire axb;

    // Lowered by the frontend into a cone procedure, inlined at its call site.
    xor x1 (axb, a, b);
    xor x2 (g, axb, 1'b0);

    task doit;
        input v;
        begin
            y = ~v;
        end
    endtask

    task chain;
        input v;
        begin
            carry = v & 1'b1;
        end
    endtask

    always @(a or b) begin
        doit(a);
        chain(b);
        z = carry;
    end

endmodule
