-- #################################################################################################
-- #  << NEO430 - Processor Top Entity with AXI4-Lite-Compatible Master Interface >>               #
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

entity neo430_top_axi4lite is
  generic (
    -- general configuration --
    CLOCK_SPEED  : natural := 100000000; -- main clock in Hz
    IMEM_SIZE    : natural := 4*1024; -- internal IMEM size in bytes, max 32kB (default=4kB)
    DMEM_SIZE    : natural := 2*1024; -- internal DMEM size in bytes, max 28kB (default=2kB)
    -- additional configuration --
    USER_CODE    : std_logic_vector(15 downto 0) := x"0000"; -- custom user code
    -- module configuration --
    MULDIV_USE   : boolean := true; -- implement multiplier/divider unit? (default=true)
    WB32_USE     : boolean := true; -- implement WB32 unit? (default=true)
    WDT_USE      : boolean := true; -- implement WDT? (default=true)
    GPIO_USE     : boolean := true; -- implement GPIO unit? (default=true)
    TIMER_USE    : boolean := true; -- implement timer? (default=true)
    UART_USE     : boolean := true; -- implement UART? (default=true)
    CRC_USE      : boolean := true; -- implement CRC unit? (default=true)
    CFU_USE      : boolean := false; -- implement custom functions unit? (default=false)
    PWM_USE      : boolean := true; -- implement PWM controller?
    TWI_USE      : boolean := true; -- implement two wire serial interface? (default=true)
    SPI_USE      : boolean := true; -- implement SPI? (default=true)
    TRNG_USE     : boolean := false; -- implement TRNG? (default=false)
    EXIRQ_USE    : boolean := true; -- implement EXIRQ? (default=true)
    FREQ_GEN_USE : boolean := true; -- implement FREQ_GEN? (default=true)
    -- boot configuration --
    BOOTLD_USE   : boolean := true; -- implement and use bootloader? (default=true)
    IMEM_AS_ROM  : boolean := false -- implement IMEM as read-only memory? (default=false)
  );
  port (
    -- GPIO --
    gpio_o        : out std_logic_vector(15 downto 0); -- parallel output
    gpio_i        : in  std_logic_vector(15 downto 0); -- parallel input
    -- pwm channels --
    pwm_o         : out std_logic_vector(03 downto 0); -- pwm channels
    -- arbitrary frequency generator --
    freq_gen_o    : out std_logic_vector(02 downto 0); -- programmable frequency output
    -- UART --
    uart_txd_o    : out std_logic; -- UART send data
    uart_rxd_i    : in  std_logic; -- UART receive data
    -- SPI --
    spi_sclk_o    : out std_logic; -- serial clock line
    spi_mosi_o    : out std_logic; -- serial data line out
    spi_miso_i    : in  std_logic; -- serial data line in
    spi_cs_o      : out std_logic_vector(05 downto 0); -- SPI CS
    twi_sda_io    : inout std_logic; -- twi serial data line
    twi_scl_io    : inout std_logic; -- twi serial clock line
    -- external interrupts --
    ext_irq_i     : in  std_logic_vector(07 downto 0); -- external interrupt request lines
    ext_ack_o     : out std_logic_vector(07 downto 0); -- external interrupt request acknowledges
    -- AXI Lite-Compatible Master Interface --
    -- Clock and Reset
    m_axi_aclk    : in std_logic;
    m_axi_aresetn : in std_logic;
    -- Write Address Channel
    m_axi_awaddr  : out std_logic_vector(31 downto 0);
    m_axi_awvalid : out std_logic;
    m_axi_awready : in  std_logic;
    m_axi_awprot  : out std_logic_vector(2 downto 0);
    -- Write Data Channel
    m_axi_wdata   : out std_logic_vector(31 downto 0);
    m_axi_wstrb   : out std_logic_vector(3 downto 0);
    m_axi_wvalid  : out std_logic;
    m_axi_wready  : in  std_logic;
    -- Read Address Channel
    m_axi_araddr  : out std_logic_vector(31 downto 0);
    m_axi_arvalid : out std_logic;
    m_axi_arready : in  std_logic;
    m_axi_arprot  : out std_logic_vector(2 downto 0);
    -- Read Data Channel
    m_axi_rdata   : in  std_logic_vector(31 downto 0);
    m_axi_rresp   : in  std_logic_vector(1 downto 0);
    m_axi_rvalid  : in  std_logic;
    m_axi_rready  : out std_logic;
    -- Write Response Channel
    m_axi_bresp   : in  std_logic_vector(1 downto 0);
    m_axi_bvalid  : in  std_logic;
    m_axi_bready  : out std_logic
  );
end neo430_top_axi4lite;

