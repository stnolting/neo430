-- #################################################################################################
-- #  << NEO430 - 16x16=32-Bit Multiply/Acuumulate Unit >>                                         #
-- # ********************************************************************************************* #
-- #  NOTE: This unit is NOT fully compatible to the original TI MSP430 multiplier!                #
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
-- #  Stephan Nolting, Hannover, Germany                                               19.07.2016  #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.neo430_package.all;

entity neo430_cfu is
  port (
    -- host access --
    clk_i  : in  std_ulogic; -- global clock line
    rden_i : in  std_ulogic; -- read enable
    wren_i : in  std_ulogic_vector(01 downto 0); -- write enable
    addr_i : in  std_ulogic_vector(15 downto 0); -- address
    data_i : in  std_ulogic_vector(15 downto 0); -- data in
    data_o : out std_ulogic_vector(15 downto 0)  -- data out
  );
end neo430_cfu;

architecture neo430_cfu_mac16_rtl of neo430_cfu is

  -- interface register addresses --
  constant mac16_base_c : std_ulogic_vector(15 downto 0) := cfu_base_c;
  constant mac16_size_c : natural := cfu_size_c;

  constant mac16_mpy_addr_c    : std_ulogic_vector(15 downto 0) := cfu_reg0_addr_c; -- -/w: operand A for unsigned multiplication
  constant mac16_mpys_addr_c   : std_ulogic_vector(15 downto 0) := cfu_reg1_addr_c; -- -/w: operand A for signed multiplication
  constant mac16_mac_addr_c    : std_ulogic_vector(15 downto 0) := cfu_reg2_addr_c; -- -/w: operand A for unsigned dmultiply-and-add
  constant mac16_macs_addr_c   : std_ulogic_vector(15 downto 0) := cfu_reg3_addr_c; -- -/w: operand A for signed dmultiply-and-add
  constant mac16_op2_addr_c    : std_ulogic_vector(15 downto 0) := cfu_reg4_addr_c; -- -/w: operand B (for all operations)
  constant mac16_reslo_addr_c  : std_ulogic_vector(15 downto 0) := cfu_reg5_addr_c; -- r/-: low part of result
  constant mac16_reshi_addr_c  : std_ulogic_vector(15 downto 0) := cfu_reg6_addr_c; -- r/-: high part of result

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size(mac16_size_c); -- low address boundary bit

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal addr   : std_ulogic_vector(15 downto 0); -- access address
  signal wr_en  : std_ulogic; -- only full 16-bit word accesses!

  -- accessible regs --
  signal op_a, op_b : std_ulogic_vector(15 downto 0);
  signal mac_res    : std_ulogic_vector(32 downto 0);

  -- control --
  signal mode   : std_ulogic_vector(01 downto 0); -- function select
  signal run    : std_ulogic;
  signal run_s0 : std_ulogic;
  signal run_s1 : std_ulogic;

  -- mac core --
  signal op_a_int, op_a_ff : std_ulogic_vector(16 downto 0);
  signal op_b_int, op_b_ff : std_ulogic_vector(16 downto 0);
  signal mul_res           : std_ulogic_vector(33 downto 0);

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = mac16_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= mac16_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wr_en  <= acc_en and wren_i(0) and wren_i(1);


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      run <= '0';
      if (wr_en = '1') then -- only full word accesses!
        -- operands --
        case addr is
          when mac16_mpy_addr_c | mac16_mpys_addr_c | mac16_mac_addr_c | mac16_macs_addr_c => -- operand A
            op_a <= data_i;
          when mac16_op2_addr_c => -- operand B
            run  <= '1'; -- trigger operation
            op_b <= data_i;
          when others =>
            NULL;
        end case;
        -- operation --
        case (addr) is
          when mac16_mpy_addr_c =>
            mode <= "00"; -- multiply unsigned
          when mac16_mpys_addr_c =>
            mode <= "01"; -- multiply signed
          when mac16_mac_addr_c =>
            mode <= "10"; -- multiply-accumulate unsigned
          when mac16_macs_addr_c =>
            mode <= "11"; -- multiply-accumulate signed
          when others =>
            NULL;
        end case;
      end if;
    end if;
  end process wr_access;

  -- signed/unsigned mode --
  op_a_int <= (op_a(15) and mode(0)) & op_a(15 downto 0); -- final operand A (16-bit)
  op_b_int <= (op_b(15) and mode(0)) & op_b(15 downto 0); -- final operand B (16-bit)


  -- MAC core -----------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  mac_core: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- stage 0 --
      op_a_ff <= op_a_int;
      op_b_ff <= op_b_int;
      run_s0  <= run;
      -- stage 1 --
      mul_res <= std_ulogic_vector(signed(op_a_ff) * signed(op_b_ff));
      run_s1  <= run_s0;
      -- stage 2 --
      if (run_s1 = '1') then
        if (mode(1) = '1') then -- accumulate
          mac_res <= std_ulogic_vector(unsigned('0' & mac_res(31 downto 0)) + unsigned('0' & mul_res(31 downto 0)));
        else -- simple multiplication
          mac_res <= '0' & mul_res(31 downto 0);
        end if;
      end if;
    end if;
  end process mac_core;


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0');
      if (acc_en = '1') and (rden_i = '1') then -- valid read access
        if (addr = mac16_reslo_addr_c) then
          data_o <= mac_res(15 downto 00);
        else -- mac16_reshi_addr_c =>
            data_o <= mac_res(31 downto 16);
        end if;
      end if;
    end if;
  end process rd_access;


end neo430_cfu_mac16_rtl;
