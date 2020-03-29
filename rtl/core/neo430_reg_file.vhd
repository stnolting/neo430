-- #################################################################################################
-- #  << NEO430 - CPU Register File >>                                                             #
-- # ********************************************************************************************* #
-- # General data registers, program counter, status register and constant generator.              #
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

entity neo430_reg_file is
  generic (
    BOOTLD_USE  : boolean := true; -- implement and use bootloader?
    IMEM_AS_ROM : boolean := false -- implement IMEM as read-only memory?
  );
  port (
    -- global control --
    clk_i  : in  std_ulogic; -- global clock, rising edge
    rst_i  : in  std_ulogic; -- global reset, low-active, async
    -- data input --
    alu_i  : in  std_ulogic_vector(15 downto 0); -- data from alu
    addr_i : in  std_ulogic_vector(15 downto 0); -- data from addr unit
    flag_i : in  std_ulogic_vector(04 downto 0); -- new ALU flags
    -- control --
    ctrl_i : in  std_ulogic_vector(ctrl_width_c-1 downto 0);
    -- data output --
    data_o : out std_ulogic_vector(15 downto 0); -- read data
    sreg_o : out std_ulogic_vector(15 downto 0)  -- current SR
  );
end neo430_reg_file;

architecture neo430_reg_file_rtl of neo430_reg_file is

  -- boot address for PC --
  -- boot from beginning of boot ROM (boot_base_c) if bootloader is used, otherwise boot from beginning of IMEM (imem_base_c)
  -- By not using a reset-like init of the PC, the whole register file (except for SR and CG)
  -- can be mapped to distributed RAM saving logic resources
  constant pc_boot_addr_c : std_ulogic_vector(15 downto 0) := cond_sel_stdulogicvector_f(BOOTLD_USE, boot_base_c, imem_base_c);

  -- register file (including dummy regs) --
  type   reg_file_t is array (15 downto 0) of std_ulogic_vector(15 downto 0);
  signal reg_file : reg_file_t;
  signal sreg     : std_ulogic_vector(15 downto 0);
  signal sreg_int : std_ulogic_vector(15 downto 0);

  --- RAM attribute to inhibit bypass-logic - Altera only! ---
  attribute ramstyle : string;
  attribute ramstyle of reg_file : signal is "no_rw_check";

  -- misc --
  signal in_data : std_ulogic_vector(15 downto 0); -- input selection

