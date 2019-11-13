-- #################################################################################################
-- #  << NEO430 - Serial Peripheral Interface >>                                                   #
-- # ********************************************************************************************* #
-- # Frame format: 8-bit, MSB or LSB first, 2 clock modes, 8 clock speeds, 8 dedicated CS lines.   #
-- # Interrupt: SPI_transfer_done                                                                  #
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
-- # Stephan Nolting, Hannover, Germany                                                 13.11.2019 #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;

entity neo430_spi is
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
    spi_sclk_o  : out std_ulogic; -- SPI serial clock
    spi_mosi_o  : out std_ulogic; -- SPI master out, slave in
    spi_miso_i  : in  std_ulogic; -- SPI master in, slave out
    spi_cs_o    : out std_ulogic_vector(07 downto 0); -- SPI CS 0..7
    -- interrupt --
    spi_irq_o   : out std_ulogic -- transmission done interrupt
  );
end neo430_spi;

architecture neo430_spi_rtl of neo430_spi is

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size_f(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size_f(spi_size_c); -- low address boundary bit

  -- control reg bits --
  constant ctrl_spi_en_c      : natural :=  0; -- r/w: spi enable
  constant ctrl_spi_cpha_c    : natural :=  1; -- r/w: spi clock phase
  constant ctrl_spi_irq_en_c  : natural :=  2; -- r/w: spi transmission done interrupt enable
  constant ctrl_spi_prsc0_c   : natural :=  3; -- r/w: spi prescaler select bit 0
  constant ctrl_spi_prsc1_c   : natural :=  4; -- r/w: spi prescaler select bit 1
  constant ctrl_spi_prsc2_c   : natural :=  5; -- r/w: spi prescaler select bit 2
  constant ctrl_spi_cs_sel0_c : natural :=  6; -- r/w: spi CS select bit 0
  constant ctrl_spi_cs_sel1_c : natural :=  7; -- r/w: spi CS select bit 0
  constant ctrl_spi_cs_sel2_c : natural :=  8; -- r/w: spi CS select bit 0
  constant ctrl_spi_cs_set_c  : natural :=  9; -- r/w: spi CS select enable
  constant ctrl_spi_dir_c     : natural := 10; -- r/w: shift direction (0: MSB first, 1: LSB first)
  -- ...
  constant ctrl_spi_busy_c    : natural := 15; -- r/-: spi transceiver is busy

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal addr   : std_ulogic_vector(15 downto 0); -- access address
  signal wr_en  : std_ulogic; -- word write enable
  signal rd_en  : std_ulogic; -- read enable

  -- accessible regs --
  signal ctrl : std_ulogic_vector(15 downto 0);

  -- clock generator --
  signal spi_clk : std_ulogic;

  -- spi transceiver --
  signal spi_busy     : std_ulogic;
  signal spi_state0   : std_ulogic;
  signal spi_state1   : std_ulogic;
  signal spi_rtx_sreg : std_ulogic_vector(07 downto 0);
  signal spi_bitcnt   : std_ulogic_vector(03 downto 0);
  signal spi_miso_ff0 : std_ulogic;
  signal spi_miso_ff1 : std_ulogic;

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = spi_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= spi_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wr_en  <= acc_en and wren_i;
  rd_en  <= acc_en and rden_i;


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (wr_en = '1') then
        if (addr = spi_ctrl_addr_c) then
          ctrl <= data_i;
        end if;
      end if;
    end if;
  end process wr_access;


  -- Clock Selection ----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  -- clock enable --
  clkgen_en_o <= ctrl(ctrl_spi_en_c);

  -- spi clock select --
  spi_clk <= clkgen_i(to_integer(unsigned(ctrl(ctrl_spi_prsc2_c downto ctrl_spi_prsc0_c))));


  -- SPI transceiver ----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  spi_rtx_unit: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- input (MISO) synchronizer --
      spi_miso_ff0 <= spi_miso_i;
      spi_miso_ff1 <= spi_miso_ff0;
      -- arbiter --
      spi_irq_o <= '0';
      if (spi_state0 = '0') or (ctrl(ctrl_spi_en_c) = '0') then -- idle or disabled
        spi_bitcnt <= "1000"; -- 8 bit transfer size
        spi_state1 <= '0';
        spi_mosi_o <= '0';
        spi_sclk_o <= '0';
        if (ctrl(ctrl_spi_en_c) = '0') then -- disabled
          spi_busy <= '0';
        elsif (wr_en = '1') and (addr = spi_rtx_addr_c) then
          spi_rtx_sreg <= data_i(7 downto 0);
          spi_busy     <= '1';
        end if;
        spi_state0 <= spi_busy and spi_clk; -- start with next new clock pulse
      else -- transmission in progress
        if (spi_state1 = '0') then -- first half of transmission
          spi_sclk_o <= ctrl(ctrl_spi_cpha_c);
          if (ctrl(ctrl_spi_dir_c) = '0') then
            spi_mosi_o <= spi_rtx_sreg(7); -- MSB first
          else
            spi_mosi_o <= spi_rtx_sreg(0); -- LSB first
          end if;
          if (spi_clk = '1') then
            spi_state1   <= '1';
            if (ctrl(ctrl_spi_cpha_c) = '0') then
              if (ctrl(ctrl_spi_dir_c) = '0') then
                spi_rtx_sreg <= spi_rtx_sreg(6 downto 0) & spi_miso_ff1; -- MSB first
              else
                spi_rtx_sreg <= spi_miso_ff1 & spi_rtx_sreg(7 downto 1); -- LSB first
              end if;
            end if;
            spi_bitcnt <= std_ulogic_vector(unsigned(spi_bitcnt) - 1);
          end if;
        else -- second half of transmission
          spi_sclk_o <= not ctrl(ctrl_spi_cpha_c);
          if (spi_clk = '1') then
            spi_state1 <= '0';
            if (ctrl(ctrl_spi_cpha_c) = '1') then
              if (ctrl(ctrl_spi_dir_c) = '0') then
                spi_rtx_sreg <= spi_rtx_sreg(6 downto 0) & spi_miso_ff1; -- MSB first
              else
                spi_rtx_sreg <= spi_miso_ff1 & spi_rtx_sreg(7 downto 1); -- LSB first
              end if;
            end if;
            if (spi_bitcnt = "0000") then
              spi_state0 <= '0';
              spi_busy   <= '0';
              spi_irq_o  <= ctrl(ctrl_spi_irq_en_c);
            end if;
          end if;
        end if;
      end if;
    end if;
  end process spi_rtx_unit;

  -- direct user-defined CS --  
  spi_cs_o(0) <= '0' when (ctrl(ctrl_spi_cs_set_c) = '1') and (ctrl(ctrl_spi_cs_sel2_c downto ctrl_spi_cs_sel0_c) = "000") else '1';
  spi_cs_o(1) <= '0' when (ctrl(ctrl_spi_cs_set_c) = '1') and (ctrl(ctrl_spi_cs_sel2_c downto ctrl_spi_cs_sel0_c) = "001") else '1';
  spi_cs_o(2) <= '0' when (ctrl(ctrl_spi_cs_set_c) = '1') and (ctrl(ctrl_spi_cs_sel2_c downto ctrl_spi_cs_sel0_c) = "010") else '1';
  spi_cs_o(3) <= '0' when (ctrl(ctrl_spi_cs_set_c) = '1') and (ctrl(ctrl_spi_cs_sel2_c downto ctrl_spi_cs_sel0_c) = "011") else '1';
  spi_cs_o(4) <= '0' when (ctrl(ctrl_spi_cs_set_c) = '1') and (ctrl(ctrl_spi_cs_sel2_c downto ctrl_spi_cs_sel0_c) = "100") else '1';
  spi_cs_o(5) <= '0' when (ctrl(ctrl_spi_cs_set_c) = '1') and (ctrl(ctrl_spi_cs_sel2_c downto ctrl_spi_cs_sel0_c) = "101") else '1';
  spi_cs_o(6) <= '0' when (ctrl(ctrl_spi_cs_set_c) = '1') and (ctrl(ctrl_spi_cs_sel2_c downto ctrl_spi_cs_sel0_c) = "110") else '1';
  spi_cs_o(7) <= '0' when (ctrl(ctrl_spi_cs_set_c) = '1') and (ctrl(ctrl_spi_cs_sel2_c downto ctrl_spi_cs_sel0_c) = "111") else '1';


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0');
      if (rd_en = '1') then
        if (addr = spi_ctrl_addr_c) then
          data_o(ctrl_spi_en_c)      <= ctrl(ctrl_spi_en_c);
          data_o(ctrl_spi_cpha_c)    <= ctrl(ctrl_spi_cpha_c);
          data_o(ctrl_spi_irq_en_c)  <= ctrl(ctrl_spi_irq_en_c);
          data_o(ctrl_spi_prsc0_c)   <= ctrl(ctrl_spi_prsc0_c);
          data_o(ctrl_spi_prsc1_c)   <= ctrl(ctrl_spi_prsc1_c);
          data_o(ctrl_spi_prsc2_c)   <= ctrl(ctrl_spi_prsc2_c);
          data_o(ctrl_spi_cs_sel0_c) <= ctrl(ctrl_spi_cs_sel0_c);
          data_o(ctrl_spi_cs_sel1_c) <= ctrl(ctrl_spi_cs_sel1_c);
          data_o(ctrl_spi_cs_sel2_c) <= ctrl(ctrl_spi_cs_sel2_c);
          data_o(ctrl_spi_cs_set_c)  <= ctrl(ctrl_spi_cs_set_c);
          data_o(ctrl_spi_dir_c)     <= ctrl(ctrl_spi_dir_c);
          data_o(ctrl_spi_busy_c)    <= spi_busy;
        else -- spi_rtx_addr_c
          data_o(7 downto 0) <= spi_rtx_sreg;
        end if;
      end if;
    end if;
  end process rd_access;


end neo430_spi_rtl;
