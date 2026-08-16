-- Fixture for the permanent_suspension regression (hif-backend#46).
--
-- A VHDL process with no sensitivity list that ends in `wait;` - suspend and
-- never resume. It runs once, top to bottom, and stops. Verilog spells that
-- `initial`, and emitting `always` for it produces a zero-delay infinite loop
-- that Icarus rejects at elaboration.
--
-- Two processes, because the interesting cases fail differently:
--
--   p_once   the reported shape: statements, then `wait;`, and nothing else.
--            The only Wait in the process is the permanent one.
--   p_delay  a process that waits for time *and then* suspends permanently.
--            This is the one a rule like "a process containing any wait can be
--            woken, so it is an always" gets wrong: there is a resumable wait
--            in it, and the process still must not loop. It also proves the
--            permanent wait is decided on its own rather than weighed against
--            the others.
--
-- The signals are only written, never read, and are plain scalars: a vector
-- output port with no stated initial value currently regenerates with an
-- unparseable `4'buuuu` initializer (hif-backend#57's neighbour, hif-backend#55),
-- which would stop this fixture compiling for an unrelated reason.
library ieee;
use ieee.std_logic_1164.all;

entity permanent_suspension is
    port (
        a    : in  std_logic;
        once : out std_logic;
        late : out std_logic
    );
end entity;

architecture rtl of permanent_suspension is
begin

    p_once : process
    begin
        once <= a;
        wait;
    end process;

    p_delay : process
    begin
        late <= '0';
        wait for 10 ns;
        late <= a;
        wait;
    end process;

end architecture;
