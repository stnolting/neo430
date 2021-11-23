:information_source: Please check out the follow-up project: [The NEORV32 RISC-V Processor](https://github.com/stnolting/neorv32)

# The NEO430 Processor

[![license](https://img.shields.io/github/license/stnolting/neo430)](https://github.com/stnolting/neo430/blob/master/LICENSE)
[![release](https://img.shields.io/github/v/release/stnolting/neo430)](https://github.com/stnolting/neo430/releases)
![processor status](https://img.shields.io/badge/processor%20status-stable-brightgreen)

## Table of Content

* [Introduction](#Introduction)
* [Processor Features](#Processor-Features)
* [Differences to the Original MSP430 Processors](#Differences-to-the-Original-MSP430-Processors)
* [Top Entity](#Top-Entity)
* [FPGA Implementation Results](#FPGA-Implementation-Results)
* [HW-SW Ecosystem](#HW-SW-Ecosystem)
* [Performance](#Performance)
* [**Quick Start**](#Quick-Start)
* [Change Log](#Change-Log)
* [Contact](#Contact)
* [Disclaimer, Proprietary and Legal Notice](#Disclaimer-Proprietary-and-Legal-Notice)



## Introduction

Welcome to __[The NEO430 Processor](https://github.com/stnolting/neo430)__ project!

You need a small but still powerful, customizable and microcontroller-like processor system for your next FPGA project?
Then the NEO430 is the right choice for you.

This processor is based on the Texas Instruments MSP430(TM) ISA and provides compatibility with the original
instruction set (see differences below). The NEO430 is not an exact MSP430 clone – it is more like a complete new implementation from the bottom up. The
processor features a very small outline, already implementing standard features like a timer, a watchdog, UART, TWI and SPI
serial interfaces, general purpose IO ports, an internal bootloader and of course internal memory for program code and data.
All of the implemented peripheral modules are optional – so if you do not need them, you can exclude them from synthesis to
reduce the size of the system. Any additional modules, which make a more customized system, can be connected via a
Wishbone-compatible bus interface or directly implemented within the processor. By this, you can built a system,
that perfectly fits your needs.

It is up to you to use the NEO430 as stand-alone, configurable and extensible microcontroller, or to use it as controller
within a more complex SoC design.

The high-level software development is based on the free TI msp430-gcc compiler tool chain. You can either use Windows
(PowerShell or Linux Subsystem) or Linux as build environment for your applications – the project supports both worlds.

This project is intended to work "out of the box". Just synthesize the test setup from this project, upload it to your
FPGA board of choice (the NEO430 uses a FPGA vendor-independent VHDL description) and start exploring the capabilities of
the NEO430 processor. Application program generation works by executing a single "make" command. Jump to the
"Let’s Get It Started" chapter in the NEO430 documentary, which provides a lot of guides and tutorials to make your first
NEO430 setup run: [![NEO430 Datasheet](https://raw.githubusercontent.com/stnolting/neo430/master/doc/figures/PDF_32.png) NEO430 Datasheet](https://raw.githubusercontent.com/stnolting/neo430/master/doc/NEO430.pdf "NEO430 Datasheet from GitHub")



## Processor Features

![NEO430 Overview](https://raw.githubusercontent.com/stnolting/neo430/master/doc/figures/neo430_arch.png)

(optional modules are marked using dashed lines in the figure above)


- 16-bit open source soft-core microcontroller-like processor system
- Full support of the original [MSP430 instruction set architecture](https://raw.githubusercontent.com/stnolting/neo430/master/doc/instruction_set.pdf)
- Code-efficient CISC-like instruction capabilities
- Tool chain based on free [TI msp430-gcc compiler](http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSPGCC/latest/index_FDS.html "TI `msp430-gcc` compiler") (also available [here on github](https://github.com/stnolting/msp430-gcc))
- Application compilation scripts ([makefiles](https://github.com/stnolting/neo430/blob/master/sw/example/blink_led/Makefile)) for Windows Powershell / Windows Subsystem for Linux / native Linux
- Software requirements (regardless of platform):
  - TI `msp430-gcc` compiler
  - native `GCC` compiler
  - `GNU Make`
- Completely described in behavioral, platform-independent VHDL (no macros, primitives, attributes, etc. used)
- Fully synchronous design, no latches, no gated clocks
- Very low resource requirements and high operating frequency
- Internal [DMEM](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_dmem.vhd) (RAM, for data) and [IMEM](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_imem.vhd) (RAM or ROM, for code), configurable sizes
- Customizable processor hardware configuration:
  - Optional multiplier/divider unit ([MULDIV](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_muldiv.vhd))
  - Optional high-precision timer ([TIMER](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_timer.vhd))
  - Optional universal asynchronous receiver and transmitter ([UART](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_uart.vhd))
  - Optional serial peripheral interface master ([SPI](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_spi.vhd)), 8 or 16 bit transfer data size, 6 dedicated chip-select lines
  - Optional I2C-compatible two wire serial interface master ([TWI](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_twi.vhd)) supporting clock stretching
  - Optional general purpose parallel IO port ([GPIO](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_gpio.vhd)), 16 inputs & 16 outputs, with pin-change interrupt and PWM option
  - Optional 32-bit Wishbone bus master interface adapter ([WB32](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_wb_interface.vhd)) - including bridges to [Avalon (TM](https://github.com/stnolting/neo430/blob/master/rtl/top_templates/neo430_top_avm.vhd)) bus and [AXI4-Lite (TM](https://github.com/stnolting/neo430/blob/master/rtl/top_templates/neo430_top_axi4lite.vhd))
  - Optional watchdog timer ([WDT](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_wdt.vhd))
  - Optional cyclic redundancy check unit ([CRC16/32](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_crc.vhd))
  - Optional custom functions unit ([CFU](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_cfu.vhd)) for processor-internal user-defined processor extensions
  - Optional 4 channel PWM controller with 4 or 8 bit resolution ([PWM](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_pwm.vhd))
  - Optional Galois Ring Oscillator (GARO) based true random number generator ([TRNG](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_trng.vhd)) with de-biasing and internal post-processing
  - Optional external interrupts controller with 8 independent channels ([EXIRQ](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_exirq.vhd)), can also be used for software-triggered interrupts (traps, breakpoints, etc.)
  - Optional NCO-based programmable frequency generator ([FREQ_GEN](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_freq_gen.vhd)) with 3 independent output channels
  - Optional internal [bootloader](https://github.com/stnolting/neo430/blob/master/sw/bootloader/bootloader.c) (2kB ROM) with serial user console and automatic application boot from external SPI flash (like the FPGA configuration storage)



## Differences to the Original MSP430 Processors

- Completely different processor modules with different functionality
- Up to 48kB instruction memory and 12kB data memory
- NEO430 tool chain (makefiles, boot-code and linker script) required for application compilation
- Custom binary executable format
- No hardware debugging interface
- No analog components
- No support of TI's Code Composer Studio
- No support of CPU's DADD operation
- _Implicit_ software support of the NEO430 multiplier only via experimental ABI-override mode**
- Just 4 CPU interrupt channels (can be extended via the external IRQ controller)
- Single clock domain for complete processor
- Different numbers of instruction execution cycles
- Only one power-down (sleep) mode

**) A quite promising experimental mode to allow implicit multiplier usage (just write A*B in your code and the compiler
will automatically utilize the multiplier unit for this)



## Top Entity

The top entity of the processor is [**neo430_top.vhd**](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_top.vhd) (from the rtl\core folder).
Just instantiate this file in your project and you are ready to go! All signals of this top entity are of type *std_ulogic* or *std_ulogic_vector*, respectively.
If you need a top entity with resolved signals (*std_logic*), take a look at the [top_templates](https://github.com/stnolting/neo430/blob/master/rtl/top_templates) folder.
These alternative top entities also support AXI or Avalon connectivity.


### Generics

The main features and the actually implemented peripheral modules are configured via the top unit's generics.

| Generic Name | Type                    | Default Value | Function                                                 |
|:-------------|:-----------------------:|:-------------:|:---------------------------------------------------------|
| CLOCK_SPEED  | natural                 | 100000000     | Clock speed of CPU clock input "clk_i" in Hz             |
| IMEM_SIZE    | natural                 | 4*1024        | Size of internal instruction memory in bytes (max 48 kB) |
| DMEM_SIZE    | natural                 | 2*1024        | Size of internal data memory in bytes (max 12 kB)        |
| USER_CODE    | std_ulogic_vector(15:0) | x"0000"       | 16-bit custom user code, can be read by user software    |
| MULDIV_USE   | boolean                 | true          | Implement multiplier/divider unit (MULDIV)               |
| WB32_USE     | boolean                 | true          | Implement Wishbone interface adapter (WB32)              |
| WDT_USE      | boolean                 | true          | Implement watchdog timer (WDT)                           |
| GPIO_USE     | boolean                 | true          | Implement general purpose parallel in/out port (GPIO)    |
| TIMER_USE    | boolean                 | true          | Implement high-precision timer (TIMER)                   |
| UART_USE     | boolean                 | true          | Implement UART serial communication unit (UART)          |
| CRC_USE      | boolean                 | true          | Implement checksum computation unit (CRC16/32)           |
| CFU_USE      | boolean                 | false         | Implement custom functions unit (CFU)                    |
| PWM_USE      | boolean                 | true          | Implement pulse width controller (PWM)                   |
| TWI_USE      | boolean                 | true          | Implement two wire serial interface unit (TWI)           |
| SPI_USE      | boolean                 | true          | Implement serial peripheral interface unit (SPI)         |
| TRNG_USE     | boolean                 | false         | Implement true random number generator (TRNG)            |
| EXIRQ_USE    | boolean                 | true          | Implement external interrupts controller (EXIRQ)         |
| FREQ_GEN_USE | boolean                 | true          | Implement programmable frequency generator (FREQ_GEN)    |
| BOOTLD_USE   | boolean                 | true          | Implement and auto-start internal bootloader             |
| IMEM_AS_ROM  | boolean                 | false         | Implement internal instruction memory as read-only       |


### Signals

Regarding unused unit's signals: Connect all unused inputs to low and leave all unused outputs 'open'.
Signal driections are seen from the processor.

| Signal Name  | Width | Direction | HW Unit  | Function                                                 |
|:-------------|:-----:|:---------:|:--------:|:---------------------------------------------------------|
| clk_i        | 1     | In        | -        | Global clock line; all FFs triggering on rising edge     |
| rst_i        | 1     | In        | -        | Global reset, low-active                                 |
| gpio_o       | 16    | Out       | GPIO     | General purpose parallel output                          |
| gpio_i       | 16    | In        | GPIO     | General purpose parallel input                           |
| pwm_o        | 4     | Out       | PWM      | Pulse width modulation channels                          |
| timer_fg_o   | 1     | Out       | FREQ_GEN | Programmable frequency generator output                  |
| uart_txd_o   | 1     | Out       | UART     | UART serial transmitter                                  |
| uart_rxd_i   | 1     | In        | UART     | UARt serial receiver                                     |
| spi_sclk_o   | 1     | Out       | SPI      | SPI master clock                                         |
| spi_mosi_o   | 1     | Out       | SPI      | SPI serial data output                                   |
| spi_miso_i   | 1     | In        | SPI      | SPI serial data input                                    |
| spi_cs_o     | 8     | Out       | SPI      | SPI chip select lines (active-low)                       |
| twi_sda_io   | 1     | InOut     | TWI      | TWI master serial data line (external pull-up required)  |
| twi_scl_io   | 1     | InOut     | TWI      | TWI master serial clock line (external pull-up required) |
| wb_adr_o     | 32    | Out       | WB32     | Wishbone slave address                                   |
| wb_dat_i     | 32    | In        | WB32     | Wishbone write data                                      |
| wb_dat_o     | 32    | Out       | WB32     | Wishbone read data                                       |
| wb_we_o      | 1     | Out       | WB32     | Wishbone write enable                                    |
| wb_sel_o     | 1     | Out       | WB32     | Wishbone byte enable                                     |
| wb_stb_o     | 1     | Out       | WB32     | Wishbone strobe                                          |
| wb_cyc_o     | 1     | Out       | WB32     | Wishbone valid cycle                                     |
| wb_ack_i     | 1     | In        | WB32     | Wishbone transfer acknowledge                            |
| ext_irq_i    | 8     | In        | EXIRQ    | Interrupt request lines, high-active                     |
| ext_ack_o    | 8     | Out       | EXIRQ    | Interrupt acknowledge, high-active, single-shot          |



## FPGA Implementation Results

Mapping results generated for HW version 0x0406. The full (default) hardware configuration includes
all optional processor modules (excluding the CFU and the TRNG), an IMEM size of 4kB and a DMEM
size of 2kB. The minimal configuration only includes the CPU (including IMEM and DMEM) and the GPIO module.
Results generated with Xilinx Vivado 2019.2, Intel Quartus Prime Lite 17.1 and Lattice Radiant 1.1 (Synplify)

| __Xilinx Artix-7 (XC7A35TICSG324-1L)__ | LUTs      | FFs          | BRAMs    | DSPs   | f_max*  |
|:---------------------------------------|:---------:|:------------:|:--------:|:------:|:-------:|
| Full (default) configuration:          | 1036 (5%) | 1144 (2.75%) | 2.5 (5%) | 0 (0%) | 100 MHz |
| Minimal configuration (CPU + GPIO):    |  576 (3%) |  266 (0.6%)  |   1 (2%) | 0 (0%) | 100 MHz |

| __Intel Cyclone IV (EP4CE22F17C6)__ | LUTs      | FFs       | Memory bits | DSPs   | f_max   |
|:------------------------------------|:---------:|:---------:|:-----------:|:------:|:-------:|
| Full (default) configuration:       | 1869 (8%) | 1137 (5%) | 65800 (11%) | 0 (0%) | 121 MHz |
| Minimal configuration (CPU + GPIO): |  590 (3%) |  230 (1%) |  49408 (8%) | 0 (0%) | 122 MHz |

| __Lattice iCE40 UltraPlus** (iCE40UP5K-SG48I)__  | LUTs       | FFs        | EBRs    | DSPs   | SPRAMs  | f_max*    |
|:-------------------------------------------------|:----------:|:----------:|:-------:|:------:|:-------:|:---------:|
| Full (default) configuration:                    | 3928 (74%) | 1923 (36%) | 9 (30%) | 0 (0%) | 2 (50%) | 20.25 MHz |
| Minimal configuration (CPU + GPIO + Bootloader): | 1812 (34%) |  755 (14%) | 4 (13%) | 0 (0%) | 2 (50%) | 20.25 MHz |

*) Constrained

**) Using optimized memory modules for IMEM (32kB) & DMEM (12kB) from the `rtl\fpga_specific\lattice_ice40up` folder


### Device Utilization by Entity

The following table shows the required resources for each module of the NEO430 processor system. Note that the provided
numbers only represent a coarse overview as logic elements might be merged and optimized beyond entity boundaries.

Mapping results generated for HW version 0x0406. The full (default) hardware configuration includes all optional
processor modules (excluding the CFU but including the TRNG), an IMEM size of 4kB and a DMEM size of
2kB. Results were generated using Intel Quartus Prime Lite 17.1.

| __Intel Cyclone IV (EP4CE22F17C6)__    | LUTs | FFs | Memory Bits | DSPs |
|:---------------------------------------|:----:|:---:|:-----------:|:----:|
| Bootloader Memory (Boot ROM, 2kB)      | 2    | 1   | 16384       | 0    |
| Central Processing Unit (CPU)          | 525  | 169 | 264         | 0    |
| Checksum Unit (CRC)                    | 111  | 94  | 0           | 0    |
| Custom Functions Unit (CFU)*           | -    | -   | -           | -    |
| Data Memory (DMEM, 2kB)                | 5    | 1   | 16384       | 0    |
| External Interrupts Controller (EXIRQ) | 70   | 55  | 0           | 0    |
| Frequency Generator (FREQ_GEN)         | 140  | 130 | 0           | 0    |
| GPIO Port Unit (GPIO)                  | 50   | 45  | 0           | 0    |
| High-Precision Timer (TIMER)           | 66   | 57  | 0           | 0    |
| Instruction Memory (IMEM, 4kB)         | 5    | 1   | 32768       | 0    |
| Multiplier & Divider (MULDIV)          | 209  | 134 | 0           | 0    |
| Pulse-Width Modulation Unit (PWM)      | 96   | 66  | 0           | 0    |
| Serial Peripheral Interface (SPI)      | 82   | 59  | 0           | 0    |
| System Info Memory (SYSCONFIG)         | 12   | 11  | 0           | 0    |
| True Random Number Generator (TRNG)    | 92   | 76  | 0           | 0    |
| Two Wire Interface (TWI)               | 78   | 43  | 0           | 0    |
| Universal Asynchronous Rx/Tx (UART)    | 130  | 91  | 0           | 0    |
| Watchdog Timer (WDT)                   | 53   | 37  | 0           | 0    |
| Wishbone Interface (WB32)              | 129  | 117 | 0           | 0    |

*) Hardware requirements defined by user application



## HW-SW Ecosystem

The NEO430 Processor project provides driver libraries for the CPU itself and all included peripheral modules. These libraries
provide a certain level of hardware abstraction and allow an easy usage of the different hardware modules. Modules that cannot
be used "explicitly" (like CPU modules or the different memories) are not listed below. 

| Hardware unit                          | VHDL source | C library source | C library header | SW example project |
|:---------------------------------------|:-----------:|:----------------:|:----------------:|:------------------:|
| Main CPU defines file                  | - | - | [neo430.h](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/include/neo430.h) | - |
| Central Processing Unit (CPU)          | [neo430_cpu.vhd](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_cpu.vhd) | [neo430_cpu.c](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/source/neo430_cpu.c) | [neo430_cpu.h](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/include/neo430_cpu.h) |- |
| Custom Functions Unit (CFU)            | [neo430_cfu.vhd](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_cfu.vhd) | provided by CFU designer | provided by CFU designer | [example](https://github.com/stnolting/neo430/tree/master/sw/example/cfu_test) |
| Checksum Unit (CRC16/32)               | [neo430_crc.vhd](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_crc.vhd) | [neo430_crc.c](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/source/neo430_crc.c) | [neo430_crc.h](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/include/neo430_crc.h) | [example](https://github.com/stnolting/neo430/tree/master/sw/example/crc_test) |
| External Interrupts Controller (EXIRQ) | [neo430_exirq.vhd](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_exirq.vhd) | [neo430_exirq.c](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/source/neo430_exirq.c) | [neo430_exirq.h](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/include/neo430_exirq.h) | [example](https://github.com/stnolting/neo430/tree/master/sw/example/exirq_test) |
| Frequency Generator (FREQ_GEN)         | [neo430_freq_gen.vhd](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_freq_gen.vhd) | [neo430_freq_gen.c](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/source/neo430_freq_gen.c) | [neo430_freq_gen.h](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/include/neo430_freq_gen.h) | [example](https://github.com/stnolting/neo430/tree/master/sw/example/freq_gen_demo) |
| IO Port Unit (GPIO)                    | [neo430_gpio.vhd](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_gpio.vhd) | [neo430_gpio.c](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/source/neo430_gpio.c) | [neo430_gpio.h](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/include/neo430_gpio.h) | [example](https://github.com/stnolting/neo430/tree/master/sw/example/blink_led) |
| Multiplier & Divider (MULDIV)          | [neo430_muldiv.vhd](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_muldiv.vhd) | [neo430_muldiv.c](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/source/neo430_muldiv.c) | [neo430_muldiv.h](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/include/neo430_muldiv.h) | [example](https://github.com/stnolting/neo430/tree/master/sw/example/muldiv_test) |
| Pulse-Width Modulation Unit (PWM)      | [neo430_pwm.vhd](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_pwm.vhd) | [neo430_pwm.c](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/source/neo430_pwm.c) | [neo430_pwm.h](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/include/neo430_pwm.h) | [example](https://github.com/stnolting/neo430/tree/master/sw/example/pwm_demo) |
| Serial Peripheral Interface (SPI)      | [neo430_spi.vhd](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_spi.vhd) | [neo430_spi.c](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/source/neo430_spi.c) | [neo430_spi.h](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/include/neo430_spi.h) | [example](https://github.com/stnolting/neo430/tree/master/sw/bootloader) |
| High-Precision Timer (TIMER)           | [neo430_timer.vhd](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_timer.vhd) | [neo430_timer.c](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/source/neo430_timer.c) | [neo430_timer.h](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/include/neo430_timer.h) | [example](https://github.com/stnolting/neo430/tree/master/sw/example/timer_simple) |
| True Random Number Generator (TRNG)    | [neo430_trng.vhd](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_trng.vhd) | [neo430_trng.c](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/source/neo430_trng.c) | [neo430_trng.h](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/include/neo430_trng.h) | [example](https://github.com/stnolting/neo430/tree/master/sw/example/trng_test) |
| Two Wire Interface (TWI)               | [neo430_twi.vhd](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_twi.vhd) | [neo430_twi.c](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/source/neo430_twi.c) | [neo430_twi.h](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/include/neo430_twi.h) | [example](https://github.com/stnolting/neo430/tree/master/sw/example/twi_test) |
| Universal Asynchronous Rx/Tx (UART)    | [neo430_uart.vhd](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_uart.vhd) | [neo430_uart.c](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/source/neo430_uart.c) | [neo430_uart.h](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/include/neo430_uart.h) | [example](https://github.com/stnolting/neo430/tree/master/sw/example/uart_irq) |
| Watchdog Timer (WDT)                   | [neo430_wdt.vhd](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_wdt.vhd) | [neo430_wdt.c](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/source/neo430_wdt.c) | [neo430_wdt.h](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/include/neo430_wdt.h) | [example](https://github.com/stnolting/neo430/tree/master/sw/example/wdt_test) |
| Wishbone Interface (WB32)              | [neo430_wb_interface.vhd](https://github.com/stnolting/neo430/blob/master/rtl/core/neo430_wb_interface.vhd) | [neo430_wishbone.c](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/source/neo430_wishbone.c) | [neo430_wishbone.h](https://github.com/stnolting/neo430/blob/master/sw/lib/neo430/include/neo430_wishbone.h) | [example](https://github.com/stnolting/neo430/tree/master/sw/example/wb_terminal) |

All software projects are compiled using an [application makefile](https://github.com/stnolting/neo430/blob/master/sw/example/blink_led/makefile).


## Performance

In contrast to most mainstream processors the NEO430 processor does not implement a "classic" pipelined instruction execution. Instead,
a **multi-cycle instruction execution scheme** is used: Each single instruction is executed in a series of micro instructions
requiring several clock cycles to complete. The main benefit of this execution style is the highly reduced logic overhead as no
complex pipeline hazard detection and resolving logic is required. This makes the NEO430 even smaller - at the cost of a reduced IPC
(*instructions per cycle*). Also, the MSP430 ISA is not really compatible to the classic (DLX/MIPS-like) pipeline scheme due to
its complex operand and adressing modes (e.g., ALU operations executing directly on memory data). However, this concept allows
the processor to use very **dense and powerfull CISC-like operations**.

Furthermore, the multi-cycle architecture features a **very short crtitical path** when compared to other (even 32-bit)
processors. Thus, the NEO430 can operate at very high frequencies even on low-cost (e.g., +120MHz on an Intel Cyclone IV)
and low-power FPGAs (e.g., +20MHz on a Lattice iCE40 UltraPlus) even without any area constraints.

Depending on the format / operand addressing mode of an instruction, the actual execution can take 3 to 10 clock cycles. If all possible instruction
types and formats are executed in an equally distributed manner (this is the worst case), the average CPI (clock cycles per instruction)
evaluates to **7.33 cycles/instruction resulting in 0.136 MIPS per MHz (again: worst case)**.


### CoreMark Benchmark

The [CoreMark CPU benchmark](https://www.eembc.org/coremark/) was executed on the NEO430 and is available in the
[sw/example/coremark](https://github.com/stnolting/neo430/blob/master/sw/example/coremark) project folder This benchmark
tests the capabilities of a CPU itself rather than the functions provided by the whole system / SoC.

~~~
Configuration
Hardware: 100 MHz, 32kB IMEM, 12kB DMEM, HW verison 0x0407, peripherals used: TIMER, UART, MULDIV
Software: msp430-gcc 8.3.0 for Linux, MEM_METHOD is MEM_STACK, 2000 CoreMark iterations
~~~

| __Optimization/Configuration__ | __Executable Size__ | __CoreMark Score__ | __Relative Score__  |
|:-------------------------------|:-------------------:|:------------------:|:-------------------:|
| -Os                            | 12150 bytes         | 6.57               | 0.065 CoreMarks/MHz |
| -O2                            | 14600 bytes         | 7.16               | 0.072 CoreMarks/MHz |
| -Os + NEO430_MULDIV*           | 12118 bytes         | 14.43              | 0.144 CoreMarks/MHz |
| -O2 + NEO430_MULDIV*           | 14562 bytes         | 17.68              | 0.176 CoreMarks/MHz |
| -Os + NEO430_MULDIV* (DSP**)   | 12060 bytes         | 15.63              | 0.156 CoreMarks/MHz |
| -O2 + NEO430_MULDIV* (DSP**)   | 14510 bytes         | 19.42              | 0.194 CoreMarks/MHz |

*) These results were generated using the "NEO430_HWMUL_ABI_OVERRIDE" feature flag, which allows to map implicit multiplications
in the source code via compiler primitives directly to the multiplier core of the MULDIV unit. For more information see
chapter "Multiplier and Divider Unit (MULDIV)" of [NEO430.pdf](https://raw.githubusercontent.com/stnolting/neo430/master/doc/NEO430.pdf).

**) This setup uses the FPGA's embedded multipliers (DSP blocks) for the MULDIV's multiplier unit. The "NEO430_HWMUL_DSP" feature flag eliminates
the wait cycles usually required to wait for the result generated by the default serial MULDIV multiplier core.

Even though a score of 6.57 can outnumber certain architectures and configurations (see the score table on the CoreMark
homepage), the relative score of 0.065 coremarks per second might sound pretty low. True. But you have to keep in mind that benchmark
was executed using only the resources of the CPU itself. The CPU consists of only ~520 Intel Cyclone IV LUTs and does not
contain any sophisticated ALU operations like multiplication or barrel shifting. When including NEO430 MULDIV unit (using the
"NEO430_HWMUL_ABI_OVERRIDE" feature and using embedded DSP blocks ("NEO430_HWMUL_DSP")) the CoreMark score is increased to 19.42.
By explicitly using additional HW accelerators from the NEO430 ecosystem (e.g. the CRC unit) the performance can be further increased.



## Quick Start

 * At first, get the most recent version the NEO430 Processor project from GitHub:
 
   * Clone the NEO430 repository using `git` from the command line (suggested for easy project updates via `git pull`):
 
     ~~~
     git clone https://github.com/stnolting/neo430.git
     ~~~

   * Or download the current version of this project as ZIP file: [neo430-master.zip](https://github.com/stnolting/neo430/archive/master.zip)

 * Next, install the free `MSP430-GCC` compiler toolchain from the TI homepage (select the "compiler only" package according to your system OS):

   https://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSPGCC/latest/index_FDS.html
   
   * Alternatively, you can [directly download a copy](https://github.com/stnolting/msp430-gcc) of the msp430-gcc toolchain for Windows/Linux from github

 * Make sure `GNU Make` and a native `GCC` compiler are installed (double check for the newest versions)

 * Create a new HW project with your FPGA synthesis tool of choice. Add all files from the `rtl\core` folder to this project (and add them to a new library called "neo430").

 * Follow the instructions from the "Let's Get It Started" section of the NEO430 documentary: [![NEO430 Datasheet](https://raw.githubusercontent.com/stnolting/neo430/master/doc/figures/PDF_32.png) NEO430 Datasheet](https://raw.githubusercontent.com/stnolting/neo430/master/doc/NEO430.pdf "NEO430 Datasheet from GitHub")

 * This documentary will guide you to create a simple test setup, which serves as ["hello word" FPGA demo](https://github.com/stnolting/neo430/blob/master/rtl/top_templates/neo430_test.vhd): 

![NEO430 Test Example Setup](https://raw.githubusercontent.com/stnolting/neo430/master/doc/figures/test_setup.jpg)

 * The NEO430 project also includes some example programs from which you can start your own application: [SW example projects](https://github.com/stnolting/neo430/tree/master/sw/example)

 * Have fun! =)



## Change Log

The official change log of this project can be found at the end of the [![NEO430 Datasheet](https://raw.githubusercontent.com/stnolting/neo430/master/doc/figures/PDF_32.png) NEO430 Datasheet](https://raw.githubusercontent.com/stnolting/neo430/master/doc/NEO430.pdf "NEO430 Datasheet from GitHub").



## Contact

If you have any questions, bug reports, ideas or if you are facing problems with the NEO430 or want to give some kinf of feedback, open a
[new issue](https://github.com/stnolting/neo430/issues) or directly drop me a line:

  stnolting@gmail.com



## Citation

If you are using the NEO430 Processor in some kind of publication, please cite it as follows:

> S. Nolting, "The NEO430 Processor", github.com/stnolting/neo430



## Disclaimer, Proprietary and Legal Notice

This is a hobby project released under the BSD 3-Clause license. No copyright infringement intended.

**BSD 3-Clause License**

Copyright (c) 2020, Stephan Nolting. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or other materials
provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors may be used to
endorse or promote products derived from this software without specific prior written
permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.


"MSP430" and "Code Composer Studio" are trademarks of Texas Instruments Corporation.

"Windows" is a trademark of Microsoft Corporation.

"Virtex", "Artix" and "Vivado" are trademarks of Xilinx Inc.

"Cyclone", "Quartus" and "Avalon Bus" are trademarks of Intel Corporation.

"iCE40 UltraPlus", "Lattice Radiant" and "Lattice Diamond" are trademarks of Lattice Semiconductor Corporation.

"AXI", "AXI4" and "AXI4-Lite" are trademarks of Arm Holdings plc.
