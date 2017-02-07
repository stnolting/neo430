-- #################################################################################################
-- #  << NEO430 - 16-bit Wishbone-compatible Memory Component >>                                   #
-- # ********************************************************************************************* #
-- #  'addr_width_g' specifies the effective WORD (16-bit!) address range of the device.           #
-- #  Thus, the actual memory size is 2*2^addr_width_g.                                            #
-- #  Only the lower 16-bit of the input/output data ports are used.                               #
-- # ********************************************************************************************* #
-- # This file is part of the NEO430 Processor project: http://opencores.org/project,neo430        #
-- # Copyright 2015-2016, Stephan Nolting: stnolting@gmail.com                                     #
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
-- #  Stephan Nolting, Hannover, Germany                                               22.09.2016  #
-- #################################################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity wb_mem is
  generic (
    addr_width_g : natural := 8; -- effective word-address width
    ram_en_g     : boolean := false -- use this as RAM or only as ROM?
  );
  port (
    -- wishbone bus interface --
    wb_clk_i  : in  std_ulogic; -- main clock
    wb_cyc_i  : in  std_ulogic; -- valid cycle
    wb_stb_i  : in  std_ulogic; -- valid strobe
    wb_we_i   : in  std_ulogic; -- write enable
    wb_addr_i : in  std_ulogic_vector(31 downto 0); -- access address
    wb_data_i : in  std_ulogic_vector(31 downto 0); -- write data
    wb_data_o : out std_ulogic_vector(31 downto 0); -- read data
    wb_ack_o  : out std_ulogic  -- acknowledge
  );
end wb_mem;

architecture wb_mem_rtl of wb_mem is

  -- memory --
  type mem_file_t is array (0 to (2**addr_width_g)-1) of std_ulogic_vector(15 downto 0);
  signal mem_file : mem_file_t;

  --- RAM attribute to inhibit bypass-logic - ALTERA ONLY! ---
  attribute ramstyle : string;
  attribute ramstyle of mem_file : signal is "no_rw_check";

  -- misc --
  signal rb_en  : std_ulogic;
  signal r_data : std_ulogic_vector(15 downto 0);

begin

  -- Memory Access ------------------------------------------------------------
  -- -----------------------------------------------------------------------------
    mem_file_access: process(wb_clk_i)
    begin
      if rising_edge(wb_clk_i) then
        if (ram_en_g = true) then -- is RAM?
          if ((wb_cyc_i and wb_stb_i) = '1') and (wb_we_i = '1') then -- valid write access
            mem_file(to_integer(unsigned(wb_addr_i(addr_width_g downto 1)))) <= wb_data_i(15 downto 0); -- word aligned
          end if;
        end if;
        r_data   <= mem_file(to_integer(unsigned(wb_addr_i(addr_width_g downto 1)))); -- word aligned
        rb_en    <= wb_cyc_i and wb_stb_i and (not wb_we_i); -- read-back control
        wb_ack_o <= wb_cyc_i and wb_stb_i; -- wishbone acknowledge
      end if;
    end process mem_file_access;

  -- output gate --
  wb_data_o <= x"0000" & r_data when (rb_en = '1') else (others=> '0');


end wb_mem_rtl;
