// Fixture for the for_init_declaration regression (hif-backend#47).
//
// `repeat (n) @( ... )` is the point. verilog2hif lowers it into a For that
// declares its own index and carries that index's starting value on the
// declaration - the one HIF shape Verilog cannot spell in a loop header, since
// `for (integer i = 1; ...)` is SystemVerilog.
//
// The count is 3 rather than 1 so that the loop has to run a specific number of
// times: a regenerated design whose index starts at the wrong value, or whose
// init clause is missing so the index keeps its previous value, releases at a
// different edge and the testbench sees it.
//
// The write after the loop is what makes the wait observable. Without it the
// design would have no output that depends on when the loop finished.
module for_init_declaration(
    input  wire       clk,
    output reg  [3:0] o
);
    always begin
        repeat (3) @(posedge clk);
        o = 4'd4;
    end
endmodule
