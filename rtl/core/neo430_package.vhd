-- #################################################################################################
-- #  << NEO430 - Processor Package >>                                                             #
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
-- #  Stephan Nolting, Hannover, Germany                                               22.02.2017  #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package neo430_package is

  -- Processor Hardware Version -------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  constant hw_version_c : std_ulogic_vector(15 downto 0) := x"0108"; -- no touchy!

  -- Internal Functions ---------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  function index_size(input: natural) return natural;
  function bit_reversal(input: std_ulogic_vector) return std_ulogic_vector;
  function set_bits(input: std_ulogic_vector) return natural;
  function leading_zeros(input: std_ulogic_vector) return natural;
  function cond_sel_natural(cond: boolean; val_t: natural; val_f: natural) return natural;
  function cond_sel_stdulogicvector(cond: boolean; val_t: std_ulogic_vector; val_f: std_ulogic_vector) return std_ulogic_vector;
  function bool_to_ulogic(cond: boolean) return std_ulogic;
  function bin_to_gray(input : std_ulogic_vector) return std_ulogic_vector;
  function gray_to_bin(input : std_ulogic_vector) return std_ulogic_vector;

  -- Address Space Layout -------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------

  -- Main Memory: IMEM(ROM/RAM) --
  constant imem_base_c : std_ulogic_vector(15 downto 0) := x"0000"; -- base address, fixed!

  -- Main Memory: DMEM(RAM) --
  constant dmem_base_c : std_ulogic_vector(15 downto 0) := x"8000"; -- base address, fixed for now!

  -- Boot ROM --
  constant boot_base_c : std_ulogic_vector(15 downto 0) := x"F000"; -- bootloader base address, fixed!
  constant boot_size_c : natural := 2048; -- bytes, max 2048 bytes!

  -- IO: IO Area --
  -- Each internal IO device (except sysconfig) must not use more than 16 bytes address space!
  constant io_base_c : std_ulogic_vector(15 downto 0) := x"FF80";
  constant io_size_c : natural := 128; -- bytes, fixed!

  -- IO: Multiplier (obsolete!) --
