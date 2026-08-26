// Fixture for hif-backend#84.
//
// `casez` and `casex` were both regenerated as a plain `case`. Under `case` the
// comparison is exact, so a label like 4'bzzz1 matches only a selector that is
// literally zzz1: every wildcard alternative becomes unreachable and control
// falls through to the default. Valid Verilog, exit 0, reparses cleanly, and
// different values - which is why this fixture is simulated rather than only
// inspected.
//
// Both wildcard forms are here because both were affected. The issue reports
// casez and notes casex as untested; it is affected identically, and the HIF
// distinguishes them (caseSemantics CASE_Z vs CASE_X).
//
// The plain `case` is a control: CASE_LITERAL must keep printing `case`, and a
// fix that reached for the wildcard keyword unconditionally would be caught
// here rather than in review.
//
// The three encoders are priority encoders - lowest set bit wins - so a
// fallthrough to the default is visible on almost every input rather than on a
// single crafted one.
module casez_wildcards(
    input  [3:0] req,
    input  [1:0] sel,
    output reg [1:0] grant_z,
    output reg [1:0] grant_x,
    output reg [1:0] plain
);
    always @(*) begin
        casez (req)
            4'b???1: grant_z = 2'd0;
            4'b??10: grant_z = 2'd1;
            4'b?100: grant_z = 2'd2;
            default: grant_z = 2'd3;
        endcase
    end

    always @(*) begin
        casex (req)
            4'b???1: grant_x = 2'd0;
            4'b??10: grant_x = 2'd1;
            4'b?100: grant_x = 2'd2;
            default: grant_x = 2'd3;
        endcase
    end

    always @(*) begin
        case (sel)
            2'b00: plain = 2'd0;
            2'b01: plain = 2'd1;
            default: plain = 2'd3;
        endcase
    end
endmodule
