# Introduction

Welcome to the NEO430 Processor project!

You need a small but still powerful, customizable and microcontroller-like
processor system for your next FPGA design? Then the NEO430 is the perfect
choice for you!

This processor is based on the Texas Instruments MSP430 ISA and provides 100%
compatibility with the original instruction set. The NEO430 is not an MSP430
clone – it is more a complete new implementation from the bottom up. The
processor features a very small outline, already implementing standard
features like a timer, a watchdog, UART and SPI serial interfaces, general
purpose IO ports, an internal bootloader and of course internal memory for
program code and data. All of the peripheral modules are optional – so if you
do not need them, you can exclude them from implementation to reduce the size
of the system. Any additional modules, which make a more customized system,
can be connected via a Wishbone-compatible bus interface. By this, you can
built a system, that perfectly fits your needs.

It is up to you to use the NEO430 as stand-alone, configurable and extensible
microcontroller, or to include  it as controller within a more complex SoC
design.

The high-level software development is based on the free TI msp430-gcc
compiler tool chain. You can either use Windows or Linux/Cygwin as build
environment for your applications – the project comes with build scripts
for both worlds. The example folder of this project features several demo
programs, from which you can start creating your own NEO430 applications.

This project is intended to work "out of the box". Just synthesize the test
setup from this project, upload it to your FPGA board of choice and start
exploring the capabilities of the NEO430 processor. Application program
generation (and even installation) works by executing a single "make" command.
Jump to the "Let’s Get It Started" chapter, which provides a lot of guides and
tutorials to make your first NEO430 setup run:
https://github.com/stnolting/neo430/blob/master/doc/NEO430.pdf


## Processor Features

<img src="https://github.com/stnolting/neo430/blob/master/doc/figures/neo430_arch.png" width="500px"/>
(optional modules are marked using dashed lines)


- 16-bit open source soft-core microcontroller-like processor system
- Code-efficient CISC-like instruction capabilities
- Full support of the original MSP430 instruction set architecture
- Tool chain based on free TI msp430-gcc compiler
- Application compilation scripts for Windows and Linux/Cygwin
- Completely described in behavioral, platform-independent VHDL
- Fully synchronous design, no latches, no gated clocks
- Operates at high frequencies (150 MHz) - no clock-domain crossing required for integration
- Very small outline and high operating frequency compared to other implementations ;)
- Internal DMEN (RAM, for data) and IMEM (RAM or ROM, for code), configurable sizes
- One external interrupt line
- Customizable processor hardware configuration
- Optional custom function unit (CFU) to add your custom memory-mapped functions
- Optional high-precision timer (TIMER)
- Optional USART interface; UART and SPI (USART)
- Optional general purpose parallel IO port (GPIO), 16 inputs, 16 outputs, with pin-change interrupt
- Optional 32-bit Wishbone bus interface adapter (WB32)
- Optional watchdog timer (WDT)
- Optional internal bootloader (2kB ROM)


## Differences to TI's original MSP430 processors

- Completely different processor modules with different functionality
- No hardware multiplier support (but emulated in software)
- Maximum of 32kB instruction memory and 28kB data memory
- Specific memory map – included NEO430 linker script and compilation script required
- Custom binary executable format
- Only 4 CPU interrupt channels (instead of 16)
- Single clock domain for complete processor
- Different numbers of instruction execution cycles
- Only one power-down (sleep) mode
- Wishbone-compatible interface to attach custom IP
- Internal bootloader with text interface (via UART serial port)


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



<img src="https://github.com/stnolting/neo430/blob/master/doc/figures/oshw_logo.png" width="100px"/>
