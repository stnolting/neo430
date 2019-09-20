-- #################################################################################################
-- #  << NEO430 - 16-Bit Unsigned Multiplier & Divider Unit >>                                     #
-- # ********************************************************************************************* #
-- # NOTE: This unit uses "repeated trial subtraction" as division algorithm.                      #
-- # NOTE: This unit uses "shifted add" as multiplication algorithm. Set 'use_dsp_mul_c' in the    #
-- # package file to TRUE to use DSP slices for multiplication.                                    #
-- # Division: DIVIDEND / DIVIDER = QUOTIENT + REMAINDER (16-bit) / DIVIDER (16-bit)               #
-- # Multiplication: FACTOR1 * FACTOR2 = PRODUCT (32-bit)                                          #
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
-- # Stephan Nolting, Hannover, Germany                                                 29.04.2019 #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;

entity neo430_muldiv is
  port (
    -- host access --
    clk_i  : in  std_ulogic; -- global clock line
    rden_i : in  std_ulogic; -- read enable
    wren_i : in  std_ulogic; -- write enable
    addr_i : in  std_ulogic_vector(15 downto 0); -- address
    data_i : in  std_ulogic_vector(15 downto 0); -- data in
    data_o : out std_ulogic_vector(15 downto 0)  -- data out
  );
end neo430_muldiv;

architecture neo430_muldiv_rtl of neo430_muldiv is

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size_f(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size_f(muldiv_size_c); -- low address boundary bit

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal addr   : std_ulogic_vector(15 downto 0); -- access address
  signal wr_en  : std_ulogic; -- only full 16-bit word accesses!

  -- accessible regs --
  signal opa, opb   : std_ulogic_vector(15 downto 0);
  signal resx, resy : std_ulogic_vector(15 downto 0);
  signal operation  : std_ulogic;

  -- arithmetic core & arbitration --
  signal start     : std_ulogic;
  signal run       : std_ulogic;
  signal enable    : std_ulogic_vector(15 downto 0);
  signal try_sub   : std_ulogic_vector(16 downto 0);
  signal remainder : std_ulogic_vector(15 downto 0);
  signal quotient  : std_ulogic_vector(15 downto 0);
  signal product   : std_ulogic_vector(31 downto 0);
  signal do_add    : std_ulogic_vector(16 downto 0);

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = muldiv_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= muldiv_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wr_en  <= acc_en and wren_i;


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      start <= '0';
      if (wr_en = '1') then -- only full word accesses!
        -- operands --
        if (addr = muldiv_opa_addr_c) then -- dividend or factor 1
          opa <= data_i;
        end if;
        if (addr = muldiv_opb_div_addr_c) or (addr = muldiv_opb_mul_addr_c) then -- divisor or factor 2
          opb <= data_i;
          start <= '1'; -- trigger operation
        end if;
        -- operation: division/multiplication --
        if (addr = muldiv_opb_div_addr_c) then -- division
          operation <= '1';
        else -- multiplication
          operation <= '0';
        end if;
      end if;
    end if;
  end process wr_access;


  -- Arithmetic core ----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  arithmetic_core: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- arbitration --
      enable <= enable(14 downto 0) & start;
      if (start = '1') then
        run <= '1';
      elsif (enable(15) = '1') then -- all done?
        run <= '0';
      end if;

      -- division core --
      if (operation = '1') then
        if (start = '1') then -- load dividend
          quotient  <= opa;
          remainder <= (others => '0');
        elsif (run = '1') then
          quotient <= quotient(14 downto 0) & (not try_sub(16));
          if (try_sub(16) = '0') then -- still overflowing
            remainder <= try_sub(15 downto 0);
          else -- underflow
            remainder <= remainder(14 downto 0) & quotient(15);
          end if;
        end if;
      -- multiplication core --
      else
        if (use_dsp_mul_c = false) then -- implement serial multiplication
          if (start = '1') then -- load factor 1
            product(31 downto 16) <= (others => '0');
            product(15 downto  0) <= opa;
          elsif (run = '1') then
            product(31 downto 15) <= do_add(16 downto 0);
            product(14 downto  0) <= product(15 downto 1);
          end if;
        else -- use DSP for multiplication
          product(31 downto 0) <= std_ulogic_vector(unsigned(opa) * unsigned(opb));
        end if;
      end if;
    end if;
  end process arithmetic_core;

  -- DIV: try another subtraction --
  try_sub <= std_ulogic_vector(unsigned('0' & remainder(14 downto 0) & quotient(15)) - unsigned('0' & opb));

  -- MUL: do another addition
  do_add <= std_ulogic_vector(unsigned('0' & product(31 downto 16)) + unsigned('0' & opb)) when (product(0) = '1') else ('0' & product(31 downto 16));


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0');
      if (acc_en = '1') and (rden_i = '1') then -- valid read access
        if (addr = muldiv_resx_addr_c) then
          data_o <= resx; -- quotient or product low word
        else -- muldiv_resy_addr_c =>
          data_o <= resy; -- remainder or product high word
        end if;
      end if;
    end if;
  end process rd_access;

  -- result selection --
  resx <= product(15 downto  0) when (operation = '0') else quotient;
  resy <= product(31 downto 16) when (operation = '0') else remainder;


end neo430_muldiv_rtl;
