-- #################################################################################################
-- #  << NEO430 - CPU Register File >>                                                             #
-- # ********************************************************************************************* #
-- #  General data registers, program counter, status register and constant generator.             #
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
-- #  Stephan Nolting, Hannover, Germany                                               11.11.2017  #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;

entity neo430_reg_file is
  generic (
    BOOTLD_USE : boolean := true -- implement and use bootloader?
  );
  port (
    -- global control --
    clk_i  : in  std_ulogic; -- global clock, rising edge
    rst_i  : in  std_ulogic; -- global reset, low-active, async
    -- data input --
    alu_i  : in  std_ulogic_vector(15 downto 0); -- data from alu
    addr_i : in  std_ulogic_vector(15 downto 0); -- data from addr unit
    flag_i : in  std_ulogic_vector(03 downto 0); -- new ALU flags
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
  constant pc_boot_addr_c : std_ulogic_vector(15 downto 0) := cond_sel_stdulogicvector(BOOTLD_USE, boot_base_c, imem_base_c);

  -- register file (including dummy regs) --
  type   reg_file_t is array (15 downto 0) of std_ulogic_vector(15 downto 0);
  signal reg_file : reg_file_t;
  signal sreg     : std_ulogic_vector(15 downto 0);

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
      -- status register --
      if ((ctrl_i(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) = reg_sr_c) and
          (ctrl_i(ctrl_rf_ad_c) = '0') and (ctrl_i(ctrl_rf_wb_en_c) = '1')) then -- only write in reg-addr-mode!
        sreg(sreg_c_c) <= in_data(sreg_c_c);
        sreg(sreg_z_c) <= in_data(sreg_z_c);
        sreg(sreg_n_c) <= in_data(sreg_n_c);
        sreg(sreg_i_c) <= in_data(sreg_i_c);
        sreg(sreg_s_c) <= in_data(sreg_s_c);
        sreg(sreg_v_c) <= in_data(sreg_v_c);
        sreg(sreg_q_c) <= in_data(sreg_q_c);
        sreg(sreg_r_c) <= in_data(sreg_r_c);
      else -- automatic update
        sreg(sreg_q_c) <= '0'; -- auto-clear
        if (ctrl_i(ctrl_rf_dsleep_c) = '1') then -- disable sleep mode
          sreg(sreg_s_c) <= '0';
        end if;
        if (ctrl_i(ctrl_rf_fup_c) = '1') then -- update ALU flags
          sreg(sreg_c_c) <= flag_i(flag_c_c);
          sreg(sreg_z_c) <= flag_i(flag_z_c);
          sreg(sreg_n_c) <= flag_i(flag_n_c);
          sreg(sreg_v_c) <= flag_i(flag_v_c);
        end if;
      end if;
    end if;
  end process sreg_write;

  -- status register output --
  sreg_o <= sreg;

  -- gp regs (including PC, dummy SR and dummy CG) --
  rf_write: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (ctrl_i(ctrl_rf_ad_c) = '0') and (ctrl_i(ctrl_rf_wb_en_c) = '1') then -- only write in reg-addr-mode!
        reg_file(to_integer(unsigned(ctrl_i(ctrl_rf_adr3_c downto ctrl_rf_adr0_c)))) <= in_data;
      end if;
    end if;
  end process rf_write;


  -- Register File Read Access ------------------------------------------------
  -- -----------------------------------------------------------------------------
  rf_read: process(ctrl_i, reg_file, sreg)
    variable const_sel_v : std_ulogic_vector(02 downto 0);
  begin
    if ((ctrl_i(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) = reg_sr_c) or
        (ctrl_i(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) = reg_cg_c)) then
      -- constant generator / SR read access --
      const_sel_v := ctrl_i(ctrl_rf_adr0_c) & ctrl_i(ctrl_rf_as1_c) & ctrl_i(ctrl_rf_as0_c);
      case const_sel_v is
        when "000"  => data_o <= sreg; -- read SR
        when "001"  => data_o <= x"0000"; -- absolute addressing mode
        when "010"  => data_o <= x"0004"; -- +4
        when "011"  => data_o <= x"0008"; -- +8
        when "100"  => data_o <= x"0000"; --  0
        when "101"  => data_o <= x"0001"; -- +1
        when "110"  => data_o <= x"0002"; -- +2
        when "111"  => data_o <= x"FFFF"; -- -1
        when others => data_o <= x"0000";
      end case;
    else
      -- sp / gp register file read access --
      data_o <= reg_file(to_integer(unsigned(ctrl_i(ctrl_rf_adr3_c downto ctrl_rf_adr0_c))));
    end if;
  end process rf_read;


end neo430_reg_file_rtl;
