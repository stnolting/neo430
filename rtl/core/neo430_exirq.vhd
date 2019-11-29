-- #################################################################################################
-- #  << NEO430 - External Interrupts Controller >>                                                #
-- # ********************************************************************************************* #
-- # This unit provides 8 maskable external interrupt lines with according ACK lines. The IRQ      #
-- # can be globally set to trigger either on a high-level or on a rising edge. Each line has a    #
-- # unique enable bit. The acknowledge output is set high for one clock cycle to confirm the      #
-- # interrupt has been sampled and has also been cpatured by the handler function.                #
-- # All external interrupt requests are forwarded to a single CPU interrupt. The according        #
-- # has to check the SRC bits in the unit's control register to determine the actual source and   #
-- # start the according handler function.                                                         #
-- # If several IRQs occur at the same time, the one with highest priority is executed while the   #
-- # others are kept in a buffer. The buffer is reset when the global enable flag of the unit is   #
-- # cleared.                                                                                      #
-- # ext_irq_i(0) has highest priority while ext_irq_i(7) has the lowest priority.                 #
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
-- # Stephan Nolting, Hannover, Germany                                                 29.11.2019 #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;

entity neo430_exirq is
  port (
    -- host access --
    clk_i     : in  std_ulogic; -- global clock line
    rden_i    : in  std_ulogic; -- read enable
    wren_i    : in  std_ulogic; -- write enable
    addr_i    : in  std_ulogic_vector(15 downto 0); -- address
    data_i    : in  std_ulogic_vector(15 downto 0); -- data in
    data_o    : out std_ulogic_vector(15 downto 0); -- data out
    -- cpu interrupt --
    cpu_irq_o : out std_ulogic;
    -- external interrupt lines --
    ext_irq_i : in  std_ulogic_vector(7 downto 0); -- IRQ
    ext_ack_o : out std_ulogic_vector(7 downto 0)  -- acknowledge
  );
end neo430_exirq;

