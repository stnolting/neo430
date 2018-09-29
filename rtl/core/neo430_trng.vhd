-- #################################################################################################
-- #  << NEO430 - True Random Number Generator >>                                                  #
-- # ********************************************************************************************* #
-- # True random number generator based on free running oscillators. These oscillators are built   #
-- # from single inverters, where the output is directly used as input again. The feedback is de-  #
-- # coupled using latches, which are transparent when the TRNG is running. The latches are auto-  #
-- # matically initilized (reset) before operation.                                                #
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
-- # Stephan Nolting, Hannover, Germany                                                 29.09.2018 #
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
    wren_i : in  std_ulogic; -- write enable
    addr_i : in  std_ulogic_vector(15 downto 0); -- address
    data_i : in  std_ulogic_vector(15 downto 0); -- data in
    data_o : out std_ulogic_vector(15 downto 0)  -- data out
  );
end neo430_trng;

architecture neo430_trng_rtl of neo430_trng is

  -- ADVANCED user configuration -------------------------------------------------------------------
  constant num_trngs_c : natural := 4; -- number of random generators (= #oscillators, default = 4)
  -- -----------------------------------------------------------------------------------------------

  -- control register bits --
  constant ctrl_rnd_en_c : natural := 0; -- -/w: TRNG enable

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size_f(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size_f(trng_size_c); -- low address boundary bit

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal wren   : std_ulogic; -- full word write enable
  signal rden   : std_ulogic; -- read enable

  -- random number generator --
  signal rnd_gen    : std_ulogic_vector(num_trngs_c-1 downto 0);
  signal rnd_reset  : std_ulogic_vector(num_trngs_c-1 downto 0);
  signal rnd_enable : std_ulogic;
  signal rnd_sync0  : std_ulogic_vector(num_trngs_c-1 downto 0);
  signal rnd_sync1  : std_ulogic_vector(num_trngs_c-1 downto 0);
  signal rnd_bit    : std_ulogic;
  signal rnd_sreg   : std_ulogic_vector(7 downto 0);
  signal rnd_cnt    : std_ulogic_vector(2 downto 0);
  signal rnd_data   : std_ulogic_vector(7 downto 0); -- accessibe data register (read-only)

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = trng_base_c(hi_abb_c downto lo_abb_c)) else '0';
  wren   <= acc_en and wren_i;
  rden   <= acc_en and rden_i;

  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- write access --
      if (wren = '1') then
        rnd_enable <= data_i(ctrl_rnd_en_c);
      end if;
      -- using individual resets for each OSC - derived from a shift register - to prevent the synthesis tool
      -- from removing all but one OSC (since they implement "logical identical functions")
      rnd_reset <= rnd_reset(num_trngs_c-2 downto 0) & (not rnd_enable);
    end if;
  end process wr_access;


  -- Random Generator ---------------------------------------------------------
  -- -----------------------------------------------------------------------------
  random_generator: process(rnd_reset, rnd_enable, rnd_gen)
  begin
    for i in 0 to num_trngs_c-1 loop
      if (rnd_reset(i) = '1') then
        rnd_gen(i) <= '0';
      elsif (rnd_enable = '1') then -- yeah, these ARE latches ;)
        rnd_gen(i) <= not rnd_gen(i); -- here we have the oscillators
      end if;
    end loop; -- i
  end process random_generator;

  -- synchronize output of each oscillator --
  random_generator_sync: process(clk_i)
  begin
    if rising_edge(clk_i) then
      rnd_sync0 <= rnd_gen;
      rnd_sync1 <= rnd_sync0; -- no metastability beyond this point!
    end if;
  end process random_generator_sync;

  -- XOR all generators --
  random_generator_xor: process(rnd_sync1)
    variable rnd_v : std_ulogic;
  begin
    rnd_v := rnd_sync1(0);
    for i in 1 to num_trngs_c-1 loop
      rnd_v := rnd_v xor rnd_sync1(i);
    end loop; -- i
    rnd_bit <= rnd_v;
  end process random_generator_xor;


  -- Random Data Shift Register -----------------------------------------------
  -- -----------------------------------------------------------------------------
  data_sreg: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- acquire data --
      if (rnd_enable = '0') then
        rnd_cnt <= (others => '0');
      else
        rnd_cnt  <= std_ulogic_vector(unsigned(rnd_cnt) + 1);
        rnd_sreg <= rnd_sreg(6 downto 0) & rnd_bit;
      end if;
      -- sample output byte --
      if (rnd_cnt = "000") and (rnd_enable = '1') then
        rnd_data <= rnd_sreg;
      end if;
    end if;
  end process data_sreg;


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0');
      if (rden = '1') then
        data_o(7 downto 0) <= rnd_data;
      end if;
    end if;
  end process rd_access;


end neo430_trng_rtl;
