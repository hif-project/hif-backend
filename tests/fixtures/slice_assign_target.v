// Regression fixture for hif-backend#103: assignment targets that are not the
// whole object. Writing part of a vector is ordinary RTL - byte swaps, field
// packing, status words assembled from several sources - and hif2vhdl aborted
// on every one of them.
//
// Both shapes the defect covers are here, because they reach the same line in
// VHDLPrinter::visitAssign by different node types: a part-select is a Slice
// wrapping the identifier, a bit-select is a Member wrapping it. Two of each,
// so a fix that resolves only the first assignment of a process still fails.
//
// Deliberately kept to shapes a simulator accepts: `y[3:0][1]` reaches the
// same code path through a Member over a Slice, but iverilog rejects it
// ("the number of indices (2) is greater than the number of dimensions (1)"),
// so pinning it here would pin something that is not legal Verilog-2005.
module slice_assign_target(
    input  wire [7:0] a,
    input  wire [7:0] b,
    output reg  [7:0] y,
    output reg  [7:0] z
);
    always @(*) begin
        y[3:0] = a[3:0];
        y[7:4] = b[7:4];
        z[0]   = a[0];
        z[7]   = b[7];
    end
endmodule
