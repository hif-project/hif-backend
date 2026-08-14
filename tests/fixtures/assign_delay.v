// Regression fixture (hif-backend#24): the delay on a delayed continuous
// assignment must survive the round trip.
//
// `t` is delayed; `y` and `z` read it and respond immediately. `y` also
// depends on `c`, which `t` does not, so a regenerated design that delays the
// whole process rather than just `t` shows up as `y` reacting late to `c`.
//
// The explicit `timescale is what the delay counts in: without one the source
// and the regenerated design would be simulated at different scales and no
// trace comparison would mean anything.
`timescale 1ns / 1ps

module assign_delay(input a, input b, input c, output y, output z);
  wire t;

  assign #2 t = a & b;
  assign y = t ^ c;
  assign z = t | b;
endmodule
