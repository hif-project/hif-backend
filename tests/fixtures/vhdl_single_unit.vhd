-- Regression fixture (hif-backend#27), minimal form: one entity, no hierarchy.
--
-- The last design unit emitted was never flushed, and here it is also the
-- first, so hif2vhdl produced no output at all for this design while still
-- exiting 0. Reported against a hierarchical design, where only the parent
-- looked lost - this is the whole of it.
library ieee; use ieee.std_logic_1164.all;
entity vhdl_single_unit is
  port (a, b : in std_logic; y : out std_logic);
end vhdl_single_unit;
architecture rtl of vhdl_single_unit is
begin
  y <= a and b;
end rtl;
