-- #################################################################################################
-- #  << NEO430 - Processor Top Entity using resolved signal types (std_logic) only >>             #
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
-- # Stephan Nolting, Hannover, Germany                                                 23.01.2018 #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;

entity neo430_top_std_logic is
  generic (
    -- general configuration --
    CLOCK_SPEED : natural := 100000000; -- main clock in Hz
    IMEM_SIZE   : natural := 4*1024; -- internal IMEM size in bytes, max 32kB (default=4kB)
    DMEM_SIZE   : natural := 2*1024; -- internal DMEM size in bytes, max 28kB (default=2kB)
    -- additional configuration --
    USER_CODE   : std_logic_vector(15 downto 0) := x"0000"; -- custom user code
    -- module configuration --
    DADD_USE    : boolean := true; -- implement DADD instruction? (default=true)
    MULDIV_USE  : boolean := true; -- implement multiplier/divider unit? (default=true)
    WB32_USE    : boolean := true; -- implement WB32 unit? (default=true)
    WDT_USE     : boolean := true; -- implement WDT? (default=true)
    GPIO_USE    : boolean := true; -- implement GPIO unit? (default=true)
    TIMER_USE   : boolean := true; -- implement timer? (default=true)
    USART_USE   : boolean := true; -- implement USART? (default=true)
    CRC_USE     : boolean := true; -- implement CRC unit? (default=true)
    CFU_USE     : boolean := false; -- implement custom functions unit? (default=false)
    PWM_USE     : boolean := true; -- implement PWM controller?
    -- boot configuration --
    BOOTLD_USE  : boolean := true; -- implement and use bootloader? (default=true)
    IMEM_AS_ROM : boolean := false -- implement IMEM as read-only memory? (default=false)
  );
  port (
    -- global control --
    clk_i       : in  std_logic; -- global clock, rising edge
    rst_i       : in  std_logic; -- global reset, async, low-active
    -- gpio --
    gpio_o      : out std_logic_vector(15 downto 0); -- parallel output
    gpio_i      : in  std_logic_vector(15 downto 0); -- parallel input
    -- pwm channels --
    pwm_o       : out std_logic_vector(02 downto 0); -- pwm channels
    -- serial com --
    uart_txd_o  : out std_logic; -- UART send data
    uart_rxd_i  : in  std_logic; -- UART receive data
    spi_sclk_o  : out std_logic; -- serial clock line
    spi_mosi_o  : out std_logic; -- serial data line out
    spi_miso_i  : in  std_logic; -- serial data line in
    spi_cs_o    : out std_logic_vector(05 downto 0); -- SPI CS 0..5
    -- 32-bit wishbone interface --
    wb_adr_o    : out std_logic_vector(31 downto 0); -- address
    wb_dat_i    : in  std_logic_vector(31 downto 0); -- read data
    wb_dat_o    : out std_logic_vector(31 downto 0); -- write data
    wb_we_o     : out std_logic; -- read/write
    wb_sel_o    : out std_logic_vector(03 downto 0); -- byte enable
    wb_stb_o    : out std_logic; -- strobe
    wb_cyc_o    : out std_logic; -- valid cycle
    wb_ack_i    : in  std_logic; -- transfer acknowledge
    -- interrupts --
    irq_i       : in  std_logic; -- external interrupt request line
    irq_ack_o   : out std_logic  -- external interrupt request acknowledge
  );
end neo430_top_std_logic;

architecture neo430_top_std_logic_rtl of neo430_top_std_logic is

  -- other signals for conversion --
  constant usrcode_c    : std_ulogic_vector(15 downto 0) := std_ulogic_vector(USER_CODE);
  signal clk_i_int      : std_ulogic;
  signal rst_i_int      : std_ulogic;
  signal gpio_o_int     : std_ulogic_vector(15 downto 0);
  signal gpio_i_int     : std_ulogic_vector(15 downto 0);
  signal pwm_o_int      : std_ulogic_vector(02 downto 0);
  signal uart_txd_o_int : std_ulogic;
  signal uart_rxd_i_int : std_ulogic;
  signal spi_sclk_o_int : std_ulogic;
  signal spi_mosi_o_int : std_ulogic;
  signal spi_miso_i_int : std_ulogic;
  signal spi_cs_o_int   : std_ulogic_vector(05 downto 0);
  signal irq_i_int      : std_ulogic;
  signal irq_ack_o_int  : std_ulogic;
  signal wb_adr_o_int   : std_ulogic_vector(31 downto 0);
  signal wb_dat_i_int   : std_ulogic_vector(31 downto 0);
  signal wb_dat_o_int   : std_ulogic_vector(31 downto 0);
  signal wb_we_o_int    : std_ulogic;
  signal wb_sel_o_int   : std_ulogic_vector(03 downto 0);
  signal wb_stb_o_int   : std_ulogic;
  signal wb_cyc_o_int   : std_ulogic;
  signal wb_ack_i_int   : std_ulogic;

