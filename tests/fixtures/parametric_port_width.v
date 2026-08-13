module parametric_port_width #(
    parameter WIDTH = 8
) (
    input  wire             valid,
    output wire [WIDTH-1:0] bus
);
    assign bus = valid ? 8'hFF : 8'h00;
endmodule