begin

  -- Input Operand Selection --------------------------------------------------
  -- -----------------------------------------------------------------------------
  in_data <= pc_boot_addr_c when (ctrl_i(ctrl_rf_boot_c)   = '1') else
             addr_i         when (ctrl_i(ctrl_rf_in_sel_c) = '1') else alu_i;


  -- Register File Write Access -----------------------------------------------
  -- -----------------------------------------------------------------------------
  sreg_write: process(rst_i, clk_i)
  begin
    if (rst_i = '0') then
      sreg <= (others => '0'); -- here we NEED a true hardware reset
    elsif rising_edge(clk_i) then
      -- physical status register --
      if ((ctrl_i(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) = reg_sr_c) and (ctrl_i(ctrl_rf_wb_en_c) = '1')) then -- valid SREG write
        sreg(sreg_c_c) <= in_data(sreg_c_c);
        sreg(sreg_z_c) <= in_data(sreg_z_c);
        sreg(sreg_n_c) <= in_data(sreg_n_c);
        sreg(sreg_i_c) <= in_data(sreg_i_c);
        sreg(sreg_s_c) <= in_data(sreg_s_c);
        sreg(sreg_v_c) <= in_data(sreg_v_c);
        sreg(sreg_q_c) <= in_data(sreg_q_c);
        if (use_xalu_c = true) then -- implement parity computation?
          sreg(sreg_p_c) <= in_data(sreg_p_c);
        end if;
        if (IMEM_AS_ROM = false) then -- r-flag is 0 when IMEM is ROM
          sreg(sreg_r_c) <= in_data(sreg_r_c);
        end if;
      else -- automatic update
        sreg(sreg_q_c) <= '0'; -- auto-clear
        -- disable sleep mode --
        if (ctrl_i(ctrl_rf_dsleep_c) = '1') then
          sreg(sreg_s_c) <= '0';
        end if;
        -- disable interrupt enable --
        if (ctrl_i(ctrl_rf_dgie_c) = '1') then
          sreg(sreg_i_c) <= '0';
        end if;
         -- update ALU flags --
        if (ctrl_i(ctrl_rf_fup_c) = '1') then
          sreg(sreg_c_c) <= flag_i(flag_c_c);
          sreg(sreg_z_c) <= flag_i(flag_z_c);
          sreg(sreg_n_c) <= flag_i(flag_n_c);
          sreg(sreg_v_c) <= flag_i(flag_v_c);
          if (use_xalu_c = true) then -- implement parity computation?
            sreg(sreg_p_c) <= flag_i(flag_p_c);
          end if;
        end if;
      end if;
    end if;
  end process sreg_write;

  -- construct logical status register --
  sreg_combine: process(sreg)
  begin
    -- SREG for system --
    sreg_o <= (others => '0');
    sreg_o(sreg_c_c) <= sreg(sreg_c_c);
    sreg_o(sreg_z_c) <= sreg(sreg_z_c);
    sreg_o(sreg_n_c) <= sreg(sreg_n_c);
    sreg_o(sreg_i_c) <= sreg(sreg_i_c);
    sreg_o(sreg_s_c) <= sreg(sreg_s_c);
    sreg_o(sreg_v_c) <= sreg(sreg_v_c);
    sreg_o(sreg_q_c) <= sreg(sreg_q_c);
    sreg_o(sreg_r_c) <= sreg(sreg_r_c);
    if (use_xalu_c = true) then -- implement parity computation?
      sreg_o(sreg_p_c) <= sreg(sreg_p_c);
    end if;
    -- SREG for user --
    sreg_int <= (others => '0');
    sreg_int(sreg_c_c) <= sreg(sreg_c_c);
    sreg_int(sreg_z_c) <= sreg(sreg_z_c);
    sreg_int(sreg_n_c) <= sreg(sreg_n_c);
    sreg_int(sreg_i_c) <= sreg(sreg_i_c);
    sreg_int(sreg_s_c) <= sreg(sreg_s_c);
    sreg_int(sreg_v_c) <= sreg(sreg_v_c);
  --sreg_int(sreg_q_c) <= sreg(sreg_q_c); -- is always zero for user
    sreg_int(sreg_r_c) <= sreg(sreg_r_c);
    if (use_xalu_c = true) then -- implement parity computation?
      sreg_int(sreg_p_c) <= sreg(sreg_p_c);
    end if;
  end process sreg_combine;

  -- general purpose register file (including PC, SP, dummy SR and dummy CG) --
  rf_write: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (ctrl_i(ctrl_rf_wb_en_c) = '1') then -- valid register file write
        reg_file(to_integer(unsigned(ctrl_i(ctrl_rf_adr3_c downto ctrl_rf_adr0_c)))) <= in_data;
      end if;
    end if;
  end process rf_write;


  -- Register File Read Access ------------------------------------------------
  -- -----------------------------------------------------------------------------
  rf_read: process(ctrl_i, reg_file, sreg_int)
    variable const_sel_v : std_ulogic_vector(2 downto 0);
  begin
    if ((ctrl_i(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) = reg_sr_c) or
        (ctrl_i(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) = reg_cg_c)) then
      -- constant generator / SR read access --
      const_sel_v := ctrl_i(ctrl_rf_adr0_c) & ctrl_i(ctrl_rf_as1_c) & ctrl_i(ctrl_rf_as0_c);
      case const_sel_v is
        when "000"  => data_o <= sreg_int; -- read SR
        when "001"  => data_o <= x"0000"; -- absolute addressing mode
        when "010"  => data_o <= x"0004"; -- +4
        when "011"  => data_o <= x"0008"; -- +8
        when "100"  => data_o <= x"0000"; --  0
        when "101"  => data_o <= x"0001"; -- +1
        when "110"  => data_o <= x"0002"; -- +2
        when "111"  => data_o <= x"FFFF"; -- -1
        when others => data_o <= (others => '-');
      end case;
    else -- gp register file read access
      data_o <= reg_file(to_integer(unsigned(ctrl_i(ctrl_rf_adr3_c downto ctrl_rf_adr0_c))));
    end if;
  end process rf_read;


end neo430_reg_file_rtl;
