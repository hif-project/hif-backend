// Drives the two inputs through all four combinations. The design's own
// $display/$write calls are what produce the trace, so the trace existing at
// all is the property under test: with the calls dropped, this prints nothing.
`timescale 1ns/1ps
module system_task_calls_tb;
  reg a, b;
  wire sum, cout;

  system_task_calls dut (.a(a), .b(b), .sum(sum), .cout(cout));

  initial begin
    a = 1'b0; b = 1'b0; #5;
    a = 1'b1; b = 1'b0; #5;
    a = 1'b1; b = 1'b1; #5;
    a = 1'b0; b = 1'b1; #5;
    $finish;
  end
endmodule
