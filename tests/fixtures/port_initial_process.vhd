-- Fixture for hif-backend#36: a VHDL output port's explicit initial value was
-- dropped when a process drove the port, so the regenerated Verilog read x
-- where the source read the stated value.
--
-- Three shapes, each of which the fix has to get right:
--   q : out std_logic := '1'   stated initial value on a process-driven port.
--                              The process writes q only when a is '1', so the
--                              value survives the process's first run and is
--                              observable for as long as a stays '0'. Written
--                              this way on purpose: a process that assigned q
--                              unconditionally would overwrite the initial
--                              value immediately and the test would pass
--                              whether or not the value was emitted.
--   s : out std_logic          no stated initial value. vhdl2hif still gives
--                              it a VALUE - the 'U' default - so this is the
--                              control that distinguishes a value the source
--                              wrote from one it did not. It must get no
--                              initialization at all.
--   t : out std_logic := '1'   stated initial value on a port driven by a
--                              *concurrent* assignment. That port is a net, so
--                              its value must NOT be restated procedurally:
--                              an `initial` assignment to a wire does not
--                              compile, and a second continuous assign would
--                              resolve to x. This is the hif-backend#30 gate
--                              seen from the other side.

library ieee;
use ieee.std_logic_1164.all;

entity port_initial_process is
  port (a : in  std_logic;
        q : out std_logic := '1';
        s : out std_logic;
        t : out std_logic := '1');
end port_initial_process;

architecture rtl of port_initial_process is
begin

  t <= a;

  process (a)
  begin
    if a = '1' then
      q <= '0';
    end if;
  end process;

end rtl;
