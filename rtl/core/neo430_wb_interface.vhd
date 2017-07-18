-- #################################################################################################
-- #  << NEO430 - 32-bit Wishbone Interface >>                                                     #
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

library work;
use work.neo430_package.all;

entity neo430_wb_interface is
  port (
    -- host access --
    clk_i    : in  std_ulogic; -- global clock line
    rden_i   : in  std_ulogic; -- read enable
    wren_i   : in  std_ulogic_vector(01 downto 0); -- write enable
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
  constant hi_abb_c : natural := index_size(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size(wb32_size_c); -- low address boundary bit

  -- control reg bits --
  constant ctrl_byte_en0_c : natural :=  0; -- -/w: wishbone data byte enable bit 0
  constant ctrl_byte_en1_c : natural :=  1; -- -/w: wishbone data byte enable bit 1
  constant ctrl_byte_en2_c : natural :=  2; -- -/w: wishbone data byte enable bit 2
  constant ctrl_byte_en3_c : natural :=  3; -- -/w: wishbone data byte enable bit 3
  constant ctrl_pmode_c    : natural :=  4; -- -/w: 0: standard mode, 1: pipelined mode
  constant ctrl_pending_c  : natural := 15; -- r/-: pending wb transfer

  -- access control --
  signal acc_en  : std_ulogic; -- module access enable
  signal addr    : std_ulogic_vector(15 downto 0); -- access address
  signal wr_en   : std_ulogic; -- word write enable
  signal bwr_en  : std_ulogic_vector(01 downto 0); -- byte write enable

  -- accessible regs --
  signal wb_addr   : std_ulogic_vector(31 downto 0);
  signal wb_rdata  : std_ulogic_vector(31 downto 0);
  signal wb_wdata  : std_ulogic_vector(31 downto 0);
  signal pending   : std_ulogic; -- pending transfer?
  signal pipelined : std_ulogic; -- pipelined mode enable
  signal byte_en   : std_ulogic_vector(03 downto 0);

  -- misc --
  signal enable : std_ulogic;

begin

  -- Access control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en    <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = wb32_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr      <= wb32_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  bwr_en(0) <= acc_en and wren_i(0);
  bwr_en(1) <= acc_en and wren_i(1);
  wr_en     <= acc_en and wren_i(1) and wren_i(0);


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      wb_we_o <= '0';
      for i in 0 to 1 loop
        if (bwr_en(i) = '1') then
          case addr is
            when wb32_rd_adr_lo_addr_c =>
              wb_addr(i*8+7 downto i*8) <= data_i(i*8+7 downto i*8);
            when wb32_rd_adr_hi_addr_c =>
              wb_addr(i*8+7+16 downto i*8+16) <= data_i(i*8+7 downto i*8);
            when wb32_wr_adr_lo_addr_c =>
              wb_addr(i*8+7 downto i*8) <= data_i(i*8+7 downto i*8);
              wb_we_o <= '1';
            when wb32_wr_adr_hi_addr_c =>
              wb_addr(i*8+7+16 downto i*8+16) <= data_i(i*8+7 downto i*8);
              wb_we_o <= '1';
            when wb32_data_lo_addr_c =>
              wb_wdata(i*8+7 downto i*8) <= data_i(i*8+7 downto i*8);
            when wb32_data_hi_addr_c =>
              wb_wdata(i*8+7+16 downto i*8+16) <= data_i(i*8+7 downto i*8);
            when wb32_ctrl_addr_c =>
              if (i = 0) then -- low byte
                byte_en(0) <= data_i(ctrl_byte_en0_c);
                byte_en(1) <= data_i(ctrl_byte_en1_c);
                byte_en(2) <= data_i(ctrl_byte_en2_c);
                byte_en(3) <= data_i(ctrl_byte_en3_c);
                pipelined  <= data_i(ctrl_pmode_c);
              else
                NULL;
              end if;
            when others =>
              NULL;
          end case;
        end if;
      end loop; -- i
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
        wb_stb_o <= not pipelined; -- keep activated if standard/classic cycle 
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
  enable <= '0' when (byte_en = "0000") else '1';

  -- valid cycle signal --
  wb_cyc_o <= pending;


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0');
      if (rden_i = '1') and (acc_en = '1') then
        case addr is
          when wb32_data_lo_addr_c =>
            data_o <= wb_rdata(15 downto 00);
          when wb32_data_hi_addr_c =>
            data_o <= wb_rdata(31 downto 16);
          when others =>
--        when wb32_ctrl_addr_c =>
            data_o(ctrl_pending_c) <= pending;
        end case;
      end if;
    end if;
  end process rd_access;


end neo430_wb_interface_rtl;
