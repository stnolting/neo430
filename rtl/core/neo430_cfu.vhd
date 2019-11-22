-- #################################################################################################
-- #  << NEO430 - Custom Functions Unit >>                                                         #
-- # ********************************************************************************************* #
-- # This unit is a template for implementing custom functions, which are directly memory-mapped   #
-- # into the CPU's IO address space. The address space of this unit is 16 bytes large. This unit  #
-- # can only be accessed using full word (16-bit) accesses.                                       #
-- # In the original state, this unit only provides 8 16-bit register, that do not perform any     #
-- # kind of data manipulation.                                                                    #
-- # Exemplary applications: Cryptography, complex arithmetic, rocket science, ...                 #
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
-- # Stephan Nolting, Hannover, Germany                                                 22.11.2019 #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;

entity neo430_cfu is
  port (
    -- host access --
    clk_i  : in  std_ulogic; -- global clock line
    rden_i : in  std_ulogic; -- read enable
    wren_i : in  std_ulogic; -- write enable
    addr_i : in  std_ulogic_vector(15 downto 0); -- address
    data_i : in  std_ulogic_vector(15 downto 0); -- data in
    data_o : out std_ulogic_vector(15 downto 0)  -- data out
    -- custom IOs --
--  ...
  );
end neo430_cfu;

architecture neo430_cfu_rtl of neo430_cfu is

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size_f(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size_f(cfu_size_c); -- low address boundary bit

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal addr   : std_ulogic_vector(15 downto 0); -- access address
  signal wren   : std_ulogic; -- full word write enable
  signal rden   : std_ulogic; -- read enable

  -- accessible regs (8x16-bit) --
  signal user_reg0 : std_ulogic_vector(15 downto 0);
  signal user_reg1 : std_ulogic_vector(15 downto 0);
  signal user_reg2 : std_ulogic_vector(15 downto 0);
  signal user_reg3 : std_ulogic_vector(15 downto 0);
  signal user_reg4 : std_ulogic_vector(15 downto 0);
  signal user_reg5 : std_ulogic_vector(15 downto 0);
  signal user_reg6 : std_ulogic_vector(15 downto 0);
  signal user_reg7 : std_ulogic_vector(15 downto 0);

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  -- These assignments are required to check if this unit is accessed at all.
  -- Do NOT modify this for your custom application (unless you really know what you are doing)!
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = cfu_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= cfu_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wren   <= acc_en and wren_i;
  rden   <= acc_en and rden_i;


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  -- Here we are writing to the interface registers of the module. This unit can only be accessed
  -- in full 16-bit word mode!
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- write access to user registers --
      if (wren = '1') then -- valid write access
        if (addr = cfu_reg0_addr_c) then user_reg0 <= data_i; end if;
        if (addr = cfu_reg1_addr_c) then user_reg1 <= data_i; end if;
        if (addr = cfu_reg2_addr_c) then user_reg2 <= data_i; end if;
        if (addr = cfu_reg3_addr_c) then user_reg3 <= data_i; end if;
        if (addr = cfu_reg4_addr_c) then user_reg4 <= data_i; end if;
        if (addr = cfu_reg5_addr_c) then user_reg5 <= data_i; end if;
        if (addr = cfu_reg6_addr_c) then user_reg6 <= data_i; end if;
        if (addr = cfu_reg7_addr_c) then user_reg7 <= data_i; end if;
      end if;
    end if;
  end process wr_access;

  -- >>> UNIT HARDWARE RESET <<< --
  -- The IO devices DO NOT feature a dedicated reset signal, so make sure your CFU does not require a defined initial state.
  -- If you really require a defined initial state, implement a software reset by implementing a control register with an
  -- enable bit, which resets all internal states when cleared.


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  -- This is the read access process. Data must be asserted synchronously to the output data bus
  -- and thus, with exactly 1 cycle delay. The units always output a full 16-bit word, no matter if we want to
  -- read 8- or 16-bit. For actual 8-bit read accesses the corresponding byte is selected in the
  -- hardware of the CPU core.
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0'); -- this is crucial for the final OR-ing of all IO device's outputs
      if (rden = '1') then -- valid read access
        case addr is
          when cfu_reg0_addr_c => data_o <= user_reg0;
          when cfu_reg1_addr_c => data_o <= user_reg1;
          when cfu_reg2_addr_c => data_o <= user_reg2;
          when cfu_reg3_addr_c => data_o <= user_reg3;
          when cfu_reg4_addr_c => data_o <= user_reg4;
          when cfu_reg5_addr_c => data_o <= user_reg5;
          when cfu_reg6_addr_c => data_o <= user_reg6;
          when cfu_reg7_addr_c => data_o <= user_reg7;
          when others          => data_o <= (others => '-');
        end case;
      end if;
    end if;
  end process rd_access;


end neo430_cfu_rtl;
