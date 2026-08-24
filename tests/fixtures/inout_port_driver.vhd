-- Regression fixture for hif-backend#71: a VHDL `inout` port that a process
-- drives was emitted as a bare Verilog `inout` - a net - while the process
-- assigned to it procedurally. A net is not a valid procedural l-value, so the
-- regenerated design did not elaborate, and hif2verilog exited 0.
--
-- Four things have to hold at once, and the fixture carries one port for each:
--
--   b    driven from a process, so it needs the reg-plus-continuous-assign
--        lowering. This is the reported case.
--
--   b    also *released*, by assigning 'Z'. VHDL spells high impedance as a
--        value of the resolved type rather than as a separate enable, so this
--        is what says the lowering does not have to invent one: the value
--        reaches Verilog as 1'bz and the continuous assign passes it through.
--
--   mon  reads `b` inside the same process. The read must stay on the *port*,
--        not follow the assignment onto the driver reg - what a process reads
--        from an inout is the resolved value on the net, including whatever an
--        external driver is contributing. Getting this wrong would be silent:
--        it compiles either way and only differs when someone else drives.
--
--   c    an inout driven by a *concurrent* assignment rather than a process.
--        The control: a net is already the right declaration there, so it must
--        come out untouched, with no driver reg of its own.
library ieee;
use ieee.std_logic_1164.all;

entity inout_port_driver is
  port (
    en  : in    std_logic;
    d   : in    std_logic;
    b   : inout std_logic;
    mon : out   std_logic;
    src : in    std_logic;
    c   : inout std_logic
  );
end entity;

architecture rtl of inout_port_driver is
begin

  process(en, d, b)
  begin
    if en = '1' then
      b <= d;
    else
      b <= 'Z';
    end if;
    mon <= b;
  end process;

  c <= src;

end architecture;
