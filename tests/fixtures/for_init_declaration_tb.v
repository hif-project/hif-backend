// Testbench for hif-backend#47.
//
// Module-name compatible with both the fixture and the Verilog hif2verilog
// regenerates from it, so one testbench compiles against either and the traces
// are comparable line for line.
//
// What is being measured is *when* the design releases, not merely that it
// does. `repeat (3) @(posedge clk)` has to wait three rising edges and then
// write o. The samples straddle the third edge:
//
//   t=20  two edges have passed. o must still be unwritten.
//   t=30  the third edge has passed. o must be 4.
//
// A loop whose index is never initialised, or initialised to the wrong value,
// releases at a different edge and one of those two samples changes. Checking
// only the final value would pass for any loop that eventually terminates.

`timescale 1ns / 1ns

module for_init_declaration_tb;

    reg clk;
    wire [3:0] o;

    for_init_declaration dut (.clk(clk), .o(o));

    // Posedges at t = 5, 15, 25, 35, ...
    initial begin
        clk = 1'b0;
        forever #5 clk = ~clk;
    end

    task check;
        input [8*16-1:0] label;
        begin
            $display("%0s o=%b", label, o);
        end
    endtask

    initial begin
        #20 check("t20_two_edges  ");
        #10 check("t30_three_edges");
        #10 check("t40_four_edges ");
        $finish;
    end

endmodule
