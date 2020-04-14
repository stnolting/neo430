-- #################################################################################################
-- #  << NEO430 - Processor Top Entity >>                                                          #
-- # ********************************************************************************************* #
-- # This is the top entity of the NEO430 processor. Instantiate this unit in your own project and #
-- # define all the configuration generics according to your needs. Alternatively, you can use one #
-- # of the other top entities provided in rtl\top_templates                                       #
-- # Check the processor's documentary for more information: doc\NEO430.pdf                        #
-- # ********************************************************************************************* #
-- #  The NEO430 processor:                                                                        #
-- #  - Reset and clock generators                                                                 #
-- #  - External IRQ synchronizer                                                                  #
-- #  - NEO430 CPU (MSP430(TM)-ISA-compatible) (CPU)                                               #
-- #  - Internal RAM or ROM (configurable size) for code (IMEM)                                    #
-- #  - Internal RAM (configurable size) for data (and code) (DMEM)                                #
-- #  - Sysconfig (infomem for various system information) (SYSCONFIG)                             #
-- #  - Optional 16-bit multiplier/divider unit (MULDIV)                                           #
-- #  - Optional 16-bit IN and 16-bit OUT GPIO port with pin-change interrupt (GPIO)               #
-- #  - Optional 32-bit Wishbone interface (WB32)                                                  #
-- #  - Optional High precision timer (TIMER)                                                      #
-- #  - Optional Universal Asynchronous Receiver and Transmitter (UART)                            #
-- #  - Optional Serial Peripheral Interface (SPI)                                                 #
-- #  - Optional Internal ROM for bootloader (BOOTLD)                                              #
-- #  - Optional Watchdog Timer (WDT)                                                              #
-- #  - Optional CRC16/32 Module (CRC16/32)                                                        #
-- #  - Optional Custom Functions Unit to implement user-defined processor extension (CFU)         #
-- #  - Optional Pulse Width Modulation controller (PWM)                                           #
-- #  - Optional Two Wire Serial Interface (TWI)                                                   #
-- #  - Optional True Random Number Generator (TRNG)                                               #
-- #  - Optional External Interrupts Controller (EXIRQ)                                            #
-- #  - Optional Arbitrary Frequency Generator (FREQ_GEN)                                          #
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

