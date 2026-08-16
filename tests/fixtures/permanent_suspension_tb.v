// Testbench for hif-backend#46.
//
// There is no VHDL simulator in this project's environment (no ghdl, no nvc,
// locally or in CI), so permanent_suspension.vhd cannot be simulated and
// compared against. The oracle below is computed by hand from that VHDL.
//
// Compiling is not the property under test. A process regenerated as `always`
// instead of `initial` fails to elaborate here, which the test would catch on
// its own - but it would also catch a fix that merely silenced the loop while
// leaving the process running. So the stimulus proves the processes actually
// *stopped*:
//
//   a is 1 while both processes do their work, and is driven to 0 afterwards.
//   A process that suspended permanently never sees that change. One that
//   looped would run again and pick it up, and both outputs would follow a
//   down to 0.
//
// The `late` samples straddle its 10 ns delay, so a dropped delay is caught
// too rather than passing because the final value happens to be right.
//
// Only `late` is sampled. p_once assigns at time zero, as does this
// testbench's stimulus, so what it captures is a race: `a` may not have been
// driven yet when the design reads it. That is a property of writing both in
// the same time step, not of the translation - and VHDL would give 'U' there
// for the same reason - so asserting a value for `once` would be asserting the
// race. p_once is covered structurally instead, by requiring the regenerated
// module to contain no `always` block at all.

`timescale 1ns / 1ns

module permanent_suspension_tb;

    reg a;
    wire once, late;

    permanent_suspension dut (.a(a), .once(once), .late(late));

    task check;
        input [8*20-1:0] label;
        begin
            $display("%0s late=%b", label, late);
        end
    endtask

    initial begin
        a = 1'b1;

        // Before p_delay's 10 ns has elapsed: once already took a, late is
        // still the 0 it was given at time zero.
        #5  check("t05_before_delay   ");

        // After the delay: late has taken a.
        #10 check("t15_after_delay    ");

        // Change a with both processes suspended. Neither output may move
        // again - and if p_delay had looped, it would reassign late to 0 here
        // and then to the new a ten nanoseconds later.
        #5  a = 1'b0;                    // t = 20
        #15 check("t35_a_changed      ");
        #20 check("t55_still_suspended");

        $finish;
    end

endmodule
