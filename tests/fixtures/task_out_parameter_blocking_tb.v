// Testbench for hif-backend#70. Self-checking, so a wrong value names itself.
//
// The two checks are timed differently on purpose, because the two directions
// of the defect show up differently:
//
//   y_out    is read at the *first* activation. The out-parameter lag is a
//            one-activation delay, so it drains: reading later would see the
//            right value for the wrong reason and the test would pass against
//            the defect it exists to catch.
//
//   y_inout  is read after four activations. The inout copy-in/copy-out pair
//            never converges, so the point there is the opposite one - that no
//            amount of running fixes it.
//
// Both procedures assign literals, so neither expected value depends on how
// many times the process has run.
module task_out_parameter_blocking_tb;
    reg a;
    wire y_out;
    wire y_inout;
    integer failures;

    task_out_parameter_blocking dut (
        .a(a), .y_out(y_out), .y_inout(y_inout)
    );

    task check;
        input [8*32:1] name;
        input actual;
        input expected;
        begin
            if (actual !== expected) begin
                $display("FAIL: %0s is %b, expected %b", name, actual, expected);
                failures = failures + 1;
            end
        end
    endtask

    initial begin
        failures = 0;

        // First activation of the process, and the only one that can tell a
        // correct copy-back from a lagging one.
        #1 a = 1'b0;
        #1 check("y_out after 1st activation", y_out, 1'b1);

        // Three more, so the inout check below is about non-convergence rather
        // than about being early.
        #1 a = 1'b1;
        #1 a = 1'b0;
        #1 a = 1'b1;
        #1 check("y_inout after 4 activations", y_inout, 1'b0);

        if (failures == 0) begin
            $display("ALL CHECKS PASSED");
        end
        $finish;
    end
endmodule
