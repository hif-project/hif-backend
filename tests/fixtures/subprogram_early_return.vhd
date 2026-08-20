-- Regression fixture for hif-backend#63 and #73: a subprogram's early `return`
-- was not an exit, so the statements it was written to skip ran anyway.
--
-- Three subprograms, chosen so that each leg of the lowering is pinned:
--
--   maybe  a void procedure whose early return must skip the assignment after
--          it. This is #63: what is lost is control flow.
--   pick   a function whose early return carries the value. The trailing
--          `return` overwrote it, making the function a constant. This is #73:
--          what is lost is the value, and it was silent - the printer warned
--          only on the valueless shapes.
--   ident  the control: a single trailing `return`. Reaching the end of the
--          body already leaves it, so this one must acquire neither a block
--          label nor a `disable`. It is what says the fix did not churn the
--          output of every subprogram the printer has ever emitted.
--
-- Deliberately no `out` parameter anywhere: the procedures drive signals visible
-- in their declarative scope instead. A procedure parameter that is not `in` was
-- emitted as a bare `;` until hif-backend#64, and this fixture must not depend
-- on that to compile.
library ieee;
use ieee.std_logic_1164.all;

entity subprogram_early_return is
  port (
    a      : in  std_logic;
    y_proc : out std_logic;
    y_func : out std_logic;
    y_tail : out std_logic
  );
end entity;

architecture rtl of subprogram_early_return is

  procedure maybe(v : in std_logic) is
  begin
    if v = '0' then
      y_proc <= '0';
      return;
    end if;
    y_proc <= '1';
  end procedure;

  function pick(v : std_logic) return std_logic is
  begin
    if v = '0' then
      return '1';
    end if;
    return '0';
  end function;

  function ident(v : std_logic) return std_logic is
  begin
    return v;
  end function;

begin

  process(a)
  begin
    maybe(a);
    y_func <= pick(a);
    y_tail <= ident(a);
  end process;

end architecture;
