-- #################################################################################################
-- #  << NEO430 - Processor Test Implementation (neo430_test.vhd) >>                               #
-- # ********************************************************************************************* #
-- #  If you do not have an own design (yet), you can use this unit as top entity to play with     #
-- #  the NEO430 processor. Take a look at the project's documentary (chapter "Let's Get It        #
-- #  Started!") to get more information.                                                          #
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

entity neo430_test is
  port (
    -- global control --
    clk_i      : in  std_ulogic; -- global clock, rising edge
    rst_i      : in  std_ulogic; -- global reset, async, LOW-active
    -- parallel io --
    gpio_o     : out std_ulogic_vector(07 downto 0); -- parallel output
    -- serial com --
    uart_txd_o : out std_ulogic; -- UART send data
    uart_rxd_i : in  std_ulogic  -- UART receive data
  );
end neo430_test;

architecture neo430_test_rtl of neo430_test is

  -- local signals --
  signal gpio_out : std_ulogic_vector(15 downto 0);
  signal rst_int  : std_ulogic;

begin

  -- The Core of the Problem --------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_top_test_inst: neo430_top
  generic map (
    -- general configuration --
    CLOCK_SPEED  => 100000000,        -- main clock in Hz
    IMEM_SIZE    => 4*1024,           -- internal IMEM size in bytes, max 48kB (default=4kB)
    DMEM_SIZE    => 2*1024,           -- internal DMEM size in bytes, max 12kB (default=2kB)
    -- additional configuration --
    USER_CODE    => x"CAFE",          -- custom user code
    -- module configuration --
    MULDIV_USE   => true,             -- implement multiplier/divider unit? (default=true)
    WB32_USE     => true,             -- implement WB32 unit? (default=true)
    WDT_USE      => true,             -- implement WDT? (default=true)
    GPIO_USE     => true,             -- implement GPIO unit? (default=true)
    TIMER_USE    => true,             -- implement timer? (default=true)
    UART_USE     => true,             -- implement UART? (default=true)
    CRC_USE      => true,             -- implement CRC unit? (default=true)
    CFU_USE      => false,            -- implement custom functions unit? (default=false)
    PWM_USE      => true,             -- implement PWM controller? (default=true)
    TWI_USE      => true,             -- implement two wire serial interface? (default=true)
    SPI_USE      => true,             -- implement SPI? (default=true)
    TRNG_USE     => false,            -- implement TRNG? (default=false)
    EXIRQ_USE    => true,             -- implement EXIRQ? (default=true)
    FREQ_GEN_USE => true,             -- implement FREQ_GEN? (default=true)
    -- boot configuration --
    BOOTLD_USE   => true,             -- implement and use bootloader? (default=true)
    IMEM_AS_ROM  => false             -- implement IMEM as read-only memory? (default=false)
  )
  port map (
    -- global control --
    clk_i      => clk_i,              -- global clock, rising edge
    rst_i      => rst_int,            -- global reset, async, low-active
    -- gpio --
    gpio_o     => gpio_out,           -- parallel output
    gpio_i     => x"0000",            -- parallel input
    -- pwm channels --
    pwm_o      => open,               -- pwm channels
    -- arbitrary frequency generator --
    freq_gen_o => open,               -- programmable frequency output
    -- serial com --
    uart_txd_o => uart_txd_o,         -- UART send data
    uart_rxd_i => uart_rxd_i,         -- UART receive data
    spi_sclk_o => open,               -- serial clock line
    spi_mosi_o => open,               -- serial data line out
    spi_miso_i => '0',                -- serial data line in
    spi_cs_o   => open,               -- SPI CS
    twi_sda_io => open,               -- twi serial data line
    twi_scl_io => open,               -- twi serial clock line
    -- 32-bit wishbone interface --
    wb_adr_o   => open,               -- address
    wb_dat_i   => x"00000000",        -- read data
    wb_dat_o   => open,               -- write data
    wb_we_o    => open,               -- read/write
    wb_sel_o   => open,               -- byte enable
    wb_stb_o   => open,               -- strobe
    wb_cyc_o   => open,               -- valid cycle
    wb_ack_i   => '0',                -- transfer acknowledge
    -- external interrupts --
    ext_irq_i  => "00000000",         -- external interrupt request lines
    ext_ack_o  => open                -- external interrupt request acknowledges
  );

  -- constrain output signals --
  gpio_o <= gpio_out(7 downto 0);

  -- internal reset (must be low-active!) --
  rst_int <= rst_i; -- invert me?!


end neo430_test_rtl;
