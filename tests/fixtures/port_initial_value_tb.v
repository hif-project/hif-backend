// Drives nothing: the design under test has no inputs. That is the point -
// every output is driven by a constant continuous assignment and nothing else,
// so the only question is whether it is driven at all.
//
// The trace prints each output twice, once as a value and once as a bare
// "driven/undriven" verdict, so a failure says whether the value came out
// wrong or the port came out with no driver.
`timescale 1ns/1ps
module port_initial_value_tb;

    wire [31:0] c;
    wire [7:0]  m;
    wire        b;

    port_initial_value dut (.c(c), .m(m), .b(b));

    initial begin
        // Settle. Continuous assignments take effect at time 0, but sampling
        // strictly after it keeps this independent of scheduling order.
        #1;
        $display("c=%0d driven=%0d", c, (^c === 1'bx) ? 0 : 1);
        $display("m=%0h driven=%0d", m, (^m === 1'bx) ? 0 : 1);
        $display("b=%0b driven=%0d", b, (b === 1'bx) ? 0 : 1);
        $finish;
    end

endmodule
