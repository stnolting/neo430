-- #################################################################################################
-- #  << NEO430 - PWM Controller >>                                                                #
-- # ********************************************************************************************* #
-- # Simple 3-channel PWM controller with 8 bit resolution. The PWM update frequency can either be #
-- # set to fast PWM mode (high PWM frequency) or to slow PWM mode (low PWM frequency).            #
-- # Note that all registers are read-only!                                                        #
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
-- # Stephan Nolting, Hannover, Germany                                                 01.02.2018 #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;

entity neo430_pwm is
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
    -- pwm output channels --
    pwm_o       : out std_ulogic_vector(02 downto 0)
  );
end neo430_pwm;

architecture neo430_pwm_rtl of neo430_pwm is

  -- ADVANCED user configuration ----------------------------------------------------------
  constant pwm_resolution_c : natural := 8; -- pwm resolution in bits (max 16, default=8)
  -- --------------------------------------------------------------------------------------

  -- internal configuration --
  constant num_pwm_channels_c : natural := 3; -- number of PWM channels - FIXED!

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size(pwm_size_c); -- low address boundary bit

  -- Control register bits --
  constant ctrl_enable_c : natural := 0; -- -/w: PWM enable
  constant ctrl_fmode_c  : natural := 1; -- -/w: 1 = fast PWM mode, 0 = slow PWM mode

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal addr   : std_ulogic_vector(15 downto 0); -- access address
  signal wren   : std_ulogic; -- word write enable

  -- accessible regs --
  type pwm_ch_t is array (0 to num_pwm_channels_c-1) of std_ulogic_vector(pwm_resolution_c-1 downto 0);
  signal pwm_ch : pwm_ch_t;
  signal enable : std_ulogic;
  signal fmode  : std_ulogic;

  -- prescaler clock generator --
  signal prsc_tick : std_ulogic;

  -- pwm counter --
  signal pwm_cnt : std_ulogic_vector(pwm_resolution_c-1 downto 0);

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = pwm_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= pwm_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wren   <= acc_en and wren_i(1) and wren_i(0);


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (wren = '1') then
        case addr is
          when pwm_ctrl_addr_c =>
            enable <= data_i(ctrl_enable_c);
            fmode  <= data_i(ctrl_fmode_c);
          when pwm_ch0_addr_c =>
            pwm_ch(0) <= data_i(pwm_resolution_c-1 downto 0);
          when pwm_ch1_addr_c =>
            pwm_ch(1) <= data_i(pwm_resolution_c-1 downto 0);
          when pwm_ch2_addr_c =>
            pwm_ch(2) <= data_i(pwm_resolution_c-1 downto 0);
          when others =>
            NULL;
        end case;
      end if;
    end if;
  end process wr_access;


  -- PWM Core -----------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  pwm_core: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- pwm counter --
      if (enable = '0') then 
        pwm_cnt <= (others => '0');
      elsif (prsc_tick = '1') then
        pwm_cnt <= std_ulogic_vector(unsigned(pwm_cnt) + 1);
      end if;
      -- channels --
      for i in 0 to num_pwm_channels_c-1 loop
        if (unsigned(pwm_cnt) >= unsigned(pwm_ch(i))) or (enable = '0') then
          pwm_o(i) <= '0';
        else
          pwm_o(i) <= '1';
        end if;
      end loop; -- i, pwm channel
    end if;
  end process pwm_core;

  -- PWM frequency select --
  clkgen_en_o <= enable; -- enable clock generator
  prsc_tick   <= clkgen_i(clk_div2048_c) when (fmode = '0') else clkgen_i(clk_div2_c);


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  data_o <= (others => '0'); -- module is write-only!


end neo430_pwm_rtl;
