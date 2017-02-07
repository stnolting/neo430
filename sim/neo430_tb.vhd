-- #################################################################################################
-- #  << NEO430 - Simple testbench >>                                                              #
-- # ********************************************************************************************* #
-- #  This simple testbench instantiates the top entity of the NEO430 processors, generates clock  #
-- #  and reset signals and outputs data send via the processor's UART to the simulator console.   #
-- # ********************************************************************************************* #
-- # This file is part of the NEO430 Processor project: http://opencores.org/project,neo430        #
-- # Copyright 2015-2016, Stephan Nolting: stnolting@gmail.com                                     #
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
-- #  Stephan Nolting, Hanover, Germany                                                20.12.2016  #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

library work;
use work.neo430_package.all;
use std.textio.all;

entity neo430_tb is
end neo430_tb;

architecture neo430_tb_rtl of neo430_tb is

  -- User Configuration ---------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  constant t_clock_c   : time := 5 ns; -- main clock period
  constant f_clock_c   : real := 100000000.0; -- main clock in Hz
  constant baud_rate_c : real := 19200.0; -- standard UART baudrate
  -- -------------------------------------------------------------------------------------------

  -- internal configuration --
  constant baud_val_c : real := f_clock_c / baud_rate_c;

  -- reduced ASCII table --
  type ascii_t is array (0 to 95-1) of character;
  constant ascii_lut : ascii_t := (' ', '!', '"', '#', '$', '%', '&', ''', '(', ')', '*', '+', ',', '-',
  '.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', '@', 'A',
  'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U',
  'V', 'W', 'X', 'Y', 'Z', '[', '\', ']', '^', '_', '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
  'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~');

  -- The core of the problem --
  component neo430_top
  port (
    -- global control --
    clk_i      : in  std_ulogic; -- global clock, rising edge
    rst_i      : in  std_ulogic; -- global reset, sync, LOW-active
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
  end component;

  -- generators --
  signal clk_gen, rst_gen : std_ulogic := '0';

  -- local signals --
  signal uart_txd : std_ulogic;
  signal spi_data : std_ulogic;

  -- simulation uart receiver --
  signal uart_rx_sync     : std_ulogic_vector(04 downto 0) := "11111";
  signal uart_rx_busy     : std_ulogic := '0';
  signal uart_rx_sreg     : std_ulogic_vector(08 downto 0) := "000000000";
  signal uart_rx_baud_cnt : real;
  signal uart_rx_bitcnt   : natural;

begin

  -- Clock/Reset Generator ----------------------------------------------------
  -- -----------------------------------------------------------------------------
  clk_gen <= not clk_gen after t_clock_c;
  rst_gen <= '0', '1' after 60*t_clock_c;


  -- CPU Core -----------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_top_inst: neo430_top
  port map (
    -- global control --
    clk_i      => clk_gen, -- global clock, rising edge
    rst_i      => rst_gen, -- global reset, sync
    -- parallel io --
    pio_o      => open,    -- parallel output
    pio_i      => x"0000", -- parallel input
    -- serial com --
    uart_txd_o => uart_txd, -- UART send data
    uart_rxd_i => '0',      -- UART receive data
    spi_sclk_o => open,     -- serial clock line
    spi_mosi_o => spi_data, -- serial data line out
    spi_miso_i => spi_data, -- serial data line in
    spi_cs_o   => open      -- SPI CS 0..5
  );


  -- Dummy UART Receiver ------------------------------------------------------
  -- -----------------------------------------------------------------------------
  uart_rx_unit: process(clk_gen)
    variable i, j     : natural;
    file uart_rx_file : text is out "uart_rx_dump.txt";
    variable line_tmp : line;
  begin
    if rising_edge(clk_gen) then
      -- synchronizer --
      uart_rx_sync <= uart_txd & uart_rx_sync(4 downto 1);
      -- arbiter --
      if (uart_rx_busy = '0') then -- idle
        uart_rx_busy     <= '0';
        uart_rx_baud_cnt <= round(0.5 * baud_val_c);
        uart_rx_bitcnt   <= 9;
        if (uart_rx_sync(2 downto 0) = "001") then -- start bit? (falling edge)
          uart_rx_busy <= '1';
        end if;
      else
        if (uart_rx_baud_cnt = 0.0) then
          uart_rx_baud_cnt <= round(baud_val_c);
          if (uart_rx_bitcnt = 0) then
            uart_rx_busy <= '0'; -- done
            i := to_integer(unsigned(uart_rx_sreg(8 downto 1)));
            j := i - 32;
            if (j < 0) or (j > 95) then
              j := 0; -- undefined = SPACE
            end if;

            if (i < 32) or (j > 32+95) then
              report "UART: Received data: (" & integer'image(i) & ")"; -- print code
            else
              report "UART: Received data: " & ascii_lut(j); -- print ASCII
            end if;

            if (i = 10) then -- Linux line break
              writeline(uart_rx_file, line_tmp);
            elsif (i /= 13) then -- Remove additional carriage return
              write(line_tmp, ascii_lut(j));
            end if;
          else
            uart_rx_sreg   <= uart_rx_sync(0) & uart_rx_sreg(8 downto 1);
            uart_rx_bitcnt <= uart_rx_bitcnt - 1;
          end if;
        else
          uart_rx_baud_cnt <= uart_rx_baud_cnt - 1.0;
        end if;
      end if;
    end if;
  end process uart_rx_unit;


end neo430_tb_rtl;
