// Fixture for hif-backend#40.
//
// Three process shapes in one module, so that the fix can be shown to change
// exactly the one it is about:
//
//   - `initial` blocks, which have nothing to wake them up and so must come
//     back as `initial` rather than as a zero-delay `always` loop;
//   - an `always` block with a real sensitivity list, which must be left
//     alone.
//
// The second initial block carries a dependency between two statements. An
// initial block that came back with its assignments reordered or made
// non-blocking would leave `dep` at x rather than 2, so the emitted block is
// checked for what it computes and not only for the keyword it starts with.

module initial_process (
    input  wire       sel,
    output reg  [3:0] once,
    output reg  [3:0] dep,
    output reg  [3:0] live
);

    // No sensitivity list and no wait: runs once, at time zero.
    initial begin
        once = 4'd5;
    end

    // Runs once too, but the second statement reads what the first wrote.
    initial begin
        dep = 4'd1;
        dep = dep + 4'd1;
    end

    // A genuine level-sensitive process. This one is re-triggerable and must
    // stay an `always`, or the design stops responding to sel after time zero.
    always @(sel) begin
        live = sel ? 4'd9 : 4'd3;
    end

endmodule
