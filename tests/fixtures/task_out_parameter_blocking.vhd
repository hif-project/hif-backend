-- Regression fixture for hif-backend#70: an assignment to a task's out or inout
-- argument was emitted with the non-blocking operator "<=".
--
-- A task argument is copied back to its actual when the task *returns*, while a
-- non-blocking assignment updates the task-local storage only at the end of the
-- time step - after the copy-back has already run. The two directions fail
-- differently, and this fixture carries one of each:
--
--   signal s : out    -> the copy-back publishes the value the *previous* call
--                        computed, so the design is one activation late and the
--                        first activation publishes 'x'.
--
--   signal s : inout  -> an inout argument is copied *in* at entry as well as
--                        out at return, so each call overwrites the local with
--                        the still-stale actual before scheduling the new
--                        value. The actual never takes it: it stays 'x' for as
--                        many activations as you care to run.
--
-- Both procedures assign a *literal*, so the value they publish does not depend
-- on the activation count. That is what lets the testbench read them at the
-- first activation and call the result wrong rather than merely early - the
-- distinction the whole defect lives in.
--
-- Deliberately kept separate from task_out_parameter, which is hif-backend#64's
-- regression: that one is about the parameter being *declared* at all, and its
-- fixture is built the opposite way - to stay valid while this defect was
-- unfixed - so it cannot show this one.
library ieee;
use ieee.std_logic_1164.all;

entity task_out_parameter_blocking is
  port (
    a       : in  std_logic;
    y_out   : out std_logic;
    y_inout : out std_logic
  );
end entity;

architecture rtl of task_out_parameter_blocking is

  -- Driven through an inout parameter, then observed on a port. An inout formal
  -- needs an actual it can also read, so it cannot be an out port directly.
  signal io_sig : std_logic;

  procedure set_high(signal s : out std_logic) is
  begin
    s <= '1';
  end procedure;

  procedure clear_io(signal s : inout std_logic) is
  begin
    s <= '0';
  end procedure;

begin

  process(a)
  begin
    set_high(y_out);
    clear_io(io_sig);
  end process;

  y_inout <= io_sig;

end architecture;
