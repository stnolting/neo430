-- #################################################################################################
-- #  << NEO430 - Address Generator Unit >>                                                        #
-- # ********************************************************************************************* #
-- # Address computation and memory address register (MAR).                                        #
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
      else -- direct output of reg file
        mem_addr_o <= reg_i;
      end if;
    else
      mem_addr_o <= mem_addr_reg;
    end if;
  end process memory_addr_out;


end neo430_addr_gen_rtl;
