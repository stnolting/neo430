-- #################################################################################################
-- #  << NEO430 - Bootloader ROM >>                                                                #
-- # ********************************************************************************************* #
-- # This memory includes the in-place executable image of the NEO430 bootloader. See the          #
-- # processor's documentary to get more information.                                              #
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
-- # Stephan Nolting, Hannover, Germany                                                 09.08.2018 #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;
use neo430.neo430_bootloader_image.all; -- this file is generated by the image generator

entity neo430_boot_rom is
  port (
    clk_i  : in  std_ulogic; -- global clock line
    rden_i : in  std_ulogic; -- read enable
    addr_i : in  std_ulogic_vector(15 downto 0); -- address
    data_o : out std_ulogic_vector(15 downto 0)  -- data out
  );
end neo430_boot_rom;

architecture neo430_boot_rom_rtl of neo430_boot_rom is

  -- local types --
  type boot_img_t is array (0 to boot_size_c/2-1) of std_ulogic_vector(15 downto 0);

  -- init function --
  impure function init_boot_rom(init : bootloader_init_image_t) return boot_img_t is
    variable mem_v : boot_img_t;
  begin
    for i in 0 to boot_size_c/2-1 loop
      mem_v(i) := init(i);
    end loop; -- i
    return mem_v;
  end function init_boot_rom;

  -- local signals --
  signal acc_en : std_ulogic;
  signal rden   : std_ulogic;
  signal rdata  : std_ulogic_vector(15 downto 0);
  signal addr   : natural range 0 to boot_size_c/2-1;

  -- bootloader image --
  constant boot_img : boot_img_t := init_boot_rom(bootloader_init_image);

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i >= boot_base_c) and (addr_i < std_ulogic_vector(unsigned(boot_base_c) + boot_size_c)) else '0';
  addr <= to_integer(unsigned(addr_i(index_size_f(boot_size_c/2) downto 1))); -- word aligned


  -- Memory Access ------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  mem_file_access: process(clk_i)
  begin
    -- check max size --
    if (boot_size_c > 2*1024) then
      assert false report "Boot ROM size out of range! Max 2kB!" severity error;
    end if;
    if rising_edge(clk_i) then
      rden <= rden_i and acc_en;
      if (acc_en = '1') then -- reduce switching activity when not accessed
        rdata <= boot_img(addr);
      end if;
    end if;
  end process mem_file_access;

  -- output gate --
  data_o <= rdata when (rden = '1') else (others => '0');


end neo430_boot_rom_rtl;