begin

  -- CPU ----------------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_top_inst: neo430_top
  generic map (
    -- general configuration --
    CLOCK_SPEED => CLOCK_SPEED,       -- main clock in Hz
    IMEM_SIZE   => IMEM_SIZE,         -- internal IMEM size in bytes, max 32kB (default=4kB)
    DMEM_SIZE   => DMEM_SIZE,         -- internal DMEM size in bytes, max 28kB (default=2kB)
    -- additional configuration --
    USER_CODE   => usrcode_c,         -- custom user code
    -- module configuration --
    DADD_USE    => DADD_USE,          -- implement DADD instruction? (default=true)
    MULDIV_USE  => MULDIV_USE,        -- implement multiplier/divider unit? (default=true)
    WB32_USE    => WB32_USE,          -- implement WB32 unit? (default=true)
    WDT_USE     => WDT_USE,           -- implement WDT? (default=true)
    GPIO_USE    => GPIO_USE,          -- implement GPIO unit? (default=true)
    TIMER_USE   => TIMER_USE,         -- implement timer? (default=true)
    USART_USE   => USART_USE,         -- implement USART? (default=true)
    CRC_USE     => CRC_USE,           -- implement CRC unit? (default=true)
    CFU_USE     => CFU_USE,           -- implement CF unit? (default=false)
    PWM_USE     => PWM_USE,           -- implement PWM controller? (default=true)
    -- boot configuration --
    BOOTLD_USE  => BOOTLD_USE,        -- implement and use bootloader? (default=true)
    IMEM_AS_ROM => IMEM_AS_ROM        -- implement IMEM as read-only memory? (default=false)
  )
  port map (
    -- global control --
    clk_i      => clk_i_int,          -- global clock, rising edge
    rst_i      => rst_i_int,          -- global reset, async, low-active
    -- parallel io --
    gpio_o     => gpio_o_int,         -- parallel output
    gpio_i     => gpio_i_int,         -- parallel input
    -- pwm channels --
    pwm_o      => pwm_o_int,          -- pwm channels
    -- serial com --
    uart_txd_o => uart_txd_o_int,     -- UART send data
    uart_rxd_i => uart_rxd_i_int,     -- UART receive data
    spi_sclk_o => spi_sclk_o_int,     -- serial clock line
    spi_mosi_o => spi_mosi_o_int,     -- serial data line out
    spi_miso_i => spi_miso_i_int,     -- serial data line in
    spi_cs_o   => spi_cs_o_int,       -- SPI CS 0..5
    -- 32-bit wishbone interface --
    wb_adr_o   => wb_adr_o_int,       -- address
    wb_dat_i   => wb_dat_i_int,       -- read data
    wb_dat_o   => wb_dat_o_int,       -- write data
    wb_we_o    => wb_we_o_int,        -- read/write
    wb_sel_o   => wb_sel_o_int,       -- byte enable
    wb_stb_o   => wb_stb_o_int,       -- strobe
    wb_cyc_o   => wb_cyc_o_int,       -- valid cycle
    wb_ack_i   => wb_ack_i_int,       -- transfer acknowledge
    -- interrupts --
    irq_i      => irq_i_int,          -- external interrupt request line
    irq_ack_o  => irq_ack_o_int       -- external interrupt request acknowledge
  );


  -- Type Conversion ----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  clk_i_int      <= std_ulogic(clk_i);
  rst_i_int      <= std_ulogic(rst_i);
  gpio_i_int     <= std_ulogic_vector(gpio_i);
  uart_rxd_i_int <= std_ulogic(uart_rxd_i);
  spi_miso_i_int <= std_ulogic(spi_miso_i);
  wb_dat_i_int   <= std_ulogic_vector(wb_dat_i);
  wb_ack_i_int   <= std_ulogic(wb_ack_i);
  irq_i_int      <= std_logic(irq_i);

  gpio_o         <= std_logic_vector(gpio_o_int);
  pwm_o          <= std_logic_vector(pwm_o_int);
  uart_txd_o     <= std_logic(uart_txd_o_int);
  spi_sclk_o     <= std_logic(spi_sclk_o_int);
  spi_mosi_o     <= std_logic(spi_mosi_o_int);
  spi_cs_o       <= std_logic_vector(spi_cs_o_int);
  wb_adr_o       <= std_logic_vector(wb_adr_o_int);
  wb_dat_o       <= std_logic_vector(wb_dat_o_int);
  wb_we_o        <= std_logic(wb_we_o_int);
  wb_sel_o       <= std_logic_vector(wb_sel_o_int);
  wb_stb_o       <= std_logic(wb_stb_o_int);
  wb_cyc_o       <= std_logic(wb_cyc_o_int);
  irq_ack_o      <= std_logic(irq_ack_o_int);


end neo430_top_std_logic_rtl;
