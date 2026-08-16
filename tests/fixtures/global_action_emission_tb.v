// Testbench for hif-backend#32.
//
// There is no VHDL simulator in this project's environment (no ghdl, no nvc,
// locally or in CI), so the source of global_action_emission.vhd cannot be
// simulated and compared against. The oracle below is therefore computed by
// hand from that VHDL and stated literally:
//
//   s = a and b
//   t = s
//   u = s or c
//   d = a xor b, delayed by 2 ns
//
// The sample times straddle each delay boundary (+1 ns and +3 ns after a
// change), so a dropped delay and a honoured one produce different traces
// rather than the same one.

`timescale 1ns / 1ns

module global_action_emission_tb;

    reg a, b, c;
    wire t, u, d;

    global_action_emission dut (.a(a), .b(b), .c(c), .t(t), .u(u), .d(d));

    task check;
        input [8*16-1:0] label;
        begin
            $display("%0s t=%b u=%b d=%b", label, t, u, d);
        end
    endtask

    initial begin
        a = 1'b0; b = 1'b0; c = 1'b0;

        // Settle: s=0, t=0, u=0, d=0^0=0 (after 2 ns).
        #5  check("init            ");

        // a=1,b=1 -> s=1, t=1, u=1 immediately; d=1^1=0, unchanged.
        #5  a = 1'b1; b = 1'b1;      // t = 10
        #1  check("ab_high_plus1   ");   // t = 11
        #2  check("ab_high_plus3   ");   // t = 13

        // a=1,b=0 -> s=0, t=0, u=0 immediately; d goes 0->1 at t=22.
        #7  a = 1'b1; b = 1'b0;      // t = 20
        #1  check("b_low_plus1     ");   // t = 21, d must still be 0
        #2  check("b_low_plus3     ");   // t = 23, d must now be 1

        // c=1 -> u=1 immediately (s is still 0); d goes 1->0 at t=32.
        #7  a = 1'b0; b = 1'b0; c = 1'b1;  // t = 30
        #1  check("c_high_plus1    ");   // t = 31, d must still be 1
        #2  check("c_high_plus3    ");   // t = 33, d must now be 0

        $finish;
    end

endmodule
