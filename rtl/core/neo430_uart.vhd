-- #################################################################################################
-- #  << NEO430 - Universal Asynchronous Receiver and Transmitter >>                               #
-- # ********************************************************************************************* #
-- # Fixed frame config: 8-bit, no parity bit, 1 stop bit, variable BAUD rate.                     #
-- # Interrupt: UART_RX_available [OR] UART_TX_done                                                #
-- # ********************************************************************************************* #
-- # BSD 3-Clause License                                                                          #
-- #                                                                                               #
-- # Copyright (c) 2020, Stephan Nolting. All rights reserved.                                     #
-- #                                                                                               #
-- # Redistribution and use in source and binary forms, with or without modification, are          #
-- # permitted provided that the following conditions are met:                                     #
-- #                                                                                               #
-- # 1. Redistributions of source code must retain the above copyright notice, this list of        #
-- #    conditions and the following disclaimer.                                                   #
-- #                                                                                               #
-- # 2. Redistributions in binary form must reproduce the above copyright notice, this list of     #
-- #    conditions and the following disclaimer in the documentation and/or other materials        #
-- #    provided with the distribution.                                                            #
-- #                                                                                               #
-- # 3. Neither the name of the copyright holder nor the names of its contributors may be used to  #
-- #    endorse or promote products derived from this software without specific prior written      #
-- #    permission.                                                                                #
-- #                                                                                               #
-- # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   #
-- # OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               #
-- # MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    #
-- # COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     #
-- # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE #
-- # GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    #
-- # AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     #
-- # NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  #
-- # OF THE POSSIBILITY OF SUCH DAMAGE.                                                            #
-- # ********************************************************************************************* #
-- # The NEO430 Processor - https://github.com/stnolting/neo430                                    #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;

entity neo430_uart is
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
    -- com lines --
    uart_txd_o  : out std_ulogic;
    uart_rxd_i  : in  std_ulogic;
    -- interrupts --
    uart_irq_o  : out std_ulogic  -- uart rx/tx interrupt
  );
end neo430_uart;

