// Testbench for hif-backend#71. Self-checking, so a wrong value names itself.
//
// The interesting checks are the ones an external driver makes possible, which
// is why this testbench has one: `b` is a bidirectional net, and a lowering
// that drives it unconditionally - or that lets the process read back its own
// driver reg instead of the net - compiles and looks right until somebody else
// drives the same wire.
module inout_port_driver_tb;
    reg en;
    reg d;
    reg src;
    reg ext_en;
    reg ext_val;
    wire b;
    wire c;
    wire mon;
    integer failures;

    inout_port_driver dut (
        .en(en), .d(d), .b(b), .mon(mon), .src(src), .c(c)
    );

    // The other party on the bidirectional net.
    assign b = ext_en ? ext_val : 1'bz;

    task check;
        input [8*40:1] name;
        input actual;
        input expected;
        begin
            if (actual !== expected) begin
                $display("FAIL: %0s is %b, expected %b", name, actual, expected);
                failures = failures + 1;
            end
        end
    endtask

    initial begin
        failures = 0;
        ext_en = 1'b0;
        ext_val = 1'b0;

        // Driving. Before the fix the design did not elaborate at all, so this
        // is the reported symptom's direct check.
        en = 1'b1; d = 1'b1; #1;
        check("b while driven high", b, 1'b1);
        check("mon while driven high", mon, 1'b1);

        en = 1'b1; d = 1'b0; #1;
        check("b while driven low", b, 1'b0);
        check("mon while driven low", mon, 1'b0);

        // Releasing. VHDL says this with a value, 'Z', not with an enable, so
        // this is what says the unconditional continuous driver is enough.
        en = 1'b0; #1;
        check("b once released", b, 1'bz);

        // Reading back what somebody else drives. This is the check that fails
        // if the process's read of the port followed its write onto the driver
        // reg: mon would report what this module last drove, not what is on
        // the net.
        ext_en = 1'b1; ext_val = 1'b1; #1;
        check("b while ext drives", b, 1'b1);
        check("mon while ext drives (must read the net)", mon, 1'b1);

        // Contention resolves to x, exactly as std_logic resolution does for
        // '0' against '1'.
        en = 1'b1; d = 1'b0; #1;
        check("b under contention", b, 1'bx);

        // The control: an inout driven concurrently is an ordinary net and is
        // untouched by the lowering.
        ext_en = 1'b0; src = 1'b1; #1;
        check("c (concurrently driven inout)", c, 1'b1);

        if (failures == 0) begin
            $display("ALL CHECKS PASSED");
        end
        $finish;
    end
endmodule
