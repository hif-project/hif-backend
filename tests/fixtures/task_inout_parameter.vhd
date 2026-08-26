-- The dir_inout half of hif-backend#64: a procedure parameter declared
-- `signal ... : inout` was emitted as a bare `;` exactly as an `out` one was.
--
-- Separate from task_out_parameter.vhd because it cannot go through the same
-- gate. The correct emission is a Verilog task `inout` argument, which is legal
-- Verilog-2001 (IEEE Std 1364-2005, 10.2.1) and which iverilog accepts, but
-- which `verilog2hif` cannot reparse (hif-frontend#25). Its value cannot be
-- asserted either: a task `inout` argument is copied *in* at entry as well as
-- out at return, so with the non-blocking assignment of hif-backend#70 each call
-- overwrites the local with the stale actual and the value never converges.
--
-- So this fixture's gate is: the parameter is declared, the file compiles. That
-- is precisely what #64 is about, and no more.
--
-- The actual is an architecture-level signal rather than an `inout` port on
-- purpose: a VHDL `inout` port driven from a process regenerates as a Verilog
-- net and is not a valid procedural l-value (hif-backend#71). `io_sig` is
-- process-driven, so it is emitted as a `reg`, which is the valid l-value a task
-- `inout` argument needs.
library ieee;
use ieee.std_logic_1164.all;

entity task_inout_parameter is
  port (
    a    : in  std_logic;
    y_io : out std_logic
  );
end entity;

architecture rtl of task_inout_parameter is

  signal io_sig : std_logic;

  procedure clear_io(signal s : inout std_logic) is
  begin
    s <= '0';
  end procedure;

begin

  process(a)
  begin
    clear_io(io_sig);
    y_io <= io_sig;
  end process;

end architecture;
