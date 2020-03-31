-- #################################################################################################
-- #  << NEO430 - CPU Top Entity >>                                                                #
-- # ********************************************************************************************* #
-- # Top entity of the NEO430 CPU.                                                                 #
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

entity neo430_cpu is
  generic (
    BOOTLD_USE  : boolean := true; -- implement and use bootloader?
    IMEM_AS_ROM : boolean := false -- implement IMEM as read-only memory?
  );
  port (
    -- global control --
    clk_i      : in  std_ulogic; -- global clock, rising edge
    rst_i      : in  std_ulogic; -- global reset, low-active, async
    -- memory interface --
    mem_rd_o   : out std_ulogic; -- memory read enable
    mem_imwe_o : out std_ulogic; -- allow writing to IMEM
    mem_wr_o   : out std_ulogic_vector(01 downto 0); -- byte memory write enable
    mem_addr_o : out std_ulogic_vector(15 downto 0); -- address
    mem_data_o : out std_ulogic_vector(15 downto 0); -- write data
    mem_data_i : in  std_ulogic_vector(15 downto 0); -- read data
    -- interrupt system --
    irq_i      : in  std_ulogic_vector(03 downto 0)  -- interrupt requests
  );
end neo430_cpu;

architecture neo430_cpu_rtl of neo430_cpu is

  -- local signals --
  signal mem_addr  : std_ulogic_vector(15 downto 0); -- memory address
  signal mdi       : std_ulogic_vector(15 downto 0); -- memory data_in
  signal mdi_gate  : std_ulogic_vector(15 downto 0); -- memory data_in power gate
  signal mdo_gate  : std_ulogic_vector(15 downto 0); -- memory data_out power gate
  signal ctrl_bus  : std_ulogic_vector(ctrl_width_c-1 downto 0); -- main control spine
  signal sreg      : std_ulogic_vector(15 downto 0); -- current status register
  signal alu_flags : std_ulogic_vector(04 downto 0); -- new ALU flags
  signal imm       : std_ulogic_vector(15 downto 0); -- branch offset
  signal rf_read   : std_ulogic_vector(15 downto 0); -- RF read data
  signal alu_res   : std_ulogic_vector(15 downto 0); -- ALU result
  signal addr_fb   : std_ulogic_vector(15 downto 0); -- address feedback
  signal irq_sel   : std_ulogic_vector(01 downto 0); -- IRQ vector
  signal dio_swap  : std_ulogic; -- data in/out swap
  signal bw_ff     : std_ulogic; -- byte/word access flag
  signal rd_ff     : std_ulogic; -- is read access

begin

  -- Control Unit -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_control_inst: neo430_control
  port map (
    -- global control --
    clk_i      => clk_i,      -- global clock, rising edge
    rst_i      => rst_i,      -- global reset, low-active, async
    -- memory interface --
    instr_i    => mem_data_i, -- instruction word from memory
    -- control --
    sreg_i     => sreg,       -- current status register
    ctrl_o     => ctrl_bus,   -- control signals
    irq_vec_o  => irq_sel,    -- irq channel address
    imm_o      => imm,        -- branch offset
    -- irq lines --
    irq_i      => irq_i       -- IRQ lines
  );


  -- Register File ------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_reg_file_inst: neo430_reg_file
  generic map (
    BOOTLD_USE  => BOOTLD_USE, -- implement and use bootloader?
    IMEM_AS_ROM => IMEM_AS_ROM -- implement IMEM as read-only memory?
  )
  port map (
    -- global control --
    clk_i      => clk_i,      -- global clock, rising edge
    rst_i      => rst_i,      -- global reset, low-active, async
    -- data input --
    alu_i      => alu_res,    -- data from alu
    addr_i     => addr_fb,    -- data from addr unit
    flag_i     => alu_flags,  -- new ALU flags
    -- control --
    ctrl_i     => ctrl_bus,   -- control signals
    -- data output --
    data_o     => rf_read,    -- read data
    sreg_o     => sreg        -- current SR
  );


  -- ALU ----------------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_alu_inst: neo430_alu
  port map (
    -- global control --
    clk_i      => clk_i,      -- global clock, rising edge
    -- operands --
    reg_i      => rf_read,    -- data from reg file
    mem_i      => mdi,        -- data from memory
    sreg_i     => sreg,       -- current SR
    -- control --
    ctrl_i     => ctrl_bus,   -- control signals
    -- results --
    data_o     => alu_res,    -- result
    flag_o     => alu_flags   -- new ALU flags
  );


  -- Address Generator --------------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_addr_gen_inst: neo430_addr_gen
  port map(
    -- global control --
    clk_i      => clk_i,      -- global clock, rising edge
    -- data input --
    reg_i      => rf_read,    -- reg file input
    mem_i      => mdi,        -- memory input
    imm_i      => imm,        -- branch offset
    irq_sel_i  => irq_sel,    -- IRQ vector
    -- control --
    ctrl_i     => ctrl_bus,   -- control signals
    -- data output --
    mem_addr_o => mem_addr,   -- memory address
    dwb_o      => addr_fb     -- data write back output
  );


  -- Memory Access ------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  memory_control: process(clk_i)
  begin
    if rising_edge(clk_i) then
      bw_ff    <= ctrl_bus(ctrl_alu_bw_c);
      dio_swap <= ctrl_bus(ctrl_alu_bw_c) and mem_addr(0);
      rd_ff    <= ctrl_bus(ctrl_mem_rd_c);
    end if;
  end process memory_control;

  -- Memory R/W interface --
  mem_rd_o <= ctrl_bus(ctrl_mem_rd_c);

  -- activate both WE lines when in word mode, use corresponding WE line when in byte mode
  mem_wr_o(0) <= ctrl_bus(ctrl_mem_wr_c) when (bw_ff = '0') else (ctrl_bus(ctrl_mem_wr_c) and (not mem_addr(0)));
  mem_wr_o(1) <= ctrl_bus(ctrl_mem_wr_c) when (bw_ff = '0') else (ctrl_bus(ctrl_mem_wr_c) and      mem_addr(0) );

  -- only allow write-access to IMEM when r-flag is set --
  mem_imwe_o <= sreg(sreg_r_c);

  -- data in/out swap --
  mdi_gate   <= mem_data_i when ((rd_ff = '1') or (low_power_mode_c = false)) else (others => '0'); -- AND GATE to reduce switching activity in low power mode
  mdi        <= mdi_gate   when (dio_swap = '0') else mdi_gate(7 downto 0) & mdi_gate(15 downto 8);
  mdo_gate   <= alu_res    when (dio_swap = '0') else alu_res(7 downto 0) & alu_res(15 downto 8);
  mem_data_o <= mdo_gate   when ((ctrl_bus(ctrl_mem_wr_c) = '1') or (low_power_mode_c = false)) else (others => '0'); -- AND GATE to reduce switching activity in low power mode

  -- address output --
  mem_addr_o <= mem_addr(15 downto 1) & '0'; -- word-aligned addresses only beyond this point


end neo430_cpu_rtl;
