-- #################################################################################################
-- # << NEO430 - Watchdog Timer >>                                                                 #
-- # ********************************************************************************************* #
-- # The internal counter is 16 bit wide and triggers a HW reset when overflowing. The clock is    #
-- # selected via the clk_sel bits of the control register. The WDT can only operate when the      #
-- # enable bit is set. A write access to the WDT can only be performed, if the higher byte of the #
-- # written data contains the specific WDT password (0x47). I a write access occurs with a wrong  #
-- # password, a HW reset is triggered, but only if the WDT is enabled.                            #
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

entity neo430_wdt is
  port (
    -- host access --
    clk_i       : in  std_ulogic; -- global clock line
    rst_i       : in  std_ulogic; -- external reset, low-active, use as async
    rden_i      : in  std_ulogic; -- read enable
    wren_i      : in  std_ulogic; -- write enable
    addr_i      : in  std_ulogic_vector(15 downto 0); -- address
    data_i      : in  std_ulogic_vector(15 downto 0); -- data in
    data_o      : out std_ulogic_vector(15 downto 0); -- data out
    -- clock generator --
    clkgen_en_o : out std_ulogic; -- enable clock generator
    clkgen_i    : in  std_ulogic_vector(07 downto 0);
    -- system reset --
    rst_o       : out std_ulogic -- timeout reset, low_active, use it as async!
  );
end neo430_wdt;

architecture neo430_wdt_rtl of neo430_wdt is

  -- IO space: module base address --
  constant hi_abb_c : natural := index_size_f(io_size_c)-1; -- high address boundary bit
  constant lo_abb_c : natural := index_size_f(wdt_size_c); -- low address boundary bit

  -- Watchdog access password - do not change! --
  constant wdt_password_c : std_ulogic_vector(07 downto 0) := x"47";

  -- Control register bits --
  constant ctrl_clksel0_c : natural := 0; -- r/w: prescaler select bit 0
  constant ctrl_clksel1_c : natural := 1; -- r/w: prescaler select bit 1
  constant ctrl_clksel2_c : natural := 2; -- r/w: prescaler select bit 2
  constant ctrl_enable_c  : natural := 3; -- r/w: WDT enable
  constant ctrl_rcause_c  : natural := 4; -- r/-: reset cause (0: external, 1: watchdog timeout)
  constant ctrl_rpwfail_c : natural := 5; -- r/-: watchdog reset caused by wrong password access when '1'

  -- access control --
  signal acc_en        : std_ulogic; -- module access enable
  signal pwd_ok        : std_ulogic; -- password correct
  signal fail, fail_ff : std_ulogic; -- unauthorized access
  signal wren          : std_ulogic;

  -- accessible regs --
  signal rst_source : std_ulogic; -- source of the system reset: '0' = external, '1' = watchdog timeout
  signal pw_fail    : std_ulogic; -- watchdog reset caused by wrong password access
  signal enable     : std_ulogic;
  signal clk_sel    : std_ulogic_vector(02 downto 0);

  -- reset counter --
  signal cnt      : std_ulogic_vector(16 downto 0);
  signal rst_gen  : std_ulogic_vector(03 downto 0);
  signal rst_sync : std_ulogic_vector(01 downto 0);

  -- prescaler clock generator --
  signal prsc_tick : std_ulogic;

begin

  -- Access Control -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  acc_en <= '1' when (addr_i(hi_abb_c downto lo_abb_c) = wdt_base_c(hi_abb_c downto lo_abb_c)) else '0';
  pwd_ok <= '1' when (data_i(15 downto 8) = wdt_password_c) else '0'; -- password check
  wren   <= '1' when ((acc_en = '1') and (wren_i = '1') and (pwd_ok = '1')) else '0'; -- write access ok
  fail   <= '1' when ((acc_en = '1') and (wren_i = '1') and (pwd_ok = '0')) else '0'; -- write access fail!


  -- Write Access, Reset Generator --------------------------------------------
  -- -----------------------------------------------------------------------------
  wdt_core: process(clk_i)
  begin
    if rising_edge(clk_i) then
      if (rst_i = '0') or (rst_sync(1) = '0') then -- external or internal reset
        enable  <= '0'; -- disable WDT
        clk_sel <= (others => '1'); -- slowest clock rst_source
        rst_gen <= (others => '1'); -- do NOT fire on reset!
      else
        -- control register write access --
        if (wren = '1') then -- allow write if password is correct
          enable  <= data_i(ctrl_enable_c);
          clk_sel <= data_i(ctrl_clksel2_c downto ctrl_clksel0_c);
        end if;
        -- reset generator - enabled and (overflow or unauthorized access)? --
        if (enable = '1') and ((cnt(cnt'left) = '1') or (fail_ff = '1')) then
          rst_gen <= (others => '0');
        else
          rst_gen <= rst_gen(rst_gen'left-1 downto 0) & '1';
        end if;
      end if;
    end if;
  end process wdt_core;

  -- enable external clock generator --
  clkgen_en_o <= enable;


  -- Counter Update -----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  cnt_sync: process(clk_i)
  begin
    if rising_edge(clk_i) then
      -- clock_en buffer --
      prsc_tick <= clkgen_i(to_integer(unsigned(clk_sel)));
      -- unauthorized access buffer --
      fail_ff <= fail;
      -- reset synchronizer --
      rst_sync <= rst_sync(0) & rst_gen(rst_gen'left);
      -- counter update --
      if (wren = '1') then -- clear counter on write access (manual watchdog reset)
        cnt <= (others => '0');
      elsif (enable = '1') and (prsc_tick = '1') then
        cnt <= std_ulogic_vector(unsigned('0' & cnt(cnt'left-1 downto 0)) + 1);
      end if;
    end if;
  end process cnt_sync;

  -- system reset --
  rst_o <= rst_sync(1);


  -- Reset Cause Indicator ----------------------------------------------------
  -- -----------------------------------------------------------------------------
  rst_cause: process(rst_i, clk_i)
  begin
    if (rst_i = '0') then
      rst_source <= '0';
      pw_fail    <= '0';
    elsif rising_edge(clk_i) then
      rst_source <= rst_source or (cnt(cnt'left) and enable) or (fail_ff and enable); -- set on WDT timeout or access error
      pw_fail    <= (pw_fail or (fail_ff and enable)) and (not (cnt(cnt'left) and enable)); -- set on failed access, clear on WDT timeout
    --pw_fail    <= (pw_fail and (not (cnt(cnt'left) and enable))) or (fail_ff and enable); -- clear on WDT timeout, set on failed access
    end if;
  end process rst_cause;


  -- Read Access --------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  read_access: process(clk_i)
  begin
    if rising_edge(clk_i) then
      data_o <= (others => '0');
      if (acc_en = '1') and (rden_i = '1') then
        data_o(ctrl_clksel2_c downto ctrl_clksel0_c) <= clk_sel;
        data_o(ctrl_enable_c)  <= enable;
        data_o(ctrl_rcause_c)  <= rst_source;
        data_o(ctrl_rpwfail_c) <= pw_fail;
      end if;
    end if;
  end process read_access;


end neo430_wdt_rtl;
