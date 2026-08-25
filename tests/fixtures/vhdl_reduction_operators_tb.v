// ----------------------------------------------------------------------------
// Testbench for hif-backend#92.
//
// The design under test here is the one that has been round-tripped through
// VHDL: Verilog -> HIF -> VHDL -> HIF -> Verilog. The reference is Verilog's
// own reduction operators, evaluated in this testbench on the same input, so
// the comparison is against the language rather than against a hand-written
// table that could encode the same mistake as the lowering.
//
// Sweeping all sixteen values of `a` rather than a chosen few: a chain built
// with the wrong bit operator, or over the wrong bits, agrees with the right
// one on some inputs. `and_red` in particular is 1 only for a=1111, so a
// single unlucky sample would prove nothing.
// ----------------------------------------------------------------------------
`timescale 1ns/1ps

module vhdl_reduction_operators_tb;

    reg  [3:0] a;
    reg  [3:0] b;
    wire or_red;
    wire and_red;
    wire xor_red;
    wire [3:0] bitwise;

    integer i;
    integer failures = 0;

    vhdl_reduction_operators dut (
        .a(a), .b(b), .or_red(or_red), .and_red(and_red), .xor_red(xor_red), .bitwise(bitwise));

    initial begin
        b = 4'b1010;

        for (i = 0; i < 16; i = i + 1) begin
            a = i[3:0];
            #5;
            if (or_red === (|a) && and_red === (&a) && xor_red === (^a) && bitwise === ((a | b) & (a ^ b))) begin
                $display("PASS a=%b -> or=%b and=%b xor=%b bitwise=%b", a, or_red, and_red, xor_red, bitwise);
            end else begin
                $display("FAIL a=%b -> or=%b and=%b xor=%b bitwise=%b (expected %b %b %b %b)",
                         a, or_red, and_red, xor_red, bitwise, (|a), (&a), (^a), ((a | b) & (a ^ b)));
                failures = failures + 1;
            end
        end

        if (failures == 0) begin
            $display("ALL CHECKS PASSED");
        end else begin
            $display("%0d CHECK(S) FAILED", failures);
        end
        $finish;
    end

endmodule
