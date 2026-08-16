// Fixture for hif-backend#30: a constant continuous assignment is folded by
// verilog2hif into the driven port's initial value, and hif2verilog printed
// that value nowhere - Verilog-2001 has no place for an initializer inside an
// ANSI port list - so the module regenerated with no driver for the output.
//
// Every output here is driven by nothing but a constant continuous assignment,
// which is the whole point: if the fold is not written back out, each of them
// regenerates undriven and reads x.
//
// Three shapes:
//   c  a plain sized constant
//   m  a replication whose count is a parameter, so the folded value is an
//      expression rather than a literal and cannot be re-emitted by printing
//      a constant
//   b  a one-bit constant, which is the shape a stubbed status flag takes
module port_initial_value #(parameter N = 4) (
    output [31:0] c,
    output [7:0]  m,
    output        b
);

  assign c = 32'd7;
  assign m = {N{1'b1}};
  assign b = 1'b1;

endmodule
