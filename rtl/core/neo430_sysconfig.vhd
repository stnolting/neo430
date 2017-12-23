-- #################################################################################################
-- #  << NEO430 - System Configuration Memory >>                                                   #
-- # ********************************************************************************************* #
-- #  - Lower 8 word addresses*: ROM for HW info ( 8 entries)                                      #
-- #  - Upper 8 word addresses* (2 x mirrored): RAM for the 4 interrupt vector addresses           #
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
-- #  Stephan Nolting, Hannover, Germany                                               01.12.2017  #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;

entity neo430_sysconfig is
  generic (
    -- general configuration --
    CLOCK_SPEED : natural := 100000000; -- main clock in Hz
    IMEM_SIZE   : natural := 4*1024; -- internal IMEM size in bytes
    DMEM_SIZE   : natural := 2*1024; -- internal DMEM size in bytes
    -- additional configuration --
    USER_CODE   : std_ulogic_vector(15 downto 0) := x"0000"; -- custom user code
    -- module configuration --
    DADD_USE    : boolean := true; -- implement DADD instruction?
    MULDIV_USE  : boolean := true; -- implementcustom MULDIV unit?
    WB32_USE    : boolean := true; -- implement WB32 unit?
    WDT_USE     : boolean := true; -- implement WDT?
    GPIO_USE    : boolean := true; -- implement GPIO unit?
    TIMER_USE   : boolean := true; -- implement timer?
    USART_USE   : boolean := true; -- implement USART?
    -- boot configuration --
    BOOTLD_USE  : boolean := true; -- implement and use bootloader?
    IMEM_AS_ROM : boolean := false -- implement IMEM as read-only memory?
  );
  port (
    clk_i  : in  std_ulogic; -- global clock line
    rden_i : in  std_ulogic; -- read enable
    wren_i : in  std_ulogic_vector(01 downto 0); -- write enable
    addr_i : in  std_ulogic_vector(15 downto 0); -- address
    data_i : in  std_ulogic_vector(15 downto 0); -- data in
    data_o : out std_ulogic_vector(15 downto 0)  -- data out
  );
end neo430_sysconfig;

architecture neo430_sysconfig_rtl of neo430_sysconfig is

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size(sysconfig_size_c); -- low address boundary bit

  -- access control --
  signal acc_en    : std_ulogic; -- access enable
  signal addr      : std_ulogic_vector(15 downto 0);
  signal info_addr : std_ulogic_vector(02 downto 0);
  signal irqv_addr : std_ulogic_vector(01 downto 0);

  -- misc --
  signal f_clk : std_ulogic_vector(31 downto 0);

  -- system information ROM --
  type info_mem_t is array (0 to 7) of std_ulogic_vector(15 downto 0);
  signal sysinfo_mem : info_mem_t; -- ROM

  -- interrupt vector RAM --
  type irqv_mem_t is array (0 to 3) of std_ulogic_vector(15 downto 0);
  signal irqvec_mem : irqv_mem_t; -- RAM

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = sysconfig_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= sysconfig_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned

  info_addr <= addr(index_size(sysconfig_size_c/2)-1 downto 1);
  irqv_addr <= addr(index_size(sysconfig_size_c/4)-1 downto 1);
  

  -- Construct Info Mem -------------------------------------------------------
  -- -----------------------------------------------------------------------------
  -- CPUID0: HW version --
  sysinfo_mem(0) <= hw_version_c; -- HW version

  -- CPUID1: System setup (features) --
  sysinfo_mem(1)(00) <= bool_to_ulogic(MULDIV_USE);  -- MULDIV present?
  sysinfo_mem(1)(01) <= bool_to_ulogic(WB32_USE);    -- WB32 present?
  sysinfo_mem(1)(02) <= bool_to_ulogic(WDT_USE);     -- WDT present?
  sysinfo_mem(1)(03) <= bool_to_ulogic(GPIO_USE);    -- GPIO present?
  sysinfo_mem(1)(04) <= bool_to_ulogic(TIMER_USE);   -- TIMER present?
  sysinfo_mem(1)(05) <= bool_to_ulogic(USART_USE);   -- USART present?
  sysinfo_mem(1)(06) <= bool_to_ulogic(DADD_USE);    -- DADD instruction present?
  sysinfo_mem(1)(07) <= bool_to_ulogic(BOOTLD_USE);  -- bootloader present?
  sysinfo_mem(1)(08) <= bool_to_ulogic(IMEM_AS_ROM); -- IMEM implemented as true ROM?
  sysinfo_mem(1)(15 downto 9) <= (others => '0');    -- reserved

  -- CPUID2: User code --
  sysinfo_mem(2) <= USER_CODE;

  -- CPUID3: IMEM (ROM/RAM) size --
  sysinfo_mem(3) <= std_ulogic_vector(to_unsigned(IMEM_SIZE, 16)); -- size in bytes

  -- CPUID4: DMEM (RAM) base address --
  sysinfo_mem(4) <= dmem_base_c;

  -- CPUID5: DMEM (RAM) size --
  sysinfo_mem(5) <= std_ulogic_vector(to_unsigned(DMEM_SIZE, 16)); -- size in bytes

  -- CPUID6/CPUID7: Clock speed --
  f_clk <= std_ulogic_vector(to_unsigned(CLOCK_SPEED, 32));
  sysinfo_mem(6) <= f_clk(15 downto 00); -- clock speed LO
  sysinfo_mem(7) <= f_clk(31 downto 16); -- clock speed HI


  -- Read/Write Access --------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rw_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- write access (IRQ vectors) --
      if (wren_i = "11") and (acc_en = '1') and (addr_i(lo_abb_c-1) = '1') then
        irqvec_mem(to_integer(unsigned(irqv_addr))) <= data_i;
      end if;
      -- read access --
      if ((rden_i and acc_en) = '1') then
        if (addr_i(lo_abb_c-1) = '0') then -- read INFOMEM
          data_o <= sysinfo_mem(to_integer(unsigned(info_addr)));
        else -- read IRQ vector
          data_o <= irqvec_mem(to_integer(unsigned(irqv_addr)));
        end if;
      else
        data_o <= x"0000";
      end if;
    end if;
  end process rw_access;


end neo430_sysconfig_rtl;
