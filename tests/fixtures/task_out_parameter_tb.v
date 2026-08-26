// Testbench for hif-backend#64. Self-checking, so a wrong value names itself.
//
// The design's driving procedures publish literals, so what is checked here does
// not depend on the activation count. The process is sensitive to `a` alone, so
// `a` is toggled a few times before the values are read.
//
// y_copy mirrors `a` rather than a literal, so it is the one value that used to
// be left unchecked: hif-backend#70 made a task's out parameter copy back one
// activation late, and asserting it would have been testing that defect rather
// than this one. #70 is fixed, so it is asserted here now. The activation-timing
// aspect of #70 has its own regression, task_out_parameter_blocking.
module task_out_parameter_tb;
    reg a;
    wire y_high;
    wire y_var;
    wire y_copy;
    integer failures;

    task_out_parameter dut (
        .a(a), .y_high(y_high), .y_var(y_var), .y_copy(y_copy)
    );

    task check;
        input [8*24:1] name;
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

        #1 a = 1'b0;
        #1 a = 1'b1;
        #1 a = 1'b0;
        #1 a = 1'b1;
        #1;

        // Each of these is what its procedure assigns unconditionally. Before
        // the fix the design did not compile at all, so these pin the mapping of
        // the parameter to a real task argument whose value is copied back -
        // which is more than "it parses now".
        check("y_high (signal : out)",  y_high, 1'b1);
        check("y_var (variable : out)", y_var,  1'b1);

        // Mirrors `a` through a procedure taking one `in` and one `out` signal
        // parameter, so this is both "the pair was declared" and "the copy-back
        // carries the value of this activation, not the last one".
        check("y_copy (in + out)", y_copy, a);

        if (failures == 0) begin
            $display("ALL CHECKS PASSED");
        end
        $finish;
    end
endmodule
