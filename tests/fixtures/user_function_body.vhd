-- ----------------------------------------------------------------------------
-- Fixture for hif-backend#57.
--
-- A user-defined function's body was dropped entirely: hif2verilog emitted the
-- `function`/`endfunction` wrapper and its parameters, but nothing between
-- `begin` and `end`. The result compiled and simulated, and every call returned
-- x -- so a design whose logic lives in a function regenerated into one that
-- computes nothing, at exit code 0.
--
-- Two return shapes, because they exercise different halves of the emission:
--
--   both_high / either_high  return boolean. Verilog has no boolean, and a
--                            1-bit `function name;` with no range is the
--                            correct spelling, so these also pin down that the
--                            absent return type is right rather than missing.
--   invert                   returns a vector, so the function carries a range
--                            (`function [3:0] invert;`) and the assignment has
--                            to be width-correct.
--
-- The process uses the boolean functions in if/elsif conditions so that a
-- function returning x is not merely a wrong bit but selects the wrong branch,
-- which is how the defect actually presented.
--
-- `w` states an initial value deliberately. A vector out port that states none
-- gets vhdl2hif's implicit 'U' default, which regenerates as the unparseable
-- `4'buuuu` -- hif-backend#55, a different defect on a different branch. Left
-- unstated, this fixture would fail to compile for that reason instead of this
-- one, and the test would not be about #57 at all.
-- ----------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

entity user_function_body is
  port (a : in  std_logic;
        b : in  std_logic;
        y : out std_logic;
        z : out std_logic;
        v : in  std_logic_vector(3 downto 0);
        w : out std_logic_vector(3 downto 0) := "0000");
end entity;

architecture rtl of user_function_body is

  function both_high(l : std_logic; r : std_logic) return boolean is
  begin
    return (l = '1') and (r = '1');
  end function;

  function either_high(l : std_logic; r : std_logic) return boolean is
  begin
    return (l = '1') or (r = '1');
  end function;

  function invert(d : std_logic_vector(3 downto 0)) return std_logic_vector is
  begin
    return not d;
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

  process (v)
  begin
    w <= invert(v);
  end process;

end architecture;
