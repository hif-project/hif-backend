// Testbench for the hif-backend#38 fixture.
//
// Drives every combination of a and b so that both tasks run and the cone's
// output is exercised, and prints all four outputs. The source and the
// regenerated design are compared against each other, so a task that was
// emitted but wired up differently shows here rather than only failing to
// compile.

module tb;

    reg  a;
    reg  b;
    wire y;
    wire z;
    wire g;

    user_task dut (
        .a(a),
        .b(b),
        .y(y),
        .z(z),
        .g(g)
    );

    initial begin
        a = 1'b0; b = 1'b0;
        #10 $display("t=%0t a=%b b=%b y=%b z=%b g=%b", $time, a, b, y, z, g);
        a = 1'b1; b = 1'b0;
        #10 $display("t=%0t a=%b b=%b y=%b z=%b g=%b", $time, a, b, y, z, g);
        a = 1'b1; b = 1'b1;
        #10 $display("t=%0t a=%b b=%b y=%b z=%b g=%b", $time, a, b, y, z, g);
        a = 1'b0; b = 1'b1;
        #10 $display("t=%0t a=%b b=%b y=%b z=%b g=%b", $time, a, b, y, z, g);
        $finish;
    end

endmodule
