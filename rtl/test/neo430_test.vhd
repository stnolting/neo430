-- #################################################################################################
-- #  << NEO430 - Processor Test Implementation (neo430_test.vhd) >>                               #
-- # ********************************************************************************************* #
-- # If you do not have an own design (yet), you can use this unit as top entity to play with      #
-- # the NEO430 processor. Take a look at the project's documentary to get more information.       #
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
-- #  Stephan Nolting, Hannover, Germany                                               17.07.2017  #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

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

  component neo430_top
    generic (
      -- general configuration --
      CLOCK_SPEED : natural; -- main clock in Hz
      IMEM_SIZE   : natural; -- internal IMEM size in bytes, max 32kB (default=4kB)
      DMEM_SIZE   : natural; -- internal DMEM size in bytes, max 28kB (default=2kB)
      -- additional configuration --
      USER_CODE   : std_ulogic_vector(15 downto 0); -- custom user code
      -- module configuration --
      DADD_USE    : boolean; -- implement DADD instruction? (default=true)
      CFU_USE     : boolean; -- implement custom function unit? (default=false)
      WB32_USE    : boolean; -- implement WB32 unit? (default=true)
      WDT_USE     : boolean; -- implement WBT? (default=true)
      GPIO_USE    : boolean; -- implement GPIO unit? (default=true)
      TIMER_USE   : boolean; -- implement timer? (default=true)
      USART_USE   : boolean; -- implement USART? (default=true)
      -- boot configuration --
      BOOTLD_USE  : boolean; -- implement and use bootloader? (default=true)
      IMEM_AS_ROM : boolean -- implement IMEM as read-only memory? (default=false)
    );
    port (
      -- global control --
      clk_i      : in  std_ulogic; -- global clock, rising edge
      rst_i      : in  std_ulogic; -- global reset, async, LOW-active
      -- gpio --
      gpio_o     : out std_ulogic_vector(15 downto 0); -- parallel output
      gpio_i     : in  std_ulogic_vector(15 downto 0); -- parallel input
      -- serial com --
      uart_txd_o : out std_ulogic; -- UART send data
      uart_rxd_i : in  std_ulogic; -- UART receive data
      spi_sclk_o : out std_ulogic; -- serial clock line
      spi_mosi_o : out std_ulogic; -- serial data line out
      spi_miso_i : in  std_ulogic; -- serial data line in
      spi_cs_o   : out std_ulogic_vector(05 downto 0); -- SPI CS 0..5
      -- 32-bit wishbone interface --
      wb_adr_o   : out std_ulogic_vector(31 downto 0); -- address
      wb_dat_i   : in  std_ulogic_vector(31 downto 0); -- read data
      wb_dat_o   : out std_ulogic_vector(31 downto 0); -- write data
      wb_we_o    : out std_ulogic; -- read/write
      wb_sel_o   : out std_ulogic_vector(03 downto 0); -- byte enable
      wb_stb_o   : out std_ulogic; -- strobe
      wb_cyc_o   : out std_ulogic; -- valid cycle
      wb_ack_i   : in  std_ulogic; -- transfer acknowledge
      -- external interrupt --
      irq_i      : in  std_ulogic; -- external interrupt request line
      irq_ack_o  : out std_ulogic  -- external interrupt request acknowledge
    );
  end component;

  signal gpio_out : std_ulogic_vector(15 downto 0);

begin

  -- The Core of the Problem --------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_top_test_inst: neo430_top
  generic map (
    -- general configuration --
    CLOCK_SPEED => 100000000,         -- main clock in Hz
    IMEM_SIZE   => 4*1024,            -- internal IMEM size in bytes, max 32kB (default=4kB)
    DMEM_SIZE   => 2*1024,            -- internal DMEM size in bytes, max 28kB (default=2kB)
    -- additional configuration --
    USER_CODE   => x"CAFE",           -- custom user code
    -- module configuration --
    DADD_USE    => true,              -- implement DADD instruction? (default=true)
    CFU_USE     => false,             -- implement custom function unit? (default=false)
    WB32_USE    => true,              -- implement WB32 unit? (default=true)
    WDT_USE     => true,              -- implement WBT? (default=true)
    GPIO_USE    => true,              -- implement GPIO unit? (default=true)
    TIMER_USE   => true,              -- implement timer? (default=true)
    USART_USE   => true,              -- implement USART? (default=true)
    -- boot configuration --
    BOOTLD_USE  => true,              -- implement and use bootloader? (default=true)
    IMEM_AS_ROM => false              -- implement IMEM as read-only memory? (default=false)
  )
  port map (
    -- global control --
    clk_i      => clk_i,              -- global clock, rising edge
    rst_i      => rst_i,              -- global reset, sync
    -- gpio --
    gpio_o     => gpio_out,           -- parallel output
    gpio_i     => x"0000",            -- parallel input
    -- serial com --
    uart_txd_o => uart_txd_o,         -- UART send data
    uart_rxd_i => uart_rxd_i,         -- UART receive data
    spi_sclk_o => open,               -- serial clock line
    spi_mosi_o => open,               -- serial data line out
    spi_miso_i => '0',                -- serial data line in
    spi_cs_o   => open,               -- SPI CS 0..5
    -- 32-bit wishbone interface --
    wb_adr_o   => open,               -- address
    wb_dat_i   => x"00000000",        -- read data
    wb_dat_o   => open,               -- write data
    wb_we_o    => open,               -- read/write
    wb_sel_o   => open,               -- byte enable
    wb_stb_o   => open,               -- strobe
    wb_cyc_o   => open,               -- valid cycle
    wb_ack_i   => '0',                -- transfer acknowledge
    -- external interrupt --
    irq_i      => '0',                -- external interrupt request line
    irq_ack_o  => open                -- external interrupt request acknowledge
  );

  gpio_o <= gpio_out(7 downto 0);


end neo430_test_rtl;
