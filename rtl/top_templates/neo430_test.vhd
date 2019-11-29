-- #################################################################################################
-- #  << NEO430 - Processor Test Implementation (neo430_test.vhd) >>                               #
-- # ********************************************************************************************* #
-- #  If you do not have an own design (yet), you can use this unit as top entity to play with     #
-- #  the NEO430 processor. Take a look at the project's documentary (chapter "Let's Get It        #
-- #  Started!") to get more information.                                                          #
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
-- # Stephan Nolting, Hannover, Germany                                                 28.11.2019 #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;

entity neo430_test is
  port (
    -- global control --
    clk_i      : in  std_ulogic; -- global clock, rising edge
    rst_i      : in  std_ulogic; -- global reset, async, LOW-active
    -- parallel io --
    gpio_o     : out std_ulogic_vector(07 downto 0); -- parallel output
    -- serial com --
    uart_txd_o : out std_ulogic; -- UART send data
    uart_rxd_i : in  std_ulogic  -- UART receive data
  );
end neo430_test;

architecture neo430_test_rtl of neo430_test is

  -- local signals --
  signal gpio_out : std_ulogic_vector(15 downto 0);
  signal rst_int  : std_ulogic;
  signal twi_sda  : std_logic;
  signal twi_scl  : std_logic;

begin

  -- The Core of the Problem --------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_top_test_inst: neo430_top
  generic map (
    -- general configuration --
    CLOCK_SPEED => 100000000,         -- main clock in Hz
    IMEM_SIZE   => 4*1024,            -- internal IMEM size in bytes, max 48kB (default=4kB)
    DMEM_SIZE   => 2*1024,            -- internal DMEM size in bytes, max 12kB (default=2kB)
    -- additional configuration --
    USER_CODE   => x"CAFE",           -- custom user code
    -- module configuration --
    MULDIV_USE  => true,              -- implement multiplier/divider unit? (default=true)
    WB32_USE    => true,              -- implement WB32 unit? (default=true)
    WDT_USE     => true,              -- implement WDT? (default=true)
    GPIO_USE    => true,              -- implement GPIO unit? (default=true)
    TIMER_USE   => true,              -- implement timer? (default=true)
    UART_USE    => true,              -- implement UART? (default=true)
    CRC_USE     => true,              -- implement CRC unit? (default=true)
    CFU_USE     => false,             -- implement custom functions unit? (default=false)
    PWM_USE     => true,              -- implement PWM controller? (default=true)
    TWI_USE     => true,              -- implement two wire serial interface? (default=true)
    SPI_USE     => true,              -- implement SPI? (default=true)
    TRNG_USE    => false,             -- implement TRNG? (default=false)
    EXIRQ_USE   => true,              -- implement EXIRQ? (default=true)
    -- boot configuration --
    BOOTLD_USE  => true,              -- implement and use bootloader? (default=true)
    IMEM_AS_ROM => false              -- implement IMEM as read-only memory? (default=false)
  )
  port map (
    -- global control --
    clk_i      => clk_i,              -- global clock, rising edge
    rst_i      => rst_int,            -- global reset, async, low-active
    -- gpio --
    gpio_o     => gpio_out,           -- parallel output
    gpio_i     => x"0000",            -- parallel input
    -- pwm channels --
    pwm_o      => open,               -- pwm channels
    -- serial com --
    uart_txd_o => uart_txd_o,         -- UART send data
    uart_rxd_i => uart_rxd_i,         -- UART receive data
    spi_sclk_o => open,               -- serial clock line
    spi_mosi_o => open,               -- serial data line out
    spi_miso_i => '0',                -- serial data line in
    spi_cs_o   => open,               -- SPI CS 0..7
    twi_sda_io => twi_sda,            -- twi serial data line
    twi_scl_io => twi_scl,            -- twi serial clock line
    -- 32-bit wishbone interface --
    wb_adr_o   => open,               -- address
    wb_dat_i   => x"00000000",        -- read data
    wb_dat_o   => open,               -- write data
    wb_we_o    => open,               -- read/write
    wb_sel_o   => open,               -- byte enable
    wb_stb_o   => open,               -- strobe
    wb_cyc_o   => open,               -- valid cycle
    wb_ack_i   => '0',                -- transfer acknowledge
    -- external interrupts --
    ext_irq_i  => "00000000",         -- external interrupt request lines
    ext_ack_o  => open                -- external interrupt request acknowledges
  );

  -- constrain output signals --
  gpio_o <= gpio_out(7 downto 0);

  -- internal reset (must be low-active!) --
  rst_int <= rst_i; -- invert me?!

  -- twi --
  twi_sda <= 'H';
  twi_scl <= 'H';


end neo430_test_rtl;
