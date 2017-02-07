/* ################################################################################################# */
/* #  < NEO430 MSPGCC bootloader linker script >                                                   # */
/* # ********************************************************************************************* # */
/* # This file is part of the NEO430 Processor project: http://opencores.org/project,neo430        # */
/* # Copyright 2015-2016, Stephan Nolting: stnolting@gmail.com                                     # */
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
/* # source; if not, download it from http://www.gnu.org/licenses/lgpl-3.0.en.html                 # */
/* # ********************************************************************************************* # */
/* #  Stephan Nolting, Hannover, Germany                                               27.11.2016  # */
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

      *(.init .init.*);
      *(.init0); /* Start here after reset. */
      *(.init1); /* User definable */
      *(.init2); /* Initialize stack */
      *(.init3); /* Initialize hardware */
      *(.init4); /* Copy data to .data, clear bss */
      *(.init5); /* User definable. */
      *(.init6); /* C++ constructors. */
      *(.init7); /* User definable. */
      *(.init8); /* User definable. */
      *(.init9); /* Call main(). */

      *(.fini9); /* Falls into here after main(). User definable. */
      *(.fini8); /* User definable. */
      *(.fini7); /* User definable. */
      *(.fini6); /* C++ destructors. */
      *(.fini5); /* User definable. */
      *(.fini4); /* User definable. */
      *(.fini3); /* User definable. */
      *(.fini2); /* User definable. */
      *(.fini1); /* User definable. */
      *(.fini0); /* Infinite loop after program termination. */
      *(.fini .fini.*);

      . = ALIGN(2);
      __ctors_start = .;
      KEEP(*(.ctors));
      __ctors_end = .;
      __dtors_start = .;
      KEEP(*(.dtors));
      __dtors_end = .;

      . = ALIGN(2);
      *(.text .text.*);
      *(.near.text .near.text.*);

      . = ALIGN(2);
      *(.rodata .rodata.*);
      *(.data .data.*);
      *(.sdata .sdata.*);

    } > boot_rom
}
