#################################################################################################
#  << NEO430 Processor Project >>                                                               #
# ********************************************************************************************* #
# This file is part of the NEO430 Processor project: http://opencores.org/project,neo430        #
# Copyright 2015-2017, Stephan Nolting: stnolting@gmail.com                                     #
#                                                                                               #
# This source file may be used and distributed without restriction provided that this copyright #
# statement is not removed from the file and that any derivative work contains the original     #
# copyright notice and the associated disclaimer.                                               #
#                                                                                               #
# This source file is free software; you can redistribute it and/or modify it under the terms   #
# of the GNU Lesser General Public License as published by the Free Software Foundation,        #
# either version 3 of the License, or (at your option) any later version.                       #
#                                                                                               #
# This source is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;      #
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     #
# See the GNU Lesser General Public License for more details.                                   #
#                                                                                               #
# You should have received a copy of the GNU Lesser General Public License along with this      #
# source; if not, download it from http://www.gnu.org/licenses/lgpl-3.0.en.html                 #
# ********************************************************************************************* #
#  Stephan Nolting, Hannover, Germany                                               03.01.2017  #
#################################################################################################

Hello and welcome to the NEO430 Processor project!

This processor project is the latest one in my list of open-source soft core projects.
It is my newest attempt to provide a small and nevertheless powerful processing platform for
embedded applications. I designed the CPU of this project completely from scratch, based only
on the original TI MSP430 specifications. Most of the IO devices – like the UART or the timer –
came from my previous projects (STORM and Atlas2k). Finally, the NEO430 processor came out.
This processor is 100% ISA-compatible (!) to the famous TI(TM) MSP430 instruction set
architecture. Further information about the instruction set, the available addressing modes and
general CPU operation can be found in the "MSP430xl xx Family User's Guide", available at
www.ti.com/lit/ug/slau049f/slau049f.pdf.

The NEO430 features a very small outline, only implementing elementary modules like a single
timer and basic peripherals and communication modules. Any additional IP blocks, which make a
more customized system, can be connected via a Wishbone-compatible bus interface. Also, most
of the processor-internal modules can be excluded from implementation, if their features are
not required. By this, you can build a system-on-chip, that perfectly fits your needs without
any unnecessary hardware overhead.

The software development for this processor system is based on the free TI msp430-gcc compiler
tool chain. Make sure to get a copy of the sources to make this project run. You can either use
Windows or Linux/Cygwin as build environment for your applications – the project comes with
build scripts for both worlds! The example folder of this project features several demo
programs, from which you can start creating your own NEO430 applications.

This project is intended to work "out of the box". The default system setup can be synthesized
and uploaded to your FPGA of choice. Application installation work by executing a single
"make" command. Jump to the "Let’s Get It Started" chapter to get your system running right now!
That’s all you need to do to get your first NEO430 project started. So, have fun with this project! ;)


Processor Features
* 16-bit RISC open source soft-core processor
* Full support of the original MSP430 instruction set architecture
* Tool chain based on free TI msp430-gcc compiler (C / C++)
* Application compilation scripts for Windows and Linux/Cygwin
* Completely described in behavioral, platform-independent VHDL93, no "exotic" VHDL packages used
* Very small and high operating frequency compared to other implementations ;)
* Internal DMEN (RAM, for data) and IMEM (RAM or ROM, for code), configurable sizes
* One external interrupt line
* Fully customizable hardware outline
* Optional high-precision timer (TIMER)
* Optional USART interface; UART and SPI (USART)
* Optional parallel IO port (16 inputs, 16 outputs) with pin-change interrupt (PIO)
* Optional 32-bit Wishbone bus interface adapter (WB32)
* Optional watchdog timer (WDT)
* Optional internal bootloader (2kB ROM):
* Upload new application image via UART
  -> Program external SPI EEPROM
  -> Boot from external SPI EEPROM
  -> Core dump
  -> Automatic boot sequence


Main Differences to TI's Original MSP430 Architecture
Since the NEO430 is not intended as MSP430 clone, there are several differences to TI's
original architecture. Existing programs must be modified and re-compiled to
successfully run on the NEO430.

The main differences are:

* Completely different processor modules with different functionality
* No hardware multiplier support yet (but emulated in software)
* Maximum of 32kB IMEM and 28kB DMEM
* Specific memory map – included NEO430 linker script and compilation script required
* Just 4 CPU interrupt channels (instead of 16)
* Single clock domain for complete processor
* Different numbers of instruction execution cycles
* Only one power-down (sleep) mode
* Wishbone-compatible interface to attach custom IP (e.g. modules from opencores.org)
* Internal bootloader with text interface (via UART serial port)
