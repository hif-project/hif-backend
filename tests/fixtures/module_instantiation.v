module and_or(input a, input b, output and_out, output or_out);
  assign and_out = a & b;
  assign or_out  = a | b;
endmodule

module select2(input x, input y, input sel, output z);
  assign z = sel ? y : x;
endmodule

module module_instantiation(input a, input b, input sel, output result);
  wire and_out, or_out;
  and_or u_and_or(.a(a), .b(b), .and_out(and_out), .or_out(or_out));
  select2 u_select2(.x(and_out), .y(or_out), .sel(sel), .z(result));
endmodule
