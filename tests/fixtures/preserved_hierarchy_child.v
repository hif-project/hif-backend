// Regression fixture (hif-backend#26), child module. Both of its outputs are
// driven by its own logic, so both stay variables in the regenerated child -
// what changes is how the parent declares the nets bound to them.
module preserved_hierarchy_child(
    input a,
    input b,
    output sum,
    output carry
);
  assign sum   = a ^ b;
  assign carry = a & b;
endmodule
