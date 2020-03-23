-- #################################################################################################
-- #  << NEO430 - Arithmetical/Logical Unit >>                                                     #
-- # ********************************************************************************************* #
-- # Main data processing ALU and operand registers. DADD instruction is not supported!            #
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

entity neo430_alu is
  port (
    -- global control --
    clk_i  : in  std_ulogic; -- global clock, rising edge
    -- operands --
    reg_i  : in  std_ulogic_vector(15 downto 0); -- data from reg file
    mem_i  : in  std_ulogic_vector(15 downto 0); -- data from memory
    sreg_i : in  std_ulogic_vector(15 downto 0); -- current SR
    -- control --
    ctrl_i : in  std_ulogic_vector(ctrl_width_c-1 downto 0);
    -- results --
    data_o : out std_ulogic_vector(15 downto 0); -- result
    flag_o : out std_ulogic_vector(04 downto 0)  -- new ALU flags
  );
end neo430_alu;

architecture neo430_alu_rtl of neo430_alu is

  signal op_data  : std_ulogic_vector(15 downto 0); -- operand data
  signal op_a_ff  : std_ulogic_vector(15 downto 0); -- operand register A
  signal op_b_ff  : std_ulogic_vector(15 downto 0); -- operand register B
  signal add_res  : std_ulogic_vector(17 downto 0); -- adder/subtractor kernel result
  signal alu_res  : std_ulogic_vector(15 downto 0); -- alu result
  signal data_res : std_ulogic_vector(15 downto 0); -- final alu result
  signal zero     : std_ulogic; -- zero detector
  signal negative : std_ulogic; -- sign detector
  signal parity   : std_ulogic; -- parity detector

