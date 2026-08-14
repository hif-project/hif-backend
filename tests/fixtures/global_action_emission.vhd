-- Fixture for hif-backend#32: hif2verilog dropped a view's GlobalAction, so
-- every VHDL concurrent signal assignment was lost and the regenerated module
-- had the right ports and an empty body.
--
-- Four shapes, each of which the fix has to get right:
--   s <= a and b            an internal signal driven concurrently, which must
--                           regenerate as "wire" rather than "reg"
--   t <= s                  an output port driven concurrently, which must
--                           regenerate as "output wire" rather than "output reg"
--   u <= s or c             a concurrent assignment *reading* another one's
--                           target, so the two have to agree on s being a net
--   d <= a xor b after 2 ns a delayed concurrent assignment; the delay is the
--                           hif-backend#24 machinery, which until this fix had
--                           no VHDL-derived assignment to travel on
library ieee;
use ieee.std_logic_1164.all;

entity global_action_emission is
  port (a : in  std_logic;
        b : in  std_logic;
        c : in  std_logic;
        t : out std_logic;
        u : out std_logic;
        d : out std_logic);
end global_action_emission;

architecture rtl of global_action_emission is
  signal s : std_logic;
begin
  s <= a and b;
  t <= s;
  u <= s or c;
  d <= a xor b after 2 ns;
end rtl;
