-- Regression fixture (hif-backend#27): a hierarchical VHDL design, whose
-- parent unit hif2vhdl wrote as a zero-byte file.
--
-- The order matters. The parent is written second, and the bug was that the
-- design unit emitted last never had its stream flushed - so a fixture with
-- the parent first would pass while still being broken.
library ieee; use ieee.std_logic_1164.all;
entity vhdl_hierarchy_child is
  port (a, b : in std_logic; sum, carry : out std_logic);
end vhdl_hierarchy_child;
architecture rtl of vhdl_hierarchy_child is
begin
  sum   <= a xor b;
  carry <= a and b;
end rtl;

library ieee; use ieee.std_logic_1164.all;
entity vhdl_hierarchy is
  port (a, b, cin : in std_logic; sum, cout : out std_logic);
end vhdl_hierarchy;
architecture rtl of vhdl_hierarchy is
  component vhdl_hierarchy_child
    port (a, b : in std_logic; sum, carry : out std_logic);
  end component;
  signal s1, c1, c2 : std_logic;
begin
  u_ha1 : vhdl_hierarchy_child port map (a => a, b => b, sum => s1, carry => c1);
  u_ha2 : vhdl_hierarchy_child port map (a => s1, b => cin, sum => sum, carry => c2);
  cout <= c1 or c2;
end rtl;
