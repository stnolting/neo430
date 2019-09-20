## Introduction

Welcome to the __NEO430 Processor__ project!

You need a small but still powerful, customizable and microcontroller-like
processor system for your next FPGA design? Then the NEO430 is the perfect
choice for you!

This processor is based on the Texas Instruments MSP430(TM) ISA and provides 100%
compatibility with the original instruction set. The NEO430 is not an MSP430
clone – it is more a complete new implementation from the bottom up. The
processor features a very small outline, already implementing standard
features like a timer, a watchdog, UART, TWI and SPI serial interfaces, general
purpose IO ports, an internal bootloader and of course internal memory for
program code and data. All of the peripheral modules are optional – so if you
do not need them, you can exclude them from implementation to reduce the size
of the system. Any additional modules, which make a more customized system,
can be connected via a Wishbone-compatible bus interface. By this, you can
built a system, that perfectly fits your needs.

It is up to you to use the NEO430 as stand-alone, configurable and extensible
microcontroller, or to include it as controller within a more complex SoC
design.

The high-level software development is based on the free TI msp430-gcc
compiler tool chain. You can either use Windows or Linux as build
environment for your applications – the project comes with build scripts
for both worlds. The sw\example folder of this project features several demo
programs from which you can start creating your own NEO430 applications.

This project is intended to work "out of the box". Just synthesize the test
setup from this project, upload it to your FPGA board of choice (the NEO430 uses
a vendor-independent VHDL description) and start exploring the capabilities of
the NEO430 processor. Application program generation (and installation) works
by executing a single "make" command. Jump to the "Let’s Get It Started"
chapter, which provides a lot of guides and tutorials to make your first
NEO430 setup run:
https://github.com/stnolting/neo430/blob/master/doc/NEO430.pdf


## Processor Features

<img src="https://github.com/stnolting/neo430/blob/master/doc/figures/neo430_arch.png" width="500px"/>
(optional modules are marked using dashed lines in the figure above)


- 16-bit open source soft-core microcontroller-like processor system
- Code-efficient CISC-like instruction capabilities
- Full support of the original MSP430 instruction set architecture
- Tool chain based on free TI msp430-gcc compiler
- Application compilation scripts for Windows Powershell / Windows Subsystem for Linux / native Linux
- Completely described in behavioral, platform-independent VHDL
- Fully synchronous design, no latches, no gated clocks
- Very small outline and high operating frequency
- Internal DMEN (RAM, for data) and IMEM (RAM or ROM, for code), configurable sizes
- One external interrupt line
- Customizable processor hardware configuration
- Optional multiplier/divider unit (MULDIV)
- Optional high-precision timer (TIMER)
- Optional universal asynchronous receiver and transmitter (UART)
- Optional serial peripheral unit (SPI)
- Optional two wire serial interface (TWI)
- Optional general purpose parallel IO port (GPIO), 16 inputs, 16 outputs, with pin-change interrupt
- Optional 32-bit Wishbone bus interface adapter (WB32) - including bridges to Avalon(TM) bus and AXI4-Lite(TM)
- Optional watchdog timer (WDT)
- Optional cyclic redundancy check unit (CRC16/32)
- Optional custom functions unit (CFU) for user-defined processor extensions
- Optional 4 channel PWM controller with 1 to 8 bit resolution (PWM)
- Optional internal bootloader (2kB ROM) with serial user console and automatic boot from external SPI EEPROM


## Differences to TI's Original MSP430(TM) Processors

- Completely different processor modules with different functionality
- Up to 48kB instruction memory and 12kB data memory
- Specific memory map – included NEO430 linker script and compilation script required
- Custom binary executable format
- Just 4 CPU interrupt channels
- Single clock domain for complete processor
- Different numbers of instruction execution cycles
- Only one power-down (sleep) mode
- Wishbone-compatible interface to attach custom IP
- Internal bootloader with text interface (via UART serial port)
- *NO* built-in support of floating point types yet (i.e. float & double)


## Implementation Results

Mapping results generated for HW version 0x0300. The full (default) configuration includes
all optional processor modules (excluding the CFU), an IMEM size of 4kB and a DMEM size of 2kB.
Results generated with Xilinx Vivado 2017.3 and Intel Quartus Prime Lite 17.1

| __Xilinx Artix-7 (XC7A35TICSG324-1L)__  | LUTs      | FFs      | BRAMs    | DSPs   | f_max*  |
|:----------------------------------------|:---------:|:--------:|:--------:|:------:|:-------:|
| Full (default) configuration:           | 1006 (5%) | 952 (2%) | 2.5 (5%) | 0 (0%) | 100 MHz |
| Minimal configuration (CPU + GPIO):     | 879  (4%) | 287 (1%) | 1   (2%) | 0 (0%) | 100 MHz |

| __Intel/Altera Cyclone IV (EP4CE22F17C6)__  | LUTs      | FFs      | Memory bits  | DSPs   | f_max   |
|:--------------------------------------------|:---------:|:--------:|:------------:|:------:|:-------:|
| Full (default) configuration:               | 1676 (8%) | 940 (4%) | 65792  (11%) | 0 (0%) | 116 MHz |
| Minimal configuration (CPU + GPIO):         | 602  (3%) | 228 (1%) | 49408   (8%) | 0 (0%) | 124 MHz |

*) Constrained


## Let's Get It Started!

 * At first, make sure to get the most recent version of this project from GitHub:
 
    ~~~
    git clone https://github.com/stnolting/neo430.git
    ~~~

 * Next, install the compiler toolchain from the TI homepage (select the "compiler only" package):

  http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSPGCC/latest/index_FDS.html

 * Follow the instructions from the "Let's Get It Started" section of the NEO430 documentary:

  https://github.com/stnolting/neo430/blob/master/doc/NEO430.pdf

 * The NEO430 documentary will guide you to create a simple test setup, which serves as "hello world" FPGA demo: 

<img src="https://github.com/stnolting/neo430/blob/master/doc/figures/test_setup.jpg" width="400px"/>

 * This project also includes some example programs, from which you can start:

  https://github.com/stnolting/neo430/tree/master/sw/example

 * Have fun! =)


## Questions?

If you have any questions, bug reports, ideas or if you are facing problems with the NEO430, feel free to drop me a line:

  stnolting@gmail.com


## Citation

If you are using the NEO430 for some kind of publication, please cite it as follows:

> S. Nolting, "The NEO430 Processor", github.com/stnolting/neo430


## Proprietary Notice

"MSP430" is a trademark of Texas Instruments Corporation.

"Windows" is a trademark of Microsoft Corporation.

"Virtex", "Artix", "ISE" and "Vivado" are trademarks of Xilinx Inc.

"Cyclone", "Quartus" and "Avalon Bus" are trademarks of Intel Corporation.

"AXI", "AXI4" and "AXI4-Lite" are trademarks of Arm Holdings plc.



<img src="https://github.com/stnolting/neo430/blob/master/doc/figures/oshw_logo.png" width="100px"/>
