-- #################################################################################################
-- #  << NEO430 - Serial Communication Unit >>                                                     #
-- # ********************************************************************************************* #
-- #  Synchronous & asynchronous serial transceivers.                                              #
-- #  UART: 8-bit, no parity bit, 1 stop bit, variable BAUD rate.                                  #
-- #  SPI: 8-bit, MSB first, 2 clock modes, 8 clock speeds, 6 dedicated CS lines.                  #
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
-- #  Stephan Nolting, Hannover, Germany                                               28.09.2017  #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.neo430_package.all;

entity neo430_usart is
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
    -- com lines --
    uart_txd_o  : out std_ulogic;
    uart_rxd_i  : in  std_ulogic;
    spi_sclk_o  : out std_ulogic; -- SPI serial clock
    spi_mosi_o  : out std_ulogic; -- SPI master out, slave in
    spi_miso_i  : in  std_ulogic; -- SPI master in, slave out
    spi_cs_o    : out std_ulogic_vector(05 downto 0); -- SPI CS 0..5
    -- interrupts --
    usart_irq_o : out std_ulogic  -- spi transmission done / uart rx/tx interrupt
  );
end neo430_usart;

architecture neo430_usart_rtl of neo430_usart is

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size(usart_size_c); -- low address boundary bit

  -- accessible regs --
  signal ctrl      : std_ulogic_vector(15 downto 0);
  signal baud      : std_ulogic_vector(07 downto 0);
  signal baud_prsc : std_ulogic_vector(02 downto 0);

  -- control reg bits --
  constant ctrl_usart_en_c     : natural :=  0; -- r/w: USART enable
  constant ctrl_uart_rx_irq_c  : natural :=  1; -- r/w: uart rx done interrupt enable
  constant ctrl_uart_tx_irq_c  : natural :=  2; -- r/w: uart tx done interrupt enable
  constant ctrl_uart_tx_busy_c : natural :=  3; -- r:   uart transmitter is busy
  constant ctrl_spi_cpha_c     : natural :=  4; -- r/w: spi clock phase
  constant ctrl_spi_irq_en_c   : natural :=  5; -- r/w: spi transmission done interrupt enable
  constant ctrl_spi_busy_c     : natural :=  6; -- r:   spi transceiver is busy
  constant ctrl_spi_prsc0_c    : natural :=  7; -- r/w: spi prescaler select bit 0
  constant ctrl_spi_prsc1_c    : natural :=  8; -- r/w: spi prescaler select bit 1
  constant ctrl_spi_prsc2_c    : natural :=  9; -- r/w: spi prescaler select bit 2
  constant ctrl_spi_cs0_c      : natural := 10; -- r/w: spi direct CS 0
  constant ctrl_spi_cs1_c      : natural := 11; -- r/w: spi direct CS 1
  constant ctrl_spi_cs2_c      : natural := 12; -- r/w: spi direct CS 2
  constant ctrl_spi_cs3_c      : natural := 13; -- r/w: spi direct CS 3
  constant ctrl_spi_cs4_c      : natural := 14; -- r/w: spi direct CS 4
  constant ctrl_spi_cs5_c      : natural := 15; -- r/w: spi direct CS 5

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal addr   : std_ulogic_vector(15 downto 0); -- access address
  signal wr_en  : std_ulogic; -- word write enable

  -- clock generators --
  signal uart_clk     : std_ulogic;
  signal uart_prsc    : std_ulogic;
  signal uart_prsc_ff : std_ulogic;
  signal spi_clk      : std_ulogic;
  signal spi_prsc     : std_ulogic;
  signal spi_prsc_ff  : std_ulogic;

  -- uart tx unit --
  signal uart_tx_start    : std_ulogic;
  signal uart_tx_busy     : std_ulogic;
  signal uart_tx_bitcnt   : std_ulogic_vector(03 downto 0);
  signal uart_tx_sreg     : std_ulogic_vector(09 downto 0);
  signal uart_tx_baud_cnt : std_ulogic_vector(07 downto 0);
  signal uart_tx_done     : std_ulogic;

  -- uart rx unit --
  signal uart_rx_sync     : std_ulogic_vector(04 downto 0);
  signal uart_rx_avail    : std_ulogic;
  signal uart_rx_start    : std_ulogic;
  signal uart_rx_busy     : std_ulogic;
  signal uart_rx_busy_ff  : std_ulogic;
  signal uart_rx_bitcnt   : std_ulogic_vector(03 downto 0);
  signal uart_rx_sreg     : std_ulogic_vector(08 downto 0);
  signal uart_rx_reg      : std_ulogic_vector(07 downto 0);
  signal uart_rx_baud_cnt : std_ulogic_vector(07 downto 0);

  -- spi transceiver --
  signal spi_busy     : std_ulogic;
  signal spi_irq      : std_ulogic;
  signal spi_state0   : std_ulogic;
  signal spi_state1   : std_ulogic;
  signal spi_rtx_sreg : std_ulogic_vector(07 downto 0);
  signal spi_bitcnt   : std_ulogic_vector(03 downto 0);
  signal spi_miso_ff0 : std_ulogic;
  signal spi_miso_ff1 : std_ulogic;

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = usart_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= usart_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wr_en  <= acc_en and wren_i(1) and wren_i(0);


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (wr_en = '1') then
        if (addr = usart_baud_addr_c) then
          baud      <= data_i(07 downto 0);
          baud_prsc <= data_i(10 downto 8);
        end if;
        if (addr = usart_ctrl_addr_c) then
          ctrl <= data_i;
        end if;
      end if;
    end if;
  end process wr_access;


  -- Clock Selection ----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  clk_gen: process(clk_i)
  begin
    if rising_edge(clk_i) then
      uart_prsc_ff <= uart_prsc;
      spi_prsc_ff  <= spi_prsc;
    end if;
  end process clk_gen;

  -- clock enable --
  clkgen_en_o <= ctrl(ctrl_usart_en_c);

  -- spi clock select / edge detection --
  spi_prsc <= clkgen_i(to_integer(unsigned(ctrl(ctrl_spi_prsc2_c downto ctrl_spi_prsc0_c))));
  spi_clk  <= (not spi_prsc_ff) and spi_prsc; -- rising edge

  -- uart clock select / edge detection --
  uart_prsc <= clkgen_i(to_integer(unsigned(baud_prsc(2 downto 0))));
  uart_clk  <= (not uart_prsc_ff) and uart_prsc; -- rising edge


  -- UART transmitter ---------------------------------------------------------
  -- -----------------------------------------------------------------------------
  uart_tx_unit: process(clk_i)
  begin
    if rising_edge(clk_i) then
      uart_tx_done <= '0';
      if (uart_tx_busy = '0') or (ctrl(ctrl_usart_en_c) = '0') then -- idle or disabled
        uart_tx_busy     <= '0';
        uart_tx_baud_cnt <= baud(7 downto 0);
        uart_tx_bitcnt   <= "1010"; -- 10 bit
        if (ctrl(ctrl_usart_en_c) = '0') then
          uart_tx_start <= '0';
        elsif (wr_en = '1') and (addr = usart_uart_rtx_addr_c) then
          uart_tx_sreg  <= '1' & data_i(7 downto 0) & '0'; -- stopbit & data & startbit
          uart_tx_start <= '1';
        end if;
        uart_tx_busy <= uart_tx_start and uart_clk;
      elsif (uart_clk = '1') then
        uart_tx_start <= '0';
        if (uart_tx_baud_cnt = x"00") then
          uart_tx_baud_cnt <= baud(7 downto 0);
          uart_tx_bitcnt   <= std_ulogic_vector(unsigned(uart_tx_bitcnt) - 1);
          uart_tx_sreg     <= '1' & uart_tx_sreg(9 downto 1);
          if (uart_tx_bitcnt = "0000") then
            uart_tx_busy <= '0'; -- done
            uart_tx_done <= '1';
          end if;
        else
          uart_tx_baud_cnt <= std_ulogic_vector(unsigned(uart_tx_baud_cnt) - 1);
        end if;
      end if;
    end if;
  end process uart_tx_unit;

  -- transmitter output --
  uart_txd_o <= uart_tx_sreg(0);


  -- UART receiver ------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  uart_rx_unit: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- synchronizer --
      uart_rx_sync <= uart_rxd_i & uart_rx_sync(4 downto 1);
      -- arbiter --
      if (uart_rx_busy = '0') or (ctrl(ctrl_usart_en_c) = '0') then -- idle or disabled
        uart_rx_busy     <= '0';
        uart_rx_baud_cnt <= '0' & baud(7 downto 1); -- half baud rate to sample in middle of bit
        uart_rx_bitcnt   <= "1001"; -- 9 bit (startbit + 8 data bits, ignore stop bit/s)
        if (ctrl(ctrl_usart_en_c) = '0') then
          uart_rx_start <= '0';
          uart_rx_reg   <= (others => '0'); -- to ensure defined state when reading
        elsif (uart_rx_sync(2 downto 0) = "001") then -- start bit? (falling edge)
          uart_rx_start <= '1';
        end if;
        uart_rx_busy <= uart_rx_start and uart_clk;
      elsif (uart_clk = '1') then
        uart_rx_start <= '0'; -- done
        if (uart_rx_baud_cnt = x"00") then
          uart_rx_baud_cnt <= baud(7 downto 0);
          uart_rx_bitcnt   <= std_ulogic_vector(unsigned(uart_rx_bitcnt) - 1);
          uart_rx_sreg     <= uart_rx_sync(0) & uart_rx_sreg(8 downto 1);
          if (uart_rx_bitcnt = "0000") then
            uart_rx_busy <= '0'; -- done
            uart_rx_reg  <= uart_rx_sreg(8 downto 1);
          end if;
        else
          uart_rx_baud_cnt <= std_ulogic_vector(unsigned(uart_rx_baud_cnt) - 1);
        end if;
      end if;
    end if;
  end process uart_rx_unit;

  -- RX available arbiter --
  uart_rx_avail_arb: process(clk_i)
  begin
    if rising_edge(clk_i) then
      uart_rx_busy_ff <= uart_rx_busy;
      if (ctrl(ctrl_usart_en_c) = '0') or ((uart_rx_avail = '1') and (rden_i = '1') and (acc_en = '1') and (addr = usart_uart_rtx_addr_c)) then
        uart_rx_avail <= '0';
      elsif (uart_rx_busy_ff = '1') and (uart_rx_busy = '0') then
        uart_rx_avail <= '1';
      end if;
    end if;
  end process uart_rx_avail_arb;


  -- SPI transceiver ----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  spi_rtx_unit: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- input (MISO) synchronizer --
      spi_miso_ff0 <= spi_miso_i;
      spi_miso_ff1 <= spi_miso_ff0;
      -- arbiter --
      spi_irq <= '0';
      if (spi_state0 = '0') or (ctrl(ctrl_usart_en_c) = '0') then -- idle or disabled
        spi_bitcnt <= "1000"; -- 8 bit transfer size
        spi_state1 <= '0';
        spi_mosi_o <= '0';
        spi_sclk_o <= '0';
        if (ctrl(ctrl_usart_en_c) = '0') then -- disabled
          spi_busy <= '0';
        elsif (wr_en = '1') and (addr = usart_spi_rtx_addr_c) then
          spi_rtx_sreg <= data_i(7 downto 0);
          spi_busy     <= '1';
        end if;
        spi_state0 <= spi_busy and spi_clk; -- start with next new clock pulse
      else -- transmission in progress
        if (spi_state1 = '0') then -- first half of transmission
          spi_sclk_o <= ctrl(ctrl_spi_cpha_c);
          spi_mosi_o <= spi_rtx_sreg(7); -- MSB first
          if (spi_clk = '1') then
            spi_state1   <= '1';
            if (ctrl(ctrl_spi_cpha_c) = '0') then
              spi_rtx_sreg <= spi_rtx_sreg(6 downto 0) & spi_miso_ff1; -- MSB first
            end if;
            spi_bitcnt   <= std_ulogic_vector(unsigned(spi_bitcnt) - 1);
          end if;
        else -- second half of transmission
          spi_sclk_o <= not ctrl(ctrl_spi_cpha_c);
          if (spi_clk = '1') then
            spi_state1 <= '0';
            if (ctrl(ctrl_spi_cpha_c) = '1') then
              spi_rtx_sreg <= spi_rtx_sreg(6 downto 0) & spi_miso_ff1; -- MSB first
            end if;
            if (spi_bitcnt = "0000") then
              spi_state0 <= '0';
              spi_busy   <= '0';
              spi_irq    <= '1';
            end if;
          end if;
        end if;
      end if;
    end if;
  end process spi_rtx_unit;

  -- direct CS --
  spi_cs_o(0) <= not ctrl(ctrl_spi_cs0_c);
  spi_cs_o(1) <= not ctrl(ctrl_spi_cs1_c);
  spi_cs_o(2) <= not ctrl(ctrl_spi_cs2_c);
  spi_cs_o(3) <= not ctrl(ctrl_spi_cs3_c);
  spi_cs_o(4) <= not ctrl(ctrl_spi_cs4_c);
  spi_cs_o(5) <= not ctrl(ctrl_spi_cs5_c);


  -- Interrupt ----------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  -- UART Rx data available [OR] UART Tx complete [OR] SPI transmission done
  usart_irq_o <= (uart_rx_busy_ff and (not uart_rx_busy) and ctrl(ctrl_uart_rx_irq_c)) or
                 (uart_tx_done and ctrl(ctrl_uart_tx_irq_c)) or
                 (spi_irq and ctrl(ctrl_spi_irq_en_c));


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0');
      if (rden_i = '1') and (acc_en = '1') then
        case addr is
          when usart_uart_rtx_addr_c =>
            data_o(7 downto 0) <= uart_rx_reg;
            data_o(15) <= uart_rx_avail;
          when usart_baud_addr_c =>
            data_o(7 downto 0) <= baud;
            data_o(10 downto 8)<= baud_prsc;
          when usart_ctrl_addr_c =>
            data_o <= ctrl;
            data_o(ctrl_spi_busy_c) <= spi_busy;
            data_o(ctrl_uart_tx_busy_c) <= uart_tx_busy or uart_tx_start;
          when others =>
--        when usart_spi_rtx_addr_c =>
            data_o(7 downto 0) <= spi_rtx_sreg;
        end case;
      end if;
    end if;
  end process rd_access;


end neo430_usart_rtl;
