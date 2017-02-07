-- #################################################################################################
-- #  << NEO430 - System Top Entity >>                                                             #
-- # ********************************************************************************************* #
-- #  This is the top entity of the complete processor system. Hence, only the actual IO ports     #
-- #  are propagated to the outer world (not the Wishbone bus, since this is an internal thing).   #
-- #  Additional user IP blocks should be added to this entity.                                    #
-- #                                                                                               #
-- #  Make sure to use THIS FILE as top entity of your design.                                     #
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

entity neo430_top is
  port (
    -- global control --
    clk_i      : in  std_ulogic; -- global clock, rising edge
    rst_i      : in  std_ulogic; -- global reset, async, LOW-active
    -- parallel io --
    pio_o      : out std_ulogic_vector(15 downto 0); -- parallel output
    pio_i      : in  std_ulogic_vector(15 downto 0); -- parallel input
    -- serial com --
    uart_txd_o : out std_ulogic; -- UART send data
    uart_rxd_i : in  std_ulogic; -- UART receive data
    spi_sclk_o : out std_ulogic; -- serial clock line
    spi_mosi_o : out std_ulogic; -- serial data line out
    spi_miso_i : in  std_ulogic; -- serial data line in
    spi_cs_o   : out std_ulogic_vector(05 downto 0) -- SPI CS 0..5
  );
end neo430_top;

architecture neo430_top_rtl of neo430_top is

  -- the core of the problem --
  component neo430_core
  port (
    -- global control --
    clk_i      : in  std_ulogic; -- global clock, rising edge
    rst_i      : in  std_ulogic; -- global reset, sync
    -- parallel io --
    pio_o      : out std_ulogic_vector(15 downto 0); -- parallel output
    pio_i      : in  std_ulogic_vector(15 downto 0); -- parallel input
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
    irq_i      : in  std_ulogic  -- external interrupt request line
  );
  end component;

  -- internal wishbone bus --
  type wb_bus_t is record
    adr : std_ulogic_vector(31 downto 0); -- address
    di  : std_ulogic_vector(31 downto 0); -- slave input data
    do  : std_ulogic_vector(31 downto 0); -- slave output data
    we  : std_ulogic; -- write enable
    sel : std_ulogic_vector(03 downto 0); -- byte enable
    stb : std_ulogic; -- strobe
    cyc : std_ulogic; -- valid cycle
    ack : std_ulogic; -- transfer acknowledge
  end record;
  signal wishbone : wb_bus_t;

  -- external IRQ --
  signal irq : std_ulogic;

begin

  -- NEO430 Processor Core ----------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_inst: neo430_core
  port map (
    -- global control --
    clk_i      => clk_i,              -- global clock, rising edge
    rst_i      => rst_i,              -- global reset, async, LOW-active
    -- parallel io --
    pio_o      => pio_o,              -- parallel output
    pio_i      => pio_i,              -- parallel input
    -- serial com --
    uart_txd_o => uart_txd_o,         -- UART send data
    uart_rxd_i => uart_rxd_i,         -- UART receive data
    spi_sclk_o => spi_sclk_o,         -- serial clock line
    spi_mosi_o => spi_mosi_o,         -- serial data line out
    spi_miso_i => spi_miso_i,         -- serial data line in
    spi_cs_o   => spi_cs_o,           -- SPI CS 0..5
    -- 32-bit wishbone interface --
    wb_adr_o   => wishbone.adr,       -- address
    wb_dat_i   => wishbone.do,        -- read data
    wb_dat_o   => wishbone.di,        -- write data
    wb_we_o    => wishbone.we,        -- read/write
    wb_sel_o   => wishbone.sel,       -- byte enable
    wb_stb_o   => wishbone.stb,       -- strobe
    wb_cyc_o   => wishbone.cyc,       -- valid cycle
    wb_ack_i   => wishbone.ack,       -- transfer acknowledge
    -- external interrupt --
    irq_i      => irq                 -- external interrupt request line
  );


  -- Wishbone Component -------------------------------------------------------
  -- -----------------------------------------------------------------------------

  -- Here you can add your custom Wishbone IP components

--wishbone.adr
  wishbone.do <= (others => '0');
--wishbone.di
--wishbone.we
--wishbone.sel
--wishbone.stb
--wishbone.cyc
  wishbone.ack <= '0';

  irq <= '0'; -- high-active, level-triggered!!!


end neo430_top_rtl;
