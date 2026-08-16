// Testbench for the hif-backend#36 fixture.
//
// There is no VHDL simulator here, so the source cannot be simulated for
// comparison. The expected values are computed by hand from the VHDL and
// stated in the test script; this testbench just makes them observable.
//
// The first sample is the one that matters: a is still '0', so the process has
// not written q, and q must read the initial value the entity declared. Before
// the fix it reads x.

module tb;

    reg  a;
    wire q;
    wire s;
    wire t;

    port_initial_process dut (
        .a(a),
        .q(q),
        .s(s),
        .t(t)
    );

    initial begin
        a = 1'b0;
        #10;
        $display("t=%0t a=%b q=%b s=%b t=%b", $time, a, q, s, t);
        a = 1'b1;
        #10;
        $display("t=%0t a=%b q=%b s=%b t=%b", $time, a, q, s, t);
        a = 1'b0;
        #10;
        $display("t=%0t a=%b q=%b s=%b t=%b", $time, a, q, s, t);
        $finish;
    end

endmodule
