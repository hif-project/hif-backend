-- Regression fixture for hif-backend#64: a procedure parameter that is not
-- `in` was emitted as a bare `;`, so the task referenced names it never
-- declared and the file did not parse.
--
-- Covers the two ways a procedure parameter reaches the printer as dir_out,
-- plus the dir_in control that always worked:
--
--   signal s   : out  -> the ordinary way a procedure drives a signal
--   variable s : out  -> reached with a variable actual, as VHDL requires for a
--                        variable-class formal; passing a signal there would be
--                        illegal VHDL that vhdl2hif only happens to accept
--   signal s   : in   -> the control
--
-- dir_inout lives in task_inout_parameter.vhd rather than here, because it
-- cannot be taken through the same gate: the regenerated `inout s;` task
-- argument is legal Verilog that iverilog accepts but verilog2hif cannot reparse
-- (hif-frontend#25). Keeping it separate means this fixture can require the full
-- round trip.
--
-- The driving procedures assign a *literal* rather than their input. That is
-- deliberate: it makes the value they publish independent of how many times the
-- process has run, so the behavioural checks say the copy-back happened without
-- also depending on when it happened. The timing itself is hif-backend#70's,
-- and is covered by task_out_parameter_blocking.
library ieee;
use ieee.std_logic_1164.all;

entity task_out_parameter is
  port (
    a      : in  std_logic;
    y_high : out std_logic;
    y_var  : out std_logic;
    y_copy : out std_logic
  );
end entity;

architecture rtl of task_out_parameter is

  procedure set_high(signal s : out std_logic) is
  begin
    s <= '1';
  end procedure;

  procedure set_var(variable s : out std_logic) is
  begin
    s := '1';
  end procedure;

  procedure copy(signal src : in std_logic; signal dst : out std_logic) is
  begin
    dst <= src;
  end procedure;

begin

  process(a)
    variable tmp : std_logic;
  begin
    set_high(y_high);
    set_var(tmp);
    y_var <= tmp;
    copy(a, y_copy);
  end process;

end architecture;
