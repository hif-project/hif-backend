// ----------------------------------------------------------------------------
// Testbench for hif-backend#57.
//
// Self-checking rather than trace-comparing: there is no VHDL simulator in this
// environment, so the reference is what the source says the functions compute,
// written out here as expected values. Each check prints PASS or FAIL with the
// values, so a failure names the case rather than only reporting a diff.
//
// The broken emission returned x from every call, which made every if/elsif
// condition false and drove y and z to 0 in all cases. A test that only checked
// the a=0,b=0 row would pass against that, so all three rows are checked.
// ----------------------------------------------------------------------------
`timescale 1ns/1ps

module user_function_body_tb;

    reg a, b;
    reg [3:0] v;
    wire y, z;
    wire [3:0] w;

    integer failures = 0;

    user_function_body dut (.a(a), .b(b), .y(y), .z(z), .v(v), .w(w));

    task check_yz;
        input exp_y;
        input exp_z;
        begin
            if (y === exp_y && z === exp_z) begin
                $display("PASS a=%b b=%b -> y=%b z=%b", a, b, y, z);
            end else begin
                $display("FAIL a=%b b=%b -> y=%b z=%b (expected y=%b z=%b)", a, b, y, z, exp_y, exp_z);
                failures = failures + 1;
            end
        end
    endtask

    task check_w;
        input [3:0] exp_w;
        begin
            if (w === exp_w) begin
                $display("PASS v=%b -> w=%b", v, w);
            end else begin
                $display("FAIL v=%b -> w=%b (expected w=%b)", v, w, exp_w);
                failures = failures + 1;
            end
        end
    endtask

    initial begin
        // both_high false, either_high false -> else branch.
        a = 1'b0; b = 1'b0; #5 check_yz(1'b0, 1'b0);

        // both_high true -> first branch. Stuck at 0 before the fix.
        a = 1'b1; b = 1'b1; #5 check_yz(1'b1, 1'b0);

        // both_high false, either_high true -> elsif branch. Also stuck at 0.
        a = 1'b1; b = 1'b0; #5 check_yz(1'b0, 1'b1);
        a = 1'b0; b = 1'b1; #5 check_yz(1'b0, 1'b1);

        // The vector-returning function, whose result is a value rather than a
        // branch selector.
        v = 4'b0000; #5 check_w(4'b1111);
        v = 4'b1010; #5 check_w(4'b0101);

        if (failures == 0) begin
            $display("ALL CHECKS PASSED");
        end else begin
            $display("%0d CHECK(S) FAILED", failures);
        end
        $finish;
    end

endmodule
