-- #################################################################################################
-- #  << NEO430 - 16-Bit Unsigned Multiplier & Divider Unit >>                                     #
-- # ********************************************************************************************* #
-- # NOTE: This unit uses "repeated trial subtraction" as division algorithm (restoring).          #
-- # NOTE: This unit uses "shifted add" as multiplication algorithm. Set 'use_dsp_mul_c' in the    #
-- # package file to TRUE to use DSP slices for multiplication.                                    #
-- #                                                                                               #
-- # The division unit only supports unsigned divisions.                                           #
-- # The multiplication unit supports signed and unsigned division.                                #
-- #                                                                                               #
-- # Division: DIVIDEND / DIVIDER = QUOTIENT + REMAINDER (16-bit) / DIVIDER (16-bit)               #
-- # Multiplication: FACTOR1 * FACTOR2 = PRODUCT (32-bit)                                          #
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
  signal rd_en  : std_ulogic;

  -- accessible regs --
  signal opa, opb   : std_ulogic_vector(15 downto 0);
  signal resx, resy : std_ulogic_vector(15 downto 0);
  signal operation  : std_ulogic; -- '1' division, '0' multiplication
  signal signed_op  : std_ulogic;

  -- arithmetic core & arbitration --
  signal start       : std_ulogic;
  signal run         : std_ulogic;
  signal enable      : std_ulogic_vector(15 downto 0);
  signal try_sub     : std_ulogic_vector(16 downto 0);
  signal remainder   : std_ulogic_vector(15 downto 0);
  signal quotient    : std_ulogic_vector(15 downto 0);
  signal product     : std_ulogic_vector(31 downto 0);
  signal do_add      : std_ulogic_vector(16 downto 0);
  signal sign_cycle  : std_ulogic;
  signal opa_sext    : std_ulogic;
  signal opb_sext    : std_ulogic;
  signal p_sext      : std_ulogic;
  signal dsp_mul_res : std_ulogic_vector(33 downto 0);

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = muldiv_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= muldiv_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wr_en  <= acc_en and wren_i;
  rd_en  <= acc_en and rden_i;


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      start    <= '0';
      opa_sext <= opa(opa'left) and signed_op;
      opb_sext <= opb(opb'left) and signed_op;
      if (wr_en = '1') then -- only full word accesses!
        -- operands --
        if (addr = muldiv_opa_resx_addr_c) then -- dividend or factor 1
          opa <= data_i;
        end if;
        if (addr = muldiv_opb_umul_resy_addr_c) or
           (addr = muldiv_opb_smul_addr_c) or
           (addr = muldiv_opb_udiv_addr_c) then -- divisor or factor 2
          opb   <= data_i;
          start <= '1'; -- trigger operation
        end if;
        -- operation: division/multiplication --
        if (addr = muldiv_opb_umul_resy_addr_c) or (addr = muldiv_opb_smul_addr_c) then -- multiplication
          operation <= '0';
        else -- division
          operation <= '1';
        end if;
        -- signed/unsigned operation --
        if (addr = muldiv_opb_smul_addr_c) then
          signed_op <= '1';
        else
          signed_op <= '0';
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
            product(31 downto 16) <= (others => opa_sext);
            product(15 downto  0) <= opa;
          elsif (run = '1') then
            product(31 downto 15) <= do_add(16 downto 0);
            product(14 downto  0) <= product(15 downto 1);
          end if;
        else -- use DSP for multiplication
          product(31 downto 0) <= dsp_mul_res(31 downto 0);
        end if;
      end if;
    end if;
  end process arithmetic_core;

  -- DSP multiplication --
  dsp_mul_res <= std_ulogic_vector(signed(opa_sext & opa) * signed(opb_sext & opb));

  -- DIV: try another subtraction --
  try_sub <= std_ulogic_vector(unsigned('0' & remainder(14 downto 0) & quotient(15)) - unsigned('0' & opb));

  -- MUL: do another addition --
  mul_update: process(product, sign_cycle, p_sext, opb_sext, opb)
  begin
    if (product(0) = '1') then
      if (sign_cycle = '1') then -- for signed operation only: take care of negative weighted MSB
        do_add <= std_ulogic_vector(unsigned(p_sext & product(31 downto 16)) - unsigned(opb_sext & opb));
      else
        do_add <= std_ulogic_vector(unsigned(p_sext & product(31 downto 16)) + unsigned(opb_sext & opb));
      end if;
    else
      do_add <= p_sext & product(31 downto 16);
    end if;
  end process mul_update;

  sign_cycle <= enable(enable'left) and signed_op;
  p_sext     <= product(product'left) and signed_op;


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0');
      if (rd_en = '1') then -- valid read access
        if (addr = muldiv_opa_resx_addr_c) then
          data_o <= resx; -- quotient or product low word
        else -- muldiv_opb_umul_resy_addr_c =>
          data_o <= resy; -- remainder or product high word
        end if;
      end if;
    end if;
  end process rd_access;

  -- result selection --
  resx <= product(15 downto  0) when (operation = '0') else quotient;
  resy <= product(31 downto 16) when (operation = '0') else remainder;


end neo430_muldiv_rtl;
