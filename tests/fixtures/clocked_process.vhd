-- Fixture for the clocked_process regression (hif-backend#51).
--
-- Three clocked processes, covering the three things the rebuild has to get
-- right, in one entity so a single simulation checks all of them:
--
--   q_async  'event spelling, asynchronous reset. The shape the issue reported:
--            `if rst = '1' ... elsif clk'event and clk = '1'`. Both the clock
--            edge and the reset assertion have to reach the sensitivity list.
--   q_sync   rising_edge spelling, synchronous reset. The reset is tested
--            *inside* the clocked branch and must stay there - promoting it to
--            the sensitivity list would turn it asynchronous, which the
--            testbench detects directly.
--   q_fall   falling_edge, no reset. The whole if disappears and its body
--            becomes the process body, under a negedge.
--
-- Every port is a scalar std_logic on purpose. A vector output port with no
-- stated initial value currently regenerates with `initial vec = 4'buuuu;`,
-- which is not parseable Verilog (hif-backend#55) - unrelated to this issue,
-- and it would stop this fixture from compiling for the wrong reason.
--
-- The outputs are only written, never read, so no output-port readback rule
-- is involved either.
library ieee;
use ieee.std_logic_1164.all;

entity clocked_process is
    port (
        clk     : in  std_logic;
        rst     : in  std_logic;
        d       : in  std_logic;
        q_async : out std_logic;
        q_sync  : out std_logic;
        q_fall  : out std_logic
    );
end entity;

architecture rtl of clocked_process is
begin

    -- Asynchronous reset, 'event spelling.
    process (clk, rst)
    begin
        if rst = '1' then
            q_async <= '0';
        elsif clk'event and clk = '1' then
            q_async <= d;
        end if;
    end process;

    -- Synchronous reset, rising_edge spelling.
    process (clk)
    begin
        if rising_edge(clk) then
            if rst = '1' then
                q_sync <= '0';
            else
                q_sync <= d;
            end if;
        end if;
    end process;

    -- No reset, falling edge.
    process (clk)
    begin
        if falling_edge(clk) then
            q_fall <= d;
        end if;
    end process;

end architecture;
