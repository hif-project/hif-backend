// Fixture for hif-backend#83, Verilog side.
//
// A function-local variable regenerated as `reg [4:0] wide_fn = 5'bxxxxx;`
// inside the function body. Verilog allows a variable declaration assignment
// only at module level, so the output did not parse.
//
// The source states no initial value here. The all-x one comes from
// verilog2hif, which gives every reg that default; it says nothing the
// declaration does not already say, since an uninitialized reg reads x. So the
// right output for this shape is a bare declaration, with the value neither
// restated on the declaration nor re-emitted as a statement.
//
// The task alongside is a control rather than a second case: verilog2hif hoists
// a Verilog task's local to module level, where a declaration assignment is
// legal, so that shape was never broken and must stay exactly as it was.
module function_local_declaration(
    input  [3:0] a,
    input  [3:0] b,
    output [4:0] fn_y,
    output reg [4:0] tk_y
);
    function [4:0] add_fn;
        input [3:0] x;
        input [3:0] z;
        reg [4:0] wide_fn;
        begin
            wide_fn = {1'b0, x} + {1'b0, z};
            add_fn  = wide_fn;
        end
    endfunction

    task add_tk;
        input [3:0] x;
        input [3:0] z;
        output [4:0] r;
        reg [4:0] wide_tk;
        begin
            wide_tk = {1'b0, x} + {1'b0, z};
            r = wide_tk;
        end
    endtask

    assign fn_y = add_fn(a, b);
    always @(*) add_tk(a, b, tk_y);
endmodule
