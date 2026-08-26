// Testbench for hif-backend#51.
//
// There is no VHDL simulator in this project's environment (no ghdl, no nvc,
// locally or in CI), so clocked_process.vhd cannot be simulated and compared
// against. The oracle below is computed by hand from that VHDL and stated
// literally.
//
// The clock is driven explicitly rather than by a repeating toggle, because
// what has to be proved is *when* each output changes relative to the edges,
// and every interesting stimulus deliberately lands between two edges:
//
//   - d moves while the clock is idle. An edge-triggered flop ignores it until
//     the next active edge. A process rebuilt as `always @(clk, rst)` - level
//     sensitive, the mistake this rebuild could easily make - would follow it
//     immediately, and the trace would differ at that sample.
//   - rst is asserted while the clock is idle. q_async must fall at once and
//     q_sync must not, which is the whole difference between an asynchronous
//     and a synchronous reset. If the rebuild promoted the synchronous reset
//     into the sensitivity list, q_sync would fall early and be caught here.
//
// Sampling is one time unit after each event so that a nonblocking update has
// settled, and never on an edge itself.
//
// q_fall is deliberately not sampled until it has seen a real 1 -> 0 edge.
// Driving clk from x to 0 at time zero is a negedge in *Verilog*, so the
// falling-edge flop fires there and takes d - but VHDL's falling_edge() is
// false for a transition out of 'U', so the source design does not. That is a
// genuine difference between the two languages' edge definitions rather than
// anything this translation chooses, and asserting the Verilog value for it
// would be writing a Verilog oracle while claiming it came from the VHDL.
// Sampling after the first real edge keeps the oracle honest: from t31 the two
// languages agree, and that is what the trace checks.
//
// q_sync stays x through the early samples for the matching reason - x to 0 is
// not a posedge - and there VHDL agrees, since 'U' is what an unwritten signal
// holds.

`timescale 1ns / 1ns

module clocked_process_tb;

    reg clk, rst, d;
    wire q_async, q_sync, q_fall;

    clocked_process dut (
        .clk(clk),
        .rst(rst),
        .d(d),
        .q_async(q_async),
        .q_sync(q_sync),
        .q_fall(q_fall)
    );

    // Before q_fall has seen a real edge, only the two the languages agree on.
    task check_early;
        input [8*24-1:0] label;
        begin
            $display("%0s q_async=%b q_sync=%b", label, q_async, q_sync);
        end
    endtask

    task check;
        input [8*24-1:0] label;
        begin
            $display("%0s q_async=%b q_sync=%b q_fall=%b", label, q_async, q_sync, q_fall);
        end
    endtask

    initial begin
        clk = 1'b0;
        rst = 1'b1;
        d   = 1'b0;

        // Asynchronous reset is asserted at time zero, with no clock edge at
        // all. q_async must already be 0.
        #5  check_early("t05_reset_no_edge      ");

        #5  rst = 1'b0;                 // t = 10, still no edge

        // d rises while the clock is idle. Nothing may follow it yet.
        #5  d = 1'b1;                   // t = 15
        #5  check_early("t20_d_high_no_edge     ");

        // First rising edge: q_async and q_sync both take d = 1.
        #5  clk = 1'b1;                 // t = 25
        #1  check_early("t26_after_posedge      ");

        // Falling edge: q_fall takes d = 1. The other two hold.
        #4  clk = 1'b0;                 // t = 30
        #1  check("t31_after_negedge      ");

        // Reset asserted between edges. This is the discriminating sample:
        // q_async must fall immediately, q_sync must still read 1.
        #4  rst = 1'b1;                 // t = 35
        #1  check("t36_async_rst_no_edge  ");

        // Next rising edge applies the synchronous reset too.
        #4  clk = 1'b1;                 // t = 40
        #1  check("t41_after_posedge_rst  ");

        // Release reset, drop d, and take one more pair of edges so the
        // falling-edge flop is exercised with a changed value.
        #4  rst = 1'b0;                 // t = 45
        #2  d = 1'b0;                   // t = 47
        #3  clk = 1'b0;                 // t = 50, negedge -> q_fall takes 0
        #1  check("t51_after_negedge_d0   ");

        #4  clk = 1'b1;                 // t = 55, posedge -> q_async/q_sync take 0
        #1  check("t56_after_posedge_d0   ");

        $finish;
    end

endmodule
