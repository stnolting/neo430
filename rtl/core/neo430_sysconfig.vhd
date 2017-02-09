-- #################################################################################################
-- #  << NEO430 - System Configuration Memory >>                                                   #
-- # ********************************************************************************************* #
-- #  - Lower 16 addresses*: ROM for HW info -> Clock speed, HW version, CPU/system features, ...  #
-- #  - Upper 16 addresses* (2 x mirrored): RAM for interrupt vector addresses (4x)                #
-- #    *) byte addresses                                                                          #
-- # ********************************************************************************************* #
-- # This file is part of the NEO430 Processor project: http://opencores.org/project,neo430        #
-- # Copyright 2015-2017, Stephan Nolting: stnolting@gmail.com                                     #
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
-- #  Stephan Nolting, Hannover, Germany                                               08.02.2017  #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.neo430_package.all;

entity neo430_sysconfig is
  generic (
    USER_CODE : std_ulogic_vector(15 downto 0) := x"0000" -- custom user code
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
  signal acc_en : std_ulogic; -- access enable
  signal addr   : std_ulogic_vector(15 downto 0);

  -- misc --
  signal rden   : std_ulogic;
  signal rdata0 : std_ulogic_vector(15 downto 0);
  signal rdata1 : std_ulogic_vector(15 downto 0);
  signal rd_sel : std_ulogic;
  signal f_clk  : std_ulogic_vector(31 downto 0);

  -- internal memories --
  type sc_mem_t is array (0 to 7) of std_ulogic_vector(15 downto 0);
  signal sysinfo_mem   : sc_mem_t; -- ROM
  signal sysconfig_mem : sc_mem_t; -- RAM

  --- RAM attribute to inhibit bypass-logic - Altera only! ---
  attribute ramstyle : string;
  attribute ramstyle of sysinfo_mem : signal is "no_rw_check, M512";
  attribute ramstyle of sysconfig_mem : signal is "no_rw_check, M512";

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = sysconfig_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= sysconfig_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned


  -- Construct Info Mem -------------------------------------------------------
  -- -----------------------------------------------------------------------------
  -- CPUID0: HW version --
  sysinfo_mem(0) <= hw_version_c; -- HW version

  -- CPUID1: System setup (features) --
  sysinfo_mem(1)(00) <= '0';                          -- obsolete: MAC16
  sysinfo_mem(1)(01) <= bool_to_ulogic(wb32_use_c);   -- WB32 present?
  sysinfo_mem(1)(02) <= bool_to_ulogic(wdt_use_c);    -- WDT present?
  sysinfo_mem(1)(03) <= bool_to_ulogic(pio_use_c);    -- PIO present?
  sysinfo_mem(1)(04) <= bool_to_ulogic(timer_use_c);  -- TIMER present?
  sysinfo_mem(1)(05) <= bool_to_ulogic(usart_use_c);  -- USART present?
  sysinfo_mem(1)(06) <= bool_to_ulogic(synth_dadd_c); -- DADD instruction present?
  sysinfo_mem(1)(07) <= bool_to_ulogic(bootld_use_c); -- bootloader present?
  sysinfo_mem(1)(08) <= bool_to_ulogic(imem_rom_c);   -- implement IMEM as true ROM?
  sysinfo_mem(1)(15 downto 9) <= (others => '0');     -- reserved

  -- CPUID2: User code --
  sysinfo_mem(2) <= USER_CODE;

  -- CPUID3: IMEM (ROM) size --
  sysinfo_mem(3) <= std_ulogic_vector(to_unsigned(imem_size_c, 16)); -- size in bytes

  -- CPUID4: DMEM (RAM) base address --
  sysinfo_mem(4) <= dmem_base_c;

  -- CPUID5: DMEM (RAM) size --
  sysinfo_mem(5) <= std_ulogic_vector(to_unsigned(dmem_size_c, 16)); -- size in bytes

  -- CPUID6/CPUID7: Clock speed --
  f_clk <= std_ulogic_vector(to_unsigned(clock_speed_c, 32));
  sysinfo_mem(6) <= f_clk(15 downto 00); -- clock speed LO
  sysinfo_mem(7) <= f_clk(31 downto 16); -- clock speed HI


  -- Read/Write Access --------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rw_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (wren_i = "11") and (acc_en = '1') and (addr_i(lo_abb_c-1) = '1') then -- write access to IRQ config
        sysconfig_mem(to_integer(unsigned(addr(index_size(sysconfig_size_c/2)-1 downto 1)))) <= data_i;
      end if;
      rden   <= rden_i and acc_en;
      rd_sel <= addr_i(lo_abb_c-1);
      rdata0 <= sysinfo_mem(to_integer(unsigned(addr(index_size(sysconfig_size_c/2)-1 downto 1))));
      rdata1 <= sysconfig_mem(to_integer(unsigned(addr(index_size(sysconfig_size_c/2)-1 downto 1))));
    end if;
  end process rw_access;

  -- output gate --
  data_o <= x"0000" when (rden = '0') else rdata0 when (rd_sel = '0') else rdata1;


end neo430_sysconfig_rtl;
