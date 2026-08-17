-- ----------------------------------------------------------------------------
-- Fixture for hif-backend#55.
--
-- vhdl2hif gives every signal and port the IEEE-1164 'U' default whether the
-- source stated one or not, so the emitted Verilog must distinguish "the source
-- wrote an initial value" from "the frontend supplied one". Three ports make
-- that distinction testable:
--
--   sc  scalar,  no stated initial value -> no initializer. Already correct
--                before the fix, and kept here as the control that says what
--                the vector case is being held to.
--   vec vector,  no stated initial value -> no initializer. This is the defect:
--                it regenerated as `initial vec = 4'buuuu;`, and `u` is not a
--                Verilog binary digit, so the file did not parse.
--   pre vector,  stated initial value    -> initializer preserved. Present so
--                that suppressing every vector initializer cannot pass this
--                test: the rule is about implicit defaults, not about vectors.
--
-- Every port is driven from a process and only under `a = '1'`, so each one is
-- a reg whose stated value has to survive as an `initial` assignment rather
-- than a continuous assign (the hif-backend#36 path).
-- ----------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

entity vector_unknown_default is
  port (a   : in  std_logic;
        sc  : out std_logic;
        vec : out std_logic_vector(3 downto 0);
        pre : out std_logic_vector(3 downto 0) := "0101");
end entity;

architecture rtl of vector_unknown_default is
begin
  process (a)
  begin
    if a = '1' then
      sc  <= '1';
      vec <= "1010";
      pre <= "1100";
    end if;
  end process;
end architecture;
