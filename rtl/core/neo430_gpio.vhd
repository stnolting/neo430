-- #################################################################################################
-- #  << NEO430 - General Purpose Parallel IO Unit >>                                              #
-- # ********************************************************************************************* #
-- # 16-bit parallel input & output unit. Any pin-change (HI->LO or LO->HI) triggers the IRQ.      #
-- # Pins used for the pin change interrupt are selected using a 16-bit mask.                      #
-- # The PWM controller can be used to module the GPIO controller's output.                        #
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
-- # Stephan Nolting, Hannover, Germany                                                 21.11.2019 #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;

entity neo430_gpio is
  port (
    -- host access --
    clk_i      : in  std_ulogic; -- global clock line
    rden_i     : in  std_ulogic; -- read enable
    wren_i     : in  std_ulogic; -- write enable
    addr_i     : in  std_ulogic_vector(15 downto 0); -- address
    data_i     : in  std_ulogic_vector(15 downto 0); -- data in
    data_o     : out std_ulogic_vector(15 downto 0); -- data out
    -- parallel io --
    gpio_o     : out std_ulogic_vector(15 downto 0);
    gpio_i     : in  std_ulogic_vector(15 downto 0);
    -- GPIO PWM --
    gpio_pwm_i : in  std_ulogic;
    -- interrupt --
    irq_o      : out std_ulogic
  );
end neo430_gpio;

architecture neo430_gpio_rtl of neo430_gpio is

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size_f(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size_f(gpio_size_c); -- low address boundary bit

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal addr   : std_ulogic_vector(15 downto 0); -- access address
  signal wren   : std_ulogic; -- word write enable
  signal rden   : std_ulogic; -- read enable

  -- accessible regs --
  signal dout, din : std_ulogic_vector(15 downto 0); -- r/w
  signal irq_mask  : std_ulogic_vector(15 downto 0); -- -/w

  -- misc --
  signal irq_raw, sync_in, in_buf : std_ulogic_vector(15 downto 0);

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = gpio_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= gpio_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wren   <= acc_en and wren_i;
  rden   <= acc_en and rden_i;


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (wren = '1') then
        if (addr = gpio_out_addr_c) then
          dout <= data_i;
        end if;
        if (addr = gpio_irqmask_addr_c) then
          irq_mask <= data_i;
        end if;
      end if;
    end if;
  end process wr_access;

  -- (PWM modulated) output --
  gpio_o <= dout when (gpio_pwm_i = '1') else (others => '0');


  -- IRQ Generator ------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  irq_generator: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- input synchronizer --
      in_buf  <= gpio_i;
      din     <= in_buf;
      sync_in <= din;
      -- IRQ --
      irq_o <= or_all_f(irq_raw);
    end if;
  end process irq_generator;

  -- any transition triggers an interrupt (if enabled for according input pin) --
  irq_raw <= (din xor sync_in) and irq_mask;


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- read access --
      data_o <= (others => '0');
      if (rden = '1') then
        if (addr = gpio_in_addr_c) then
          data_o <= din;
        else -- gpio_out_addr_c
          data_o <= dout;
        end if;
      end if;
    end if;
  end process rd_access;


end neo430_gpio_rtl;
