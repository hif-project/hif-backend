-- The one multi-clause wait whose lowering needs no `fork`: with no timeout
-- there is only one reason to resume, so the event-and-condition loop stands on
-- its own inside the named block.
--
-- Kept as a separate fixture from multi_clause_wait.vhd so that the round-trip
-- property can still be required of *something* on this path. The three
-- combinations that do carry a timeout emit a `fork`, which verilog2hif cannot
-- parse (hif-frontend#26), so they cannot be asked to reparse; this one can, and
-- is, which localises that gap to `fork` rather than to the lowering.
library ieee;
use ieee.std_logic_1164.all;

entity multi_clause_wait_no_timeout is
  port (
    a : in  std_logic;
    b : in  std_logic;
    q : out std_logic
  );
end entity;

architecture rtl of multi_clause_wait_no_timeout is
begin

  p : process
  begin
    q <= '0';
    wait on a until b = '1';
    q <= '1';
    wait;
  end process;

end architecture;
