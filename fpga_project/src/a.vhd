library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity counter is
    Port ( clk : in STD_LOGIC;
           led : out STD_LOGIC_VECTOR(7 downto 0));
end counter;

architecture Behavioral of counter is
    signal count : unsigned(23 downto 0) := (others => '0');
begin
    process(clk)
    begin
        if rising_edge(clk) then
            count <= count + 1;
        end if;
    end process;
    
    led <= std_logic_vector(count(23 downto 16));
end Behavioral;