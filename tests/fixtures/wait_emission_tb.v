// Testbench shared by the original and the regenerated design
// (hif-backend#42).
//
// Everything the DUT reacts to happens on a whole `timescale unit, and every
// sample is taken half a unit away from one. That offset is deliberate: the
// frontend records a process-local assignment as a signal write, so the
// regenerated design uses non-blocking `<=` where the source wrote blocking
// `=`. Both settle to the same value within the same time step, but they do
// not settle at the same point *inside* it, so a sample taken at the instant
// of an event could differ for a reason that has nothing to do with `wait`.
//
// The trace therefore records what the design does, not how it schedules:
// which value `o` holds between events, and for how long. A dropped wait shows
// up immediately - without its suspensions the process runs to completion at
// time zero, so `o` reaches 4'd4 in the first half unit and never moves again.
`timescale 1ns / 1ps

module wait_emission_tb;
    reg clk;
    reg en;
    wire [3:0] o;

    wait_emission dut(.clk(clk), .en(en), .o(o));

    // 10-unit period starting low, so the rising edges are at t=5, 15, 25, 35.
    initial begin
        clk = 1'b0;
        forever #5 clk = ~clk;
    end

    // `en` moves at times that are not clock edges, so the condition wait and
    // the level event control can be told apart from the edge one.
    initial begin
        en = 1'b0;
        #3  en = 1'b1;   // releases the condition wait
        #14 en = 1'b0;   // t=17: releases the level event control
        #6  en = 1'b1;   // t=23: releases the condition wait on the next pass
        #14 en = 1'b0;   // t=37: releases the level event control again
    end

    initial begin
        #0.5;
        repeat (45) begin
            $display("%0t o=%h", $time, o);
            #1;
        end
        $finish;
    end
endmodule
