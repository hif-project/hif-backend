// Regression fixture: a wire whose width depends on an unresolved module
// parameter. verilog2hif accepts this cleanly (exit 0), but hif2verilog
// crashes processing the resulting HIF, via hif-core's simplify.cpp
// ("Cannot type the expr"), and reports "Using semantics: vhdl" while
// processing Verilog-derived HIF -- itself a separate, suspicious detail.
// Minimized from hif-frontend's examples/parser_test.v.
module unresolved_param #(
    parameter WIDTH = 8
) (
    input  wire             valid,
    output wire [WIDTH-1:0] bus
);
    reg [WIDTH-1:0] temp;
    assign bus = valid ? temp : {WIDTH{1'bz}};
endmodule
