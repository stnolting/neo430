-- #################################################################################################
-- #  << NEO430 - NEO430-Compatible 2x Slave Wishbone Switch >>                                    #
-- # ********************************************************************************************* #
-- #  Switch to connect two slaves to a single master. The full address space is separated         #
-- #  into two sub-address spaces. The addressspace of slave 0 always starts at 0. The first       #
-- #  address of the address space of slave 1 is specified by the "slave1_addr_begin_g" generic.   #
-- #  "eff_addr_width_g" specifies the effective address width. Set it to 32 if you are using a    #
-- #  32-bit wide address space, or - for instance - set it to 16 if you are using a 16-bit wide   #
-- #  address space. If "eff_addr_width_g" is less than 32, the remaining upper bits of the        #
-- #  address border ("slave1_addr_begin_g") are ignored. If you are using less than 32-bit data   #
-- #  width, leave all unneccessery output ports open and set all unneccessary inputs to '0'.      #
-- #                                                                                               #
-- #  You can cascade several switches (tree-style) to create a wider switch (e.g. for 4 slave     #
-- #  a total of 3 2x switches are required).                                                      #         
-- #  This switch is designed to primarily support single-cycle transfers (classic or pipelined).  #
-- #  The switch is purely combinatorial.                                                          #
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
-- #  Stephan Nolting, Hannover, Germany                                               16.04.2016  #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity wb_switch_2x is
  generic (
    eff_addr_width_g    : natural := 32; -- effective address width (0..32 bits)
    slave1_addr_begin_g : std_ulogic_vector(31 downto 0) := x"80000000" -- beginning of slave1's address space
  );
  port (
    -- wishbone master port --
    wb_master_cyc_i  : in  std_ulogic; -- valid cycle
    wb_master_stb_i  : in  std_ulogic; -- valid strobe
    wb_master_we_i   : in  std_ulogic; -- write enable
    wb_master_addr_i : in  std_ulogic_vector(31 downto 0); -- access address
    wb_master_data_i : in  std_ulogic_vector(31 downto 0); -- write data
    wb_master_data_o : out std_ulogic_vector(31 downto 0); -- read data
    wb_master_ack_o  : out std_ulogic; -- acknowledge
    -- wishbone slave 0 port --
    wb_slave0_cyc_o  : out std_ulogic; -- valid cycle
    wb_slave0_stb_o  : out std_ulogic; -- valid strobe
    wb_slave0_we_o   : out std_ulogic; -- write enable
    wb_slave0_addr_o : out std_ulogic_vector(31 downto 0); -- access address
    wb_slave0_data_o : out std_ulogic_vector(31 downto 0); -- write data
    wb_slave0_data_i : in  std_ulogic_vector(31 downto 0); -- read data
    wb_slave0_ack_i  : in  std_ulogic; -- acknowledge
    -- wishbone slave 1 port --
    wb_slave1_cyc_o  : out std_ulogic; -- valid cycle
    wb_slave1_stb_o  : out std_ulogic; -- valid strobe
    wb_slave1_we_o   : out std_ulogic; -- write enable
    wb_slave1_addr_o : out std_ulogic_vector(31 downto 0); -- access address
    wb_slave1_data_o : out std_ulogic_vector(31 downto 0); -- write data
    wb_slave1_data_i : in  std_ulogic_vector(31 downto 0); -- read data
    wb_slave1_ack_i  : in  std_ulogic  -- acknowledge
  );
end wb_switch_2x;

architecture wb_switch_2x_rtl of wb_switch_2x is

  -- slave selector --
  signal addr, border : std_ulogic_vector(eff_addr_width_g-1 downto 0);
  signal slave_sel    : std_ulogic;
  
begin

  -- Slave Selector -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  addr      <= wb_master_addr_i(eff_addr_width_g-1 downto 0);
  border    <= slave1_addr_begin_g(eff_addr_width_g-1 downto 0);
  slave_sel <= '0' when (unsigned(addr) < unsigned(border)) else '1';


  -- Global Signals -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wb_slave0_addr_o <= wb_master_addr_i;
  wb_slave1_addr_o <= wb_master_addr_i;

  wb_slave0_data_o <= wb_master_data_i;
  wb_slave1_data_o <= wb_master_data_i;

  wb_slave0_we_o   <= wb_master_we_i;
  wb_slave1_we_o   <= wb_master_we_i;


  -- Signal Switch ------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wb_slave0_cyc_o <= wb_master_cyc_i when (slave_sel = '0') else '0';
  wb_slave0_stb_o <= wb_master_stb_i when (slave_sel = '0') else '0';

  wb_slave1_cyc_o <= wb_master_cyc_i when (slave_sel = '1') else '0';
  wb_slave1_stb_o <= wb_master_stb_i when (slave_sel = '1') else '0';


  -- Master Read-Back ---------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wb_master_data_o <= wb_slave0_data_i when (slave_sel = '0') else wb_slave1_data_i;
  wb_master_ack_o  <= wb_slave0_ack_i  when (slave_sel = '0') else wb_slave1_ack_i;


end wb_switch_2x_rtl;