architecture neo430_uart_rtl of neo430_uart is

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size_f(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size_f(uart_size_c); -- low address boundary bit

  -- accessible regs --
  signal ctrl : std_ulogic_vector(15 downto 0);

  -- control reg bits --
  constant ctrl_uart_baud0_c   : natural :=  0; -- r/w: UART baud config bit 0
  constant ctrl_uart_baud1_c   : natural :=  1; -- r/w: UART baud config bit 1
  constant ctrl_uart_baud2_c   : natural :=  2; -- r/w: UART baud config bit 2
  constant ctrl_uart_baud3_c   : natural :=  3; -- r/w: UART baud config bit 3
  constant ctrl_uart_baud4_c   : natural :=  4; -- r/w: UART baud config bit 4
  constant ctrl_uart_baud5_c   : natural :=  5; -- r/w: UART baud config bit 5
  constant ctrl_uart_baud6_c   : natural :=  6; -- r/w: UART baud config bit 6
  constant ctrl_uart_baud7_c   : natural :=  7; -- r/w: UART baud config bit 7
  constant ctrl_uart_prsc0_c   : natural :=  8; -- r/w: UART baud prsc bit 0
  constant ctrl_uart_prsc1_c   : natural :=  9; -- r/w: UART baud prsc bit 1
  constant ctrl_uart_prsc2_c   : natural := 10; -- r/w: UART baud prsc bit 2
  constant ctrl_uart_rxovr_c   : natural := 11; -- r/-: UART RX overrun
  constant ctrl_uart_en_c      : natural := 12; -- r/w: UART enable
  constant ctrl_uart_rx_irq_c  : natural := 13; -- r/w: UART rx done interrupt enable
  constant ctrl_uart_tx_irq_c  : natural := 14; -- r/w: UART tx done interrupt enable
  constant ctrl_uart_tx_busy_c : natural := 15; -- r/-: UART transmitter is busy

  -- data register flags --
  constant data_rx_avail_c : natural := 15; -- r/-: Rx data available/valid

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal addr   : std_ulogic_vector(15 downto 0); -- access address
  signal wr_en  : std_ulogic; -- word write enable
  signal rd_en  : std_ulogic; -- read enable

  -- clock generator --
  signal uart_clk : std_ulogic;

  -- uart tx unit --
  signal uart_tx_busy     : std_ulogic;
  signal uart_tx_done     : std_ulogic;
  signal uart_tx_bitcnt   : std_ulogic_vector(3 downto 0);
  signal uart_tx_sreg     : std_ulogic_vector(9 downto 0);
  signal uart_tx_baud_cnt : std_ulogic_vector(7 downto 0);

  -- uart rx unit --
  signal uart_rx_sync     : std_ulogic_vector(4 downto 0);
  signal uart_rx_avail    : std_ulogic_vector(1 downto 0);
  signal uart_rx_busy     : std_ulogic;
  signal uart_rx_busy_ff  : std_ulogic;
  signal uart_rx_bitcnt   : std_ulogic_vector(3 downto 0);
  signal uart_rx_sreg     : std_ulogic_vector(8 downto 0);
  signal uart_rx_reg      : std_ulogic_vector(7 downto 0);
  signal uart_rx_baud_cnt : std_ulogic_vector(7 downto 0);

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = uart_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= uart_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wr_en  <= acc_en and wren_i;
  rd_en  <= acc_en and rden_i;


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (wr_en = '1') then
        if (addr = uart_ctrl_addr_c) then
          ctrl <= data_i;
        end if;
      end if;
    end if;
  end process wr_access;


  -- Clock Selection ----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  -- clock enable --
  clkgen_en_o <= ctrl(ctrl_uart_en_c);

  -- uart clock select --
  uart_clk <= clkgen_i(to_integer(unsigned(ctrl(ctrl_uart_prsc2_c downto ctrl_uart_prsc0_c))));


  -- UART transmitter ---------------------------------------------------------
  -- -----------------------------------------------------------------------------
  uart_tx_unit: process(clk_i)
  begin
    if rising_edge(clk_i) then
      uart_tx_done <= '0';
      if (uart_tx_busy = '0') or (ctrl(ctrl_uart_en_c) = '0') then -- idle or disabled
        uart_tx_busy     <= '0';
        uart_tx_baud_cnt <= ctrl(ctrl_uart_baud7_c downto ctrl_uart_baud0_c);
        uart_tx_bitcnt   <= "1010"; -- 10 bit
        if (wr_en = '1') and (ctrl(ctrl_uart_en_c) = '1') and (addr = uart_rtx_addr_c) then
          uart_tx_sreg <= '1' & data_i(7 downto 0) & '0'; -- stopbit & data & startbit
          uart_tx_busy <= '1';
        end if;
      elsif (uart_clk = '1') then
        if (uart_tx_baud_cnt = x"00") then
          uart_tx_baud_cnt <= ctrl(ctrl_uart_baud7_c downto ctrl_uart_baud0_c);
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
      -- transmitter output --
      uart_txd_o <= uart_tx_sreg(0);
    end if;
  end process uart_tx_unit;


  -- UART receiver ------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  uart_rx_unit: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- synchronizer --
      uart_rx_sync <= uart_rxd_i & uart_rx_sync(4 downto 1);
      -- arbiter --
      if (uart_rx_busy = '0') or (ctrl(ctrl_uart_en_c) = '0') then -- idle or disabled
        uart_rx_busy     <= '0';
        uart_rx_baud_cnt <= '0' & ctrl(ctrl_uart_baud7_c downto ctrl_uart_baud1_c); -- half baud rate to sample in middle of bit
        uart_rx_bitcnt   <= "1001"; -- 9 bit (startbit + 8 data bits, ignore stop bit/s)
        if (ctrl(ctrl_uart_en_c) = '0') then
          uart_rx_reg <= (others => '0'); -- to ensure defined state when reading
        elsif (uart_rx_sync(2 downto 0) = "001") then -- start bit? (falling edge)
          uart_rx_busy <= '1';
        end if;
      elsif (uart_clk = '1') then
        if (uart_rx_baud_cnt = x"00") then
          uart_rx_baud_cnt <= ctrl(ctrl_uart_baud7_c downto ctrl_uart_baud0_c);
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

      -- RX available flag --
      uart_rx_busy_ff <= uart_rx_busy;
      if (ctrl(ctrl_uart_en_c) = '0') or (((uart_rx_avail(0) = '1') or (uart_rx_avail(1) = '1')) and (rd_en = '1') and (addr = uart_rtx_addr_c)) then
        uart_rx_avail <= "00";
      elsif (uart_rx_busy_ff = '1') and (uart_rx_busy = '0') then
        uart_rx_avail <= uart_rx_avail(0) & '1';
      end if;
    end if;
  end process uart_rx_unit;


  -- Interrupt ----------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  -- UART Rx data available [OR] UART Tx complete
  uart_irq_o <= (uart_rx_busy_ff and (not uart_rx_busy) and ctrl(ctrl_uart_rx_irq_c)) or (uart_tx_done and ctrl(ctrl_uart_tx_irq_c));


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0');
      if (rd_en = '1') then
        if (addr = uart_ctrl_addr_c) then
          data_o(ctrl_uart_baud0_c)   <= ctrl(ctrl_uart_baud0_c);
          data_o(ctrl_uart_baud1_c)   <= ctrl(ctrl_uart_baud1_c);
          data_o(ctrl_uart_baud2_c)   <= ctrl(ctrl_uart_baud2_c);
          data_o(ctrl_uart_baud3_c)   <= ctrl(ctrl_uart_baud3_c);
          data_o(ctrl_uart_baud4_c)   <= ctrl(ctrl_uart_baud4_c);
          data_o(ctrl_uart_baud5_c)   <= ctrl(ctrl_uart_baud5_c);
          data_o(ctrl_uart_baud6_c)   <= ctrl(ctrl_uart_baud6_c);
          data_o(ctrl_uart_baud7_c)   <= ctrl(ctrl_uart_baud7_c);
          data_o(ctrl_uart_prsc0_c)   <= ctrl(ctrl_uart_prsc0_c);
          data_o(ctrl_uart_prsc1_c)   <= ctrl(ctrl_uart_prsc1_c);
          data_o(ctrl_uart_prsc2_c)   <= ctrl(ctrl_uart_prsc2_c);
          data_o(ctrl_uart_en_c)      <= ctrl(ctrl_uart_en_c);
          data_o(ctrl_uart_rx_irq_c)  <= ctrl(ctrl_uart_rx_irq_c);
          data_o(ctrl_uart_tx_irq_c)  <= ctrl(ctrl_uart_tx_irq_c);
          data_o(ctrl_uart_rxovr_c)   <= uart_rx_avail(0) and uart_rx_avail(1);
          data_o(ctrl_uart_tx_busy_c) <= uart_tx_busy;
        else -- uart_rtx_addr_c
          data_o(data_rx_avail_c) <= uart_rx_avail(0);
          data_o(07 downto 0)     <= uart_rx_reg;
        end if;
      end if;
    end if;
  end process rd_access;


end neo430_uart_rtl;
