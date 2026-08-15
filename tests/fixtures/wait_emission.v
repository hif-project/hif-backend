// Regression fixture (hif-backend#42): every form of `wait` must survive the
// round trip as a statement of its own.
//
// visitWait was `return GuideVisitor::visitWait(o);` - no keyword printed, but
// the guide visitor still descended into the node, so each wait's condition or
// sensitivity was printed bare and ran into the statement after it. This
// module regenerated as
//
//     eno <= 4'b0001;
//
// inventing the undeclared identifier `eno` out of `en` and `o`, while
// hif2verilog exited 0.
//
// The three forms are here because they are three different HIF shapes and
// three different Verilog constructs: a condition wait sets Wait::condition, an
// event control sets one of the three sensitivity lists, and a delay sets
// Wait::time. Only the first was in the original report; emitting it alone
// would have left the other two splicing.
//
// `repeat (n) @(...)` is deliberately absent: verilog2hif lowers it to a For
// whose loop index lives in initDeclarations, which visitFor does not print, so
// such a fixture would fail on something that is not this defect
// (hif-backend#47).
//
// The explicit `timescale is what `#4` counts in. Without one the source and
// the regenerated design could be simulated at different scales and the trace
// comparison would mean nothing.
`timescale 1ns / 1ps

module wait_emission (
    input  wire       clk,
    input  wire       en,
    output reg  [3:0] o
);

    always begin
        // Condition wait. Level-sensitive: falls through at once when `en` is
        // already high, which is what distinguishes it from an event control.
        wait (en);
        o = 4'd1;

        // Edge event control.
        @(posedge clk);
        o = 4'd2;

        // Timed wait. Also the design's only delay, so it is what the emitted
        // `timescale directive has to be derived from.
        #4;
        o = 4'd3;

        // Level event control: any change on `en`.
        @(en);
        o = 4'd4;
    end

endmodule
