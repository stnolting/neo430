-- #################################################################################################
-- #  << NEO430 - CPU Register File >>                                                             #
-- # ********************************************************************************************* #
-- #  General data registers, program counter, status register and constant generator.             #
-- # ********************************************************************************************* #
-- # This file is part of the NEO430 Processor project: http://opencores.org/project,neo430        #
-- # Copyright 2015-2017, Stephan Nolting: stnolting@gmail.com                                     #
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
-- #  Stephan Nolting, Hannover, Germany                                               03.01.2017  #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.neo430_package.all;

entity neo430_reg_file is
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

  -- register file (including dummy regs) --
  type   reg_file_t is array (15 downto 0) of std_ulogic_vector(15 downto 0);
  signal reg_file : reg_file_t;
  signal sreg     : std_ulogic_vector(15 downto 0);

  --- RAM attribute to inhibit bypass-logic - Altera only! ---
  attribute ramstyle : string;
  attribute ramstyle of reg_file : signal is "no_rw_check";

  -- status flags --
  signal c_flag, z_flag, n_flag, i_flag, s_flag, v_flag, r_flag : std_ulogic;

  -- misc --
  signal in_data_tmp : std_ulogic_vector(15 downto 0); -- input selection tmp
  signal in_data     : std_ulogic_vector(15 downto 0); -- input selection
  signal boot_addr   : std_ulogic_vector(15 downto 0); -- the boot address

begin

  -- Boot address Selection ---------------------------------------------------
  -- -----------------------------------------------------------------------------
  -- Boot from beginning of IMEM if no bootloader is used
  -- Boot from beginning of boot ROM if bootloader IS used
  boot_addr <= imem_base_c when (bootld_use_c = false) else boot_base_c;


  -- Input Operand Selection --------------------------------------------------
  -- -----------------------------------------------------------------------------
  in_data_tmp <= alu_i when (ctrl_i(ctrl_rf_in_sel_c) = '0') else addr_i;
  in_data     <= in_data_tmp when (ctrl_i(ctrl_rf_boot_c) = '0') else boot_addr;


  -- Register File Write Access -----------------------------------------------
  -- -----------------------------------------------------------------------------
  sreg_write: process(rst_i, clk_i)
  begin
    if (rst_i = '0') then
      c_flag <= '0'; -- carry
      z_flag <= '0'; -- zero
      n_flag <= '0'; -- negative
      i_flag <= '0'; -- interrupts disabled
      s_flag <= '0'; -- sleep disabled
      v_flag <= '0'; -- overflow
      r_flag <= '0'; -- IMEM (ROM) write access disabled
    elsif rising_edge(clk_i) then
      -- status register --
      if ((ctrl_i(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) = reg_sr_c) and
          (ctrl_i(ctrl_rf_ad_c) = '0') and (ctrl_i(ctrl_rf_wb_en_c) = '1')) then -- only write in reg-addr-mode!
        c_flag <= in_data(sreg_c_c);
        z_flag <= in_data(sreg_z_c);
        n_flag <= in_data(sreg_n_c);
        i_flag <= in_data(sreg_i_c);
        s_flag <= in_data(sreg_s_c);
        v_flag <= in_data(sreg_v_c);
        r_flag <= in_data(sreg_r_c);
      else -- automatic update
        if (ctrl_i(ctrl_rf_dsleep_c) = '1') then -- disable sleep mode
          s_flag <= '0';
        end if;
        if (ctrl_i(ctrl_rf_fup_c) = '1') then -- update ALU flags
          c_flag <= flag_i(flag_c_c);
          z_flag <= flag_i(flag_z_c);
          n_flag <= flag_i(flag_n_c);
          v_flag <= flag_i(flag_v_c);
        end if;
      end if;
    end if;
  end process sreg_write;

  -- gp regs (including PW, dummy SR and dummy CG) --
  rf_write: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (ctrl_i(ctrl_rf_ad_c) = '0') and (ctrl_i(ctrl_rf_wb_en_c) = '1') then -- only write in reg-addr-mode!
        reg_file(to_integer(unsigned(ctrl_i(ctrl_rf_adr3_c downto ctrl_rf_adr0_c)))) <= in_data;
      end if;
    end if;
  end process rf_write;

  -- assign virtual SREG --
  virtual_sreg: process(c_flag, z_flag, n_flag, i_flag, s_flag, v_flag, r_flag)
  begin
    sreg(15 downto 0) <= (others => '0');
    sreg(sreg_c_c) <= c_flag;
    sreg(sreg_z_c) <= z_flag;
    sreg(sreg_n_c) <= n_flag;
    sreg(sreg_i_c) <= i_flag;
    sreg(sreg_s_c) <= s_flag;
    sreg(sreg_v_c) <= v_flag;
    sreg(sreg_r_c) <= r_flag;
  end process virtual_sreg;

  -- output --
  sreg_o <= sreg;


  -- Register File Read Access ------------------------------------------------
  -- -----------------------------------------------------------------------------
  rf_read: process(ctrl_i, reg_file, sreg)
    variable const_sel_v : std_ulogic_vector(02 downto 0);
    variable const_gen_v : std_ulogic_vector(15 downto 0);
  begin
    -- constant generator --
    const_sel_v := ctrl_i(ctrl_rf_adr0_c) & ctrl_i(ctrl_rf_as1_c) & ctrl_i(ctrl_rf_as0_c);
    case const_sel_v is
      when "000"  => const_gen_v := sreg; -- read SR
      when "001"  => const_gen_v := x"0000"; -- absolute addressing mode
      when "010"  => const_gen_v := x"0004"; -- +4
      when "011"  => const_gen_v := x"0008"; -- +8
      when "100"  => const_gen_v := x"0000"; --  0
      when "101"  => const_gen_v := x"0001"; -- +1
      when "110"  => const_gen_v := x"0002"; -- +2
      when "111"  => const_gen_v := x"FFFF"; -- -1
      when others => const_gen_v := x"0000";
    end case;

    -- output select --
    if ((ctrl_i(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) = reg_sr_c) or
        (ctrl_i(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) = reg_cg_c)) then
      data_o <= const_gen_v;
    else
      data_o <= reg_file(to_integer(unsigned(ctrl_i(ctrl_rf_adr3_c downto ctrl_rf_adr0_c))));
    end if;
  end process rf_read;


end neo430_reg_file_rtl;
