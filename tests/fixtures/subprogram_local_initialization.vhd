-- Fixture for hif-backend#83, VHDL side.
--
-- A VHDL subprogram's local variable may state an initial value, and that value
-- is applied on every call. It arrived in the regenerated Verilog as a
-- declaration assignment inside the function or task body, which Verilog allows
-- only at module level, so the output did not build:
--
--     reg [3:0] m = 4'b0011;   -- inside `function ... endfunction`
--
-- Both subprogram kinds are here because both reach the same printer path. The
-- Verilog side of the issue reports only functions, and that is accurate for
-- Verilog sources - verilog2hif hoists a task's local to module level - but a
-- VHDL *procedure* becomes a Verilog task with its local still inside it, so
-- the task path is reachable and was broken identically.
--
-- Deliberately std_logic_vector rather than numeric_std: an unsigned or signed
-- declaration loses its initial value entirely on the way out (#97), and a
-- function returning one has its return type mangled (#98), so either would
-- make this test fail for a defect other than the one it is about.
library ieee;
use ieee.std_logic_1164.all;

entity subprogram_local_initialization is
  port (a    : in  std_logic_vector(3 downto 0);
        fn_y : out std_logic_vector(3 downto 0);
        pr_y : out std_logic_vector(3 downto 0));
end subprogram_local_initialization;

architecture rtl of subprogram_local_initialization is

  -- The initial value is load-bearing: dropping it leaves m at 'U'/x, and the
  -- result is x for every input rather than merely a different constant.
  function mask_fn (x : std_logic_vector(3 downto 0)) return std_logic_vector is
    variable m : std_logic_vector(3 downto 0) := "0011";
  begin
    m := m and x;
    return m;
  end function;

  procedure mask_pr (x : in  std_logic_vector(3 downto 0);
                     r : out std_logic_vector(3 downto 0)) is
    variable m : std_logic_vector(3 downto 0) := "0110";
  begin
    m := m and x;
    r := m;
  end procedure;

begin

  fn_y <= mask_fn(a);

  process (a)
    variable tmp : std_logic_vector(3 downto 0);
  begin
    mask_pr(a, tmp);
    pr_y <= tmp;
  end process;

end rtl;
