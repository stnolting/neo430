/* ################################################################################################# */
/* # < NEO430 MSP430GCC bootloader linker script >                                                 # */
/* # ********************************************************************************************* # */
/* # This file is part of the NEO430 Processor project: https://github.com/stnolting/neo430        # */
/* # Copyright by Stephan Nolting: stnolting@gmail.com                                             # */
/* #                                                                                               # */
/* # This source file may be used and distributed without restriction provided that this copyright # */
/* # statement is not removed from the file and that any derivative work contains the original     # */
/* # copyright notice and the associated disclaimer.                                               # */
/* #                                                                                               # */
/* # This source file is free software; you can redistribute it and/or modify it under the terms   # */
/* # of the GNU Lesser General Public License as published by the Free Software Foundation,        # */
/* # either version 3 of the License, or (at your option) any later version.                       # */
/* #                                                                                               # */
/* # This source is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;      # */
/* # without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     # */
/* # See the GNU Lesser General Public License for more details.                                   # */
/* #                                                                                               # */
/* # You should have received a copy of the GNU Lesser General Public License along with this      # */
/* # source; if not, download it from https://www.gnu.org/licenses/lgpl-3.0.en.html                # */
/* # ********************************************************************************************* # */
/* # Stephan Nolting, Hannover, Germany                                                 19.07.2019 # */
/* ################################################################################################# */

OUTPUT_ARCH(msp430)


/* Address space layout - only the boot rom size is required here */
MEMORY
{
  boot_rom (rx) : ORIGIN = 0xF000, LENGTH = 0x0800
}

/* Final executable layout */
SECTIONS
{
    .image :
    {
      . = ALIGN(2);
      KEEP(crt0.elf(.text));

      . = ALIGN(2);
      *(.text .text.*);
      *(.near.text .near.text.*);

      . = ALIGN(2);
      *(.rodata .rodata.*);
      *(.data .data.*);
      *(.sdata .sdata.*);


      /* Unused but required by some pre-defined toolchain objects */
      PROVIDE(__romdatastart = .);
      PROVIDE(__bsssize = 0);
      PROVIDE(__romdatacopysize = 0);
      PROVIDE(__crt0_init_bss = 0);
      PROVIDE(__bssstart = 0);
      PROVIDE(__datastart = 0);
      PROVIDE(__crt0_movedata = 0);
      PROVIDE(__crt0_run_init_array = 0);
      PROVIDE(__crt0_run_fini_array = 0);
      PROVIDE(__fini_array_start = 0);
      PROVIDE(__fini_array_end = 0);
      PROVIDE(__init_array_start = 0);
      PROVIDE(__init_array_end = 0);
      PROVIDE(__preinit_array_start = 0);
      PROVIDE(__preinit_array_end = 0);

    } > boot_rom
}