--constant mul_base_c : std_ulogic_vector(15 downto 0) := x"FF80";
--constant mul_size_c : natural := 16; -- bytes

  -- IO: Wishbone32 Interface --
  constant wb32_base_c : std_ulogic_vector(15 downto 0) := x"FF90";
  constant wb32_size_c : natural := 16; -- bytes

  constant wb32_adr_lo_r_addr_c : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(wb32_base_c) + x"0000");
  constant wb32_adr_lo_w_addr_c : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(wb32_base_c) + x"0002");
  constant wb32_adr_hi_addr_c   : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(wb32_base_c) + x"0004");
  constant wb32_do_lo_addr_c    : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(wb32_base_c) + x"0006");
  constant wb32_do_hi_addr_c    : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(wb32_base_c) + x"0008");
  constant wb32_di_lo_addr_c    : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(wb32_base_c) + x"000A");
  constant wb32_di_hi_addr_c    : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(wb32_base_c) + x"000C");
  constant wb32_ctrl_addr_c     : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(wb32_base_c) + x"000E");

  -- IO: USART --
  constant usart_base_c : std_ulogic_vector(15 downto 0) := x"FFA0";
  constant usart_size_c : natural := 8; -- bytes

  constant usart_spi_rtx_addr_c  : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(usart_base_c) + x"0000");
  constant usart_uart_rtx_addr_c : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(usart_base_c) + x"0002");
  constant usart_baud_addr_c     : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(usart_base_c) + x"0004");
  constant usart_ctrl_addr_c     : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(usart_base_c) + x"0006");

  -- IO: GPIO  --
  constant gpio_base_c : std_ulogic_vector(15 downto 0) := x"FFB0";
  constant gpio_size_c : natural := 6; -- bytes

  constant gpio_in_addr_c   : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(gpio_base_c) + x"0000");
  constant gpio_out_addr_c  : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(gpio_base_c) + x"0002");
  constant gpio_ctrl_addr_c : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(gpio_base_c) + x"0004");

  -- IO: High-Precision Timer --
  constant timer_base_c : std_ulogic_vector(15 downto 0) := x"FFC0";
  constant timer_size_c : natural := 6; -- bytes

  constant timer_tcnt_addr_c  : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(timer_base_c) + x"0000");
  constant timer_thres_addr_c : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(timer_base_c) + x"0002");
  constant timer_tctrl_addr_c : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(timer_base_c) + x"0004");

  -- IO: Watchdog Timer --
  constant wdt_base_c : std_ulogic_vector(15 downto 0) := x"FFD0";
  constant wdt_size_c : natural := 2; -- bytes

  constant wbt_ctrl_addr_c : std_ulogic_vector(15 downto 0) := std_ulogic_vector(unsigned(wdt_base_c) + x"0000");

  -- IO: System Configuration --
  constant sysconfig_base_c : std_ulogic_vector(15 downto 0) := x"FFE0";
  constant sysconfig_size_c : natural := 32; -- bytes

  -- Register Addresses ---------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  constant reg_pc_c : std_ulogic_vector(3 downto 0) := x"0"; -- program counter
  constant reg_sp_c : std_ulogic_vector(3 downto 0) := x"1"; -- stack pointer
  constant reg_sr_c : std_ulogic_vector(3 downto 0) := x"2"; -- status register
  constant reg_cg_c : std_ulogic_vector(3 downto 0) := x"3"; -- constant generator

  -- Status Register ------------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  constant sreg_c_c : natural := 0;  -- carry flag
  constant sreg_z_c : natural := 1;  -- zero flag
  constant sreg_n_c : natural := 2;  -- negative flag
  constant sreg_i_c : natural := 3;  -- global interrupt enable
  constant sreg_s_c : natural := 4;  -- CPU sleep flag
  constant sreg_v_c : natural := 8;  -- overflow flag
  constant sreg_r_c : natural := 15; -- enable write access to IMEM (ROM) when set

  -- ALU Flag Bus ---------------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  constant flag_c_c : natural := 0; -- carry flag
  constant flag_z_c : natural := 1; -- zero flag
  constant flag_n_c : natural := 2; -- negative flag
  constant flag_v_c : natural := 3; -- overflow flag

  -- Main Control Bus -----------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  -- register file --
  constant ctrl_rf_in_sel_c   : natural :=  0; -- input source
  constant ctrl_rf_adr0_c     : natural :=  1; -- source/destination register address bit 0
  constant ctrl_rf_adr1_c     : natural :=  2; -- source/destination register address bit 1
  constant ctrl_rf_adr2_c     : natural :=  3; -- source/destination register address bit 2
  constant ctrl_rf_adr3_c     : natural :=  4; -- source/destination register address bit 3
  constant ctrl_rf_as0_c      : natural :=  5; -- source addressing mode bit 0
  constant ctrl_rf_as1_c      : natural :=  6; -- source addressing mode bit 1
  constant ctrl_rf_ad_c       : natural :=  7; -- destination addressing mode
  constant ctrl_rf_fup_c      : natural :=  8; -- update ALU flags
  constant ctrl_rf_wb_en_c    : natural :=  9; -- enable RF write back
  constant ctrl_rf_dsleep_c   : natural := 10; -- disable sleep mode
  constant ctrl_rf_boot_c     : natural := 11; -- inject PC boot address
  -- alu --
  constant ctrl_alu_in_sel_c  : natural := 12; -- ALU OP input select
  constant ctrl_alu_opa_wr_c  : natural := 13; -- write ALU operand A
  constant ctrl_alu_opb_wr_c  : natural := 14; -- write ALU operand B
  constant ctrl_alu_cmd0_c    : natural := 15; -- ALU command bit 0
  constant ctrl_alu_cmd1_c    : natural := 16; -- ALU command bit 1
  constant ctrl_alu_cmd2_c    : natural := 17; -- ALU command bit 2
  constant ctrl_alu_cmd3_c    : natural := 18; -- ALU command bit 3
  constant ctrl_alu_bw_c      : natural := 19; -- byte(1)/word(0) operation
  -- address generator --
  constant ctrl_adr_off0_c    : natural := 20; -- address offset selection bit 0
  constant ctrl_adr_off1_c    : natural := 21; -- address offset selection bit 1
  constant ctrl_adr_imm_en_c  : natural := 22; -- select immediate branch input
  constant ctrl_adr_mar_sel_c : natural := 23; -- select input for MAR
  constant ctrl_adr_bp_en_c   : natural := 24; -- mem addr output select, 0:MAR, 1:bypass
  constant ctrl_adr_ivec_oe_c : natural := 25; -- output IRQ if 1, else output PC
  constant ctrl_adr_mar_wr_c  : natural := 26; -- write MAR
  -- memory interface --
  constant ctrl_mem_wr_c      : natural := 27; -- write to memory
  constant ctrl_mem_rd_c      : natural := 28; -- read from memory
  -- bus size --
  constant ctrl_width_c       : natural := 29; -- control bus size

  -- Condition Codes ------------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  constant cond_ne_c : std_ulogic_vector(2 downto 0) := "000"; -- not equal
  constant cond_eq_c : std_ulogic_vector(2 downto 0) := "001"; -- equal
  constant cond_lo_c : std_ulogic_vector(2 downto 0) := "010"; -- lower
  constant cond_hs_c : std_ulogic_vector(2 downto 0) := "011"; -- higher or same
  constant cond_mi_c : std_ulogic_vector(2 downto 0) := "100"; -- negative
  constant cond_ge_c : std_ulogic_vector(2 downto 0) := "101"; -- greater or equal
  constant cond_le_c : std_ulogic_vector(2 downto 0) := "110"; -- less
  constant cond_al_c : std_ulogic_vector(2 downto 0) := "111"; -- always

  -- ALU Function Codes ---------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  constant alu_rrc_c  : std_ulogic_vector(3 downto 0) := "0000"; -- r <= a >>> 1, rotate right through carry
  constant alu_swap_c : std_ulogic_vector(3 downto 0) := "0001"; -- r <= swap bytes of a
  constant alu_rra_c  : std_ulogic_vector(3 downto 0) := "0010"; -- r <= a >>> 1, rotate right arithmetically
  constant alu_sxt_c  : std_ulogic_vector(3 downto 0) := "0011"; -- r <= a, sign extend byte
  constant alu_mov_c  : std_ulogic_vector(3 downto 0) := "0100"; -- r <= a
  constant alu_add_c  : std_ulogic_vector(3 downto 0) := "0101"; -- r <= a + b
  constant alu_addc_c : std_ulogic_vector(3 downto 0) := "0110"; -- r <= a + b + carry
  constant alu_subc_c : std_ulogic_vector(3 downto 0) := "0111"; -- r <= b - a - 1 + carry
  constant alu_sub_c  : std_ulogic_vector(3 downto 0) := "1000"; -- r <= b - a
  constant alu_cmp_c  : std_ulogic_vector(3 downto 0) := "1001"; -- b - a (no write back)
  constant alu_dadd_c : std_ulogic_vector(3 downto 0) := "1010"; -- r <= a + b (decimal)
  constant alu_bit_c  : std_ulogic_vector(3 downto 0) := "1011"; -- r <= a & b
  constant alu_bic_c  : std_ulogic_vector(3 downto 0) := "1100"; -- r <= !a & b
  constant alu_bis_c  : std_ulogic_vector(3 downto 0) := "1101"; -- r <= a | b
  constant alu_xor_c  : std_ulogic_vector(3 downto 0) := "1110"; -- r <= a xor b
  constant alu_and_c  : std_ulogic_vector(3 downto 0) := "1111"; -- r <= a & b

  -- Component: Control ---------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  component neo430_control
  generic (
    DADD_USE : boolean := true -- implement DADD instruction? (default=true)
  );
  port (
    -- global control --
    clk_i      : in  std_ulogic; -- global clock, rising edge
    rst_i      : in  std_ulogic; -- global reset, low-active, async
    -- memory interface --
    instr_i    : in  std_ulogic_vector(15 downto 0); -- instruction word from memory
    -- control --
    sreg_i     : in  std_ulogic_vector(15 downto 0); -- current status register
    ctrl_o     : out std_ulogic_vector(ctrl_width_c-1 downto 0); -- control signals
    irq_vec_o  : out std_ulogic_vector(01 downto 0); -- irq channel address
    imm_o      : out std_ulogic_vector(15 downto 0); -- branch offset
    -- irq lines --
    irq_i      : in  std_ulogic_vector(03 downto 0)  -- IRQ lines
  );
  end component;

  -- Component: Register File ---------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  component neo430_reg_file
  generic (
    BOOTLD_USE : boolean := true -- implement and use bootloader? (default=true)
  );
  port (
    -- global control --
    clk_i      : in  std_ulogic; -- global clock, rising edge
    rst_i      : in  std_ulogic; -- global reset, low-active, async
    -- data input --
    alu_i      : in  std_ulogic_vector(15 downto 0); -- data from alu
    addr_i     : in  std_ulogic_vector(15 downto 0); -- data from addr unit
    flag_i     : in  std_ulogic_vector(03 downto 0); -- new ALU flags
    -- control --
    ctrl_i     : in  std_ulogic_vector(ctrl_width_c-1 downto 0);
    -- data output --
    data_o     : out std_ulogic_vector(15 downto 0); -- read data
    sreg_o     : out std_ulogic_vector(15 downto 0)  -- current SR
  );
  end component;

  -- Component: Data ALU --------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  component neo430_alu
  generic (
    DADD_USE : boolean := true -- implement DADD instruction? (default=true)
  );
  port (
    -- global control --
    clk_i      : in  std_ulogic; -- global clock, rising edge
    -- operands --
    reg_i      : in  std_ulogic_vector(15 downto 0); -- data from reg file
    mem_i      : in  std_ulogic_vector(15 downto 0); -- data from memory
    sreg_i     : in  std_ulogic_vector(15 downto 0); -- current SR
    -- control --
    ctrl_i     : in  std_ulogic_vector(ctrl_width_c-1 downto 0);
    -- results --
    data_o     : out std_ulogic_vector(15 downto 0); -- result
    flag_o     : out std_ulogic_vector(03 downto 0)  -- new ALU flags
  );
  end component;

  -- Component: Address Generator -----------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  component neo430_addr_gen
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
  end component;

  -- Component: CPU core --------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  component neo430_cpu
  generic (
    DADD_USE   : boolean := true; -- implement DADD instruction? (default=true)
    BOOTLD_USE : boolean := true  -- implement and use bootloader? (default=true)
  );
  port(
    -- global control --
    clk_i      : in  std_ulogic; -- global clock, rising edge
    rst_i      : in  std_ulogic; -- global reset, low-active, async
    -- memory interface --
    mem_rd_o   : out std_ulogic; -- memory read
    mem_imwe_o : out std_ulogic; -- allow writing to IMEM
    mem_wr_o   : out std_ulogic_vector(01 downto 0); -- memory write
    mem_addr_o : out std_ulogic_vector(15 downto 0); -- address
    mem_data_o : out std_ulogic_vector(15 downto 0); -- write data
    mem_data_i : in  std_ulogic_vector(15 downto 0); -- read data
    -- interrupt system --
    irq_i      : in  std_ulogic_vector(03 downto 0)  -- interrupt requests
  );
  end component;

  -- Component: Instruction Memory (ROM) ----------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  component neo430_imem
  generic (
    IMEM_SIZE   : natural := 4*1024; -- internal IMEM size in bytes, max 32kB (default=4kB)
    IMEM_AS_ROM : boolean := false -- implement IMEM as read-only memory? (default=false)
  );
  port (
    clk_i  : in  std_ulogic; -- global clock line
    rden_i : in  std_ulogic; -- read enable
    wren_i : in  std_ulogic_vector(01 downto 0); -- write enable
    upen_i : in  std_ulogic; -- update enable
    addr_i : in  std_ulogic_vector(15 downto 0); -- address
    data_i : in  std_ulogic_vector(15 downto 0); -- data in
    data_o : out std_ulogic_vector(15 downto 0)  -- data out
  );
  end component;

  -- Component: Data Memory (RAM) -----------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  component neo430_dmem
  generic (
    DMEM_SIZE : natural := 2*1024 -- internal DMEM size in bytes, max 28kB (default=2kB)
  );
  port (
    clk_i  : in  std_ulogic; -- global clock line
    rden_i : in  std_ulogic; -- read enable
    wren_i : in  std_ulogic_vector(01 downto 0); -- write enable
    addr_i : in  std_ulogic_vector(15 downto 0); -- address
    data_i : in  std_ulogic_vector(15 downto 0); -- data in
    data_o : out std_ulogic_vector(15 downto 0)  -- data out
  );
  end component;

  -- Component: Bootloader ROM --------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  component neo430_boot_rom
  port (
    clk_i  : in  std_ulogic; -- global clock line
    rden_i : in  std_ulogic; -- read enable
    addr_i : in  std_ulogic_vector(15 downto 0); -- address
    data_o : out std_ulogic_vector(15 downto 0)  -- data out
  );
  end component;

  -- Component: 32bit Wishbone Interface ----------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  component neo430_wb_interface
  port (
    -- host access --
    clk_i       : in  std_ulogic; -- global clock line
    rden_i      : in  std_ulogic; -- read enable
    wren_i      : in  std_ulogic_vector(01 downto 0); -- write enable
    addr_i      : in  std_ulogic_vector(15 downto 0); -- address
    data_i      : in  std_ulogic_vector(15 downto 0); -- data in
    data_o      : out std_ulogic_vector(15 downto 0); -- data out
    -- wishbone interface --
    wb_adr_o    : out std_ulogic_vector(31 downto 0); -- address
    wb_dat_i    : in  std_ulogic_vector(31 downto 0); -- read data
    wb_dat_o    : out std_ulogic_vector(31 downto 0); -- write data
    wb_we_o     : out std_ulogic; -- read/write
    wb_sel_o    : out std_ulogic_vector(03 downto 0); -- byte enable
    wb_stb_o    : out std_ulogic; -- strobe
    wb_cyc_o    : out std_ulogic; -- valid cycle
    wb_ack_i    : in  std_ulogic  -- transfer acknowledge
  );
  end component;

  -- Component: USART -----------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  component neo430_usart
  port (
    -- host access --
    clk_i       : in  std_ulogic; -- global clock line
    rden_i      : in  std_ulogic; -- read enable
    wren_i      : in  std_ulogic_vector(01 downto 0); -- write enable
    addr_i      : in  std_ulogic_vector(15 downto 0); -- address
    data_i      : in  std_ulogic_vector(15 downto 0); -- data in
    data_o      : out std_ulogic_vector(15 downto 0); -- data out
    -- clock generator --
    clkgen_en_o : out std_ulogic; -- enable clock generator
    clkgen_i    : in  std_ulogic_vector(07 downto 0);
    -- com lines --
    uart_txd_o  : out std_ulogic;
    uart_rxd_i  : in  std_ulogic;
    spi_sclk_o  : out std_ulogic; -- SPI serial clock
    spi_mosi_o  : out std_ulogic; -- SPI master out, slave in
    spi_miso_i  : in  std_ulogic; -- SPI master in, slave out
    spi_cs_o    : out std_ulogic_vector(05 downto 0); -- SPI CS 0..5
    -- interrupts --
    usart_irq_o : out std_ulogic  -- spi transmission done / uart rx/tx interrupt
  );
  end component;

  -- Component: GPIO ------------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  component neo430_gpio
  port (
    -- host access --
    clk_i       : in  std_ulogic; -- global clock line
    rden_i      : in  std_ulogic; -- read enable
    wren_i      : in  std_ulogic_vector(01 downto 0); -- write enable
    addr_i      : in  std_ulogic_vector(15 downto 0); -- address
    data_i      : in  std_ulogic_vector(15 downto 0); -- data in
    data_o      : out std_ulogic_vector(15 downto 0); -- data out
    -- parallel io --
    gpio_o      : out std_ulogic_vector(15 downto 0);
    gpio_i      : in  std_ulogic_vector(15 downto 0);
    -- interrupt --
    irq_o       : out std_ulogic
  );
  end component;

  -- Component: High-Precision Timer --------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  component neo430_timer
  port (
    -- host access --
    clk_i       : in  std_ulogic; -- global clock line
    rden_i      : in  std_ulogic; -- read enable
    wren_i      : in  std_ulogic_vector(01 downto 0); -- write enable
    addr_i      : in  std_ulogic_vector(15 downto 0); -- address
    data_i      : in  std_ulogic_vector(15 downto 0); -- data in
    data_o      : out std_ulogic_vector(15 downto 0); -- data out
    -- clock generator --
    clkgen_en_o : out std_ulogic; -- enable clock generator
    clkgen_i    : in  std_ulogic_vector(07 downto 0);
    -- interrupt --
    irq_o       : out std_ulogic  -- interrupt request
  );
  end component;

  -- Component: Watchdog Timer --------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  component neo430_wdt
  port (
    -- host access --
    clk_i       : in  std_ulogic; -- global clock line
    rst_i       : in  std_ulogic; -- global (external) reset, low-active, use as async
    rden_i      : in  std_ulogic; -- read enable
    wren_i      : in  std_ulogic_vector(01 downto 0); -- write enable
    addr_i      : in  std_ulogic_vector(15 downto 0); -- address
    data_i      : in  std_ulogic_vector(15 downto 0); -- data in
    data_o      : out std_ulogic_vector(15 downto 0); -- data out
    -- clock generator --
    clkgen_en_o : out std_ulogic; -- enable clock generator
    clkgen_i    : in  std_ulogic_vector(07 downto 0);
    -- system reset --
    rst_o       :  out std_ulogic  -- timeout reset, low_active, use as async
  );
  end component;

  -- Component: System Configuration --------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  component neo430_sysconfig
  generic (
    -- general configuration --
    CLOCK_SPEED : natural := 100000000; -- main clock in Hz
    IMEM_SIZE   : natural := 4*1024; -- internal IMEM size in bytes, max 32kB (default=4kB)
    DMEM_SIZE   : natural := 2*1024; -- internal DMEM size in bytes, max 28kB (default=2kB)
    -- additional configuration --
    USER_CODE   : std_ulogic_vector(15 downto 0) := x"0000"; -- custom user code
    -- module configuration --
    DADD_USE    : boolean := true; -- implement DADD instruction? (default=true)
    WB32_USE    : boolean := true; -- implement WB32 unit? (default=true)
    WDT_USE     : boolean := true; -- implement WBT? (default=true)
    GPIO_USE    : boolean := true; -- implement GPIO unit? (default=true)
    TIMER_USE   : boolean := true; -- implement timer? (default=true)
    USART_USE   : boolean := true; -- implement USART? (default=true)
    -- boot configuration --
    BOOTLD_USE  : boolean := true; -- implement and use bootloader? (default=true)
    IMEM_AS_ROM : boolean := false -- implement IMEM as read-only memory? (default=false)
  );
  port (
    clk_i  : in  std_ulogic; -- global clock line
    rden_i : in  std_ulogic; -- read enable
    wren_i : in  std_ulogic_vector(01 downto 0); -- write enable
    addr_i : in  std_ulogic_vector(15 downto 0); -- address
    data_i : in  std_ulogic_vector(15 downto 0); -- data in
    data_o : out std_ulogic_vector(15 downto 0)  -- data out
  );
  end component;

