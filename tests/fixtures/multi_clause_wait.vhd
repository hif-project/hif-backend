-- Regression fixture for hif-backend#45: a VHDL wait that sets more than one of
-- condition, sensitivity and timeout has no single Verilog construct, and was
-- refused outright.
--
-- All four reachable multi-clause combinations appear once, and the stimulus is
-- built so that each resumes for a *different* reason. That matters: a lowering
-- that got the structure right but the resumption logic wrong would still make
-- every output eventually go high, so what is checked is the time each one
-- resumes at, not that it resumes.
--
--   q_all  wait on a until b = '1' for 10 ns   `a` never toggles -> timeout
--   q_st   wait on a for 10 ns                 `a` never toggles -> timeout
--   q_cs   wait on c until b = '1'             event on c, gated by b
--   q_ct   wait until b = '1' for 10 ns        the condition becoming true
--
-- `a` is deliberately left idle for the two timeout cases. A single sequential
-- block cannot express this: it would sit in the event control on `a` and never
-- reach the deadline, so the wait would never resume at all - which is exactly
-- what the shape originally proposed in the issue did.
--
-- q_cs is the re-arming case. `c` toggles twice, once while `b` is still '0';
-- that first event must NOT resume it, because the condition gates the event.
--
-- Each process ends in `wait;` so it fires once and suspends, making the
-- resumption time observable as a single transition.
library ieee;
use ieee.std_logic_1164.all;

entity multi_clause_wait is
  port (
    a     : in  std_logic;
    b     : in  std_logic;
    c     : in  std_logic;
    q_all : out std_logic;
    q_st  : out std_logic;
    q_cs  : out std_logic;
    q_ct  : out std_logic
  );
end entity;

architecture rtl of multi_clause_wait is
begin

  p_all : process
  begin
    q_all <= '0';
    wait on a until b = '1' for 10 ns;
    q_all <= '1';
    wait;
  end process;

  p_st : process
  begin
    q_st <= '0';
    wait on a for 10 ns;
    q_st <= '1';
    wait;
  end process;

  p_cs : process
  begin
    q_cs <= '0';
    wait on c until b = '1';
    q_cs <= '1';
    wait;
  end process;

  p_ct : process
  begin
    q_ct <= '0';
    wait until b = '1' for 10 ns;
    q_ct <= '1';
    wait;
  end process;

end architecture;
