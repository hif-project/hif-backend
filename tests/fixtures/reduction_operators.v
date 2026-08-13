module reduction_operators(input [3:0] a, output y_nand, output y_nor);
   assign y_nand = ~&a;
   assign y_nor  = ~|a;
endmodule