end neo430_package;

package body neo430_package is

  -- Function: Minimum required bit width ---------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  function index_size(input : natural) return natural is
  begin
    if (input = 0) then
      return 0;
    end if;
    for i in 0 to natural'high loop
      if (2**i >= input) then
        return i;
      end if;
    end loop;
    return 0;
  end function index_size;

  -- Function: Bit reversal -----------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  function bit_reversal(input : std_ulogic_vector) return std_ulogic_vector is
    variable output_v : std_ulogic_vector(input'range);
  begin
    for i in 0 to input'length-1 loop
      output_v(input'length-i-1) := input(i);
    end loop; -- i
    return output_v;
  end function bit_reversal;

  -- Function: Count number of set bits (aka population count) ------------------------------
  -- -------------------------------------------------------------------------------------------
  function set_bits(input : std_ulogic_vector) return natural is
    variable cnt_v : natural range 0 to input'length-1;
  begin
    cnt_v := 0;
    for i in input'length-1 downto 0 loop
      if (input(i) = '1') then
        cnt_v := cnt_v + 1;
      end if;
    end loop; -- i
    return cnt_v;
  end function set_bits;

  -- Function: Count leading zeros ----------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  function leading_zeros(input : std_ulogic_vector) return natural is
    variable cnt_v : natural range 0 to input'length;
  begin
    cnt_v := 0;
    for i in input'length-1 downto 0 loop
      if (input(i) = '0') then
        cnt_v := cnt_v + 1;
      else
        exit;
      end if;
    end loop; -- i
    return cnt_v;
  end function leading_zeros;

  -- Function: Conditional select natural ---------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  function cond_sel_natural(cond: boolean; val_t: natural; val_f: natural) return natural is
  begin
    if (cond = true) then
      return val_t;
    else
      return val_f;
    end if;
  end function cond_sel_natural;

  -- Function: Conditional select std_ulogic_vector -----------------------------------------
  -- -------------------------------------------------------------------------------------------
  function cond_sel_stdulogicvector(cond: boolean; val_t: std_ulogic_vector; val_f: std_ulogic_vector) return std_ulogic_vector is
  begin
    if (cond = true) then
      return val_t;
    else
      return val_f;
    end if;
  end function cond_sel_stdulogicvector;

  -- Function: Convert BOOL to STD_ULOGIC ---------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  function bool_to_ulogic(cond: boolean) return std_ulogic is
  begin
    if (cond = true) then
      return '1';
    else
      return '0';
    end if;
  end function bool_to_ulogic;

  -- Function: Binary to Gray ---------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  function bin_to_gray(input : std_ulogic_vector) return std_ulogic_vector is
    variable output_v : std_ulogic_vector(input'range);
  begin
    output_v(input'length-1) := input(input'length-1); -- keep MSB
    for i in input'length-2 to 0 loop
      output_v(i) := input(i+1) xor input(i);
    end loop; -- i
    return output_v;
  end function bin_to_gray;

  -- Function: Gray to Binary ---------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  function gray_to_bin(input : std_ulogic_vector) return std_ulogic_vector is
    variable output_v : std_ulogic_vector(input'range);
  begin
    output_v(input'length-1) := input(input'length-1); -- keep MSB
    for i in input'length-2 downto 0 loop
      output_v(i) := output_v(i+1) xor input(i);
    end loop; -- i
    return output_v;
  end function gray_to_bin;

end neo430_package;
