-- Fixture for the condition_rendering regression (hif-backend#50).
--
-- Both branch conditions are bare function calls, which is the shape that
-- reached VerilogPrinter::getValue's missing terminal case and rendered as the
-- empty string. rising_edge is not incidental: it is the most common condition
-- in synthesisable VHDL, so the defect landed on ordinary designs rather than
-- on an exotic construct.
--
-- The elsif is here to cover the second call site. visitIf renders the first
-- alternative and the subsequent ones through separate getValue calls
-- (VerilogPrinter.cpp:1049 and :1052), and a fix that only reached one of them
-- would still regenerate half the branches with an empty condition.
--
-- falling_edge(rst) rather than a reset level test, so that both conditions are
-- FunctionCalls. A plain `rst = '1'` would be an Expression, which getValue
-- already handled - it would still pass against the unfixed printer and would
-- weaken the test.
library ieee;
use ieee.std_logic_1164.all;

entity condition_rendering is
    port (
        clk : in  std_logic;
        rst : in  std_logic;
        d   : in  std_logic;
        q   : out std_logic
    );
end entity;

architecture rtl of condition_rendering is
begin
    process (clk, rst)
    begin
        if rising_edge(clk) then
            q <= d;
        elsif falling_edge(rst) then
            q <= '0';
        end if;
    end process;
end architecture;