architecture neo430_exirq_rtl of neo430_exirq is

  -- control register bits --
  constant ctrl_src0_c    : natural :=  0; -- r/-: IRQ source bit 0
  constant ctrl_src1_c    : natural :=  1; -- r/-: IRQ source bit 1
  constant ctrl_src2_c    : natural :=  2; -- r/-: IRQ source bit 2
  constant ctrl_trig_c    : natural :=  3; -- r/w: global trigger (0: high-level, 1: rising-edge)
  constant ctrl_en_c      : natural :=  4; -- r/w: unit enable
  -- ...
  constant ctrl_en_irq0_c : natural :=  8; -- r/w: IRQ 0 enable
  constant ctrl_en_irq1_c : natural :=  9; -- r/w: IRQ 1 enable
  constant ctrl_en_irq2_c : natural := 10; -- r/w: IRQ 2 enable
  constant ctrl_en_irq3_c : natural := 11; -- r/w: IRQ 3 enable
  constant ctrl_en_irq4_c : natural := 12; -- r/w: IRQ 4 enable
  constant ctrl_en_irq5_c : natural := 13; -- r/w: IRQ 5 enable
  constant ctrl_en_irq6_c : natural := 14; -- r/w: IRQ 6 enable
  constant ctrl_en_irq7_c : natural := 15; -- r/w: IRQ 7 enable

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size_f(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size_f(exirq_size_c); -- low address boundary bit

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal wren   : std_ulogic; -- full word write enable
  signal rden   : std_ulogic; -- read enable

  -- r/w accessible registers --
  signal irq_enable  : std_ulogic_vector(7 downto 0);
  signal irq_trigger : std_ulogic;
  signal enable      : std_ulogic;

  -- irq input / ack output system --
  signal irq_sync0, irq_sync1, irq_sync2, irq_raw, irq_valid, ack_mask : std_ulogic_vector(7 downto 0);

  -- controller core --
  signal irq_buf              : std_ulogic_vector(7 downto 0);
  signal irq_src, irq_src_reg : std_ulogic_vector(2 downto 0);
  signal irq_fire, state      : std_ulogic;

begin

  -- Access control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = exirq_base_c(hi_abb_c downto lo_abb_c)) else '0';
  wren   <= acc_en and wren_i;
  rden   <= acc_en and rden_i;


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (wren = '1') then
        irq_trigger <= data_i(ctrl_trig_c);
        enable      <= data_i(ctrl_en_c);
        irq_enable  <= data_i(ctrl_en_irq7_c downto ctrl_en_irq0_c);
      end if;
    end if;
  end process wr_access;


  -- Get external interrupt request --------------------------------------------
  -- -----------------------------------------------------------------------------
  exirq_sync: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- no metastability! --
      irq_sync0 <= ext_irq_i;
      irq_sync1 <= irq_sync0;
      -- sample again for edge detection --
      irq_sync2 <= irq_sync1;
    end if;
  end process exirq_sync;

  -- rising-edge or high-level detector --
  irq_raw <= (irq_sync1 and (not irq_sync2)) when (irq_trigger = '1') else irq_sync1;

  -- only pass enabled interrupt sources --
  irq_valid <= irq_raw and irq_enable;


  -- IRQ controller core ------------------------------------------------------
  -- -----------------------------------------------------------------------------
  irq_core: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- irq buffer --
      for i in 0 to 7 loop
        -- keep requests until they are acknowledged
        -- clear buffer when unit is disabled
        irq_buf(i) <= (irq_buf(i) or irq_valid(i)) and enable and (not ack_mask(i));
      end loop; -- i

      -- mini state FSM - defaults --
      cpu_irq_o <= '0';
      ext_ack_o <= (others => '0');

      -- mini state FSM --
      if (state = '0') or (enable = '0') then -- idle or deactivated
        state       <= '0';
        irq_src_reg <= irq_src; -- capture source
        if (irq_fire = '1') then
          cpu_irq_o <= '1'; -- trigger CPU
          state     <= '1'; -- goto active IRQ state
        end if;

      else -- active IRQ
        if (rden = '1') then -- ACK on when reading IRQ source
          ext_ack_o <= ack_mask;
          state     <= '0';
        end if;
      end if;
    end if;
  end process irq_core;

  -- anybody firing? --
  irq_fire <= or_all_f(irq_buf);

  -- get interrupt priority --
  irq_src <= "000" when (irq_buf(0) = '1') else
             "001" when (irq_buf(1) = '1') else
             "010" when (irq_buf(2) = '1') else
             "011" when (irq_buf(3) = '1') else
             "100" when (irq_buf(4) = '1') else
             "101" when (irq_buf(5) = '1') else
             "110" when (irq_buf(6) = '1') else
             "111";-- when (irq_buf(7) = '1') else "---";


  -- ACK priority decoder -----------------------------------------------------
  -- -----------------------------------------------------------------------------
  ack_priority_dec: process(irq_src_reg)
  begin
    case irq_src_reg is
      when "000"  => ack_mask <= "00000001";
      when "001"  => ack_mask <= "00000010";
      when "010"  => ack_mask <= "00000100";
      when "011"  => ack_mask <= "00001000";
      when "100"  => ack_mask <= "00010000";
      when "101"  => ack_mask <= "00100000";
      when "110"  => ack_mask <= "01000000";
      when "111"  => ack_mask <= "10000000";
      when others => ack_mask <= "--------";
    end case;
  end process ack_priority_dec;


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0');
      if (rden = '1') then
        data_o(ctrl_src2_c downto ctrl_src0_c) <= irq_src_reg;
        data_o(ctrl_en_irq7_c downto ctrl_en_irq0_c) <= irq_enable;
        data_o(ctrl_trig_c) <= irq_trigger;
        data_o(ctrl_en_c) <= enable;
      end if;
    end if;
  end process rd_access;


end neo430_exirq_rtl;
