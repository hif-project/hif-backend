// Two case alternatives with identical bodies. Front-end simplification merges
// them into a single SwitchAlt carrying both conditions, which the Verilog
// printer has to emit as a comma-separated label list.
//
// Reduced from the Moore-machine DUT used by the systems-verification course
// laboratory (fsmsvt_tb/rtl/fsm.sv), where STATE_2 and FINAL share a body.
module merged_case_alternatives(clk, rst, a, out1, out2, out3);
input clk, rst, a;
output out1, out2, out3;

localparam START = 2'd0, STATE_1 = 2'd1, STATE_2 = 2'd2, FINAL = 2'd3;
reg [1:0] state;

always @(posedge clk or posedge rst) begin
    if (rst) begin
        state <= START;
    end else begin
        case (state)
            START:   if (a) state <= STATE_1; else state <= START;
            STATE_1: if (a) state <= STATE_2; else state <= START;
            STATE_2: if (a) state <= FINAL;   else state <= START;
            FINAL:   if (a) state <= FINAL;   else state <= START;
        endcase
    end
end

assign out1 = (state == STATE_1);
assign out2 = (state == STATE_2);
assign out3 = (state == FINAL);
endmodule
