// Second fixture for hif-backend#40: the shape the fix must NOT change.
//
// This process has no sensitivity list either, but it suspends on a `wait`,
// so it does run more than once. Emitting it as `initial` would silently stop
// the design responding after its first pass - which is worse than the bug
// being fixed, because it compiles.
//
// Only checked structurally. hif2verilog does not currently emit the `wait`
// itself (it prints the condition bare, next to the following statement), so
// the regenerated module does not compile for reasons that have nothing to do
// with #40. That is tracked separately; this fixture is here to pin down which
// process keyword is chosen.

module initial_process_wait (
    input  wire       en,
    output reg  [3:0] o
);

    always begin
        wait (en);
        o = 4'd7;
        wait (!en);
        o = 4'd0;
    end

endmodule