architecture neo430_top_axi4lite_rtl of neo430_top_axi4lite is

  -- internal wishbone bus --
  type wb_bus_t is record
    adr : std_ulogic_vector(31 downto 0); -- address
    di  : std_ulogic_vector(31 downto 0); -- slave input data
    do  : std_ulogic_vector(31 downto 0); -- slave output data
    we  : std_ulogic; -- write enable
    sel : std_ulogic_vector(03 downto 0); -- byte enable
    stb : std_ulogic; -- strobe
    cyc : std_ulogic; -- valid cycle
    ack : std_ulogic; -- transfer acknowledge
  end record;
  signal wb_core : wb_bus_t;

  -- other signals for conversion --
  signal gpio_o_int     : std_ulogic_vector(15 downto 0);
  signal gpio_i_int     : std_ulogic_vector(15 downto 0);
  signal pwm_o_int      : std_ulogic_vector(03 downto 0);
  signal uart_txd_o_int : std_ulogic;
  signal uart_rxd_i_int : std_ulogic;
  signal spi_sclk_o_int : std_ulogic;
  signal spi_mosi_o_int : std_ulogic;
  signal spi_miso_i_int : std_ulogic;
  signal spi_cs_o_int   : std_ulogic_vector(05 downto 0);
  signal irq_i_int      : std_ulogic_vector(07 downto 0);
  signal irq_ack_o_int  : std_ulogic_vector(07 downto 0);
  signal freq_gen_o_int : std_ulogic_vector(02 downto 0);
  constant usrcode_c    : std_ulogic_vector(15 downto 0) := std_ulogic_vector(USER_CODE);

  -- AXI arbiter --
  signal read_trans     : std_ulogic;
  signal write_trans    : std_ulogic;
  signal pending_rd     : std_ulogic; -- pending read transfer
  signal pending_wr     : std_ulogic; -- pending write transfer
  signal adr_valid      : std_ulogic;
  signal wresp_ok       : std_logic;

