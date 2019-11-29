-- #################################################################################################
-- #  << NEO430 - System Configuration Memory >>                                                   #
-- # ********************************************************************************************* #
-- # This is a read only memory providing information about the processor configuration obtained   #
-- # from the top entity's generics.                                                               #
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
-- # Stephan Nolting, Hannover, Germany                                                 28.11.2019 #
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
    MULDIV_USE  : boolean := true; -- implement multiplier/divider unit?
    WB32_USE    : boolean := true; -- implement WB32 unit?
    WDT_USE     : boolean := true; -- implement WDT?
    GPIO_USE    : boolean := true; -- implement GPIO unit?
    TIMER_USE   : boolean := true; -- implement timer?
    UART_USE    : boolean := true; -- implement UART?
    CRC_USE     : boolean := true; -- implement CRC unit?
    CFU_USE     : boolean := true; -- implement CF unit?
    PWM_USE     : boolean := true; -- implement PWM controller?
    TWI_USE     : boolean := true; -- implement TWI?
    SPI_USE     : boolean := true; -- implement SPI?
    TRNG_USE    : boolean := true; -- implement TRNG?
    EXIRQ_USE   : boolean := true; -- implement EXIRQ? (default=true)
    -- boot configuration --
    BOOTLD_USE  : boolean := true; -- implement and use bootloader?
    IMEM_AS_ROM : boolean := false -- implement IMEM as read-only memory?
  );
  port (
    clk_i  : in  std_ulogic; -- global clock line
    rden_i : in  std_ulogic; -- read enable
    wren_i : in  std_ulogic; -- write enable
    addr_i : in  std_ulogic_vector(15 downto 0); -- address
    data_i : in  std_ulogic_vector(15 downto 0); -- data in
    data_o : out std_ulogic_vector(15 downto 0)  -- data out
  );
end neo430_sysconfig;

architecture neo430_sysconfig_rtl of neo430_sysconfig is

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size_f(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size_f(sysconfig_size_c); -- low address boundary bit

  -- access control --
  signal acc_en    : std_ulogic; -- access enable
  signal addr      : std_ulogic_vector(15 downto 0);
  signal rden      : std_ulogic;
  signal info_addr : std_ulogic_vector(02 downto 0);

  -- misc --
  signal f_clk : std_ulogic_vector(31 downto 0);

  -- system information ROM --
  type info_mem_t is array (0 to 7) of std_ulogic_vector(15 downto 0);
  signal sysinfo_mem : info_mem_t; -- ROM

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = sysconfig_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= sysconfig_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  rden   <= acc_en and rden_i;

  info_addr <= addr(index_size_f(sysconfig_size_c)-1 downto 1);
  

  -- Construct Info ROM -------------------------------------------------------
  -- -----------------------------------------------------------------------------
  -- CPUID0: HW version --
  sysinfo_mem(0) <= hw_version_c; -- HW version

  -- CPUID1: System setup (features) --
  sysinfo_mem(1)(00) <= bool_to_ulogic_f(MULDIV_USE);     -- MULDIV present?
  sysinfo_mem(1)(01) <= bool_to_ulogic_f(WB32_USE);       -- WB32 present?
  sysinfo_mem(1)(02) <= bool_to_ulogic_f(WDT_USE);        -- WDT present?
  sysinfo_mem(1)(03) <= bool_to_ulogic_f(GPIO_USE);       -- GPIO present?
  sysinfo_mem(1)(04) <= bool_to_ulogic_f(TIMER_USE);      -- TIMER present?
  sysinfo_mem(1)(05) <= bool_to_ulogic_f(UART_USE);       -- UART present?
  sysinfo_mem(1)(06) <= bool_to_ulogic_f(use_dadd_cmd_c); -- DADD instruction present?
  sysinfo_mem(1)(07) <= bool_to_ulogic_f(BOOTLD_USE);     -- bootloader present?
  sysinfo_mem(1)(08) <= bool_to_ulogic_f(IMEM_AS_ROM);    -- IMEM implemented as true ROM?
  sysinfo_mem(1)(09) <= bool_to_ulogic_f(CRC_USE);        -- CRC present?
  sysinfo_mem(1)(10) <= bool_to_ulogic_f(CFU_USE);        -- CFU present?
  sysinfo_mem(1)(11) <= bool_to_ulogic_f(PWM_USE);        -- PWM present?
  sysinfo_mem(1)(12) <= bool_to_ulogic_f(TWI_USE);        -- TWI present?
  sysinfo_mem(1)(13) <= bool_to_ulogic_f(SPI_USE);        -- SPI present?
  sysinfo_mem(1)(14) <= bool_to_ulogic_f(TRNG_USE);       -- TRNG present?
  sysinfo_mem(1)(15) <= bool_to_ulogic_f(EXIRQ_USE);      -- EXIRQ present?

  -- CPUID2: User code --
  sysinfo_mem(2) <= USER_CODE;

  -- CPUID3: IMEM (ROM/RAM) size --
  sysinfo_mem(3) <= std_ulogic_vector(to_unsigned(IMEM_SIZE, 16)); -- size in bytes

  -- CPUID4: reserved --
  sysinfo_mem(4) <= (others => '0');

  -- CPUID5: DMEM (RAM) size --
  sysinfo_mem(5) <= std_ulogic_vector(to_unsigned(DMEM_SIZE, 16)); -- size in bytes

  -- CPUID6/CPUID7: Clock speed --
  f_clk <= std_ulogic_vector(to_unsigned(CLOCK_SPEED, 32));
  sysinfo_mem(6) <= f_clk(15 downto 00); -- clock speed LO
  sysinfo_mem(7) <= f_clk(31 downto 16); -- clock speed HI


  -- Read Access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  read_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (rden = '1') then
        data_o <= sysinfo_mem(to_integer(unsigned(info_addr)));
      else
        data_o <= (others => '0');
      end if;
    end if;
  end process read_access;


end neo430_sysconfig_rtl;
