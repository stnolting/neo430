-- #################################################################################################
-- #  << NEO430 - PWM Controller >>                                                                #
-- # ********************************************************************************************* #
-- # Simple 4-channel PWM controller with 4 or 8 bit resolution for the duty cycle and selectable  #
-- # counter width (frequency resolution) 4 or 8 bits.                                             #
-- # Channel 3 can be used to alternatively modulate the GPIO unit's output port.                  #
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
-- # Stephan Nolting, Hannover, Germany                                                 22.11.2019 #
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
    wren_i      : in  std_ulogic; -- write enable
    addr_i      : in  std_ulogic_vector(15 downto 0); -- address
    data_i      : in  std_ulogic_vector(15 downto 0); -- data in
    data_o      : out std_ulogic_vector(15 downto 0); -- data out
    -- clock generator --
    clkgen_en_o : out std_ulogic; -- enable clock generator
    clkgen_i    : in  std_ulogic_vector(07 downto 0);
    -- GPIO output PWM --
    gpio_pwm_o  : out std_ulogic;
    -- pwm output channels --
    pwm_o       : out std_ulogic_vector(03 downto 0)
  );
end neo430_pwm;

architecture neo430_pwm_rtl of neo430_pwm is

  -- internal configuration --
  constant num_pwm_channels_c : natural := 4; -- number of PWM channels - FIXED!

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size_f(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size_f(pwm_size_c); -- low address boundary bit

  -- Control register bits --
  constant ctrl_enable_c    : natural := 0; -- -/w: PWM enable
  constant ctrl_prsc0_bit_c : natural := 1; -- -/w: prescaler select bit 0
  constant ctrl_prsc1_bit_c : natural := 2; -- -/w: prescaler select bit 1
  constant ctrl_prsc2_bit_c : natural := 3; -- -/w: prescaler select bit 2
  constant ctrl_gpio_pwm_c  : natural := 4; -- -/w: use channel 3 for GPIO controller output modulation
  constant ctrl_size_sel_c  : natural := 5; -- -/w: cnt size select (0 = 4-bit, 1 = 8-bit)

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal addr   : std_ulogic_vector(15 downto 0); -- access address
  signal wren   : std_ulogic; -- word write enable

  -- accessible regs --
  type pwm_ch_t is array (0 to num_pwm_channels_c-1) of std_ulogic_vector(7 downto 0);
  signal pwm_ch   : pwm_ch_t; -- duty cycle
  signal enable   : std_ulogic; -- enable unit
  signal gpio_pwm : std_ulogic; -- use pwm channel 3 to module GPIO unit's output port
  signal prsc     : std_ulogic_vector(2 downto 0); -- clock prescaler
  signal size_sel : std_ulogic; -- select pwm counter size

  -- constrained pwm counter --
  signal mask : std_ulogic_vector(7 downto 0);

  -- prescaler clock generator --
  signal prsc_tick : std_ulogic;

  -- pwm counter --
  signal pwm_cnt : std_ulogic_vector(7 downto 0);
  signal pwm_out : std_ulogic_vector(3 downto 0);

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = pwm_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= pwm_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wren   <= acc_en and wren_i;


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (wren = '1') then
        if (addr = pwm_ctrl_addr_c) then -- control register
          enable   <= data_i(ctrl_enable_c);
          prsc     <= data_i(ctrl_prsc2_bit_c downto ctrl_prsc0_bit_c);
          size_sel <= data_i(ctrl_size_sel_c);
          gpio_pwm <= data_i(ctrl_gpio_pwm_c);
        end if;
        if (addr = pwm_ch10_addr_c) then
          pwm_ch(0) <= data_i(07 downto 0);
          pwm_ch(1) <= data_i(15 downto 8);
        end if;
        if (addr = pwm_ch32_addr_c) then
          pwm_ch(2) <= data_i(07 downto 0);
          pwm_ch(3) <= data_i(15 downto 8);
        end if;
      end if;
    end if;
  end process wr_access;

  -- PWM frequency select --
  clkgen_en_o <= enable; -- enable clock generator
  prsc_tick   <= clkgen_i(to_integer(unsigned(prsc)));

  -- effective counter width --
  mask(3 downto 0) <= "1111";
  mask(7 downto 4) <= (others => size_sel);


  -- PWM Core -----------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  pwm_core: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- pwm counter --
      if (enable = '0') then 
        pwm_cnt <= (others => '0');
      elsif (prsc_tick = '1') then
        -- constrain counter to virtual size configured by SIZE register
        pwm_cnt <= std_ulogic_vector(unsigned(pwm_cnt) + 1);
      end if;
      -- channels --
      for i in 0 to num_pwm_channels_c-1 loop
        if (unsigned(pwm_cnt and mask) >= unsigned(pwm_ch(i))) or (enable = '0') then
          pwm_out(i) <= '0';
        else
          pwm_out(i) <= '1';
        end if;
      end loop; -- i, pwm channel
    end if;
  end process pwm_core;

  -- output --
  pwm_o(0) <= pwm_out(0);
  pwm_o(1) <= pwm_out(1);
  pwm_o(2) <= pwm_out(2);
  pwm_o(3) <= pwm_out(3) when (gpio_pwm = '0') else '0'; -- output if channel is not used for GPIO

  -- GPIO output modulation --
  gpio_pwm_o <= pwm_out(3) when (gpio_pwm = '1') else '1';


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0');
      if (acc_en = '1') and (rden_i = '1') then
        if (addr = pwm_ch10_addr_c) then -- PWM channel 0 & 1
          data_o(07 downto 0) <= pwm_ch(0);
          data_o(15 downto 8) <= pwm_ch(1);
        else -- PWM channel 2 & 3
          data_o(07 downto 0) <= pwm_ch(2);
          data_o(15 downto 8) <= pwm_ch(3);
        end if;
      end if;
    end if;
  end process rd_access;


end neo430_pwm_rtl;