entity neo430_top is
  generic (
    -- general configuration --
    CLOCK_SPEED  : natural := 100000000; -- main clock in Hz
    IMEM_SIZE    : natural := 4*1024; -- internal IMEM size in bytes, max 48kB (default=4kB)
    DMEM_SIZE    : natural := 2*1024; -- internal DMEM size in bytes, max 12kB (default=2kB)
    -- additional configuration --
    USER_CODE    : std_ulogic_vector(15 downto 0) := x"0000"; -- custom user code
    -- module configuration --
    MULDIV_USE   : boolean := true;  -- implement multiplier/divider unit? (default=true)
    WB32_USE     : boolean := true;  -- implement WB32 unit? (default=true)
    WDT_USE      : boolean := true;  -- implement WDT? (default=true)
    GPIO_USE     : boolean := true;  -- implement GPIO unit? (default=true)
    TIMER_USE    : boolean := true;  -- implement timer? (default=true)
    UART_USE     : boolean := true;  -- implement UART? (default=true)
    CRC_USE      : boolean := true;  -- implement CRC unit? (default=true)
    CFU_USE      : boolean := false; -- implement custom functions unit? (default=false)
    PWM_USE      : boolean := true;  -- implement PWM controller? (default=true)
    TWI_USE      : boolean := true;  -- implement two wire serial interface? (default=true)
    SPI_USE      : boolean := true;  -- implement SPI? (default=true)
    TRNG_USE     : boolean := false; -- implement TRNG? (default=false)
    EXIRQ_USE    : boolean := true;  -- implement EXIRQ? (default=true)
    FREQ_GEN_USE : boolean := true;  -- implement FREQ_GEN? (default=true)
    -- boot configuration --
    BOOTLD_USE   : boolean := true;  -- implement and use bootloader? (default=true)
    IMEM_AS_ROM  : boolean := false  -- implement IMEM as read-only memory? (default=false)
  );
  port (
    -- global control --
    clk_i      : in  std_ulogic; -- global clock, rising edge
    rst_i      : in  std_ulogic; -- global reset, async, LOW-active
    -- parallel io --
    gpio_o     : out std_ulogic_vector(15 downto 0); -- parallel output
    gpio_i     : in  std_ulogic_vector(15 downto 0); -- parallel input
    -- pwm channels --
    pwm_o      : out std_ulogic_vector(03 downto 0); -- pwm channels
    -- arbitrary frequency generator --
    freq_gen_o : out std_ulogic_vector(02 downto 0); -- programmable frequency output
    -- serial com --
    uart_txd_o : out std_ulogic; -- UART send data
    uart_rxd_i : in  std_ulogic; -- UART receive data
    spi_sclk_o : out std_ulogic; -- serial clock line
    spi_mosi_o : out std_ulogic; -- serial data line out
    spi_miso_i : in  std_ulogic; -- serial data line in
    spi_cs_o   : out std_ulogic_vector(05 downto 0); -- SPI CS
    twi_sda_io : inout std_logic; -- twi serial data line
    twi_scl_io : inout std_logic; -- twi serial clock line
    -- 32-bit wishbone interface --
    wb_adr_o   : out std_ulogic_vector(31 downto 0); -- address
    wb_dat_i   : in  std_ulogic_vector(31 downto 0); -- read data
    wb_dat_o   : out std_ulogic_vector(31 downto 0); -- write data
    wb_we_o    : out std_ulogic; -- read/write
    wb_sel_o   : out std_ulogic_vector(03 downto 0); -- byte enable
    wb_stb_o   : out std_ulogic; -- strobe
    wb_cyc_o   : out std_ulogic; -- valid cycle
    wb_ack_i   : in  std_ulogic; -- transfer acknowledge
    -- external interrupts --
    ext_irq_i  : in  std_ulogic_vector(07 downto 0); -- external interrupt request lines (active HI)
    ext_ack_o  : out std_ulogic_vector(07 downto 0)  -- external interrupt request acknowledges
  );
end neo430_top;

architecture neo430_top_rtl of neo430_top is

  -- generators --
  signal rst_i_sync0    : std_ulogic;
  signal rst_i_sync1    : std_ulogic;
  signal rst_gen        : std_ulogic_vector(03 downto 0) := (others => '0'); -- reset on bitstream upload
  signal ext_rst        : std_ulogic;
  signal sys_rst        : std_ulogic;
  signal wdt_rst        : std_ulogic;
  signal clk_div        : std_ulogic_vector(11 downto 0);
  signal clk_div_ff     : std_ulogic_vector(11 downto 0);
  signal clk_gen        : std_ulogic_vector(07 downto 0);
  signal timer_cg_en    : std_ulogic;
  signal uart_cg_en     : std_ulogic;
  signal spi_cg_en      : std_ulogic;
  signal wdt_cg_en      : std_ulogic;
  signal pwm_cg_en      : std_ulogic;
  signal twi_cg_en      : std_ulogic;
  signal cfu_cg_en      : std_ulogic;
  signal freq_gen_cg_en : std_ulogic;

  type cpu_bus_t is record
    rd_en : std_ulogic;
    wr_en : std_ulogic_vector(01 downto 0);
    addr  : std_ulogic_vector(15 downto 0);
    rdata : std_ulogic_vector(15 downto 0);
    wdata : std_ulogic_vector(15 downto 0);
  end record;

  -- main CPU communication bus --
  signal cpu_bus  : cpu_bus_t;
  signal io_acc   : std_ulogic;
  signal io_wr_en : std_ulogic;
  signal io_rd_en : std_ulogic;

  -- read-back data buses --
  signal rom_rdata       : std_ulogic_vector(15 downto 0);
  signal ram_rdata       : std_ulogic_vector(15 downto 0);
  signal muldiv_rdata    : std_ulogic_vector(15 downto 0);
  signal wb_rdata        : std_ulogic_vector(15 downto 0);
  signal boot_rdata      : std_ulogic_vector(15 downto 0);
  signal wdt_rdata       : std_ulogic_vector(15 downto 0);
  signal timer_rdata     : std_ulogic_vector(15 downto 0);
  signal uart_rdata      : std_ulogic_vector(15 downto 0);
  signal spi_rdata       : std_ulogic_vector(15 downto 0);
  signal gpio_rdata      : std_ulogic_vector(15 downto 0);
  signal crc_rdata       : std_ulogic_vector(15 downto 0);
  signal cfu_rdata       : std_ulogic_vector(15 downto 0);
  signal pwm_rdata       : std_ulogic_vector(15 downto 0);
  signal twi_rdata       : std_ulogic_vector(15 downto 0);
  signal trng_rdata      : std_ulogic_vector(15 downto 0);
  signal exirq_rdata     : std_ulogic_vector(15 downto 0);
  signal freq_gen_rdata  : std_ulogic_vector(15 downto 0);
  signal sysconfig_rdata : std_ulogic_vector(15 downto 0);

  -- interrupt system --
  signal irq       : std_ulogic_vector(03 downto 0);
  signal timer_irq : std_ulogic;
  signal uart_irq  : std_ulogic;
  signal spi_irq   : std_ulogic;
  signal twi_irq   : std_ulogic;
  signal gpio_irq  : std_ulogic;
  signal ext_irq   : std_ulogic;

  -- misc --
  signal imem_up_en : std_ulogic;
  signal gpio_pwm   : std_ulogic;

