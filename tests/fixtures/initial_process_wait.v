// Second fixture for hif-backend#40: the shape the fix must NOT change.
//
// This process has no sensitivity list either, but it suspends on a `wait`,
// so it does run more than once. Emitting it as `initial` would silently stop
// the design responding after its first pass - which is worse than the bug
// being fixed, because it compiles.
//
// Only checked structurally. This fixture is here to pin down which process
// keyword is chosen, not to be simulated.
//
// It used to be structural of necessity: hif2verilog emitted no `wait` at all,
// printing the condition bare next to the following statement, so the
// regenerated module did not compile for reasons unrelated to #40. That was
// hif-backend#42 and is fixed, so this fixture could now be strengthened with
// a testbench if the keyword check ever stops being enough.

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
