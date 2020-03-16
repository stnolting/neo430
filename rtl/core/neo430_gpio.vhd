-- #################################################################################################
-- #  << NEO430 - General Purpose Parallel IO Unit >>                                              #
-- # ********************************************************************************************* #
-- # 16-bit parallel input & output unit. Any pin-change (HI->LO or LO->HI) triggers the IRQ.      #
-- # Pins used for the pin change interrupt are selected using a 16-bit mask.                      #
-- # The PWM controller can be used to module the GPIO controller's output.                        #
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

entity neo430_gpio is
  port (
    -- host access --
    clk_i      : in  std_ulogic; -- global clock line
    rden_i     : in  std_ulogic; -- read enable
    wren_i     : in  std_ulogic; -- write enable
    addr_i     : in  std_ulogic_vector(15 downto 0); -- address
    data_i     : in  std_ulogic_vector(15 downto 0); -- data in
    data_o     : out std_ulogic_vector(15 downto 0); -- data out
    -- parallel io --
    gpio_o     : out std_ulogic_vector(15 downto 0);
    gpio_i     : in  std_ulogic_vector(15 downto 0);
    -- GPIO PWM --
    gpio_pwm_i : in  std_ulogic;
    -- interrupt --
    irq_o      : out std_ulogic
  );
end neo430_gpio;

architecture neo430_gpio_rtl of neo430_gpio is

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size_f(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size_f(gpio_size_c); -- low address boundary bit

  -- access control --
  signal acc_en : std_ulogic; -- module access enable
  signal addr   : std_ulogic_vector(15 downto 0); -- access address
  signal wren   : std_ulogic; -- word write enable
  signal rden   : std_ulogic; -- read enable

  -- accessible regs --
  signal dout, din : std_ulogic_vector(15 downto 0); -- r/w
  signal irq_mask  : std_ulogic_vector(15 downto 0); -- -/w

  -- misc --
  signal irq_raw, sync_in, in_buf : std_ulogic_vector(15 downto 0);

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = gpio_base_c(hi_abb_c downto lo_abb_c)) else '0';
  addr   <= gpio_base_c(15 downto lo_abb_c) & addr_i(lo_abb_c-1 downto 1) & '0'; -- word aligned
  wren   <= acc_en and wren_i;
  rden   <= acc_en and rden_i;


  -- Write access -------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  wr_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (wren = '1') then
        if (addr = gpio_out_addr_c) then
          dout <= data_i;
        end if;
        if (addr = gpio_irqmask_addr_c) then
          irq_mask <= data_i;
        end if;
      end if;
    end if;
  end process wr_access;

  -- (PWM modulated) output --
  gpio_o <= dout when (gpio_pwm_i = '1') else (others => '0');


  -- IRQ Generator ------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  irq_generator: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- input synchronizer --
      in_buf  <= gpio_i;
      din     <= in_buf;
      sync_in <= din;
      -- IRQ --
      irq_o <= or_all_f(irq_raw);
    end if;
  end process irq_generator;

  -- any transition triggers an interrupt (if enabled for according input pin) --
  irq_raw <= (din xor sync_in) and irq_mask;


  -- Read access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  rd_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- read access --
      data_o <= (others => '0');
      if (rden = '1') then
        if (addr = gpio_in_addr_c) then
          data_o <= din;
        else -- gpio_out_addr_c
          data_o <= dout;
        end if;
      end if;
    end if;
  end process rd_access;


end neo430_gpio_rtl;
