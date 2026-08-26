// ----------------------------------------------------------------------------
// Testbench for hif-backend#84.
//
// Self-checking: the reference is what the source's priority encoders compute,
// written out here as expected values.
//
// The broken emission turned every wildcard alternative unreachable, so both
// encoders answered 3 - the default - for every input. Sweeping all sixteen
// values of req rather than a chosen few means the check does not depend on
// picking the right one: fourteen of the sixteen rows differ under the defect,
// and only req=0000 and req=1000 agree by coincidence.
//
// `plain` is the control: it must keep answering from a literal `case`, so a
// fix that emitted a wildcard keyword unconditionally shows up here.
// ----------------------------------------------------------------------------
`timescale 1ns/1ps

module casez_wildcards_tb;

    reg  [3:0] req;
    reg  [1:0] sel;
    wire [1:0] grant_z;
    wire [1:0] grant_x;
    wire [1:0] plain;

    integer i;
    integer failures = 0;

    casez_wildcards dut (.req(req), .sel(sel), .grant_z(grant_z), .grant_x(grant_x), .plain(plain));

    // Lowest set bit wins; 3 when nothing below bit 3 is set.
    function [1:0] expected_grant;
        input [3:0] r;
        begin
            if (r[0])      expected_grant = 2'd0;
            else if (r[1]) expected_grant = 2'd1;
            else if (r[2]) expected_grant = 2'd2;
            else           expected_grant = 2'd3;
        end
    endfunction

    initial begin
        sel = 2'b00;

        for (i = 0; i < 16; i = i + 1) begin
            req = i[3:0];
            #5;
            if (grant_z === expected_grant(req) && grant_x === expected_grant(req)) begin
                $display("PASS req=%b -> grant_z=%b grant_x=%b", req, grant_z, grant_x);
            end else begin
                $display("FAIL req=%b -> grant_z=%b grant_x=%b (expected %b for both)",
                         req, grant_z, grant_x, expected_grant(req));
                failures = failures + 1;
            end
        end

        // The literal-case control.
        sel = 2'b00; #5;
        if (plain !== 2'd0) begin
            $display("FAIL sel=00 -> plain=%b (expected 00)", plain);
            failures = failures + 1;
        end else $display("PASS sel=00 -> plain=%b", plain);

        sel = 2'b01; #5;
        if (plain !== 2'd1) begin
            $display("FAIL sel=01 -> plain=%b (expected 01)", plain);
            failures = failures + 1;
        end else $display("PASS sel=01 -> plain=%b", plain);

        sel = 2'b10; #5;
        if (plain !== 2'd3) begin
            $display("FAIL sel=10 -> plain=%b (expected 11)", plain);
            failures = failures + 1;
        end else $display("PASS sel=10 -> plain=%b", plain);

        if (failures == 0) begin
            $display("ALL CHECKS PASSED");
        end else begin
            $display("%0d CHECK(S) FAILED", failures);
        end
        $finish;
    end

endmodule
