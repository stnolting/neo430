-- #################################################################################################
-- #  << NEO430 - Serial Peripheral Interface >>                                                   #
-- # ********************************************************************************************* #
-- # Frame format: 8-bit or 16-bit, MSB or LSB first, 2 clock modes, 8 clock speeds, 6 CS lines.   #
-- # Interrupt: SPI_transfer_done                                                                  #
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
    spi_cs_o    : out std_ulogic_vector(05 downto 0); -- SPI CS
    -- interrupt --
    spi_irq_o   : out std_ulogic -- transmission done interrupt
  );
end neo430_spi;

architecture neo430_spi_rtl of neo430_spi is

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size_f(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size_f(spi_size_c); -- low address boundary bit

  -- control reg bits --
  constant ctrl_spi_cs_sel0_c : natural :=  0; -- r/w: spi CS 0
  constant ctrl_spi_cs_sel1_c : natural :=  1; -- r/w: spi CS 1
  constant ctrl_spi_cs_sel2_c : natural :=  2; -- r/w: spi CS 2
  constant ctrl_spi_cs_sel3_c : natural :=  3; -- r/w: spi CS 3
  constant ctrl_spi_cs_sel4_c : natural :=  4; -- r/w: spi CS 4
  constant ctrl_spi_cs_sel5_c : natural :=  5; -- r/w: spi CS 5
  constant ctrl_spi_en_c      : natural :=  6; -- r/w: spi enable
  constant ctrl_spi_cpha_c    : natural :=  7; -- r/w: spi clock phase
  constant ctrl_spi_irq_en_c  : natural :=  8; -- r/w: spi transmission done interrupt enable
  constant ctrl_spi_prsc0_c   : natural :=  9; -- r/w: spi prescaler select bit 0
  constant ctrl_spi_prsc1_c   : natural := 10; -- r/w: spi prescaler select bit 1
  constant ctrl_spi_prsc2_c   : natural := 11; -- r/w: spi prescaler select bit 2
  constant ctrl_spi_dir_c     : natural := 12; -- r/w: shift direction (0: MSB first, 1: LSB first)
  constant ctrl_spi_size_c    : natural := 13; -- r/w: data size(0: 8-bit, 1: 16-bit)
  --       reserved           : natural := 14;
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
  signal spi_rtx_sreg : std_ulogic_vector(15 downto 0);
  signal spi_rx_data  : std_ulogic_vector(15 downto 0);
  signal spi_bitcnt   : std_ulogic_vector(04 downto 0);
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
        if (ctrl(ctrl_spi_size_c) = '0') then -- 8 bit mode
          spi_bitcnt <= "01000";
        else -- 16 bit mode
          spi_bitcnt <= "10000";
        end if;
        spi_state1 <= '0';
        spi_mosi_o <= '0';
        spi_sclk_o <= '0';
        if (ctrl(ctrl_spi_en_c) = '0') then -- disabled
          spi_busy <= '0';
        elsif (wr_en = '1') and (addr = spi_rtx_addr_c) then
          if (ctrl(ctrl_spi_size_c) = '0') then -- 8 bit mode
            spi_rtx_sreg <= data_i(7 downto 0) & "00000000";
          else -- 16 bit mode
            spi_rtx_sreg <= data_i(15 downto 0);
          end if;
          spi_busy <= '1';
        end if;
        spi_state0 <= spi_busy and spi_clk; -- start with next new clock pulse

      else -- transmission in progress
        if (spi_state1 = '0') then -- first half of transmission

          spi_sclk_o <= ctrl(ctrl_spi_cpha_c);
          if (ctrl(ctrl_spi_dir_c) = '0') then
            spi_mosi_o <= spi_rtx_sreg(15); -- MSB first
          else
            spi_mosi_o <= spi_rtx_sreg(0); -- LSB first
          end if;
          if (spi_clk = '1') then
            spi_state1 <= '1';
            if (ctrl(ctrl_spi_cpha_c) = '0') then
              if (ctrl(ctrl_spi_dir_c) = '0') then
                spi_rtx_sreg <= spi_rtx_sreg(14 downto 0) & spi_miso_ff1; -- MSB first
              else
                spi_rtx_sreg <= spi_miso_ff1 & spi_rtx_sreg(15 downto 1); -- LSB first
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
                spi_rtx_sreg <= spi_rtx_sreg(14 downto 0) & spi_miso_ff1; -- MSB first
              else
                spi_rtx_sreg <= spi_miso_ff1 & spi_rtx_sreg(15 downto 1); -- LSB first
              end if;
            end if;
            if (spi_bitcnt = "00000") then
              spi_state0 <= '0';
              spi_busy   <= '0';
              spi_irq_o  <= ctrl(ctrl_spi_irq_en_c);
            end if;
          end if;
        end if;
      end if;
    end if;
  end process spi_rtx_unit;

  -- SPI receiver output --
  spi_rx_data <= (x"00" & spi_rtx_sreg(7 downto 0)) when (ctrl(ctrl_spi_size_c) = '0') else spi_rtx_sreg(15 downto 0);

  -- direct user-defined CS --  
  spi_cs_o(0) <= '0' when (ctrl(ctrl_spi_cs_sel0_c) = '1') else '1';
  spi_cs_o(1) <= '0' when (ctrl(ctrl_spi_cs_sel1_c) = '1') else '1';
  spi_cs_o(2) <= '0' when (ctrl(ctrl_spi_cs_sel2_c) = '1') else '1';
  spi_cs_o(3) <= '0' when (ctrl(ctrl_spi_cs_sel3_c) = '1') else '1';
  spi_cs_o(4) <= '0' when (ctrl(ctrl_spi_cs_sel4_c) = '1') else '1';
  spi_cs_o(5) <= '0' when (ctrl(ctrl_spi_cs_sel5_c) = '1') else '1';


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
          data_o(ctrl_spi_dir_c)     <= ctrl(ctrl_spi_dir_c);
          data_o(ctrl_spi_size_c)    <= ctrl(ctrl_spi_size_c);
          data_o(ctrl_spi_cs_sel0_c) <= ctrl(ctrl_spi_cs_sel0_c);
          data_o(ctrl_spi_cs_sel1_c) <= ctrl(ctrl_spi_cs_sel1_c);
          data_o(ctrl_spi_cs_sel2_c) <= ctrl(ctrl_spi_cs_sel2_c);
          data_o(ctrl_spi_cs_sel3_c) <= ctrl(ctrl_spi_cs_sel3_c);
          data_o(ctrl_spi_cs_sel4_c) <= ctrl(ctrl_spi_cs_sel4_c);
          data_o(ctrl_spi_cs_sel5_c) <= ctrl(ctrl_spi_cs_sel5_c);
          data_o(ctrl_spi_busy_c)    <= spi_busy;
        else -- spi_rtx_addr_c
          data_o(15 downto 0) <= spi_rx_data;
        end if;
      end if;
    end if;
  end process rd_access;


end neo430_spi_rtl;
