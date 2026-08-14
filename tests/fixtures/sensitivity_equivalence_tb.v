// Testbench for the sensitivity_equivalence regression.
//
// Deliberately module-name compatible with both the original fixture and
// the Verilog hif2verilog regenerates from it, so the very same testbench
// can be compiled against either and the two traces compared verbatim.
//
// Each vector is observed twice: once after the inputs settle with clk
// held stable (this is where a reader that is not sensitive to a cone
// target reports a stale value), and once after clk toggles (this is where
// a level-sensitive clock process wrongly widened to `always @(*)` would
// have already updated early).

`timescale 1ns / 1ps

module sensitivity_equivalence_tb;
  reg a;
  reg b;
  reg cin;
  reg clk;

  wire sum;
  wire cout;
  wire shared_y;
  wire shared_z;
  wire lat;

  integer i;

  sensitivity_equivalence dut (
      .a(a),
      .b(b),
      .cin(cin),
      .clk(clk),
      .sum(sum),
      .cout(cout),
      .shared_y(shared_y),
      .shared_z(shared_z),
      .lat(lat)
  );

  initial begin
    clk = 1'b0;
    a   = 1'b0;
    b   = 1'b0;
    cin = 1'b0;

    for (i = 0; i < 8; i = i + 1) begin
      a   = (i >> 2) & 1'b1;
      b   = (i >> 1) & 1'b1;
      cin = i & 1'b1;

      // Inputs settled, clk untouched.
      #5;
      $display("vec=%0d phase=settle a=%b b=%b cin=%b sum=%b cout=%b y=%b z=%b lat=%b",
               i, a, b, cin, sum, cout, shared_y, shared_z, lat);

      // Now let the level-sensitive clock process fire.
      clk = ~clk;
      #5;
      $display("vec=%0d phase=clk    a=%b b=%b cin=%b sum=%b cout=%b y=%b z=%b lat=%b",
               i, a, b, cin, sum, cout, shared_y, shared_z, lat);
    end

    $finish;
  end
endmodule
