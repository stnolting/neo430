-- #################################################################################################
-- # << NEO430 - 32-bit Wishbone Bus Interface Adapter >>                                          #
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

entity neo430_wb_interface is
  port (
    -- host access --
    clk_i    : in  std_ulogic; -- global clock line
    rden_i   : in  std_ulogic; -- read enable
    wren_i   : in  std_ulogic; -- write enable
    addr_i   : in  std_ulogic_vector(15 downto 0); -- address
    data_i   : in  std_ulogic_vector(15 downto 0); -- data in
    data_o   : out std_ulogic_vector(15 downto 0); -- data out
    -- wishbone interface --
    wb_adr_o : out std_ulogic_vector(31 downto 0); -- address
    wb_dat_i : in  std_ulogic_vector(31 downto 0); -- read data
    wb_dat_o : out std_ulogic_vector(31 downto 0); -- write data
    wb_we_o  : out std_ulogic; -- read/write
    wb_sel_o : out std_ulogic_vector(03 downto 0); -- byte enable
    wb_stb_o : out std_ulogic; -- strobe
    wb_cyc_o : out std_ulogic; -- valid cycle
    wb_ack_i : in  std_ulogic  -- transfer acknowledge
  );
end neo430_wb_interface;

architecture neo430_wb_interface_rtl of neo430_wb_interface is

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size_f(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size_f(wb32_size_c); -- low address boundary bit

  -- control reg bits --
  constant ctrl_byte_en0_c : natural :=  0; -- -/w: wishbone data byte enable bit 0
  constant ctrl_byte_en1_c : natural :=  1; -- -/w: wishbone data byte enable bit 1
  constant ctrl_byte_en2_c : natural :=  2; -- -/w: wishbone data byte enable bit 2
  constant ctrl_byte_en3_c : natural :=  3; -- -/w: wishbone data byte enable bit 3
  constant ctrl_pending_c  : natural := 15; -- r/-: pending wb transfer

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal addr   : std_ulogic_vector(15 downto 0); -- access address
  signal wr_en  : std_ulogic;

  -- accessible regs --
  signal wb_addr   : std_ulogic_vector(31 downto 0);
  signal wb_rdata  : std_ulogic_vector(31 downto 0);
  signal wb_wdata  : std_ulogic_vector(31 downto 0);
  signal pending   : std_ulogic; -- pending transfer?
  signal byte_en   : std_ulogic_vector(03 downto 0);

  -- misc --
  signal enable : std_ulogic;

begin

  -- Access control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = wb32_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= wb32_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wr_en  <= acc_en and wren_i;


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (wr_en = '1') then -- valid word write
        if (addr = wb32_rd_adr_lo_addr_c) then
          wb_addr(15 downto 0) <= data_i;
          wb_we_o <= '0';
        end if;
        if (addr = wb32_rd_adr_hi_addr_c) then
          wb_addr(31 downto 16) <= data_i;
          wb_we_o <= '0';
        end if;
        if (addr = wb32_wr_adr_lo_addr_c) then
          wb_addr(15 downto 0) <= data_i;
          wb_we_o <= '1';
        end if;
        if (addr = wb32_wr_adr_hi_addr_c) then
          wb_addr(31 downto 16) <= data_i;
          wb_we_o <= '1';
        end if;
        if (addr = wb32_data_lo_addr_c) then
          wb_wdata(15 downto 0) <= data_i;
        end if;
        if (addr = wb32_data_hi_addr_c) then
          wb_wdata(31 downto 16) <= data_i;
        end if;
        if (addr = wb32_ctrl_addr_c) then
          byte_en(0) <= data_i(ctrl_byte_en0_c);
          byte_en(1) <= data_i(ctrl_byte_en1_c);
          byte_en(2) <= data_i(ctrl_byte_en2_c);
          byte_en(3) <= data_i(ctrl_byte_en3_c);
        end if;
      end if;
    end if;
  end process wr_access;

  -- direct output --
  wb_adr_o <= wb_addr; -- address
  wb_dat_o <= wb_wdata; -- write data
  wb_sel_o <= byte_en; -- byte enable


  -- Access arbiter -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  arbiter: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- trigger transfer --
      if (pending = '0') or (enable = '0') then
        wb_stb_o <= '0';
        pending  <= '0';
        if (wr_en = '1') and (enable = '1') and ((addr_i = wb32_rd_adr_hi_addr_c) or (addr_i = wb32_wr_adr_hi_addr_c)) then
          wb_stb_o <= '1';
          pending  <= '1';
        end if;
      else -- transfer in progress
        wb_stb_o <= '0'; -- use ONLY standard/classic cycle with single-cycle STB assertion!!
        -- waiting for ACK
        if (wb_ack_i = '1') then
          wb_rdata <= wb_dat_i; -- sample input data
          wb_stb_o <= '0';
          pending  <= '0';
        end if;
      end if;
    end if;
  end process arbiter;

  -- device actually in use? --
  enable <= or_all_f(byte_en);

  -- valid cycle signal --
  wb_cyc_o <= pending;


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0');
      if (rden_i = '1') and (acc_en = '1') then
        if (addr = wb32_data_lo_addr_c) then
          data_o <= wb_rdata(15 downto 00);
        elsif (addr = wb32_data_hi_addr_c) then
          data_o <= wb_rdata(31 downto 16);
        else -- when wb32_ctrl_addr_c =>
          data_o(ctrl_pending_c) <= pending;
        end if;
      end if;
    end if;
  end process rd_access;


end neo430_wb_interface_rtl;
