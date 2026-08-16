-- Fixture for the condition_rendering regression (hif-backend#50).
--
-- Both branch conditions are bare function calls, which is the shape that
-- reached VerilogPrinter::getValue's missing terminal case and rendered as the
-- empty string.
--
-- The elsif is here to cover the second call site. visitIf renders the first
-- alternative and the subsequent ones through separate getValue calls
-- (VerilogPrinter.cpp:1049 and :1052), and a fix that only reached one of them
-- would still regenerate half the branches with an empty condition.
--
-- The conditions were originally rising_edge/falling_edge, which is how the
-- defect was reported. They cannot stay: hif2verilog now rebuilds a clocked
-- process's edge test into Verilog edge sensitivity (hif-backend#51), so an
-- edge call is consumed before the printer sees it and would no longer
-- exercise this path at all. User-defined functions keep a bare FunctionCall
-- in a condition, which is what this test is about, and are untouched by that
-- rebuild.
--
-- Deliberately *not* simulated. A user-defined function is currently emitted
-- with an empty body (hif-backend#57), so the regenerated design compiles but
-- computes nothing - checking behaviour here would be checking that unrelated
-- defect. What this test asserts is that the condition text survives, which is
-- what #50 was about and which is independent of what the function does.
library ieee;
use ieee.std_logic_1164.all;

entity condition_rendering is
    port (
        a : in  std_logic;
        b : in  std_logic;
        y : out std_logic;
        z : out std_logic
    );
end entity;

architecture rtl of condition_rendering is

    function both_high(l : std_logic; r : std_logic) return boolean is
    begin
        return (l = '1') and (r = '1');
    end function;

    function either_high(l : std_logic; r : std_logic) return boolean is
    begin
        return (l = '1') or (r = '1');
    end function;

begin

    process (a, b)
    begin
        if both_high(a, b) then
            y <= '1';
            z <= '0';
        elsif either_high(a, b) then
            y <= '0';
            z <= '1';
        else
            y <= '0';
            z <= '0';
        end if;
    end process;

end architecture;
