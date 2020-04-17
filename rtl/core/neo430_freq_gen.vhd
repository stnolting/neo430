-- #################################################################################################
-- #  << NEO430 - Arbitrary Frequency Generator >>                                                 #
-- # ********************************************************************************************* #
-- # Number controlled oscillator-based frequency generator with three independent channels. Each  #
-- # channel has its enable flag, 16-bit tuning word register and prescaler selector. The phase    #
-- # accumulator of each channel is 17 bit wide.                                                   #
-- # f_out(x) = ((f_cpu / nco_prsc(x)) * tuning_word(x)) / 2^17   for channels x = 0,1,2           #
-- # ********************************************************************************************* #
-- # BSD 3-Clause License                                                                          #
-- #                                                                                               #
-- # Copyright (c) 2020, Stephan Nolting. All rights reserved.                                     #
-- #                                                                                               #
-- # Redistribution and use in source and binary forms, with or without modification, are          #
-- # permitted provided that the following conditions are met:                                     #
-- #                                                                                               #
-- # 1. Redistributions of source code must retain the above copyright notice, this list of        #
-- #    conditions and the following disclaimer.                                                   #
-- #                                                                                               #
-- # 2. Redistributions in binary form must reproduce the above copyright notice, this list of     #
-- #    conditions and the following disclaimer in the documentation and/or other materials        #
-- #    provided with the distribution.                                                            #
-- #                                                                                               #
-- # 3. Neither the name of the copyright holder nor the names of its contributors may be used to  #
-- #    endorse or promote products derived from this software without specific prior written      #
-- #    permission.                                                                                #
-- #                                                                                               #
-- # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   #
-- # OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               #
-- # MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    #
-- # COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     #
-- # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE #
-- # GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    #
-- # AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     #
-- # NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  #
-- # OF THE POSSIBILITY OF SUCH DAMAGE.                                                            #
-- # ********************************************************************************************* #
-- # The NEO430 Processor - https://github.com/stnolting/neo430                                    #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;

entity neo430_freq_gen is
  port (
    -- host access --
    clk_i       : in  std_ulogic; -- global clock line
    rden_i      : in  std_ulogic; -- read enable
    wren_i      : in  std_ulogic; -- write enable
    addr_i      : in  std_ulogic_vector(15 downto 0); -- address
    data_i      : in  std_ulogic_vector(15 downto 0); -- data in
    data_o      : out std_ulogic_vector(15 downto 0); -- data out
    -- clock generator --
    clkgen_en_o : out std_ulogic; -- enable clock generator
    clkgen_i    : in  std_ulogic_vector(07 downto 0);
    -- frequency generator --
    freq_gen_o  : out std_ulogic_vector(02 downto 0)  -- programmable frequency output
  );
end neo430_freq_gen;