begin

  -- CPU ----------------------------------------------------------------------
  -- -----------------------------------------------------------------------------
  neo430_top_inst: neo430_top
  generic map (
    -- general configuration --
    CLOCK_SPEED  => CLOCK_SPEED,      -- main clock in Hz
    IMEM_SIZE    => IMEM_SIZE,        -- internal IMEM size in bytes, max 48kB (default=4kB)
    DMEM_SIZE    => DMEM_SIZE,        -- internal DMEM size in bytes, max 12kB (default=2kB)
    -- additional configuration --
    USER_CODE    => usrcode_c,        -- custom user code
    -- module configuration --
    MULDIV_USE   => MULDIV_USE,       -- implement multiplier/divider unit? (default=true)
    WB32_USE     => WB32_USE,         -- implement WB32 unit? (default=true)
    WDT_USE      => WDT_USE,          -- implement WDT? (default=true)
    GPIO_USE     => GPIO_USE,         -- implement GPIO unit? (default=true)
    TIMER_USE    => TIMER_USE,        -- implement timer? (default=true)
    UART_USE     => UART_USE,         -- implement UART? (default=true)
    CRC_USE      => CRC_USE,          -- implement CRC unit? (default=true)
    CFU_USE      => CFU_USE,          -- implement CF unit? (default=false)
    PWM_USE      => PWM_USE,          -- implement PWM controller? (default=true)
    TWI_USE      => TWI_USE,          -- implement two wire serial interface? (default=true)
    SPI_USE      => SPI_USE,          -- implement SPI? (default=true)
    TRNG_USE     => TRNG_USE,         -- implement TRNG? (default=false)
    EXIRQ_USE    => EXIRQ_USE,        -- implement EXIRQ? (default=true)
    FREQ_GEN_USE => FREQ_GEN_USE,     -- implement FREQ_GEN? (default=true)
    -- boot configuration --
    BOOTLD_USE   => BOOTLD_USE,       -- implement and use bootloader? (default=true)
    IMEM_AS_ROM  => IMEM_AS_ROM       -- implement IMEM as read-only memory? (default=false)
  )
  port map (
    -- global control --
    clk_i       => m_axi_aclk,        -- global clock, rising edge
    rst_i       => m_axi_aresetn,     -- global reset, async, LOW-active
    -- parallel io --
    gpio_o      => gpio_o_int,        -- parallel output
    gpio_i      => gpio_i_int,        -- parallel input
    -- pwm channels --
    pwm_o       => pwm_o_int,         -- pwm channels
    -- arbitrary frequency generator --
    freq_gen_o  => freq_gen_o_int,    -- programmable frequency output
    -- serial com --
    uart_txd_o  => uart_txd_o_int,    -- UART send data
    uart_rxd_i  => uart_rxd_i_int,    -- UART receive data
    spi_sclk_o  => spi_sclk_o_int,    -- serial clock line
    spi_mosi_o  => spi_mosi_o_int,    -- serial data line out
    spi_miso_i  => spi_miso_i_int,    -- serial data line in
    spi_cs_o    => spi_cs_o_int,      -- SPI CS
    twi_sda_io  => twi_sda_io,        -- twi serial data line
    twi_scl_io  => twi_scl_io,        -- twi serial clock line
    -- 32-bit wishbone interface --
    wb_adr_o    => wb_core.adr,       -- address
    wb_dat_i    => wb_core.di,        -- read data
    wb_dat_o    => wb_core.do,        -- write data
    wb_we_o     => wb_core.we,        -- read/write
    wb_sel_o    => wb_core.sel,       -- byte enable
    wb_stb_o    => wb_core.stb,       -- strobe
    wb_cyc_o    => wb_core.cyc,       -- valid cycle
    wb_ack_i    => wb_core.ack,       -- transfer acknowledge
    -- interrupts --
    ext_irq_i   => irq_i_int,          -- external interrupt request line
    ext_ack_o   => irq_ack_o_int       -- external interrupt request acknowledge
  );


  -- Output Type Conversion ---------------------------------------------------
  -- -----------------------------------------------------------------------------
  gpio_i_int     <= std_ulogic_vector(gpio_i);
  uart_rxd_i_int <= std_ulogic(uart_rxd_i);
  spi_miso_i_int <= std_ulogic(spi_miso_i);
  irq_i_int      <= std_ulogic_vector(ext_irq_i);

  gpio_o         <= std_logic_vector(gpio_o_int);
  pwm_o          <= std_logic_vector(pwm_o_int);
  uart_txd_o     <= std_logic(uart_txd_o_int);
  spi_sclk_o     <= std_logic(spi_sclk_o_int);
  spi_mosi_o     <= std_logic(spi_mosi_o_int);
  spi_cs_o       <= std_logic_vector(spi_cs_o_int);
  ext_ack_o      <= std_logic_vector(irq_ack_o_int);
  freq_gen_o     <= std_logic_vector(freq_gen_o_int);


  -- Wishbone-to-AXI4-Lite-compatible Bridge ----------------------------------
  -- -----------------------------------------------------------------------------

  -- transfer type --
  read_trans  <= wb_core.cyc and wb_core.stb and (not wb_core.we);
  write_trans <= wb_core.cyc and wb_core.stb and wb_core.we;

  -- arbiter --
  axi_arbiter: process(m_axi_aclk)
  begin
    if rising_edge(m_axi_aclk) then
      if (wb_core.cyc = '0') then
        pending_rd   <= '0';
        pending_wr   <= '0';
        adr_valid    <= '0';
        m_axi_bready <= '0';
      else
        -- read/write address valid --
        if ((wb_core.cyc and wb_core.stb) = '1') then
          adr_valid <= '1';
        elsif (m_axi_awready = '1') or (m_axi_arready = '1') then
          adr_valid <= '0';
        end if;
        -- transfer read data --
        if (read_trans = '1') then
          pending_rd <= '1';
        elsif (m_axi_rvalid = '1') then
          pending_rd <= '0';
        end if;
        -- transfer write data --
        if (write_trans = '1') then
          pending_wr <= '1';
        elsif (m_axi_wready = '1') then
          pending_wr <= '0';
        end if;
        -- write response channel -
        if (write_trans = '1') then
          m_axi_bready <= '1';
        elsif (m_axi_bvalid = '1') then
          m_axi_bready <= '0';
        end if;
      end if;
    end if;
  end process axi_arbiter;

  -- Acknowledge Wishbone transfer --
  wb_core.ack   <= (pending_rd and std_ulogic(m_axi_rvalid)) or -- read transfer
--                 (pending_wr and std_ulogic(m_axi_wready)); -- write transfer
                   (wresp_ok and m_axi_bvalid); -- acknowledged write transfer

  -- Read Address Channel --
  m_axi_araddr  <= std_logic_vector(wb_core.adr);
  m_axi_arvalid <= std_logic(adr_valid) and std_logic(pending_rd);
  m_axi_arprot  <= "000"; -- data access, secure, unprivileged

  -- Read Data Channel --
  wb_core.di    <= std_ulogic_vector(m_axi_rdata);
  m_axi_rready  <= std_logic(pending_rd);

  -- Write Address Channel --
  m_axi_awaddr  <= std_logic_vector(wb_core.adr);
  m_axi_awvalid <= std_logic(adr_valid) and std_logic(pending_wr);
  m_axi_awprot  <= "000"; -- data access, secure, unprivileged

  -- Write Data Channel --
  m_axi_wdata   <= std_logic_vector(wb_core.do);
  m_axi_wstrb   <= std_logic_vector(wb_core.sel);
  m_axi_wvalid  <= std_logic(pending_wr);

  -- Write Data Response Channel --
  wresp_ok      <= '1' when (m_axi_bresp = "00") else '0';


end neo430_top_axi4lite_rtl;
