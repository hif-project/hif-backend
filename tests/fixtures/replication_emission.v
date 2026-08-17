// ----------------------------------------------------------------------------
// Fixture for hif-backend#61.
//
// A one-bit value was emitted as a bare digit -- `{1'b1, 1'b0}` regenerated as
// `{1, 0}`. Verilog gives an unsized `1` a self-determined 32-bit width, which
// a concatenation rejects ("Concatenation operand has indefinite width"), and a
// non-0/1 bit rendered as the empty string, producing `{, }` and a syntax
// error. Exit code was 0 in both cases.
//
// Every output below is declared at the EXACT natural width of its expression.
// That is load-bearing: if the target is wider, a cast is inserted and simplify
// folds the whole concatenation into a single sized literal, so the emission
// path under test is never reached and the test passes vacuously.
//
// The issue was filed against replication, but replication is only one way to
// build a concatenation -- `pc` below carries no replication at all and failed
// identically. Both are covered so a fix cannot be scoped to replication.
//
// Which operands stayed one-bit `Bit` and which were promoted to a one-bit
// `Bitvector` depends on position: VerilogParser::concat builds a left-leaning
// nest, and an operand is promoted once its sibling is already vector-typed, so
// only the innermost pair stays `Bit`. `nest` and `pc` pin down both shapes.
// ----------------------------------------------------------------------------
module replication_emission #(parameter N = 4) (
    input  wire [3:0] in,
    output wire [3:0] r0,        // {4{1'b0}}
    output wire [3:0] r1,        // {4{1'b1}}
    output wire [3:0] rx,        // {4{1'bx}}   four-state operand
    output wire [3:0] rz,        // {4{1'bz}}   four-state operand
    output wire [3:0] rvec,      // {2{in[1:0]}}          multi-bit operand
    output wire [7:0] rwide,     // {2{in}}               operand wider than one
    output wire [3:0] rparam,    // {N{1'b1}}             parameter count
    output wire [2:0] pc,        // {1'b1,1'b0,1'b1}      no replication at all
    output wire [3:0] nest,      // {2{{2{1'b1}}}}        replication of one
    output wire [5:0] inconcat,  // replication inside a larger concatenation
    output wire [3:0] mixed      // bit literal beside a vector operand
);
    assign r0       = {4{1'b0}};
    assign r1       = {4{1'b1}};
    assign rx       = {4{1'bx}};
    assign rz       = {4{1'bz}};
    assign rvec     = {2{in[1:0]}};
    assign rwide    = {2{in}};
    assign rparam   = {N{1'b1}};
    assign pc       = {1'b1, 1'b0, 1'b1};
    assign nest     = {2{{2{1'b1}}}};
    assign inconcat = {2'b01, {2{1'b1}}, in[1:0]};
    assign mixed    = {1'b1, in[2:0]};
endmodule
