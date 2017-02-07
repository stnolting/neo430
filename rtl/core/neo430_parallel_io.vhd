-- #################################################################################################
-- #  << NEO430 - General Purpose Parallel IO Unit >>                                              #
-- # ********************************************************************************************* #
-- #  16-bit parallel input & ouptut unit. Any pin-change triggers the IRQ.                        #
-- #  Configurable IRQ trigger: Rising edge, falling edge, high-active or low-active.              #
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
-- #  Stephan Nolting, Hannover, Germany                                               28.12.2016  #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.neo430_package.all;

entity neo430_parallel_io is
  port (
    -- host access --
    clk_i  : in  std_ulogic; -- global clock line
    rden_i : in  std_ulogic; -- read enable
    wren_i : in  std_ulogic_vector(01 downto 0); -- write enable
    addr_i : in  std_ulogic_vector(15 downto 0); -- address
    data_i : in  std_ulogic_vector(15 downto 0); -- data in
    data_o : out std_ulogic_vector(15 downto 0); -- data out
    -- parallel io --
    pio_o  : out std_ulogic_vector(15 downto 0);
    pio_i  : in  std_ulogic_vector(15 downto 0);
    -- interrupt --
    irq_o  : out std_ulogic
  );
end neo430_parallel_io;

architecture neo430_parallel_io_rtl of neo430_parallel_io is

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size(pio_size_c); -- low address boundary bit

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal addr   : std_ulogic_vector(15 downto 0); -- access address
  signal wr_en  : std_ulogic; -- word write enable

  -- accessible regs --
  signal irq_en    : std_ulogic;
  signal trigger   : std_ulogic_vector(01 downto 0);
  signal dout, din : std_ulogic_vector(15 downto 0);

  -- misc --
  signal irq_raw, sync_in, in_buf : std_ulogic_vector(15 downto 0);

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = pio_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= pio_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wr_en  <= acc_en and wren_i(1) and wren_i(0);


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (wr_en = '1') then
        case addr is
          when pio_out_addr_c =>
           dout <= data_i;
          when pio_ctrl_addr_c =>
            trigger <= data_i(1 downto 0);
            irq_en  <= data_i(2);
          when others =>
            NULL;
        end case;
      end if;
    end if;
  end process wr_access;

  -- output --
  pio_o <= dout;


  -- IRQ Generator ------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  irq_trigger: process(trigger, din, sync_in)
  begin
    -- trigger type --
    case trigger is
      when "00" => -- low level
        irq_raw <= not din;
      when "01" => -- high level
        irq_raw <= din;
      when "10" => -- falling edge
        irq_raw <= (not sync_in) and din;
      when others => -- rising edge
        irq_raw <= sync_in and (not din);
    end case;
  end process irq_trigger;

  -- IRQ --
  irq_o <= irq_en when (irq_raw /= x"0000") else '0';


  -- Read access and IN sync --------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- input synchronizer --
      in_buf  <= pio_i;
      sync_in <= in_buf;
      din     <= sync_in;
      -- read access --
      data_o <= (others => '0');
      if (rden_i = '1') and (acc_en = '1') then
        if (addr = pio_in_addr_c) then
          data_o <= din;
        else
          data_o <= dout;
        end if;
      end if;
    end if;
  end process rd_access;


end neo430_parallel_io_rtl;
