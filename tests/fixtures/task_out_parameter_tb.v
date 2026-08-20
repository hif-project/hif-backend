// Testbench for hif-backend#64. Self-checking, so a wrong value names itself.
//
// The design's driving procedures publish literals, so what is checked here does
// not depend on the activation count - which matters, because hif-backend#70
// makes a task's out parameter copy back one activation late. The process is
// sensitive to `a` alone, so `a` is toggled enough times for that lag to have
// drained before the values are read.
//
// y_copy mirrors `a`, so it is subject to that lag directly and its value is
// deliberately not checked - only that it was driven at all, which is what says
// the mixed in/out parameter pair was declared. Tighten it when #70 is fixed.
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

        if (y_copy !== 1'b0 && y_copy !== 1'b1) begin
            $display("FAIL: y_copy is %b - the mixed in/out procedure never drove it", y_copy);
            failures = failures + 1;
        end

        if (failures == 0) begin
            $display("ALL CHECKS PASSED");
        end
        $finish;
    end
endmodule
