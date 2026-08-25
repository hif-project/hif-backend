// ----------------------------------------------------------------------------
// Testbench for hif-backend#83, VHDL side.
//
// Self-checking rather than trace-comparing: there is no VHDL simulator in this
// environment, so the reference is what the source says the subprograms compute
// - mask_fn masks with "0011", mask_pr with "0110" - written out here as
// expected values.
//
// Compiling is not enough to prove the fix. The broken output did not build at
// all, so "it builds" would also be satisfied by simply deleting the initial
// value - and that would be the worse outcome: a design that builds, reparses
// and computes x for every input, since m would start at x and `x and a` is x.
// Simulating is what separates "the initialisation moved" from "the
// initialisation was dropped".
//
// Two different masks, so a fix that emitted one subprogram's initial value for
// both would be caught rather than passing on a coincidence.
// ----------------------------------------------------------------------------
`timescale 1ns/1ps

module subprogram_local_initialization_tb;

    reg  [3:0] a;
    wire [3:0] fn_y;
    wire [3:0] pr_y;

    integer failures = 0;

    subprogram_local_initialization dut (.a(a), .fn_y(fn_y), .pr_y(pr_y));

    task check;
        input [3:0] exp_fn;
        input [3:0] exp_pr;
        begin
            if (fn_y === exp_fn && pr_y === exp_pr) begin
                $display("PASS a=%b -> fn_y=%b pr_y=%b", a, fn_y, pr_y);
            end else begin
                $display("FAIL a=%b -> fn_y=%b pr_y=%b (expected fn_y=%b pr_y=%b)",
                         a, fn_y, pr_y, exp_fn, exp_pr);
                failures = failures + 1;
            end
        end
    endtask

    initial begin
        // All ones: the result is the mask itself, so each subprogram's own
        // initial value is read straight off the output.
        a = 4'b1111; #5 check(4'b0011, 4'b0110);

        // A partial overlap with each mask.
        a = 4'b0101; #5 check(4'b0001, 4'b0100);

        // No overlap with mask_fn, full overlap with mask_pr.
        a = 4'b1100; #5 check(4'b0000, 4'b0100);

        // Called twice with the same input, to catch an initialisation that ran
        // once instead of on every call: a VHDL local is initialised per call,
        // and a Verilog function's local is otherwise static. If the assignment
        // were hoisted out of the body, m would carry the previous call's
        // masked value and this row would differ from the first.
        a = 4'b1111; #5 check(4'b0011, 4'b0110);

        if (failures == 0) begin
            $display("ALL CHECKS PASSED");
        end else begin
            $display("%0d CHECK(S) FAILED", failures);
        end
        $finish;
    end

endmodule
