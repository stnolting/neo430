-- #################################################################################################
-- #  << NEO430 - True Random Number Generator >>                                                  #
-- # ********************************************************************************************* #
-- # This unit implements a true random number generator which uses an inverter chain as entropy   #
-- # source. The inverter chain is constructed as GARO (Galois Ring Oscillator) TRNG. The single   #
-- # inverters are connected via simple latches that are used to enbale/disable the TRNG. Also,    #
-- # these latches are used as additional delay element. By using unique enable signals for each   #
-- # latch, the synthesis tool cannot "optimize" one of the inverters out of the design. Further-  #
-- # more, the latches prevent the synthesis tool from detecting combinatorial loops.              #
-- #                                                                                               #
-- # Sources:                                                                                      #
-- #  - GARO: "Enhancing the Randomness of a Combined True Random Number Generator Based on the    #
-- #    Ring Oscillator Sampling Method" by Mieczyslaw Jessa and Lukasz Matuszewski                #
-- #  - Latches for platform independence: "Extended Abstract: The Butterfly PUF Protecting IP     #
-- #    on every FPGA" by Sandeep S. Kumar, Jorge Guajardo, Roel Maesyz, Geert-Jan Schrijen and    #
-- #    Pim Tuyls, Philips Research Europe, 2008                                                   #
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
-- # Stephan Nolting, Hannover, Germany                                                 27.11.2019 #
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

  -- user configuration --------------------------------------------------------------------------------
  constant num_oscs_c  : natural := 5; -- number of oscillators (default=5)
  constant garo_taps_c : std_ulogic_vector(num_oscs_c-2 downto 0) := "0101"; -- GARO xor feedback select
  constant use_lfsr_c  : boolean := true; -- use LFSR for post-processing (default=true)
  constant lfsr_taps_c : std_ulogic_vector(7 downto 0) := "10111000"; -- LFSR feedback taps
  -- ---------------------------------------------------------------------------------------------------

  -- control register bits --
  constant ctrl_rnd_en_c : natural := 15; -- -/w: TRNG enable

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size_f(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size_f(trng_size_c); -- low address boundary bit

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal wren   : std_ulogic; -- full word write enable
  signal rden   : std_ulogic; -- read enable

  -- random number generator --
  signal rnd_inv         : std_ulogic_vector(num_oscs_c-1 downto 0); -- inverter chain
  signal rnd_enable_sreg : std_ulogic_vector(num_oscs_c-1 downto 0); -- enable shift register
  signal rnd_enable      : std_ulogic;
  signal rnd_sync0       : std_ulogic;
  signal rnd_sync1       : std_ulogic;
  signal rnd_sreg        : std_ulogic_vector(7 downto 0); -- sample shift reg
  signal rnd_cnt         : std_ulogic_vector(2 downto 0);
  signal rnd_data        : std_ulogic_vector(7 downto 0); -- random data register (read-only)

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
      -- using individual enable signals for each inverter - derived from a shift register - to prevent the synthesis tool
      -- from removing all but one inverter (since they implement "logical identical functions")
      -- this also allows to make the trng platform independent
      rnd_enable_sreg <= rnd_enable_sreg(num_oscs_c-2 downto 0) & rnd_enable; -- activate right most inverter first
    end if;
  end process wr_access;


  -- True Random Generator ----------------------------------------------------
  -- -----------------------------------------------------------------------------
  entropy_source: process(rnd_enable_sreg, rnd_enable, rnd_inv)
  begin
    for i in 0 to num_oscs_c-1 loop
      if (rnd_enable = '0') then -- start with a defined state (latch reset)
        rnd_inv(i) <= '0';
      -- use latches to decouple the inverters
      -- by this, the synthesis tool does not complain about combinatorial loops
      elsif (rnd_enable_sreg(i) = '1') then -- uniquely enable latches to prevent synthesis from removing chain elements
        -- here we have the inverter chain --
        if (i = num_oscs_c-1) then -- left most inverter?
          rnd_inv(i) <= not rnd_inv(0); -- direct input of right most inverter (= output signal)
        else
          if (garo_taps_c(i) = '1') then
            rnd_inv(i) <= (not rnd_inv(i+1)) xor rnd_inv(0); -- use final output as feedback
          else
            rnd_inv(i) <= not rnd_inv(i+1); -- normal chain: use previous inverter's output as input
          end if;
        end if;
      end if;
    end loop; -- i
  end process entropy_source;


  -- Random Data Shift Register -----------------------------------------------
  -- -----------------------------------------------------------------------------
  data_sreg: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- synchronize output of oscillator chain --
      rnd_sync0 <= rnd_inv(0);
      rnd_sync1 <= rnd_sync0; -- no more metastability
      -- sample random data --
      if (rnd_enable = '0') then
        rnd_cnt  <= (others => '0');
        rnd_sreg <= (others => '0');
      else
        rnd_cnt <= std_ulogic_vector(unsigned(rnd_cnt) + 1);
        if (use_lfsr_c = true) then -- use LFSR for post-processing
          rnd_sreg <= rnd_sreg(6 downto 0) & (xor_all_f(rnd_sreg and lfsr_taps_c) xor rnd_sync1);
        else -- no post-processing
          rnd_sreg <= rnd_sreg(6 downto 0) & rnd_sync1;
        end if;
      end if;
      -- sample final output byte --
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
        data_o(7 downto 0)    <= rnd_data;
        data_o(ctrl_rnd_en_c) <= rnd_enable;
      end if;
    end if;
  end process rd_access;


end neo430_trng_rtl;
