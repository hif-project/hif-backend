// Testbench for hif-backend#63 and #73. Self-checking, so a wrong value names
// which subprogram produced it.
//
// The three outputs are independent functions of `a`, and each is wrong in a
// different, recognisable way when the early exit is missing:
//
//   y_proc  the procedure's guarded branch is followed by an unconditional
//           assignment, so without the exit y_proc is stuck at 1.
//   y_func  the function's trailing `return '0'` overwrites the guarded one,
//           so without the exit y_func is stuck at 0.
//   y_tail  the single-return control. It has no early exit to lose and must
//           track `a` either way; it is here to catch a lowering that damages
//           the ordinary shape.
module subprogram_early_return_tb;
    reg a;
    wire y_proc;
    wire y_func;
    wire y_tail;
    integer failures;

    subprogram_early_return dut (
        .a(a), .y_proc(y_proc), .y_func(y_func), .y_tail(y_tail)
    );

    task expect;
        input [8*8:1] name;
        input actual;
        input value;
        begin
            if (actual !== value) begin
                $display("FAIL: with a=%b, %0s is %b, expected %b", a, name, actual, value);
                failures = failures + 1;
            end
        end
    endtask

    task sample;
        input value;
        begin
            a = value;
            #1;
            expect("y_proc", y_proc, value);
            expect("y_func", y_func, ~value);
            expect("y_tail", y_tail, value);
        end
    endtask

    initial begin
        failures = 0;

        // Both directions, and each twice, so a value that is merely one
        // activation late would still be caught rather than coincidentally
        // matching.
        #1 sample(1'b1);
        #1 sample(1'b0);
        #1 sample(1'b1);
        #1 sample(1'b0);

        if (failures == 0) begin
            $display("ALL CHECKS PASSED");
        end
        $finish;
    end
endmodule