begin

  -- Input Operand Selection --------------------------------------------------
  -- -----------------------------------------------------------------------------
  op_data <= reg_i when (ctrl_i(ctrl_alu_in_sel_c) = '0') else mem_i;-- when (ctrl_i(ctrl_alu_bw_c) = '0') else (x"00" & mem_i(7 downto 0));


  -- Operand Registers --------------------------------------------------------
  -- -----------------------------------------------------------------------------
  operand_register: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- operand registers --
      if (ctrl_i(ctrl_alu_opa_wr_c) = '1') then
        op_a_ff <= op_data;
      end if;
      if (ctrl_i(ctrl_alu_opb_wr_c) = '1') then
        op_b_ff <= op_data;
      end if;
    end if;
  end process operand_register;


  -- Binary Arithmetic Core ---------------------------------------------------
  -- -----------------------------------------------------------------------------
  binary_arithmetic_core: process(ctrl_i, op_a_ff, op_b_ff, sreg_i)
    variable op_a_v             : std_ulogic_vector(15 downto 0);
    variable carry_v            : std_ulogic;
    variable carry_null_v       : std_ulogic;
    variable a_lo_v, a_hi_v     : std_ulogic_vector(8 downto 0);
    variable b_lo_v, b_hi_v     : std_ulogic_vector(8 downto 0);
    variable add_lo_v, add_hi_v : std_ulogic_vector(8 downto 0);
    variable carry_in_v         : std_ulogic_vector(0 downto 0);
    variable ova_16_v, ova_8_v  : std_ulogic;
  begin
    -- add/sub control (for operand A= --
    if (ctrl_i(ctrl_alu_cmd3_c downto ctrl_alu_cmd0_c) = alu_add_c) or 
       (ctrl_i(ctrl_alu_cmd3_c downto ctrl_alu_cmd0_c) = alu_addc_c) then -- addition
      op_a_v       := op_a_ff;
      carry_null_v := '0';
    else -- subtraction
      op_a_v       := not op_a_ff;
      carry_null_v := '1';
    end if;

    -- carry input --
    if (ctrl_i(ctrl_alu_cmd3_c downto ctrl_alu_cmd0_c) = alu_addc_c) or 
       (ctrl_i(ctrl_alu_cmd3_c downto ctrl_alu_cmd0_c) = alu_subc_c) then -- use carry in
      carry_in_v(0) := sreg_i(sreg_c_c);
    else
      carry_in_v(0) := carry_null_v; -- set default NO ACTIVE CARRY input
    end if;

    -- operands --
    a_lo_v := '0' & op_a_v(07 downto 0);
    a_hi_v := '0' & op_a_v(15 downto 8);
    b_lo_v := '0' & op_b_ff(07 downto 0);
    b_hi_v := '0' & op_b_ff(15 downto 8);

    -- adder core --
    add_lo_v := std_ulogic_vector(unsigned(a_lo_v) + unsigned(b_lo_v) + unsigned(carry_in_v(0 downto 0)));
    add_hi_v := std_ulogic_vector(unsigned(a_hi_v) + unsigned(b_hi_v) + unsigned(add_lo_v(8 downto 8)));

    -- overflow logic for the actual ADDER CORE (thx Edward!): plus + plus = minus || minus + minus = plus --
    ova_16_v := ((not op_a_v(15)) and (not op_b_ff(15)) and add_hi_v(7)) or (op_a_v(15) and op_b_ff(15) and (not add_hi_v(7)));
    ova_8_v  := ((not op_a_v(7))  and (not op_b_ff(7))  and add_lo_v(7)) or (op_a_v(7)  and op_b_ff(7)  and (not add_lo_v(7)));

    -- output --
    add_res(15 downto 0) <= add_hi_v(7 downto 0) & add_lo_v(7 downto 0); -- result
    if (ctrl_i(ctrl_alu_bw_c) = '1') then -- byte mode flags
      add_res(16) <= add_lo_v(8);
      add_res(17) <= ova_8_v;
    else -- word mode flags
      add_res(16) <= add_hi_v(8);
      add_res(17) <= ova_16_v;
    end if;
  end process binary_arithmetic_core;


  -- ALU Core -----------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  alu_core: process(ctrl_i, op_a_ff, op_b_ff, sreg_i, negative, zero, parity, add_res)
  begin
    -- defaults --
    alu_res <= op_a_ff;
    flag_o(flag_c_c) <= sreg_i(sreg_c_c); -- keep
    flag_o(flag_v_c) <= sreg_i(sreg_v_c); -- keep
    flag_o(flag_n_c) <= negative; -- update
    flag_o(flag_z_c) <= zero; -- update
    flag_o(flag_p_c) <= parity; -- update

    -- function selection --
    case ctrl_i(ctrl_alu_cmd3_c downto ctrl_alu_cmd0_c) is
      when alu_add_c | alu_addc_c | alu_sub_c | alu_subc_c | alu_cmp_c =>
        -- alu_add_c  : r <= a + b
        -- alu_addc_c : r <= a + b + c
        -- alu_sub_c  : r <= b - a
        -- alu_subc_c : r <= b - a - 1 + c
        -- alu_cmp_c  : b - a (no write back, done by ctrl arbiter)
        alu_res <= add_res(15 downto 0);
        flag_o(flag_c_c) <= add_res(16);
        flag_o(flag_v_c) <= add_res(17);

      when alu_and_c => -- r <= a & b
        alu_res <= op_a_ff and op_b_ff;
        flag_o(flag_c_c) <= not zero;
        flag_o(flag_v_c) <= '0';

      when alu_xor_c => -- r <= a xor b
        alu_res <= op_a_ff xor op_b_ff;
        flag_o(flag_c_c) <= not zero;
        flag_o(flag_v_c) <= op_a_ff(15) and op_b_ff(15); -- word mode
        if (ctrl_i(ctrl_alu_bw_c) = '1') then -- byte mode
          flag_o(flag_v_c) <= op_a_ff(7) and op_b_ff(7);
        end if;

      when alu_bic_c => -- r <= !a & b
        alu_res <= (not op_a_ff) and op_b_ff;
        flag_o(flag_c_c) <= sreg_i(sreg_c_c); -- keep
        flag_o(flag_v_c) <= sreg_i(sreg_v_c); -- keep
        flag_o(flag_n_c) <= sreg_i(sreg_n_c); -- keep
        flag_o(flag_z_c) <= sreg_i(sreg_z_c); -- keep

      when alu_bis_c => -- r <= a | b
        alu_res <= op_a_ff or op_b_ff;
        flag_o(flag_c_c) <= sreg_i(sreg_c_c); -- keep
        flag_o(flag_v_c) <= sreg_i(sreg_v_c); -- keep
        flag_o(flag_n_c) <= sreg_i(sreg_n_c); -- keep
        flag_o(flag_z_c) <= sreg_i(sreg_z_c); -- keep

      when alu_bit_c => -- r <= a & b (no write back, done by ctrl arbiter)
        alu_res <= op_a_ff and op_b_ff;
        flag_o(flag_c_c) <= not zero;
        flag_o(flag_v_c) <= '0';

      when alu_rra_c | alu_rrc_c =>
        -- alu_rra_c : r <= a >> 1, rotate right arithmetically
        -- alu_rrc_c : r <= a >> 1, rotate right through carry
        if (ctrl_i(ctrl_alu_cmd1_c) = alu_rra_c(1)) then -- alu_rra_c
          alu_res <= op_a_ff(15) & op_a_ff(15 downto 1); -- word mode
          if (ctrl_i(ctrl_alu_bw_c) = '1') then -- byte mode
            alu_res(7) <= op_a_ff(7);
          end if;
        else -- alu_rrc_c
          alu_res <= sreg_i(sreg_c_c) & op_a_ff(15 downto 1); -- word mode
          if (ctrl_i(ctrl_alu_bw_c) = '1') then -- byte mode
            alu_res(7) <= sreg_i(sreg_c_c);
          end if;
        end if;
        flag_o(flag_c_c) <= op_a_ff(0);
        flag_o(flag_v_c) <= '0';

      when alu_sxt_c => -- r <= a, sign extend byte
        for i in 8 to 15 loop
          alu_res(i) <= op_a_ff(7);
        end loop;
        alu_res(7 downto 0) <= op_a_ff(7 downto 0);
        flag_o(flag_c_c) <= not zero;
        flag_o(flag_v_c) <= '0';

      when alu_swap_c => -- r <= swap bytes of a
        alu_res <= op_a_ff(7 downto 0) & op_a_ff(15 downto 8);
        flag_o(flag_c_c) <= sreg_i(sreg_c_c); -- keep
        flag_o(flag_v_c) <= sreg_i(sreg_v_c); -- keep
        flag_o(flag_n_c) <= sreg_i(sreg_n_c); -- keep
        flag_o(flag_z_c) <= sreg_i(sreg_z_c); -- keep

      when alu_mov_c => -- r <= a
        alu_res <= op_a_ff;
        flag_o(flag_c_c) <= sreg_i(sreg_c_c); -- keep
        flag_o(flag_v_c) <= sreg_i(sreg_v_c); -- keep
        flag_o(flag_n_c) <= sreg_i(sreg_n_c); -- keep
        flag_o(flag_z_c) <= sreg_i(sreg_z_c); -- keep

      when others => -- undefined
        alu_res <= (others => '-');
        flag_o(flag_c_c) <= '-';
        flag_o(flag_v_c) <= '-';
        flag_o(flag_n_c) <= '-';
        flag_o(flag_z_c) <= '-';
        flag_o(flag_p_c) <= '-';

    end case;
  end process alu_core;


  -- Post processing logic ----------------------------------------------------
  -- -----------------------------------------------------------------------------

  -- word/byte mode mask --
  data_res(07 downto 0) <= alu_res(07 downto 0);
  data_res(15 downto 8) <= alu_res(15 downto 8) when (ctrl_i(ctrl_alu_bw_c) = '0') else x"00";

  -- zero flag --
  zero <= not or_all_f(data_res);

  -- parity flag --
  parity <= (not xor_all_f(data_res)) when (use_xalu_c = true) else '-'; -- if implemented

  -- negative flag --
  negative <= data_res(7) when (ctrl_i(ctrl_alu_bw_c) = '1') else data_res(15);

  -- final data output --
  data_o <= data_res;


end neo430_alu_rtl;