begin

  -- Reset Generator ----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  -- make sure the external reset is free of metastability and has a minimal duration of 1 clock cycle
  reset_generator_filter: process(clk_i)
  begin
    if rising_edge(clk_i) then
      rst_i_sync0 <= rst_i;
      rst_i_sync1 <= rst_i_sync0; -- no metastability, thanks
    end if;
  end process reset_generator_filter;

  -- keep internal reset active for at least 4 clock cycles
  reset_generator: process(rst_i_sync1, clk_i)
  begin
    if rising_edge(clk_i) then
      if (rst_i_sync1 = '0') then
        rst_gen <= (others => '0');
      else
        rst_gen <= rst_gen(rst_gen'left-1 downto 0) & '1';
      end if;
    end if;
  end process reset_generator;

  ext_rst <= rst_gen(rst_gen'left); -- the beautified external reset signal
  sys_rst <= ext_rst and wdt_rst;


  -- Clock Generator ----------------------------------------------------------
  -- -----------------------------------------------------------------------------
  clock_generator: process(sys_rst, clk_i)
  begin
    if (sys_rst = '0') then
      clk_div <= (others => '0');
    elsif rising_edge(clk_i) then
      -- anybody needing fresh clocks?
      if ((timer_cg_en or uart_cg_en or spi_cg_en or wdt_cg_en or pwm_cg_en or twi_cg_en or cfu_cg_en or freq_gen_cg_en) = '1') then
        clk_div <= std_ulogic_vector(unsigned(clk_div) + 1);
      end if;
    end if;
  end process clock_generator;

  clock_generator_buf: process(clk_i)
  begin
    if rising_edge(clk_i) then
      clk_div_ff <= clk_div;
    end if;
  end process clock_generator_buf;

  -- clock enable select: rising edge detectors --
  clk_gen(clk_div2_c)    <= clk_div(0)  and (not clk_div_ff(0));  -- CLK/2
  clk_gen(clk_div4_c)    <= clk_div(1)  and (not clk_div_ff(1));  -- CLK/4
  clk_gen(clk_div8_c)    <= clk_div(2)  and (not clk_div_ff(2));  -- CLK/8
  clk_gen(clk_div64_c)   <= clk_div(5)  and (not clk_div_ff(5));  -- CLK/64
  clk_gen(clk_div128_c)  <= clk_div(6)  and (not clk_div_ff(6));  -- CLK/128
  clk_gen(clk_div1024_c) <= clk_div(9)  and (not clk_div_ff(9));  -- CLK/1024
  clk_gen(clk_div2048_c) <= clk_div(10) and (not clk_div_ff(10)); -- CLK/2048
  clk_gen(clk_div4096_c) <= clk_div(11) and (not clk_div_ff(11)); -- CLK/4096


  -- The core of the problem: The CPU -----------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_cpu_inst: neo430_cpu
  generic map (
    BOOTLD_USE  => BOOTLD_USE,      -- implement and use bootloader? (default=true)
    IMEM_AS_ROM => IMEM_AS_ROM      -- implement IMEM as read-only memory?
  )
  port map (
    -- global control --
    clk_i      => clk_i,            -- global clock, rising edge
    rst_i      => sys_rst,          -- global reset, low-active, async
    -- memory interface --
    mem_rd_o   => cpu_bus.rd_en,    -- memory read
    mem_imwe_o => imem_up_en,       -- allow writing to IMEM
    mem_wr_o   => cpu_bus.wr_en,    -- memory write
    mem_addr_o => cpu_bus.addr,     -- address
    mem_data_o => cpu_bus.wdata,    -- write data
    mem_data_i => cpu_bus.rdata,    -- read data
    -- interrupt system --
    irq_i      => irq              -- interrupt request lines
  );

  -- final CPU read data --
  cpu_bus.rdata <= rom_rdata or ram_rdata or boot_rdata or muldiv_rdata or
                   wb_rdata or uart_rdata or spi_rdata or gpio_rdata or freq_gen_rdata or
                   timer_rdata or wdt_rdata or sysconfig_rdata or crc_rdata or
                   cfu_rdata or pwm_rdata or twi_rdata or trng_rdata or exirq_rdata;

  -- interrupts: priority assignment --
  irq(0) <= timer_irq;                      -- timer match (highest priority)
  irq(1) <= uart_irq or spi_irq or twi_irq; -- serial IRQ
  irq(2) <= gpio_irq;                       -- GPIO input pin change
  irq(3) <= ext_irq;                        -- external interrupt request (lowest priority)


  -- Main Memory (ROM/IMEM & RAM/DMEM) ----------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_imem_inst: neo430_imem
  generic map (
    IMEM_SIZE   => IMEM_SIZE,       -- internal IMEM size in bytes, max 32kB (default=4kB)
    IMEM_AS_ROM => IMEM_AS_ROM,     -- implement IMEM as read-only memory?
    BOOTLD_USE  => BOOTLD_USE       -- implement and use bootloader? (default=true)
  )
  port map (
    clk_i  => clk_i,                -- global clock line
    rden_i => cpu_bus.rd_en,        -- read enable
    wren_i => cpu_bus.wr_en,        -- write enable
    upen_i => imem_up_en,           -- update enable
    addr_i => cpu_bus.addr,         -- address
    data_i => cpu_bus.wdata,        -- data in
    data_o => rom_rdata             -- data out
  );

  neo430_dmem_inst: neo430_dmem
  generic map (
    DMEM_SIZE => DMEM_SIZE          -- internal DMEM size in bytes, max 28kB (default=2kB)
  )
  port map (
    clk_i  => clk_i,                -- global clock line
    rden_i => cpu_bus.rd_en,        -- read enable
    wren_i => cpu_bus.wr_en,        -- write enable
    addr_i => cpu_bus.addr,         -- address
    data_i => cpu_bus.wdata,        -- data in
    data_o => ram_rdata             -- data out
  );


  -- Boot ROM -----------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_boot_rom_inst_true:
  if (BOOTLD_USE = true) generate
    neo430_boot_rom_inst: neo430_boot_rom
    port map (
      clk_i  => clk_i,              -- global clock line
      rden_i => cpu_bus.rd_en,      -- read enable
      addr_i => cpu_bus.addr,       -- address
      data_o => boot_rdata          -- data out
    );
  end generate;

  neo430_boot_rom_inst_false:
  if (BOOTLD_USE = false) generate
    boot_rdata <= (others => '0');
  end generate;


  -- IO Access? ---------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  io_acc   <= '1' when (cpu_bus.addr(15 downto index_size_f(io_size_c)) = io_base_c(15 downto index_size_f(io_size_c))) else '0';
  io_rd_en <= cpu_bus.rd_en and io_acc;
  io_wr_en <= (cpu_bus.wr_en(0) or cpu_bus.wr_en(1)) and io_acc; -- use all accesses as full-word accesses


  -- Multiplier/Divider Unit (MULDIV) -----------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_muldiv_inst_true:
  if (MULDIV_USE = true) generate
    neo430_muldiv_inst: neo430_muldiv
    port map (
      -- host access --
      clk_i  => clk_i,              -- global clock line
      rden_i => io_rd_en,           -- read enable
      wren_i => io_wr_en,           -- write enable
      addr_i => cpu_bus.addr,       -- address
      data_i => cpu_bus.wdata,      -- data in
      data_o => muldiv_rdata        -- data out
    );
  end generate;

  neo430_muldiv_inst_false:
  if (MULDIV_USE = false) generate
    muldiv_rdata <= (others => '0');
  end generate;


  -- Wishbone Adapter (WB32) --------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_wb32_if_inst_true:
  if (WB32_USE = true) generate
    neo430_wb32_inst: neo430_wb_interface
    port map (
      -- host access --
      clk_i    => clk_i,            -- global clock line
      rden_i   => io_rd_en,         -- read enable
      wren_i   => io_wr_en,         -- write enable
      addr_i   => cpu_bus.addr,     -- address
      data_i   => cpu_bus.wdata,    -- data in
      data_o   => wb_rdata,         -- data out
      -- wishbone interface --
      wb_adr_o => wb_adr_o,         -- address
      wb_dat_i => wb_dat_i,         -- read data
      wb_dat_o => wb_dat_o,         -- write data
      wb_we_o  => wb_we_o,          -- read/write
      wb_sel_o => wb_sel_o,         -- byte enable
      wb_stb_o => wb_stb_o,         -- strobe
      wb_cyc_o => wb_cyc_o,         -- valid cycle
      wb_ack_i => wb_ack_i          -- transfer acknowledge
    );
  end generate;

  neo430_wb32_if_inst_false:
  if (WB32_USE = false) generate
    wb_rdata <= (others => '0');
    wb_adr_o <= (others => '0');
    wb_dat_o <= (others => '0');
    wb_we_o  <= '0';
    wb_sel_o <= (others => '0');
    wb_stb_o <= '0';
    wb_cyc_o <= '0';
  end generate;


  -- Universal Asynchronous Receiver & Transmitter (UART) ---------------------
  -- -----------------------------------------------------------------------------
  neo430_uart_inst_true:
  if (UART_USE = true) generate
    neo430_uart_inst: neo430_uart
    port map (
      -- host access --
      clk_i       => clk_i,         -- global clock line
      rden_i      => io_rd_en,      -- read enable
      wren_i      => io_wr_en,      -- write enable
      addr_i      => cpu_bus.addr,  -- address
      data_i      => cpu_bus.wdata, -- data in
      data_o      => uart_rdata,    -- data out
      -- clock generator --
      clkgen_en_o => uart_cg_en,    -- enable clock generator
      clkgen_i    => clk_gen,
      -- com lines --
      uart_txd_o  => uart_txd_o,
      uart_rxd_i  => uart_rxd_i,
      -- interrupts --
      uart_irq_o  => uart_irq       -- uart rx/tx interrupt
    );
  end generate;

  neo430_uart_inst_false:
  if (UART_USE = false) generate
    uart_rdata <= (others => '0');
    uart_irq   <= '0';
    uart_cg_en <= '0';
    uart_txd_o <= '1';
  end generate;


  -- Serial Peripheral Interface (SPI) ----------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_spi_inst_true:
  if (SPI_USE = true) generate
    neo430_spi_inst: neo430_spi
    port map (
      -- host access --
      clk_i       => clk_i,         -- global clock line
      rden_i      => io_rd_en,      -- read enable
      wren_i      => io_wr_en,      -- write enable
      addr_i      => cpu_bus.addr,  -- address
      data_i      => cpu_bus.wdata, -- data in
      data_o      => spi_rdata,     -- data out
      -- clock generator --
      clkgen_en_o => spi_cg_en,     -- enable clock generator
      clkgen_i    => clk_gen,
      -- com lines --
      spi_sclk_o  => spi_sclk_o,    -- SPI serial clock
      spi_mosi_o  => spi_mosi_o,    -- SPI master out, slave in
      spi_miso_i  => spi_miso_i,    -- SPI master in, slave out
      spi_cs_o    => spi_cs_o,      -- SPI CS 0..5
      -- interrupt --
      spi_irq_o   => spi_irq        -- transmission done interrupt
    );
  end generate;

  neo430_spi_inst_false:
  if (SPI_USE = false) generate
    spi_rdata  <= (others => '0');
    spi_cg_en  <= '0';
    spi_sclk_o <= '0';
    spi_mosi_o <= '0';
    spi_cs_o   <= (others => '1');
    spi_irq    <= '0';
  end generate;


  -- General Purpose Parallel IO (GPIO) ---------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_gpio_inst_true:
  if (GPIO_USE = true) generate
    neo430_gpio_inst: neo430_gpio
    port map (
      -- host access --
      clk_i      => clk_i,          -- global clock line
      rden_i     => io_rd_en,       -- read enable
      wren_i     => io_wr_en,       -- write enable
      addr_i     => cpu_bus.addr,   -- address
      data_i     => cpu_bus.wdata,  -- data in
      data_o     => gpio_rdata,     -- data out
      -- parallel io --
      gpio_o     => gpio_o,
      gpio_i     => gpio_i,
     -- GPIO PWM --
      gpio_pwm_i => gpio_pwm,
      -- interrupt --
      irq_o      => gpio_irq        -- pin-change interrupt
    );
  end generate;

  neo430_gpio_inst_false:
  if (GPIO_USE = false) generate
    gpio_rdata <= (others => '0');
    gpio_o     <= (others => '0');
    gpio_irq   <= '0';
  end generate;


  -- High Precision Timer (TIMER) ---------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_timer_inst_true:
  if (TIMER_USE = true) generate
  neo430_timer_inst: neo430_timer
    port map (
      -- host access --
      clk_i       => clk_i,         -- global clock line
      rden_i      => io_rd_en,      -- read enable
      wren_i      => io_wr_en,      -- write enable
      addr_i      => cpu_bus.addr,  -- address
      data_i      => cpu_bus.wdata, -- data in
      data_o      => timer_rdata,   -- data out
      -- clock generator --
      clkgen_en_o => timer_cg_en,   -- enable clock generator
      clkgen_i    => clk_gen,
      -- interrupt --
      irq_o       => timer_irq      -- interrupt request
    );
  end generate;

  neo430_timer_inst_false:
  if (TIMER_USE = false) generate
    timer_rdata <= (others => '0');
    timer_irq   <= '0';
    timer_cg_en <= '0';
  end generate;


  -- Watchdog Timer (WDT) -----------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_wdt_inst_true:
  if (WDT_USE = true) generate
    neo430_wdt_inst: neo430_wdt
    port map(
      -- host access --
      clk_i       => clk_i,         -- global clock line
      rst_i       => ext_rst,       -- external reset, low-active, use as async
      rden_i      => io_rd_en,      -- read enable
      wren_i      => io_wr_en,      -- write enable
      addr_i      => cpu_bus.addr,  -- address
      data_i      => cpu_bus.wdata, -- data in
      data_o      => wdt_rdata,     -- data out
      -- clock generator --
      clkgen_en_o => wdt_cg_en,     -- enable clock generator
      clkgen_i    => clk_gen,       -- clock generator
      -- system reset --
      rst_o       => wdt_rst        -- timeout reset, low-active, use as async
    );
  end generate;

  neo430_wdt_inst_false:
  if (WDT_USE = false) generate
    wdt_rdata <= (others => '0');
    wdt_rst   <= '1';
    wdt_cg_en <= '0';
  end generate;


  -- Checksum Module (CRC) ----------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_crc_inst_true:
  if (CRC_USE = true) generate
    neo430_crc_inst: neo430_crc
    port map(
      -- host access --
      clk_i  => clk_i,              -- global clock line
      rden_i => io_rd_en,           -- read enable
      wren_i => io_wr_en,           -- write enable
      addr_i => cpu_bus.addr,       -- address
      data_i => cpu_bus.wdata,      -- data in
      data_o => crc_rdata           -- data out
    );
  end generate;

  neo430_crc_inst_false:
  if (CRC_USE = false) generate
    crc_rdata <= (others => '0');
  end generate;


  -- Custom Functions Unit (CFU) ----------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_cfu_inst_true:
  if (CFU_USE = true) generate
    neo430_cfu_inst: neo430_cfu
    port map(
      -- host access --
      clk_i       => clk_i,         -- global clock line
      rden_i      => io_rd_en,      -- read enable
      wren_i      => io_wr_en,      -- write enable
      addr_i      => cpu_bus.addr,  -- address
      data_i      => cpu_bus.wdata, -- data in
      data_o      => cfu_rdata,     -- data out
      -- clock generator --
      clkgen_en_o => cfu_cg_en,     -- enable clock generator
      clkgen_i    => clk_gen
      -- add custom IOs below --
    );
  end generate;

  neo430_cfu_inst_false:
  if (CFU_USE = false) generate
    cfu_cg_en <= '0';
    cfu_rdata <= (others => '0');
  end generate;


  -- PWM Controller (PWM) -----------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_pwm_inst_true:
  if (PWM_USE = true) generate
    neo430_pwm_inst: neo430_pwm
    port map(
      -- host access --
      clk_i       => clk_i,         -- global clock line
      rden_i      => io_rd_en,      -- read enable
      wren_i      => io_wr_en,      -- write enable
      addr_i      => cpu_bus.addr,  -- address
      data_i      => cpu_bus.wdata, -- data in
      data_o      => pwm_rdata,     -- data out
      -- clock generator --
      clkgen_en_o => pwm_cg_en,     -- enable clock generator
      clkgen_i    => clk_gen,
      -- GPIO output PWM --
      gpio_pwm_o  => gpio_pwm,
      -- pwm output channels --
      pwm_o       => pwm_o
    );
  end generate;

  neo430_pwm_inst_false:
  if (PWM_USE = false) generate
    pwm_cg_en <= '0';
    gpio_pwm  <= '1';
    pwm_rdata <= (others => '0');
    pwm_o     <= (others => '0');
  end generate;


  -- Two Wire Serial Interface (SPI) ------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_twi_inst_true:
  if (TWI_USE = true) generate
    neo430_twi_inst: neo430_twi
    port map (
      -- host access --
      clk_i       => clk_i,         -- global clock line
      rden_i      => io_rd_en,      -- read enable
      wren_i      => io_wr_en,      -- write enable
      addr_i      => cpu_bus.addr,  -- address
      data_i      => cpu_bus.wdata, -- data in
      data_o      => twi_rdata,     -- data out
      -- clock generator --
      clkgen_en_o => twi_cg_en,     -- enable clock generator
      clkgen_i    => clk_gen,
      -- com lines --
      twi_sda_io  => twi_sda_io,    -- serial data line
      twi_scl_io  => twi_scl_io,    -- serial clock line
      -- interrupt --
      twi_irq_o   => twi_irq        -- transfer done IRQ
    );
  end generate;

  neo430_twi_inst_false:
  if (TWI_USE = false) generate
    twi_cg_en <= '0';
    twi_rdata <= (others => '0');
    twi_irq   <= '0';
  end generate;


  -- True Random Number Generator (TRNG) --------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_trng_inst_true:
  if (TRNG_USE = true) generate
    neo430_trng_inst: neo430_trng
    port map (
      -- host access --
      clk_i       => clk_i,         -- global clock line
      rden_i      => io_rd_en,      -- read enable
      wren_i      => io_wr_en,      -- write enable
      addr_i      => cpu_bus.addr,  -- address
      data_i      => cpu_bus.wdata, -- data in
      data_o      => trng_rdata     -- data out
    );
  end generate;

  neo430_trng_inst_false:
  if (TRNG_USE = false) generate
    trng_rdata <= (others => '0');
  end generate;


  -- External Interrupts Controller (EXIRQ) -----------------------------------
  -- -----------------------------------------------------------------------------
  neo430_exirq_inst_true:
  if (EXIRQ_USE = true) generate
    neo430_exirq_inst: neo430_exirq
    port map (
      -- host access --
      clk_i     => clk_i,           -- global clock line
      rden_i    => io_rd_en,        -- read enable
      wren_i    => io_wr_en,        -- write enable
      addr_i    => cpu_bus.addr,    -- address
      data_i    => cpu_bus.wdata,   -- data in
      data_o    => exirq_rdata,     -- data out
      -- cpu interrupt --
      cpu_irq_o => ext_irq,
      -- external interrupt lines --
      ext_irq_i => ext_irq_i,       -- IRQ
      ext_ack_o => ext_ack_o        -- acknowledge
    );
  end generate;

  neo430_exirq_inst_false:
  if (EXIRQ_USE = false) generate
    exirq_rdata <= (others => '0');
    ext_ack_o   <= (others => '0');
    ext_irq     <= '0';
  end generate;


  -- Arbitrary Frequency Generator (FREW_GEN)) --------------------------------
  -- -----------------------------------------------------------------------------
  neo430_freq_gen_inst_true:
  if (FREQ_GEN_USE = true) generate
    neo430_freq_gen_inst: neo430_freq_gen
    port map (
      -- host access --
      clk_i       => clk_i,           -- global clock line
      rden_i      => io_rd_en,        -- read enable
      wren_i      => io_wr_en,        -- write enable
      addr_i      => cpu_bus.addr,    -- address
      data_i      => cpu_bus.wdata,   -- data in
      data_o      => freq_gen_rdata,  -- data out
      -- clock generator --
      clkgen_en_o => freq_gen_cg_en,  -- enable clock generator
      clkgen_i    => clk_gen,
      -- frequency generator --
      freq_gen_o  => freq_gen_o  -- programmable frequency output
    );
  end generate;

  neo430_freq_gen_inst_false:
  if (FREQ_GEN_USE = false) generate
    freq_gen_cg_en <= '0';
    freq_gen_rdata <= (others => '0');
    freq_gen_o     <= (others => '0');
  end generate;


  -- System Configuration -----------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_sysconfig_inst: neo430_sysconfig
  generic map (
    -- general configuration --
    CLOCK_SPEED  => CLOCK_SPEED,    -- main clock in Hz
    IMEM_SIZE    => IMEM_SIZE,      -- internal IMEM size in bytes
    DMEM_SIZE    => DMEM_SIZE,      -- internal DMEM size in bytes
    -- additional configuration --
    USER_CODE    => USER_CODE,      -- custom user code
    -- module configuration --
    MULDIV_USE   => MULDIV_USE,     -- implement multiplier/divider unit?
    WB32_USE     => WB32_USE,       -- implement WB32 unit?
    WDT_USE      => WDT_USE,        -- implement WDT?
    GPIO_USE     => GPIO_USE,       -- implement GPIO unit?
    TIMER_USE    => TIMER_USE,      -- implement timer?
    UART_USE     => UART_USE,       -- implement UART?
    CRC_USE      => CRC_USE,        -- implement CRC unit?
    CFU_USE      => CFU_USE,        -- implement CFU?
    PWM_USE      => PWM_USE,        -- implement PWM?
    TWI_USE      => TWI_USE,        -- implement TWI?
    SPI_USE      => SPI_USE,        -- implement SPI?
    TRNG_USE     => TRNG_USE,       -- implement TRNG?
    EXIRQ_USE    => EXIRQ_USE,      -- implement EXIRQ?
    FREQ_GEN_USE => FREQ_GEN_USE,   -- implement FREQ_GEN?
    -- boot configuration --
    BOOTLD_USE   => BOOTLD_USE,     -- implement and use bootloader?
    IMEM_AS_ROM  => IMEM_AS_ROM     -- implement IMEM as read-only memory?
  )
  port map (
    clk_i  => clk_i,                -- global clock line
    rden_i => io_rd_en,             -- read enable
    wren_i => io_wr_en,             -- write enable
    addr_i => cpu_bus.addr,         -- address
    data_i => cpu_bus.wdata,        -- data in
    data_o => sysconfig_rdata       -- data out
  );


end neo430_top_rtl;
