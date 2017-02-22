-- #################################################################################################
-- #  << NEO430 - 32-bit Wishbone Interface >>                                                     #
-- # ********************************************************************************************* #
-- #  This interface only supports classic pipelined cycles. No bursts! Only one data              #
-- #  transmission can be pending at once. Code fetched via this interface cannot be directly      #
-- #  executed.                                                                                    #
-- #  A write transfer is started when writing to 'adr_lo_w_addr_c', a read transfer is started    #
-- #  when writing to 'adr_lo_r_addr_c'. Make sure to set the high part of the address, the        #
-- #  write-data and the byte enable signals (in the ctrl register) before. A transfer may take    #
-- #  max 256 cycles before it is automatically cancelled due to a timeout error. This can be      #
-- #  checked via bit #14 of the ctrl reg. This auto-timeout must be enabled via bit #11 of the    #
-- #  control registerThe current state of the transfer can be checked via                         #
-- #  bit #15 (pending) of the ctrl reg. This unit only performs 'pipelined' transfers. CYC stays  #
-- #  active until the cycle is acknowledged by the slave. STB is only applied for one cycle       #
-- #  (pipelined mode) or for the complete cycle (standard mode) (config via bit #13).             #
-- #  Activate this unit using the enable bit (#12). A pending transfer can be terminated by       #
-- #  clearing this bit.                                                                           #
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
-- #  Stephan Nolting, Hannover, Germany                                               14.02.2017  #
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
  constant ctrl_wb_sel0_c : natural :=  0; -- r/w: wishbone data byte enable bit 0
  constant ctrl_wb_sel1_c : natural :=  1; -- r/w: wishbone data byte enable bit 1
  constant ctrl_wb_sel2_c : natural :=  2; -- r/w: wishbone data byte enable bit 2
  constant ctrl_wb_sel3_c : natural :=  3; -- r/w: wishbone data byte enable bit 3
  constant ctrl_to_en_c   : natural := 11; -- r/w: enable timeout auto abort
  constant ctrl_enable_c  : natural := 12; -- r/w: enable wishbone interface
  constant ctrl_pipe_en_c : natural := 13; -- r/w: 0: standard mode, 1: pipelined mode
  constant ctrl_timeout_c : natural := 14; -- r/-: a timeout occured during WB bus access
  constant ctrl_pending_c : natural := 15; -- r/-: pending wb transfer

  -- access control --
  signal acc_en  : std_ulogic; -- module access enable
  signal addr    : std_ulogic_vector(15 downto 0); -- access address
  signal wr_en   : std_ulogic; -- word write enable
  -- accessible regs --
  signal wb_addr   : std_ulogic_vector(31 downto 0);
  signal wb_di     : std_ulogic_vector(31 downto 0);
  signal wb_do     : std_ulogic_vector(31 downto 0);
  signal byte_sel  : std_ulogic_vector(03 downto 0);
  signal pipelined : std_ulogic; -- pipelined mode enable
  signal enable    : std_ulogic; -- enable wishbone interface

  -- access arbiter --
  signal pending   : std_ulogic; -- pending transfer?
  signal timeout   : std_ulogic_vector(07 downto 0); -- timeout counter
  signal terr      : std_ulogic; -- timeout detector
  signal terr_ff   : std_ulogic; -- timeout indicator
  signal terr_en   : std_ulogic; -- timeout enabled

begin

  -- Access control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = wb32_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= wb32_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wr_en  <= acc_en and wren_i(1) and wren_i(0);


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (wr_en = '1') then
        case addr is
          when wb32_adr_lo_r_addr_c =>
            wb_addr(15 downto 00) <= data_i;
            wb_we_o <= '0'; -- read transfer
          when wb32_adr_lo_w_addr_c =>
            wb_addr(15 downto 00) <= data_i;
            wb_we_o <= '1'; -- write transfer
          when wb32_adr_hi_addr_c =>
            wb_addr(31 downto 16) <= data_i;
          when wb32_do_lo_addr_c =>
            wb_do(15 downto 00) <= data_i;
          when wb32_do_hi_addr_c =>
            wb_do(31 downto 16) <= data_i;
          when wb32_ctrl_addr_c =>
            byte_sel  <= data_i(ctrl_wb_sel3_c downto ctrl_wb_sel0_c);
            pipelined <= data_i(ctrl_pipe_en_c);
            enable    <= data_i(ctrl_enable_c);
            terr_en   <= data_i(ctrl_to_en_c);
          when others =>
            NULL;
        end case;
      end if;
    end if;
  end process wr_access;

  -- wb direct output --
  wb_adr_o <= wb_addr; -- address
  wb_dat_o <= wb_do; -- write data
  wb_sel_o <= byte_sel; -- byte enable


  -- Access arbiter -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  arbiter: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- trigger transfer --
      if (pending = '0') or (enable = '0') then
        wb_stb_o <= '0';
        pending  <= '0';
        if (wr_en = '1') and ((addr_i = wb32_adr_lo_r_addr_c) or (addr_i = wb32_adr_lo_w_addr_c)) then
          wb_stb_o <= '1';
          pending  <= '1';
          terr_ff  <= '0';
        end if;
      else -- transfer in progress
        wb_stb_o <= not pipelined; -- keep alive if standard cycle 
        -- waiting for ACK or timeout
        if (wb_ack_i = '1') or (terr = '1') then
          wb_stb_o <= '0';
          pending  <= '0';
        end if;
        terr_ff <= terr;
      end if;
    end if;
  end process arbiter;

  -- valid cycle signal --
  wb_cyc_o <= pending;


  -- Timeout logic ------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  timeout_logic: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (pending = '1') and (terr_en = '1') then
        timeout <= std_ulogic_vector(unsigned(timeout) - 1);
      else
        timeout <= (others => '1');
      end if;
    end if;
  end process timeout_logic;

  -- timeout detector --
  terr <= terr_en when (to_integer(unsigned(timeout)) = 0) else '0';


  -- Input sampling -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  sample_din: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (wb_ack_i = '1') and (pending = '1') then
        wb_di <= wb_dat_i;
      end if;
    end if;
  end process sample_din;


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0');
      if (rden_i = '1') and (acc_en = '1') then
        case addr is
          when wb32_di_lo_addr_c =>
            data_o <= wb_di(15 downto 00);
          when wb32_di_hi_addr_c =>
            data_o <= wb_di(31 downto 16);
          when others =>
--        when wb32_ctrl_addr_c =>
            data_o(ctrl_wb_sel3_c downto ctrl_wb_sel0_c) <= byte_sel;
            data_o(ctrl_pipe_en_c) <= pipelined;
            data_o(ctrl_pending_c) <= pending;
            data_o(ctrl_timeout_c) <= terr_ff;
            data_o(ctrl_enable_c)  <= enable;
            data_o(ctrl_to_en_c)   <= terr_en;
        end case;
      end if;
    end if;
  end process rd_access;


end neo430_wb_interface_rtl;
