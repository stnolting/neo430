-- #################################################################################################
-- #  << NEO430 - True Random Number Generator >>                                                  #
-- # ********************************************************************************************* #
-- # True random number generator based on free running oscillators. These oscillators are built   #
-- # from single inverters, where the output is directly used as input again. The feedback is de-  #
-- # coupled using latches, which are transparent when the TRNG is running. The latches are auto-  #
-- # matically initilized (reset) before using.                                                    #
-- # Adjust the number of oscillators according to your feeling ;)                                 #
-- # ********************************************************************************************* #
-- # This file is part of the NEO430 Processor project: https://github.com/stnolting/neo430        #
-- # Copyright by Stephan Nolting: stnolting@gmail.com                                             #
-- #                                                                                               #
-- # This source file may be used and distributed without restriction provided that this copyright #
-- # statement is not removed from the file and that any derivative work contains the original     #
-- # copyright notice and the associated disclaimer.                                               #
-- #                                                                                               #
-- # This source file is free software; you can redistribute it and/or modify it under the terms   #
-- # of the GNU Lesser General Public License as published by the Free Software Foundation,        #
-- # either version 3 of the License, or (at your option) any later version.                       #
-- #                                                                                               #
-- # This source is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;      #
-- # without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     #
-- # See the GNU Lesser General Public License for more details.                                   #
-- #                                                                                               #
-- # You should have received a copy of the GNU Lesser General Public License along with this      #
-- # source; if not, download it from https://www.gnu.org/licenses/lgpl-3.0.en.html                #
-- # ********************************************************************************************* #
-- # Stephan Nolting, Hannover, Germany                                                 24.04.2018 #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;

entity neo430_trng is
  port (
    -- host access --
    clk_i  : in  std_ulogic; -- global clock line
    rden_i : in  std_ulogic; -- read enable
    wren_i : in  std_ulogic_vector(01 downto 0); -- write enable
    addr_i : in  std_ulogic_vector(15 downto 0); -- address
    data_i : in  std_ulogic_vector(15 downto 0); -- data in
    data_o : out std_ulogic_vector(15 downto 0)  -- data out
  );
end neo430_trng;

architecture neo430_trng_rtl of neo430_trng is

  -- ADVANCED user configuration -------------------------------------------------------------------
  constant num_trngs_c : natural := 8; -- number of random generators (= oscillators, default = 8)
  -- -----------------------------------------------------------------------------------------------

  -- control register bits --
  constant ctrl_rnd_en_c  : natural :=  0; -- -/w: TRNG enable

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size(trng_size_c); -- low address boundary bit

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal addr   : std_ulogic_vector(15 downto 0); -- access address
  signal wren   : std_ulogic; -- full word write enable
  signal rden   : std_ulogic; -- read enable

  -- random number generator --
  signal sreg       : std_ulogic_vector(7 downto 0);
  signal cnt        : std_ulogic_vector(2 downto 0);
  signal rnd_gen    : std_ulogic_vector(num_trngs_c-1 downto 0);
  signal rnd_reset  : std_ulogic;
  signal rnd_enable : std_ulogic;
  signal rnd_bit    : std_ulogic;
  signal rnd_sync0  : std_ulogic;
  signal rnd_sync1  : std_ulogic;
  signal rnd_data   : std_ulogic_vector(7 downto 0);

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = trng_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= trng_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wren   <= acc_en and wren_i(1) and wren_i(0);
  rden   <= acc_en and rden_i;


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      rnd_reset  <= not rnd_enable;
      if (wren = '1') then -- valid write access
        rnd_enable <= data_i(ctrl_rnd_en_c);
      end if;
    end if;
  end process wr_access;


  -- RND Generators -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  random_generators: process(rnd_reset, rnd_enable, rnd_gen)
  begin
    for i in 0 to num_trngs_c-1 loop
      if (rnd_reset = '1') then
        rnd_gen(i) <= '0';
      elsif (rnd_enable = '1') then -- yeah, these ARE latches ;)
        rnd_gen(i) <= not rnd_gen(i); -- these are the oscillators
      end if;
    end loop; -- i
  end process random_generators;

  -- XOR all generators together --
  rnd_gen_xor: process(rnd_gen)
    variable rnd_v : std_ulogic;
  begin
    rnd_v := rnd_gen(0);
    for i in 1 to num_trngs_c-1 loop
      rnd_v := rnd_v xor rnd_gen(i);
    end loop; -- i
    rnd_bit <= rnd_v;
  end process rnd_gen_xor;


  -- Random Data Shift Register -----------------------------------------------
  -- -----------------------------------------------------------------------------
  data_sreg: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- synchronize data --
      rnd_sync0 <= rnd_bit;
      rnd_sync1 <= rnd_sync0;
      -- acquire data --
      if (rnd_enable = '0') then
        cnt <= (others => '0');
      else
        cnt  <= std_ulogic_vector(unsigned(cnt) + 1);
        sreg <= sreg(6 downto 0) & rnd_sync1;
        if (cnt = "000") then -- sample output byte
          rnd_data <= sreg;
        end if;
      end if;
    end if;
  end process data_sreg;


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0');
      if (rden = '1') then -- valid read access
        data_o(7 downto 0) <= rnd_data;
      end if;
    end if;
  end process rd_access;


end neo430_trng_rtl;
