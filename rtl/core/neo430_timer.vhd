-- #################################################################################################
-- #  << NEO430 - High-Precision Timer >>                                                          #
-- # ********************************************************************************************* #
-- #  Configure THRES before enabling the timer to prevent false interrupt requests.               #
-- # ********************************************************************************************* #
-- # This file is part of the NEO430 Processor project: http://opencores.org/project,neo430        #
-- # Copyright 2015-2017, Stephan Nolting: stnolting@gmail.com                                     #
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
-- # source; if not, download it from http://www.gnu.org/licenses/lgpl-3.0.en.html                 #
-- # ********************************************************************************************* #
-- #  Stephan Nolting, Hannover, Germany                                               20.12.2016  #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.neo430_package.all;

entity neo430_timer is
  port (
    -- host access --
    clk_i       : in  std_ulogic; -- global clock line
    rden_i      : in  std_ulogic; -- read enable
    wren_i      : in  std_ulogic_vector(01 downto 0); -- write enable
    addr_i      : in  std_ulogic_vector(15 downto 0); -- address
    data_i      : in  std_ulogic_vector(15 downto 0); -- data in
    data_o      : out std_ulogic_vector(15 downto 0); -- data out
    -- clock generator --
    clkgen_en_o : out std_ulogic; -- enable clock generator
    clkgen_i    : in  std_ulogic_vector(07 downto 0);
    -- interrupt --
    irq_o       : out std_ulogic  -- interrupt request
  );
end neo430_timer;

architecture neo430_timer_rtl of neo430_timer is

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size(timer_size_c); -- low address boundary bit

  -- control reg bits --
  constant tctrl_en_bit_c     : natural := 0; -- r/w: timer enable
  constant tctrl_arst_bit_c   : natural := 1; -- r/w: auto reset on match
  constant tctrl_irq_en_bit_c : natural := 2; -- r/w: interrupt enable
  constant tctrl_prsc0_bit_c  : natural := 3; -- r/w: prescaler select bit 0
  constant tctrl_prsc1_bit_c  : natural := 4; -- r/w: prescaler select bit 1
  constant tctrl_prsc2_bit_c  : natural := 5; -- r/w: prescaler select bit 2

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal addr   : std_ulogic_vector(15 downto 0); -- access address
  signal wr_en  : std_ulogic; -- word write enable

  -- timer regs --
  signal tcnt  : std_ulogic_vector(15 downto 0);
  signal thres : std_ulogic_vector(15 downto 0);
  signal tctrl : std_ulogic_vector(05 downto 0);

  -- prescaler clock generator --
  signal prsc_tick, prsc_sel, prsc_sel_ff : std_ulogic;

  -- timer control --
  signal match : std_ulogic; -- thres = tcnt

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = timer_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= timer_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wr_en  <= acc_en and wren_i(1) and wren_i(0);


  -- Write access (and timer update) ------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- tick generator --
      prsc_sel_ff <= prsc_sel;
      -- timer reg --
      if (wr_en = '1') and (addr = timer_tcnt_addr_c) then
        tcnt <= data_i;
      elsif (tctrl(tctrl_en_bit_c) = '1') then
        if (match = '1') and (tctrl(tctrl_arst_bit_c) = '1') then -- match?
          tcnt <= x"0000";
        elsif (match = '0') and (prsc_tick = '1') then -- count++ if no match
          tcnt <= std_ulogic_vector(unsigned(tcnt) + 1);
        end if;
      end if;
      -- control & threshold --
      if (wr_en = '1') then
        case addr is
          when timer_thres_addr_c =>
            thres <= data_i;
          when timer_tctrl_addr_c =>
            tctrl(tctrl_en_bit_c)     <= data_i(tctrl_en_bit_c);
            tctrl(tctrl_arst_bit_c)   <= data_i(tctrl_arst_bit_c);
            tctrl(tctrl_irq_en_bit_c) <= data_i(tctrl_irq_en_bit_c);
            tctrl(tctrl_prsc0_bit_c)  <= data_i(tctrl_prsc0_bit_c);
            tctrl(tctrl_prsc1_bit_c)  <= data_i(tctrl_prsc1_bit_c);
            tctrl(tctrl_prsc2_bit_c)  <= data_i(tctrl_prsc2_bit_c);
          when others => NULL;
        end case;
      end if;
    end if;
  end process wr_access;

  -- timer clock select / edge detection --
  prsc_sel  <= clkgen_i(to_integer(unsigned(tctrl(tctrl_prsc2_bit_c downto tctrl_prsc0_bit_c))));
  prsc_tick <= prsc_sel_ff and (not prsc_sel); -- edge detector

  -- enable external clock generator --
  clkgen_en_o <= tctrl(tctrl_en_bit_c);

  -- match --
  match <= '1' when (tcnt = thres) else '0';

  -- interrupt line --
  irq_o <= match and tctrl(tctrl_en_bit_c) and tctrl(tctrl_irq_en_bit_c);


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0');
      if (rden_i = '1') and (acc_en = '1') then
        case addr is
          when timer_tctrl_addr_c =>
            data_o(tctrl_en_bit_c)     <= tctrl(tctrl_en_bit_c);
            data_o(tctrl_arst_bit_c)   <= tctrl(tctrl_arst_bit_c);
            data_o(tctrl_irq_en_bit_c) <= tctrl(tctrl_irq_en_bit_c);
            data_o(tctrl_prsc0_bit_c)  <= tctrl(tctrl_prsc0_bit_c);
            data_o(tctrl_prsc1_bit_c)  <= tctrl(tctrl_prsc1_bit_c);
            data_o(tctrl_prsc2_bit_c)  <= tctrl(tctrl_prsc2_bit_c);
          when timer_tcnt_addr_c =>
            data_o <= tcnt;
          when others =>
        --when timer_thres_addr_c =>
            data_o <= thres;
        end case;
      end if;
    end if;
  end process rd_access;


end neo430_timer_rtl;
