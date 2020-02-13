-- #################################################################################################
-- #  << NEO430 - Custom Functions Unit >>                                                         #
-- # ********************************************************************************************* #
-- # This unit is a template for implementing custom functions, which are directly memory-mapped   #
-- # into the CPU's IO address space. The address space of this unit is 16 bytes large. This unit  #
-- # can only be accessed using full word (16-bit) accesses.                                       #
-- # In the original state, this unit only provides 8 16-bit register, that do not perform any     #
-- # kind of data manipulation.                                                                    #
-- # Exemplary applications: Cryptography, complex arithmetic, rocket science, ...                 #
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
-- # Stephan Nolting, Hannover, Germany                                                 13.02.2020 #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neo430;
use neo430.neo430_package.all;

entity neo430_cfu is
  port (
    -- host access --
    clk_i       : in  std_ulogic; -- global clock line
    rden_i      : in  std_ulogic; -- read enable
    wren_i      : in  std_ulogic; -- write enable
    addr_i      : in  std_ulogic_vector(15 downto 0); -- address
    data_i      : in  std_ulogic_vector(15 downto 0); -- data in
    data_o      : out std_ulogic_vector(15 downto 0); -- data out
    -- clock generator --
    clkgen_en_o : out std_ulogic; -- enable clock generator
    clkgen_i    : in  std_ulogic_vector(07 downto 0)
    -- custom IOs --
--  ...
  );
end neo430_cfu;

architecture neo430_cfu_rtl of neo430_cfu is

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size_f(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size_f(cfu_size_c); -- low address boundary bit

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal addr   : std_ulogic_vector(15 downto 0); -- access address
  signal wren   : std_ulogic; -- full word write enable
  signal rden   : std_ulogic; -- read enable

  -- accessible regs (8x16-bit) --
  signal cfu_ctrl_reg : std_ulogic_vector(15 downto 0);
  signal user_reg1    : std_ulogic_vector(15 downto 0);
  signal user_reg2    : std_ulogic_vector(15 downto 0);
  signal user_reg3    : std_ulogic_vector(15 downto 0);
  signal user_reg4    : std_ulogic_vector(15 downto 0);
  signal user_reg5    : std_ulogic_vector(15 downto 0);
  signal user_reg6    : std_ulogic_vector(15 downto 0);
  signal user_reg7    : std_ulogic_vector(15 downto 0);

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  -- These assignments are required to check if this unit is accessed at all.
  -- Do NOT modify this for your custom application (unless you really know what you are doing)!
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = cfu_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= cfu_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wren   <= acc_en and wren_i;
  rden   <= acc_en and rden_i;


  -- Clock System -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  -- The top unit implements a clock generator providing 8 "derived clocks"
  -- Actually, these signals must not be used as direct clock signals, but as clock enable signals.
  -- If wou want to drive a system at MAIN_CLK/8 use the following construct:

  -- if rising_edge(clk_i) then -- Always use the main clock for all clock processes!
  --   if (clkgen_i(clk_div8_c) = '1') then -- the div8 "clock" is actually a clock enable
  --     ...
  --   end if;
  -- end if;

  -- The following clock divider rates are available:
  -- clkgen_i(clk_div2_c)    -> MAIN_CLK/2
  -- clkgen_i(clk_div4_c)    -> MAIN_CLK/4
  -- clkgen_i(clk_div8_c)    -> MAIN_CLK/8
  -- clkgen_i(clk_div64_c)   -> MAIN_CLK/64
  -- clkgen_i(clk_div128_c)  -> MAIN_CLK/128
  -- clkgen_i(clk_div1024_c) -> MAIN_CLK/1024
  -- clkgen_i(clk_div2048_c) -> MAIN_CLK/2048
  -- clkgen_i(clk_div4096_c) -> MAIN_CLK/4096

  -- this signal enabled the generator driving the clkgen_i
  -- set this signal to '0' when you do not need the clkgen_i signal or when your CFU is disabled
  -- to reduce dynamic power consumption
  clkgen_en_o <= '0';


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  -- Here we are writing to the interface registers of the module. This unit can only be accessed
  -- in full 16-bit word mode!
  -- Please note, that all register of every unit are cleared during the processor boot sequence.
  -- Make cfu_reg0_addr_c the CFU's control register. This register is cleared first during booting.
  -- If the control register is cleared no actions should be taken when writing to other CFU registers.
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- write access to user registers --
      if (wren = '1') then -- valid write access
        -- use full-parallel IFs instead of a CASE to prevent some EDA tools from complaining (GHDL)
        if (addr = cfu_reg0_addr_c) then
          cfu_ctrl_reg <= data_i;
        end if;
        if (addr = cfu_reg1_addr_c) then
          user_reg1 <= data_i;
        end if;
        if (addr = cfu_reg2_addr_c) then
          user_reg2 <= data_i;
        end if;
        if (addr = cfu_reg3_addr_c) then
          user_reg3 <= data_i;
        end if;
        if (addr = cfu_reg4_addr_c) then
          user_reg4 <= data_i;
        end if;
        if (addr = cfu_reg5_addr_c) then
          user_reg5 <= data_i;
        end if;
        if (addr = cfu_reg6_addr_c) then
          user_reg6 <= data_i;
        end if;
        if (addr = cfu_reg7_addr_c) then
          user_reg7 <= data_i;
        end if;
      end if;
    end if;
  end process wr_access;

  -- >>> UNIT HARDWARE RESET <<< --
  -- The IO devices DO NOT feature a dedicated reset signal, so make sure your CFU does not require a defined initial state.
  -- If you really require a defined initial state, implement a software reset by implementing a control register with an
  -- enable bit, which resets all internal states when cleared.


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  -- This is the read access process. Data must be asserted synchronously to the output data bus
  -- and thus, with exactly 1 cycle delay. The units always output a full 16-bit word, no matter if we want to
  -- read 8- or 16-bit. For actual 8-bit read accesses the corresponding byte is selected in the
  -- hardware of the CPU core.
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0'); -- this is crucial for the final OR-ing of all IO device's outputs
      if (rden = '1') then -- valid read access
        -- use IFs instead of a CASE to prevent some EDA tools from complaining (GHDL)
        if (addr = cfu_reg0_addr_c) then
          data_o <= cfu_ctrl_reg;
        elsif (addr = cfu_reg1_addr_c) then
          data_o <= user_reg1;
        elsif (addr = cfu_reg2_addr_c) then
          data_o <= user_reg2;
        elsif (addr = cfu_reg3_addr_c) then
          data_o <= user_reg3;
        elsif (addr = cfu_reg4_addr_c) then
          data_o <= user_reg4;
        elsif (addr = cfu_reg5_addr_c) then
          data_o <= user_reg5;
        elsif (addr = cfu_reg6_addr_c) then
          data_o <= user_reg6;
        elsif (addr = cfu_reg7_addr_c) then
          data_o <= user_reg7;
        else
          data_o <= (others => '0');
        end if;
      end if;
    end if;
  end process rd_access;


end neo430_cfu_rtl;
