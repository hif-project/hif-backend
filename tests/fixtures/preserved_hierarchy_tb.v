// Testbench shared by the original and the regenerated hierarchy
// (hif-backend#26). Exhaustive over the three inputs, so a trace that matches
// the reference cannot be matching by luck.
`timescale 1ns/1ps
module preserved_hierarchy_tb;
  reg a, b, cin;
  wire sum, cout;
  integer i;

  preserved_hierarchy dut(.a(a), .b(b), .cin(cin), .sum(sum), .cout(cout));

  initial begin
    for (i = 0; i < 8; i = i + 1) begin
      a   = i[0];
      b   = i[1];
      cin = i[2];
      #1;
      $display("%b%b%b -> sum=%b cout=%b", a, b, cin, sum, cout);
    end
    $finish;
  end
endmodule
