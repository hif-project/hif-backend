// Testbench shared by the original and the regenerated design
// (hif-backend#24). Samples around each delay boundary, so it can tell "the
// delay is honoured" from "the delay is ignored" and from "too much is
// delayed":
//
//   - 1 unit after a/b change, t must NOT have moved yet;
//   - 3 units after, it must have;
//   - 1 unit after c alone changes, y must have moved already - c does not go
//     through the delayed net, so nothing should hold it back.
`timescale 1ns / 1ps

module assign_delay_tb;
    reg a;
    reg b;
    reg c;
    wire y;
    wire z;

    assign_delay dut(.a(a), .b(b), .c(c), .y(y), .z(z));

    initial begin
        a = 0;
        b = 0;
        c = 0;
        #10;

        // t rises 2 units from here.
        a = 1;
        b = 1;
        #1 $display("a,b rise +1  y=%b z=%b", y, z);
        #2 $display("a,b rise +3  y=%b z=%b", y, z);

        // c does not feed the delayed net: y must follow it at once.
        #5 c = 1;
        #1 $display("c rise +1    y=%b z=%b", y, z);

        // t falls 2 units from here.
        #5 a = 0;
        #1 $display("a fall +1    y=%b z=%b", y, z);
        #3 $display("a fall +4    y=%b z=%b", y, z);

        $finish;
    end
endmodule