architecture neo430_freq_gen_rtl of neo430_freq_gen is

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size_f(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size_f(freq_gen_size_c); -- low address boundary bit

  -- control reg bits --
  constant ctrl_en_ch0_c    : natural :=  0; -- r/w: enable NCO channel 0
  constant ctrl_en_ch1_c    : natural :=  1; -- r/w: enable NCO channel 1
  constant ctrl_en_ch2_c    : natural :=  2; -- r/w: enable NCO channel 2
  constant ctrl_ch0_prsc0_c : natural :=  3; -- r/w: prescaler select bit 0 for channel 0
  constant ctrl_ch0_prsc1_c : natural :=  4; -- r/w: prescaler select bit 1 for channel 0
  constant ctrl_ch0_prsc2_c : natural :=  5; -- r/w: prescaler select bit 2 for channel 0
  constant ctrl_ch1_prsc0_c : natural :=  6; -- r/w: prescaler select bit 0 for channel 1
  constant ctrl_ch1_prsc1_c : natural :=  7; -- r/w: prescaler select bit 1 for channel 1
  constant ctrl_ch1_prsc2_c : natural :=  8; -- r/w: prescaler select bit 2 for channel 1
  constant ctrl_ch2_prsc0_c : natural :=  9; -- r/w: prescaler select bit 0 for channel 2
  constant ctrl_ch2_prsc1_c : natural := 10; -- r/w: prescaler select bit 1 for channel 2
  constant ctrl_ch2_prsc2_c : natural := 11; -- r/w: prescaler select bit 2 for channel 2

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal addr   : std_ulogic_vector(15 downto 0); -- access address
  signal wren   : std_ulogic; -- word write enable
  signal rden   : std_ulogic; -- word read enable

  -- accessible regs --
  signal ctrl        : std_ulogic_vector(11 downto 0); -- r/w: control register
  type tuning_word_t is array (0 to 2) of std_ulogic_vector(15 downto 0);
  signal tuning_word : tuning_word_t; -- -/w: tuning word channel 0,1,2

  -- nco core --
  type phase_accu_t is array (0 to 2) of std_ulogic_vector(16 downto 0);
  signal nco_phase_accu : phase_accu_t;
  signal nco_prsc_tick  : std_ulogic_vector(2 downto 0);

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = freq_gen_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= freq_gen_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wren   <= acc_en and wren_i;
  rden   <= acc_en and rden_i;


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (wren = '1') then
        if (addr = freq_gen_ctrl_addr_c) then
          ctrl(ctrl_en_ch0_c)    <= data_i(ctrl_en_ch0_c);
          ctrl(ctrl_en_ch1_c)    <= data_i(ctrl_en_ch1_c);
          ctrl(ctrl_en_ch2_c)    <= data_i(ctrl_en_ch2_c);
          ctrl(ctrl_ch0_prsc0_c) <= data_i(ctrl_ch0_prsc0_c);
          ctrl(ctrl_ch0_prsc1_c) <= data_i(ctrl_ch0_prsc1_c);
          ctrl(ctrl_ch0_prsc2_c) <= data_i(ctrl_ch0_prsc2_c);
          ctrl(ctrl_ch1_prsc0_c) <= data_i(ctrl_ch1_prsc0_c);
          ctrl(ctrl_ch1_prsc1_c) <= data_i(ctrl_ch1_prsc1_c);
          ctrl(ctrl_ch1_prsc2_c) <= data_i(ctrl_ch1_prsc2_c);
          ctrl(ctrl_ch2_prsc0_c) <= data_i(ctrl_ch2_prsc0_c);
          ctrl(ctrl_ch2_prsc1_c) <= data_i(ctrl_ch2_prsc1_c);
          ctrl(ctrl_ch2_prsc2_c) <= data_i(ctrl_ch2_prsc2_c);
        end if;
        if (addr = freq_gen_tw_ch0_addr_c) then
          tuning_word(0) <= data_i;
        end if;
        if (addr = freq_gen_tw_ch1_addr_c) then
          tuning_word(1) <= data_i;
        end if;
        if (addr = freq_gen_tw_ch2_addr_c) then
          tuning_word(2) <= data_i;
        end if;
      end if;
    end if;
  end process wr_access;


  -- NCO core (number controlled oscillator) ----------------------------------
  -- -----------------------------------------------------------------------------
  nco_core: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- enable external clock generator --
      clkgen_en_o <= ctrl(ctrl_en_ch0_c) or ctrl(ctrl_en_ch1_c) or ctrl(ctrl_en_ch2_c);
      -- NCOs --
      for i in 0 to 2 loop
        -- NCO clock enable --
        nco_prsc_tick(i) <= clkgen_i(to_integer(unsigned(ctrl(ctrl_ch0_prsc2_c + 3*i downto ctrl_ch0_prsc0_c + 3*i))));
        -- phase accu --
        if (ctrl(ctrl_en_ch0_c + i) = '0') then -- disabled
          nco_phase_accu(i) <= (others => '0');
        elsif (nco_prsc_tick(i) = '1') then -- enabled; wait for clock enable tick
          nco_phase_accu(i) <= std_ulogic_vector(unsigned(nco_phase_accu(i)) + unsigned('0' & tuning_word(i)));
        end if;
        -- output --
        freq_gen_o(i) <= nco_phase_accu(i)(16); -- MSB (carry_out) is output
      end loop; -- i - NCO channel
    end if;
  end process nco_core;


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0');
      if (rden = '1') then
--      if (addr = freq_gen_ctrl_addr_c) then
          data_o(ctrl_en_ch0_c)    <= ctrl(ctrl_en_ch0_c);
          data_o(ctrl_en_ch1_c)    <= ctrl(ctrl_en_ch1_c);
          data_o(ctrl_en_ch2_c)    <= ctrl(ctrl_en_ch2_c);
          data_o(ctrl_ch0_prsc0_c) <= ctrl(ctrl_ch0_prsc0_c);
          data_o(ctrl_ch0_prsc1_c) <= ctrl(ctrl_ch0_prsc1_c);
          data_o(ctrl_ch0_prsc2_c) <= ctrl(ctrl_ch0_prsc2_c);
          data_o(ctrl_ch1_prsc0_c) <= ctrl(ctrl_ch1_prsc0_c);
          data_o(ctrl_ch1_prsc1_c) <= ctrl(ctrl_ch1_prsc1_c);
          data_o(ctrl_ch1_prsc2_c) <= ctrl(ctrl_ch1_prsc2_c);
          data_o(ctrl_ch2_prsc0_c) <= ctrl(ctrl_ch2_prsc0_c);
          data_o(ctrl_ch2_prsc1_c) <= ctrl(ctrl_ch2_prsc1_c);
          data_o(ctrl_ch2_prsc2_c) <= ctrl(ctrl_ch2_prsc2_c);
--      end if;
      end if;
    end if;
  end process rd_access;


end neo430_freq_gen_rtl;
