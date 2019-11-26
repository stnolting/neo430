-- #################################################################################################
-- #  << NEO430 - Main Control Unit >>                                                             #
-- # ********************************************************************************************* #
-- # Central CPU control unit (FSM).                                                               #
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
-- # Stephan Nolting, Hannover, Germany                                                 26.11.2019 #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;

entity neo430_control is
  port (
    -- global control --
    clk_i     : in  std_ulogic; -- global clock, rising edge
    rst_i     : in  std_ulogic; -- global reset, low-active, async
    -- memory interface --
    instr_i   : in  std_ulogic_vector(15 downto 0); -- instruction word from memory
    -- control --
    sreg_i    : in  std_ulogic_vector(15 downto 0); -- current status register
    ctrl_o    : out std_ulogic_vector(ctrl_width_c-1 downto 0); -- control signals
    irq_vec_o : out std_ulogic_vector(01 downto 0); -- irq channel address
    imm_o     : out std_ulogic_vector(15 downto 0); -- branch offset
    -- irq lines --
    irq_i     : in  std_ulogic_vector(03 downto 0); -- IRQ lines
    irq_ack_o : out std_ulogic_vector(03 downto 0)  -- IRQ acknowledge
  );
end neo430_control;

architecture neo430_control_rtl of neo430_control is

  -- instruction register --
  signal ir   : std_ulogic_vector(15 downto 0);
  signal ir_wren : std_ulogic;

  -- branch system --
  signal branch_taken : std_ulogic;

  -- state machine --
  type state_t is (RESET, IFETCH_0, IFETCH_1, DECODE,
    TRANS_0, TRANS_1, TRANS_2, TRANS_3, TRANS_4, TRANS_5, TRANS_6, TRANS_7,
    PUSHCALL_0, PUSHCALL_1, PUSHCALL_2,
    RETI_0, RETI_1, RETI_2, RETI_3, RETI_4,
    IRQ_0, IRQ_1, IRQ_2, IRQ_3, IRQ_4, IRQ_5);
  signal state, state_nxt  : state_t;
  signal ctrl_nxt, ctrl    : std_ulogic_vector(ctrl_width_c-1 downto 0);
  signal am_nxt, am        : std_ulogic_vector(03 downto 0); -- addressing mode
  signal mem_rd, mem_rd_ff : std_ulogic; -- memory read buffers
  signal src_nxt, src      : std_ulogic_vector(03 downto 0); -- source reg
  signal sam_nxt, sam      : std_ulogic_vector(01 downto 0); -- CMD according SRC addressing mode

  -- irq system --
  signal irq_start, irq_ack     : std_ulogic;
  signal irq_ack_mask, irq_buf  : std_ulogic_vector(3 downto 0);
  signal irq_vec_nxt, irq_vec   : std_ulogic_vector(1 downto 0);
  signal i_flag_ff0, i_flag_ff1 : std_ulogic;

