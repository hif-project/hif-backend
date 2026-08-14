// Fixture for the edge_sensitivity regression (hif-backend#21).
//
// HIF keeps a process's level, positive-edge and negative-edge sensitivity
// in three separate lists. The printer used to emit whichever of the three
// was non-empty first and discard the rest, and to put a single
// `posedge`/`negedge` keyword ahead of a whole comma-separated list rather
// than attaching one to each signal. Both halves lost sensitivity silently.
//
// Each output below covers one shape that was mis-emitted:
//
//   q_async  - `posedge clk or negedge rst_n`. The negedge list was dropped
//              outright, turning an asynchronous reset into a synchronous
//              one. This is the shape that matters most: it is the standard
//              async-reset flip-flop.
//   q_both   - `posedge clk or posedge rst`. Both signals survived, but only
//              the first kept its `posedge`, leaving rst sensitive to both
//              of its edges.
//   q_mixed  - `posedge clk or lvl`. A mixed edge/level event list, where
//              the level list was printed and the edge list dropped, losing
//              the clock entirely.
//
// The testbench drives the reset and the level signal *between* clock edges,
// which is the only way any of this is observable: a testbench that only
// ever changes inputs on a clock edge sees a synchronous and an asynchronous
// reset behave identically.

module edge_sensitivity(
    input clk,
    input rst_n,
    input rst,
    input lvl,
    input d,
    output reg q_async,
    output reg q_both,
    output reg q_mixed
);
  always @(posedge clk or negedge rst_n) begin
    if (!rst_n) q_async <= 1'b0;
    else        q_async <= d;
  end

  always @(posedge clk or posedge rst) begin
    if (rst) q_both <= 1'b0;
    else     q_both <= d;
  end

  always @(posedge clk or lvl) begin
    q_mixed <= d ^ lvl;
  end
endmodule
