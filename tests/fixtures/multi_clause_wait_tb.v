// Testbench for hif-backend#45. Self-checking on the *time* each wait resumes,
// which is what distinguishes a correct lowering from one that merely produces
// output that compiles.
//
// Stimulus: `a` never toggles, `b` rises at t=6, `c` toggles at t=3 and t=8.
//
//   q_all  wait on a until b='1' for 10 ns -> t=10, by timeout (a idle)
//   q_st   wait on a for 10 ns             -> t=10, by timeout (a idle)
//   q_cs   wait on c until b='1'           -> t=8; the t=3 event on c must be
//                                             rejected, b being '0' then
//   q_ct   wait until b='1' for 10 ns      -> t=6, when b rises
//
// Every one of the four is a different resumption reason, so a lowering that
// wired the branches up wrongly cannot pass by accident.
module multi_clause_wait_tb;
    reg b, c;
    wire q_all, q_st, q_cs, q_ct;
    integer failures;

    integer t_all, t_st, t_cs, t_ct;

    // `a` is tied to a literal rather than driven from a reg. A reg would go
    // x -> 0 at time 0, and that is a real event: the two waits sensitive to
    // `a` would resume on it instead of on their timeout, and the test would
    // report t=0 for a reason that has nothing to do with the lowering.
    multi_clause_wait dut (
        .a(1'b0), .b(b), .c(c),
        .q_all(q_all), .q_st(q_st), .q_cs(q_cs), .q_ct(q_ct)
    );

    initial begin
        t_all = -1; t_st = -1; t_cs = -1; t_ct = -1;
    end

    // First rising transition of each output is its resumption time.
    always @(posedge q_all) if (t_all < 0) t_all = $time;
    always @(posedge q_st)  if (t_st  < 0) t_st  = $time;
    always @(posedge q_cs)  if (t_cs  < 0) t_cs  = $time;
    always @(posedge q_ct)  if (t_ct  < 0) t_ct  = $time;

    task expect_time;
        input [8*8:1] name;
        input integer actual;
        input integer wanted;
        begin
            if (actual !== wanted) begin
                $display("FAIL: %0s resumed at t=%0d, expected t=%0d", name, actual, wanted);
                failures = failures + 1;
            end
        end
    endtask

    initial begin
        failures = 0;
        b = 1'b0;
        c = 1'b0;

        // `a` is tied off, so the two waits sensitive to it can only resume via
        // their timeout. A single sequential block cannot express that.
        #3  c = 1'b1;   // event on c while b is still 0 - must not resume q_cs
        #3  b = 1'b1;   // t=6: resumes q_ct
        #2  c = 1'b0;   // t=8: event on c with b now 1 - resumes q_cs
        #20;            // well past every deadline

        expect_time("q_all", t_all, 10);
        expect_time("q_st",  t_st,  10);
        expect_time("q_cs",  t_cs,  8);
        expect_time("q_ct",  t_ct,  6);

        if (failures == 0) begin
            $display("ALL CHECKS PASSED");
        end
        $finish;
    end
endmodule
