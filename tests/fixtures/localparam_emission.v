module localparam_emission(input clk, input rst, input go, output reg done);
  localparam IDLE = 2'b00, RUN = 2'b01;
  reg [1:0] state;
  always @(posedge clk) begin
    if (rst) begin
      state <= IDLE;
      done  <= 1'b0;
    end else if (go) begin
      state <= RUN;
      done  <= 1'b1;
    end
  end
endmodule
