// Regression fixture (hif-backend#26), parent module: a module hierarchy
// carried through `verilog2hif -s`.
//
// s1, c1 and c2 are driven by child instance outputs, so they must come back
// out as nets. sum is driven by a child instance output too; cout is driven by
// the parent's own logic, so it must stay a variable. The fixture needs both
// to distinguish the fix from "emit wire everywhere".
module preserved_hierarchy(
    input a,
    input b,
    input cin,
    output sum,
    output cout
);
  wire s1, c1, c2;

  preserved_hierarchy_child u_ha1(.a(a),  .b(b),   .sum(s1),  .carry(c1));
  preserved_hierarchy_child u_ha2(.a(s1), .b(cin), .sum(sum), .carry(c2));

  assign cout = c1 | c2;
endmodule