begin

  -- Branch Condition Check ---------------------------------------------------
  -- -----------------------------------------------------------------------------
  cond_check: process(instr_i, sreg_i)
  begin
    case instr_i(12 downto 10) is -- condition
      when cond_ne_c => branch_taken <= not sreg_i(sreg_z_c); -- JNE/JNZ
      when cond_eq_c => branch_taken <= sreg_i(sreg_z_c); -- JEQ/JZ
      when cond_lo_c => branch_taken <= not sreg_i(sreg_c_c); -- JNC/JLO
      when cond_hs_c => branch_taken <= sreg_i(sreg_c_c); -- JC/JHS
      when cond_mi_c => branch_taken <= sreg_i(sreg_n_c); -- JN
      when cond_ge_c => branch_taken <= not (sreg_i(sreg_n_c) xor sreg_i(sreg_v_c)); -- JGE
      when cond_le_c => branch_taken <= sreg_i(sreg_n_c) xor sreg_i(sreg_v_c); -- JL
      when cond_al_c => branch_taken <= '1'; -- JMP (always)
      when others    => branch_taken <= '0'; -- undefined
    end case;
  end process cond_check;

  -- branch offset (sign-extended) from instruction REGISTER --
  imm_o <= ir(9) & ir(9) & ir(9) & ir(9) & ir(9) & ir(9 downto 0) & '0';


  -- Arbiter State Machine Sync -----------------------------------------------
  -- -----------------------------------------------------------------------------
  arbiter_sync0: process(rst_i, clk_i)
  begin
    -- the arbiter requires a defined initial state
    if (rst_i = '0') then
      state <= RESET; -- this is crucial!
    elsif rising_edge(clk_i) then
      state <= state_nxt;
    end if;
  end process arbiter_sync0;

  arbiter_sync1: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- these signals do not need a specific reset state
      ctrl      <= ctrl_nxt;
      src       <= src_nxt;
      mem_rd_ff <= mem_rd;
      am        <= am_nxt;
      sam       <= sam_nxt;
      if (ir_wren = '1') then
        ir <= instr_i; -- instruction register
      end if;
    end if;
  end process arbiter_sync1;

  -- control bus output --
  ctrl_o <= ctrl;


  -- Arbiter State Machine Comb -----------------------------------------------
  -- -----------------------------------------------------------------------------
  arbiter_comb: process(state, instr_i, ir, ctrl, branch_taken, src, am, sam, mem_rd_ff, irq_start, sreg_i)
    variable spec_cmd_v, valid_wb_v : std_ulogic;
  begin
    -- arbiter defaults --
    state_nxt <= state;
    src_nxt   <= src; -- source reg
    am_nxt    <= am; -- addressing mode
    sam_nxt   <= sam;
    ir_wren   <= '0';
    mem_rd    <= '0';
    irq_ack   <= '0';

    -- control defaults --
    ctrl_nxt <= (others => '0');
    ctrl_nxt(ctrl_rf_adr3_c  downto ctrl_rf_adr0_c)  <= src; -- source reg A
    ctrl_nxt(ctrl_alu_cmd3_c downto ctrl_alu_cmd0_c) <= ctrl(ctrl_alu_cmd3_c downto ctrl_alu_cmd0_c); -- keep ALU function
    ctrl_nxt(ctrl_rf_as1_c   downto ctrl_rf_as0_c)   <= sam; -- SRC addressing mode
    ctrl_nxt(ctrl_adr_off2_c downto ctrl_adr_off0_c) <= "010"; -- add +2 as address offset
    ctrl_nxt(ctrl_mem_rd_c) <= mem_rd_ff; -- memory read
    ctrl_nxt(ctrl_alu_bw_c) <= ctrl(ctrl_alu_bw_c); -- keep byte/word mode

    -- special single ALU operation? --
    spec_cmd_v := '0';
    if (ir(15 downto 9) = "0001001") then -- CALL/PUSH/RETI
      spec_cmd_v := '1';
    end if;

    -- valid write back? --
    valid_wb_v := '1';
    if (ctrl(ctrl_alu_cmd3_c downto ctrl_alu_cmd0_c) = alu_cmp_c) or
       (ctrl(ctrl_alu_cmd3_c downto ctrl_alu_cmd0_c) = alu_bit_c) then
      valid_wb_v := '0';
    end if;

    -- state machine --
    case state is

      when RESET => -- init PC with boot address
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_rf_boot_c)  <= '1'; -- load boot address
        ctrl_nxt(ctrl_rf_ad_c)    <= '0'; -- DST address mode = REG
        ctrl_nxt(ctrl_rf_wb_en_c) <= '1'; -- valid RF write back
        ctrl_nxt(ctrl_rf_adr3_c  downto ctrl_rf_adr0_c) <= reg_pc_c; -- source/destination: PC
        state_nxt <= IFETCH_0;


      when IFETCH_0 => -- output and update PC & IRQ check (stay here for SLEEP)
      -- ------------------------------------------------------------
        sam_nxt <= "00"; -- SRC address mode = REG
        ctrl_nxt(ctrl_alu_bw_c) <= '0'; -- word mode
        ctrl_nxt(ctrl_rf_ad_c)  <= '0'; -- DST address mode = REG
        ctrl_nxt(ctrl_alu_cmd3_c downto ctrl_alu_cmd0_c) <= alu_mov_c; -- keep this for all following states
        ctrl_nxt(ctrl_rf_adr3_c  downto ctrl_rf_adr0_c)  <= reg_pc_c; -- source/destination: PC
        ctrl_nxt(ctrl_adr_off2_c downto ctrl_adr_off0_c) <= "010"; -- add +2
        ctrl_nxt(ctrl_rf_in_sel_c) <= '1'; -- select addr feedback
        ctrl_nxt(ctrl_adr_bp_en_c) <= '1'; -- directly output PC/IRQ vector
        if (irq_start = '1') then -- execute IRQ
          state_nxt <= IRQ_0;
        elsif (sreg_i(sreg_s_c) = '0') then -- no sleep mode = normal execution
          ctrl_nxt(ctrl_mem_rd_c)   <= '1'; -- Memory read (fast)
          ctrl_nxt(ctrl_rf_wb_en_c) <= '1'; -- valid RF write back
          state_nxt <= IFETCH_1;
        end if;

      when IFETCH_1 => -- wait for memory
      -- ------------------------------------------------------------
        state_nxt <= DECODE;


      when DECODE => -- decode applied instruction and store it to IR
      -- ------------------------------------------------------------
        ir_wren <= '1'; -- update instruction register
        ctrl_nxt(ctrl_alu_bw_c) <= instr_i(6); -- byte/word mode
        sam_nxt <= instr_i(5 downto 4);
        if (instr_i(15 downto 14) = "00") then -- branch or format II instruction
          if (instr_i(13) = '1') then -- BRANCH INSTRUCTION
          -- ------------------------------------------------------------
            ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= reg_pc_c; -- source/destination: PC
            ctrl_nxt(ctrl_adr_off2_c downto ctrl_adr_off0_c) <= "000"; -- add immediate offset
            ctrl_nxt(ctrl_rf_in_sel_c)  <= '1'; -- select addr feedback
            ctrl_nxt(ctrl_rf_wb_en_c)   <= branch_taken; -- valid RF write back if branch taken
            state_nxt <= IFETCH_0;
          elsif (instr_i(12 downto 10) = "100") then -- FORMAT II INSTRUCTION
            -- ------------------------------------------------------------
            am_nxt(0) <= instr_i(4) or instr_i(5); -- dst addressing mode
            am_nxt(3) <= '0'; -- class II
            if (instr_i(3 downto 0) = reg_cg_c) or ((instr_i(3 downto 0) = reg_sr_c) and (instr_i(5) = '1')) then -- source special?
              am_nxt(2 downto 1) <= "00"; -- source addressing mode
            else
              am_nxt(2 downto 1) <= instr_i(5 downto 4); -- source addressing mode
            end if;
            src_nxt <= instr_i(3 downto 0); -- src is also dst
            if (instr_i(15 downto 9) /= "0001001") then -- not PUSH/CALL/RETI?
              ctrl_nxt(ctrl_alu_cmd3_c downto ctrl_alu_cmd0_c) <= instr_i(10 downto 7); -- ALU function
            end if;
            ctrl_nxt(ctrl_rf_ad_c) <= instr_i(5) or instr_i(4);
            case instr_i(9 downto 7) is
              when "100"  => state_nxt <= TRANS_0; -- PUSH (via single ALU OP)
              when "101"  => state_nxt <= TRANS_0; -- CALL (via single ALU OP)
              when "110"  => state_nxt <= RETI_0; -- RETI
              when "111"  => state_nxt <= IFETCH_0; -- undefined (for detecting invalid opcodes)
              when others => state_nxt <= TRANS_0; -- single ALU OP
            end case;
          else -- Undefined (for detecting invalid opcodes)
            -- ------------------------------------------------------------
            state_nxt <= IFETCH_0;
          end if;
        else -- FORMAT I INSTRUCTION
        -- ------------------------------------------------------------
          am_nxt(3) <= '1'; -- class I
          if (instr_i(11 downto 8) = reg_cg_c) or ((instr_i(11 downto 8) = reg_sr_c) and (instr_i(5) = '1')) then -- source special?
            am_nxt(2 downto 1) <= "00"; -- source addressing mode
          else
            am_nxt(2 downto 1) <= instr_i(5 downto 4); -- source addressing mode
          end if;
          am_nxt(0) <= instr_i(7); -- dst addressing mode
          ctrl_nxt(ctrl_rf_ad_c) <= instr_i(7);
          ctrl_nxt(ctrl_alu_cmd3_c downto ctrl_alu_cmd0_c) <= instr_i(15 downto 12); -- ALU function
          src_nxt   <= instr_i(11 downto 8);
          state_nxt <= TRANS_0;
        end if;


      when TRANS_0 => -- operand transfer cycle 0
      -- ------------------------------------------------------------
        case am is -- addressing mode
          when "0001" | "0000" | "1000" =>
            -- "0001" = CLASS  I, SRC: Reg, DST: Indexed
            -- "0000" = CLASS  I, SRC/DST: register direct
            -- "1000" = CLASS II, SRC: register direct, DST: register direct
            ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= src; -- source: reg A
            ctrl_nxt(ctrl_alu_opa_wr_c) <= '1'; -- write OpA
            state_nxt <= TRANS_1;
          when "1001" =>
            -- "1001" = CLASS II, SRC: register direct, DST: indexed
            ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= reg_pc_c; -- source/destination: PC
            ctrl_nxt(ctrl_adr_mar_wr_c) <= '1'; -- write to MAR
            mem_rd <= '1'; -- Memory read
            ctrl_nxt(ctrl_adr_off2_c downto ctrl_adr_off0_c) <= "010"; -- add +2
            ctrl_nxt(ctrl_rf_in_sel_c) <= '1'; -- select addr feedback
            ctrl_nxt(ctrl_rf_wb_en_c) <= '1'; -- valid RF write back
            state_nxt <= TRANS_2;
          when "0010" | "0011" | "1010" | "1011" =>
            -- "001-" = CLASS  I, SRC/DST: indexed/symbolic/absolute
            -- "1010" = CLASS II, SRC: indexed/symbolic/absolute, DST: register direct
            -- "1011" = CLASS II, SRC: indexed/symbolic/absolute, DST: indexed
            ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= reg_pc_c; -- source/destination: PC
            ctrl_nxt(ctrl_adr_mar_wr_c) <= '1'; -- write to MAR
            mem_rd <= '1'; -- Memory read
            ctrl_nxt(ctrl_adr_off2_c downto ctrl_adr_off0_c) <= "010"; -- add +2
            ctrl_nxt(ctrl_rf_in_sel_c) <= '1'; -- select addr feedback
            ctrl_nxt(ctrl_rf_wb_en_c) <= '1'; -- valid RF write back
            state_nxt <= TRANS_1;
          when "0100" | "0101" | "1100" | "1101" =>
            -- "010-" = CLASS  I, SRC/DST: indirect
            -- "1100" = CLASS II, SRC: indirect, DST: register direct
            -- "1101" = CLASS II, SRC: indirect, DST: indexed
            ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= src; -- source: reg A
            ctrl_nxt(ctrl_adr_mar_wr_c) <= '1'; -- write to MAR
            mem_rd <= '1'; -- Memory read
            state_nxt <= TRANS_1;
          when others =>
            -- "011-" = CLASS  I, SRC/DST: indirect auto inc
            -- "1110" = CLASS II, SRC: indirect auto inc, DST: register direct
            -- "1111" = CLASS II, SRC: indirect auto inc, DST: indexed
            ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= src; -- source/destination: reg A
            ctrl_nxt(ctrl_adr_mar_wr_c) <= '1'; -- write to MAR
            mem_rd <= '1'; -- Memory read
            if (ir(6) = '0') or (src = reg_pc_c) then -- word mode (force if accessing PC)
              ctrl_nxt(ctrl_adr_off2_c downto ctrl_adr_off0_c) <= "010"; -- add +2
            else -- byte mode
              ctrl_nxt(ctrl_adr_off2_c downto ctrl_adr_off0_c) <= "001"; -- add +1
            end if;
            ctrl_nxt(ctrl_rf_in_sel_c) <= '1'; -- select addr feedback
            ctrl_nxt(ctrl_rf_wb_en_c) <= '1'; -- valid RF write back
            state_nxt <= TRANS_1;
        end case;

      when TRANS_1 => -- operand transfer cycle 1
      -- ------------------------------------------------------------
        case am is -- addressing mode
          when "0000" | "0001" | "1000" | "1001" =>
            -- "000-" = CLASS  I, SRC/DST: register direct
            -- "1000" = CLASS II, SRC: register direct, DST: register direct
            -- "1001" = CLASS II, SRC: register direct, DST: indexed
            ctrl_nxt(ctrl_rf_as1_c downto ctrl_rf_as0_c)   <= "00"; -- DST address mode = REG
            ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= ir(3 downto 0); -- source: reg B
            ctrl_nxt(ctrl_alu_opb_wr_c) <= '1'; -- write OpB
            if (spec_cmd_v = '1') then -- push or call
              state_nxt <= PUSHCALL_0;
            else
              state_nxt <= TRANS_6;
            end if;
          when "0010" | "0011" | "1010" | "0100" | "0101" | "1100" | "0110" | "0111" | "1110" =>
            -- "001-" = CLASS  I, SRC/DST: indexed/symbolic/absolute
            -- "1010" = CLASS II, SRC: indexed/symbolic/absolute, DST: register direct
            -- "010-" = CLASS  I, SRC/DST: indirect
            -- "1100" = CLASS II, SRC: indirect, DST: register direct
            -- "011-" = CLASS  I, SRC/DST: indirect auto inc,
            -- "1110" = CLASS II, SRC: indirect auto inc, DST: register direct
            ctrl_nxt(ctrl_rf_as1_c downto ctrl_rf_as0_c)   <= "00"; -- DST address mode = REG
            ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= ir(3 downto 0); -- source: reg B
            ctrl_nxt(ctrl_alu_opb_wr_c) <= '1'; -- write OpB
            state_nxt <= TRANS_2;
          when others =>
            -- "1011" = CLASS II, SRC: indexed/symbolic/absolute, DST: indexed
            -- "1101" = CLASS II, SRC: indirect, DST: indexed
            -- "1111" = CLASS II, SRC: indirect auto inc, DST: indexed
            ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= reg_pc_c; -- source/destination: PC
            ctrl_nxt(ctrl_adr_mar_wr_c) <= '1'; -- write to MAR
            mem_rd <= '1'; -- Memory read
            ctrl_nxt(ctrl_adr_off2_c downto ctrl_adr_off0_c) <= "010"; -- add +2
            ctrl_nxt(ctrl_rf_in_sel_c) <= '1'; -- select addr feedback
            ctrl_nxt(ctrl_rf_wb_en_c) <= '1'; -- valid RF write back
            state_nxt <= TRANS_2;
        end case;

      when TRANS_2 => -- operand transfer cycle 3
      -- ------------------------------------------------------------
        case am is -- addressing mode
          when "0000" | "0001" | "1000" | "1001" =>
            -- "000-" = CLASS  I, DONT CARE 
            -- "1000" = CLASS II, SRC: register direct, DST: register direct = DONT CARE
            -- "1001" = CLASS II, SRC: register direct, DST: indexed
            ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= src; -- source: reg A
            ctrl_nxt(ctrl_alu_opa_wr_c) <= '1'; -- write OpA
            state_nxt <= TRANS_3;
          when "0010" | "0011" | "1010" | "1011" =>
            -- "001-" = CLASS  I: SRC/DST: indexed/symbolic/absolute
            -- "1010" = CLASS II, SRC: indexed/symbolic/absolute, DST: register direct
            -- "1011" = CLASS II, SRC: indexed/symbolic/absolute, DST: indexed
            ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= src; -- source: reg A
            ctrl_nxt(ctrl_adr_off2_c downto ctrl_adr_off0_c) <= "1--"; -- add memory data in
            ctrl_nxt(ctrl_adr_mar_sel_c) <= '1'; -- use result from adder
            ctrl_nxt(ctrl_adr_mar_wr_c) <= '1'; -- write to MAR
            mem_rd <= '1'; -- Memory read
            state_nxt <= TRANS_3;
          when "0100" | "0101" | "1100" | "0110" | "0111" | "1110" =>
            -- "010-" = CLASS  I: SRC/DST: indirect
            -- "1100" = CLASS II, SRC: indirect, DST: register direct
            -- "011-" = CLASS  I: SRC/DST: indirect auto inc
            -- "1110" = CLASS II, SRC: indirect auto inc, DST: register direct
            ctrl_nxt(ctrl_alu_in_sel_c) <= '1'; -- get data from memory
            ctrl_nxt(ctrl_alu_opa_wr_c) <= '1'; -- write OpA
            if (spec_cmd_v = '1') then -- push or call
              state_nxt <= PUSHCALL_0;
            else
              state_nxt <= TRANS_6;
            end if;
          when others =>
            -- "1101" = CLASS II, SRC: indirect, DST: indexed
            -- "1111" = CLASS II, SRC: indirect auto inc, DST: indexed
            ctrl_nxt(ctrl_alu_in_sel_c) <= '1'; -- get data from memory
            ctrl_nxt(ctrl_alu_opa_wr_c) <= '1'; -- write OpA
            state_nxt <= TRANS_3;
        end case;

      when TRANS_3 => -- operand transfer cycle 4
      -- ------------------------------------------------------------
        case am is -- addressing mode
          when "1001" | "1011" | "1101" | "1111" =>
            -- "1001" = CLASS II, SRC: register direct, DST: indexed
            -- "1011" = CLASS II, SRC: indexed/symbolic/absolute, DST: indexed
            -- "1101" = CLASS II, SRC: indirect, DST: indexed
            -- "1111" = CLASS II, SRC: indirect auto inc, DST: indexed
            ctrl_nxt(ctrl_rf_as1_c) <= '0'; -- DST address mode = REG or INDEXED
            ctrl_nxt(ctrl_rf_as0_c) <= ir(7); -- DST address mode = REG or INDEXED
            ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= ir(3 downto 0); -- source: reg B
            ctrl_nxt(ctrl_adr_off2_c downto ctrl_adr_off0_c) <= "1--"; -- add memory data in
            ctrl_nxt(ctrl_adr_mar_sel_c) <= '1'; -- use result from adder
            ctrl_nxt(ctrl_adr_mar_wr_c) <= '1'; -- write to MAR
            if (ctrl(ctrl_alu_cmd3_c downto ctrl_alu_cmd0_c) /= alu_mov_c) then -- no read for MOV
              mem_rd <= '1'; -- Memory read
            end if;
            state_nxt <= TRANS_4;
          when others =>
            state_nxt <= TRANS_4; -- NOP / DONT CARE
        end case;

      when TRANS_4 => -- operand transfer cycle 6
      -- ------------------------------------------------------------
        case am is -- addressing mode
          when "0010" | "0011" | "1010" =>
            -- "001-" = CLASS  I, SRC/DST: indexed/symbolic/absolute
            -- "1010" = CLASS II, SRC: indexed/symbolic/absolute, DST: register direct
            ctrl_nxt(ctrl_alu_in_sel_c) <= '1'; -- get data from memory
            ctrl_nxt(ctrl_alu_opa_wr_c) <= '1'; -- write to OpA
            if (spec_cmd_v = '1') then -- push or call
              state_nxt <= PUSHCALL_0;
            else
              state_nxt <= TRANS_6;
            end if;
          when "1011" =>
            -- "1011" = CLASS II,SRC: indexed/symbolic/absolute, DST: indexed
            ctrl_nxt(ctrl_alu_in_sel_c) <= '1'; -- get data from memory
            ctrl_nxt(ctrl_alu_opa_wr_c) <= '1'; -- write to OpA
            state_nxt <= TRANS_5;
          when others =>
            -- "000-" = CLASS  I, SRD/DST:  CLASS II, 
            -- "1000" = CLASS II, SRC: register direct, DST: register direct = DONT CARE
            -- "1001" = CLASS II, SRC: register direct, DST: indexed = NOP
            -- others: NOP
            state_nxt <= TRANS_5; -- NOP / DONT CARE
        end case;

      when TRANS_5 => -- operand transfer cycle 7
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_alu_in_sel_c) <= '1'; -- get data from memory
        ctrl_nxt(ctrl_alu_opb_wr_c) <= '1'; -- write to OpA
        if (spec_cmd_v = '1') then -- push or call
          state_nxt <= PUSHCALL_0;
        else
          state_nxt <= TRANS_6; -- single/dual ALU operation
        end if;

      when TRANS_6 => -- operand transfer cycle 8
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= ir(3 downto 0); -- destination
        if (ctrl(ctrl_alu_cmd3_c downto ctrl_alu_cmd0_c) = alu_dadd_c) and (use_dadd_cmd_c = true) then
          state_nxt <= TRANS_7;
        else
          ctrl_nxt(ctrl_rf_fup_c) <= not spec_cmd_v; -- update ALU status flags
          if (am(0) = '0') then -- DST: register direct
            ctrl_nxt(ctrl_rf_wb_en_c) <= valid_wb_v; -- valid RF write back (not for CMP/BIT!)
          else -- DST: indexed
            ctrl_nxt(ctrl_mem_wr_c) <= valid_wb_v; -- valid MEM write back (not for CMP/BIT!)
          end if;
          state_nxt <= IFETCH_0; -- done!
        end if;

      when TRANS_7 => -- operand transfer cycle 9
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= ir(3 downto 0); -- destination
        ctrl_nxt(ctrl_rf_fup_c) <= not spec_cmd_v; -- update ALU status flags
        if (am(0) = '0') then -- DST: register direct
          ctrl_nxt(ctrl_rf_wb_en_c) <= valid_wb_v; -- valid RF write back (not for CMP/BIT!)
        else -- DST: indexed
          ctrl_nxt(ctrl_mem_wr_c) <= valid_wb_v; -- valid MEM write back (not for CMP/BIT!)
        end if;
        state_nxt <= IFETCH_0; -- done!


      when PUSHCALL_0 => -- PUSH/CALL cycle 0 (stack update)
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= reg_sp_c; -- source/destination: SP
        ctrl_nxt(ctrl_adr_off2_c downto ctrl_adr_off0_c) <= "011"; -- add -2
        ctrl_nxt(ctrl_adr_mar_wr_c) <= '1'; -- write to MAR
        ctrl_nxt(ctrl_adr_mar_sel_c) <= '1'; -- use result from adder
        ctrl_nxt(ctrl_rf_in_sel_c) <= '1'; -- select addr feedback
        ctrl_nxt(ctrl_rf_wb_en_c) <= '1'; -- valid RF write back
        if (ir(7) = '1') then -- CALL
          state_nxt <= PUSHCALL_1;
        else -- PUSH
          state_nxt <= PUSHCALL_2;
        end if;

      when PUSHCALL_1 => -- CALL cycle 1 (buffer PC so it can be written to memory)
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= reg_pc_c; -- source: PC
        ctrl_nxt(ctrl_alu_opa_wr_c) <= '1'; -- write to OpA
        ctrl_nxt(ctrl_rf_wb_en_c) <= '1'; -- valid RF write back
        state_nxt <= PUSHCALL_2;

      when PUSHCALL_2 => -- PUSH/CALL cycle 2 (write data to memory)
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_mem_wr_c) <= '1'; -- memory write request
        state_nxt <= IFETCH_0; -- done!


      when RETI_0 => -- RETI cycle 0: Read address of old SR and inc PC
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= reg_sp_c; -- source/destination: SP
        ctrl_nxt(ctrl_adr_mar_wr_c) <= '1'; -- write to MAR
        mem_rd <= '1'; -- Memory read
        ctrl_nxt(ctrl_adr_off2_c downto ctrl_adr_off0_c) <= "010"; -- add +2
        ctrl_nxt(ctrl_rf_in_sel_c) <= '1'; -- select addr feedback
        ctrl_nxt(ctrl_rf_wb_en_c) <= '1'; -- valid RF write back
        state_nxt <= RETI_1;

      when RETI_1 => -- RETI cycle 1: Read address of old PC and inc PC
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= reg_sp_c; -- source/destination: SP
        ctrl_nxt(ctrl_adr_mar_wr_c) <= '1'; -- write to MAR
        mem_rd <= '1'; -- Memory read
        ctrl_nxt(ctrl_adr_off2_c downto ctrl_adr_off0_c) <= "010"; -- add +2
        ctrl_nxt(ctrl_rf_in_sel_c) <= '1'; -- select addr feedback
        ctrl_nxt(ctrl_rf_wb_en_c) <= '1'; -- valid RF write back
        state_nxt <= RETI_2;

      when RETI_2 => -- RETI cycle 3: Buffer status register from MEM in OpA
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_alu_in_sel_c) <= '1'; -- get data from memory
        ctrl_nxt(ctrl_alu_opa_wr_c) <= '1'; -- write to OpA
        state_nxt <= RETI_3;

      when RETI_3 => -- RETI cycle 4: Buffer return address from MEM in OpA and write status register
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_alu_in_sel_c) <= '1'; -- get data from memory
        ctrl_nxt(ctrl_alu_opa_wr_c) <= '1'; -- write to OpA
        ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= reg_sr_c; -- destination: SR
        ctrl_nxt(ctrl_rf_wb_en_c) <= '1'; -- valid RF write back
        state_nxt <= RETI_4;

      when RETI_4 => -- RETI cycle 5: Write return address to PC
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= reg_pc_c; -- destination: PC
        ctrl_nxt(ctrl_rf_wb_en_c) <= '1'; -- valid RF write back
        state_nxt <= IFETCH_0; -- done!


      when IRQ_0 => -- IRQ processing cycle 0: SP=SP-2, disable sleep mode
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= reg_sp_c; -- source/destination: SP
        ctrl_nxt(ctrl_adr_mar_wr_c)  <= '1'; -- write to MAR
        ctrl_nxt(ctrl_adr_off2_c downto ctrl_adr_off0_c) <= "011"; -- add -2
        ctrl_nxt(ctrl_adr_mar_sel_c) <= '1'; -- use result from adder
        ctrl_nxt(ctrl_rf_in_sel_c)   <= '1'; -- select addr feedback
        ctrl_nxt(ctrl_rf_wb_en_c)    <= '1'; -- valid RF write back
        ctrl_nxt(ctrl_rf_dsleep_c)   <= '1'; -- disable sleep mode
        state_nxt <= IRQ_1;

      when IRQ_1 => -- IRQ processing cycle 1: Buffer PC for memory write
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= reg_pc_c; -- source: PC
        ctrl_nxt(ctrl_alu_opa_wr_c) <= '1'; -- write PC to OpA
        state_nxt <= IRQ_2;

      when IRQ_2 => -- IRQ processing cycle 2: Write PC (push), SP=SP-2
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_mem_wr_c)      <= '1'; -- write memory request (store PC)
        ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= reg_sp_c; -- source/destination: SP
        ctrl_nxt(ctrl_adr_mar_wr_c)  <= '1'; -- write to MAR
        ctrl_nxt(ctrl_adr_off2_c downto ctrl_adr_off0_c) <= "011"; -- add -2
        ctrl_nxt(ctrl_adr_mar_sel_c) <= '1'; -- use result from adder
        ctrl_nxt(ctrl_rf_in_sel_c)   <= '1'; -- select addr feedback
        ctrl_nxt(ctrl_rf_wb_en_c)    <= '1'; -- valid RF write back
        state_nxt <= IRQ_3;

      when IRQ_3 => -- IRQ processing cycle 3: Buffer SR for memory write, set IRQ vector address, disable interrupt enable flag in SR
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= reg_sr_c; -- source: SR
        ctrl_nxt(ctrl_alu_opa_wr_c)  <= '1'; -- write SR to OpA
        ctrl_nxt(ctrl_adr_bp_en_c)   <= '1'; -- directly output PC/IRQ vector
        ctrl_nxt(ctrl_adr_ivec_oe_c) <= '1'; -- output IRQ vector
        ctrl_nxt(ctrl_mem_rd_c)      <= '1'; -- Memory read (fast)
        ctrl_nxt(ctrl_rf_dgie_c)     <= '1'; -- disable interrupt enable flag
        irq_ack                      <= '1'; -- acknowledge IRQ
        state_nxt <= IRQ_4;

      when IRQ_4 => -- IRQ processing cycle 4: Write SR (push), get IRQ vector
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_mem_wr_c)     <= '1'; -- write memory request
        ctrl_nxt(ctrl_alu_in_sel_c) <= '1'; -- get data from memory
        ctrl_nxt(ctrl_alu_opa_wr_c) <= '1'; -- write to OpA
        state_nxt <= IRQ_5;

      when IRQ_5 => -- IRQ processing cycle 5: Store IRQ vector to PC
      -- ------------------------------------------------------------
        ctrl_nxt(ctrl_rf_adr3_c downto ctrl_rf_adr0_c) <= reg_pc_c; -- destination: PC
        ctrl_nxt(ctrl_rf_wb_en_c) <= '1'; -- valid RF write back
        state_nxt <= IFETCH_0; -- done!


      when others => -- invalid
      -- ------------------------------------------------------------
        state_nxt <= RESET;

    end case;
  end process arbiter_comb;


  -- Interrupt Controller -----------------------------------------------------
  -- -----------------------------------------------------------------------------
  irq_buffer: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- delay I flag 2 cycles to allow the interrupted program to execute at least one insruction even if we have
      -- a permanent interrupt request
      i_flag_ff0 <= sreg_i(sreg_i_c);
      i_flag_ff1 <= i_flag_ff0;
      -- interrupt vector and queue buffer --
      irq_vec <= irq_vec_nxt;
      for i in 0 to 3 loop
        irq_buf(i) <= (irq_buf(i) or irq_i(i)) and (not sreg_i(sreg_q_c)) and (not irq_ack_mask(i));
      end loop; -- i
    end if;
  end process irq_buffer;

  -- valid start of IRQ handler --
  irq_start <= '1' when (irq_buf /= "0000") and (i_flag_ff1 = '1') and (sreg_i(sreg_i_c) = '1') else '0';

  -- acknowledge mask --
  irq_ack_mask_gen: process(irq_ack, irq_vec)
    variable irq_tmp_v : std_ulogic_vector(2 downto 0);
  begin
    irq_tmp_v := irq_ack & irq_vec;
    case irq_tmp_v is
      when "100"   => irq_ack_mask <= "0001";
      when "101"   => irq_ack_mask <= "0010";
      when "110"   => irq_ack_mask <= "0100";
      when "111"   => irq_ack_mask <= "1000";
      when others  => irq_ack_mask <= "0000";
    end case;
  end process;

  -- ack output --
  irq_ack_o <= irq_ack_mask;

  -- interrupt priority encoder --
  irq_priority: process(irq_buf)
  begin
    -- use "case" here to prevent a MUX chain
    case irq_buf is
      when "0001" | "0011" | "0101" | "0111" | "1001" | "1011" | "1101" | "1111" => -- "---1"
        irq_vec_nxt <= "00";
      when "0010" | "0110" | "1010" | "1110" => -- "--10"
        irq_vec_nxt <= "01";
      when "0100" | "1100" => -- "-100"
        irq_vec_nxt <= "10";
      when others => -- "1000"
        irq_vec_nxt <= "11";
    end case;
  end process irq_priority;

  -- interrupt vector output --
  irq_vec_o <= irq_vec; -- the final address is constructed in the address generator


end neo430_control_rtl;
