-- #################################################################################################
-- #  << NEO430 - Address Generator Unit >>                                                        #
-- # ********************************************************************************************* #
-- # Address computation and memory address register (MAR).                                        #
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
-- # Stephan Nolting, Hannover, Germany                                                 14.11.2019 #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;

entity neo430_addr_gen is
  port (
    -- global control --
    clk_i      : in  std_ulogic; -- global clock, rising edge
    -- data input --
    reg_i      : in  std_ulogic_vector(15 downto 0); -- reg file input
    mem_i      : in  std_ulogic_vector(15 downto 0); -- memory input
    imm_i      : in  std_ulogic_vector(15 downto 0); -- branch offset
    irq_sel_i  : in  std_ulogic_vector(01 downto 0); -- IRQ vector
    -- control --
    ctrl_i     : in  std_ulogic_vector(ctrl_width_c-1 downto 0);
    -- data output --
    mem_addr_o : out std_ulogic_vector(15 downto 0); -- memory address
    dwb_o      : out std_ulogic_vector(15 downto 0)  -- data write back output
  );
end neo430_addr_gen;

architecture neo430_addr_gen_rtl of neo430_addr_gen is

  signal mem_addr_reg : std_ulogic_vector(15 downto 0); -- memory address register
  signal addr_add     : std_ulogic_vector(15 downto 0); -- result from address adder

begin

  -- Memory Address Adder -----------------------------------------------------
  -- -----------------------------------------------------------------------------
  memory_addr_adder: process(ctrl_i, mem_i, imm_i, reg_i)
    variable offset_v : std_ulogic_vector(15 downto 0);
  begin
    case ctrl_i(ctrl_adr_off2_c downto ctrl_adr_off0_c) is
      when "000"  => offset_v := imm_i;
      when "001"  => offset_v := x"0001"; -- +1
      when "010"  => offset_v := x"0002"; -- +2
      when "011"  => offset_v := x"FFFE"; -- -2
      when others => offset_v := mem_i;
    end case;
    addr_add <= std_ulogic_vector(unsigned(reg_i) + unsigned(offset_v));
  end process memory_addr_adder;

  -- output for write back --
  dwb_o <= addr_add;


  -- Memory Address Register --------------------------------------------------
  -- -----------------------------------------------------------------------------
  memory_addr_reg: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (ctrl_i(ctrl_adr_mar_wr_c) = '1') then
        if (ctrl_i(ctrl_adr_mar_sel_c) = '0') then
          mem_addr_reg <= reg_i;
        else
          mem_addr_reg <= addr_add;
        end if;
      end if;
    end if;
  end process memory_addr_reg;


  -- Memory Address Output ----------------------------------------------------
  -- -----------------------------------------------------------------------------
  memory_addr_out: process(ctrl_i, irq_sel_i, reg_i, mem_addr_reg)
  begin
    if (ctrl_i(ctrl_adr_bp_en_c) = '1') then
      if (ctrl_i(ctrl_adr_ivec_oe_c) = '1') then -- interrupt handler call
        mem_addr_o <= dmem_base_c; -- IRQ vectors are located at the beginning of DMEM
        mem_addr_o(2 downto 0) <= irq_sel_i & '0'; -- select according word-aligned entry
      else -- direct output of reg file (for instruction fetch only)
        mem_addr_o <= reg_i(15 downto 1) & '0'; -- instructions have to be word-aligned
      end if;
    else
      mem_addr_o <= mem_addr_reg;
    end if;
  end process memory_addr_out;


end neo430_addr_gen_rtl;
