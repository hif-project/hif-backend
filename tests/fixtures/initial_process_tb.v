// Testbench for the hif-backend#40 fixture.
//
// Samples after time zero so that the initial blocks have run, then toggles
// sel to show that the always block is still sensitive to it. A design whose
// initial blocks were dropped, or whose always block was demoted to run-once,
// prints a different trace.

module tb;

    reg        sel;
    wire [3:0] once;
    wire [3:0] dep;
    wire [3:0] live;

    initial_process dut (
        .sel (sel),
        .once(once),
        .dep (dep),
        .live(live)
    );

    initial begin
        sel = 1'b0;
        #10;
        $display("t=%0t once=%0d dep=%0d live=%0d", $time, once, dep, live);
        sel = 1'b1;
        #10;
        $display("t=%0t once=%0d dep=%0d live=%0d", $time, once, dep, live);
        sel = 1'b0;
        #10;
        $display("t=%0t once=%0d dep=%0d live=%0d", $time, once, dep, live);
        $finish;
    end

endmodule
