// Testbench for the edge_sensitivity regression (hif-backend#21).
//
// Module-name compatible with both the fixture and the Verilog hif2verilog
// regenerates from it, so one testbench can be compiled against either and
// the traces compared verbatim.
//
// Every reset and level transition happens between clock edges. That is
// deliberate: a dropped `negedge rst_n` turns an asynchronous reset into a
// synchronous one, and the two are indistinguishable unless the reset moves
// while the clock is idle.

`timescale 1ns / 1ps

module edge_sensitivity_tb;
  reg clk;
  reg rst_n;
  reg rst;
  reg lvl;
  reg d;

  wire q_async;
  wire q_both;
  wire q_mixed;

  edge_sensitivity dut (
      .clk(clk),
      .rst_n(rst_n),
      .rst(rst),
      .lvl(lvl),
      .d(d),
      .q_async(q_async),
      .q_both(q_both),
      .q_mixed(q_mixed)
  );

  task tick;
    begin
      #5 clk = 1'b1;
      #5 clk = 1'b0;
    end
  endtask

  task show;
    input [8*16-1:0] label;
    begin
      $display("%0s clk=%b rst_n=%b rst=%b lvl=%b d=%b q_async=%b q_both=%b q_mixed=%b",
               label, clk, rst_n, rst, lvl, d, q_async, q_both, q_mixed);
    end
  endtask

  initial begin
    clk   = 1'b0;
    rst_n = 1'b1;
    rst   = 1'b0;
    lvl   = 1'b0;
    d     = 1'b1;

    tick;
    show("loaded");

    // Assert the active-low reset with the clock idle. A synchronous reset
    // would not react here; an asynchronous one clears immediately.
    #5 rst_n = 1'b0;
    #5 show("rst_n_low");

    #5 rst_n = 1'b1;
    #5 show("rst_n_high");

    // Same for the active-high reset, then release it. Releasing is what
    // catches a signal that kept its place but lost its posedge: it fires
    // on the falling edge too.
    #5 rst = 1'b1;
    #5 show("rst_high");

    #5 rst = 1'b0;
    #5 show("rst_low");

    // Toggle the level signal with the clock idle, both ways.
    #5 lvl = 1'b1;
    #5 show("lvl_high");

    #5 lvl = 1'b0;
    #5 show("lvl_low");

    tick;
    show("final");

    $finish;
  end
endmodule
