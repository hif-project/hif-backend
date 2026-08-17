// ----------------------------------------------------------------------------
// Testbench for hif-backend#61.
//
// Prints every output for all 16 values of `in`, so the comparison between the
// original and the regenerated design is exhaustive over the input domain
// rather than a spot check. %b is used throughout so that x and z are compared
// as themselves -- the four-state cases are the ones that used to render as an
// empty operand, and a check that collapsed x/z to a value would not see it.
//
// The same testbench drives both designs; the harness compares the two traces.
// ----------------------------------------------------------------------------
`timescale 1ns/1ps

module replication_emission_tb;

    reg  [3:0] in;
    wire [3:0] r0, r1, rx, rz, rvec, rparam, nest, mixed;
    wire [7:0] rwide;
    wire [2:0] pc;
    wire [5:0] inconcat;
    integer i;

    replication_emission dut (
        .in(in), .r0(r0), .r1(r1), .rx(rx), .rz(rz), .rvec(rvec),
        .rwide(rwide), .rparam(rparam), .pc(pc), .nest(nest),
        .inconcat(inconcat), .mixed(mixed)
    );

    initial begin
        for (i = 0; i < 16; i = i + 1) begin
            in = i[3:0];
            #1 $display("in=%b r0=%b r1=%b rx=%b rz=%b rvec=%b rwide=%b rparam=%b pc=%b nest=%b inconcat=%b mixed=%b",
                        in, r0, r1, rx, rz, rvec, rwide, rparam, pc, nest, inconcat, mixed);
        end
        $finish;
    end

endmodule
