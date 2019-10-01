/* ################################################################################################# */
/* # < NEO430 MSP430GCC linker script >                                                            # */
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
/* # Stephan Nolting, Hannover, Germany                                                 01.03.2019 # */
/* ################################################################################################# */

OUTPUT_ARCH(msp430)


/* Relevant address space layout */
MEMORY
{
  rom  (rx) : ORIGIN = 0x0000, LENGTH = 0x1000
  ram (rwx) : ORIGIN = 0xC008, LENGTH = 0x0800 - 8
}

/* Final executable layout */
SECTIONS
{
  .text :
  {
    . = ALIGN(2);
    PROVIDE(__text_start = .);
    PROVIDE(__textstart = .);

    KEEP(crt0.elf(.text)); /* start-up/termination code */

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
    PROVIDE(__text_end = .);
    PROVIDE(__textend = .);
  } > rom

  .rodata :
  {
    . = ALIGN(2);
    PROVIDE(__rodata_start = .);
    PROVIDE(__rodatastart = .);

    *(.plt);
    . = ALIGN(2);
    *(.lower.rodata.* .lower.rodata)
    . = ALIGN(2);
    *(.rodata .rodata.* .gnu.linkonce.r.* .const .const:*)
    . = ALIGN(2);
    *(.either.rodata.* .either.rodata)
    . = ALIGN(2);
    *(.rodata1)
    . = ALIGN(2);

    KEEP (*(.gcc_except_table)) *(.gcc_except_table.*)
    PROVIDE (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE (__preinit_array_end = .);
    PROVIDE (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    PROVIDE (__init_array_end = .);
    PROVIDE (__fini_array_start = .);
    KEEP (*(.fini_array))
    KEEP (*(SORT(.fini_array.*)))
    PROVIDE (__fini_array_end = .);

    PROVIDE(__rodata_end = .);
    PROVIDE(__rodataend = .);
  } > rom
  _etext = .; /* Past last read-only (loadable) segment */

    /* ----------------------------------- */

  .data :
  {
    . = ALIGN(2);
    PROVIDE(__data_start = .);
    PROVIDE(__datastart = .);

    *(.lower.data.* .lower.data)
    *(.data .data.* .gnu.linkonce.d.*)
    *(.either.data.* .either.data)

    KEEP (*(.jcr))
    *(.data.rel.ro.local) *(.data.rel.ro*)
    *(.dynamic)

    KEEP (*(.gnu.linkonce.d.*personality*))
    SORT(CONSTRUCTORS)
    *(.data1)
    *(.got.plt) *(.got)

    . = ALIGN(2);
    *(.sdata .sdata.* .gnu.linkonce.s.* D_2 D_1)

    . = ALIGN(2);
    _edata = .;
    PROVIDE(edata = .);
    PROVIDE(__data_end = .);
    PROVIDE(__dataend = .);
  } > ram AT > rom

    /* ----------------------------------- */

  .bss :
  {
    . = ALIGN(2);
    PROVIDE(__bss_start = .);
    PROVIDE(__bssstart = .);

    *(.lower.bss.* .lower.bss)
    *(.dynbss);
    *(.sbss .sbss.*);
    *(.bss .bss.* .gnu.linkonce.b.*);
    *(.either.bss.* .either.bss)
    . = ALIGN(2);
    *(COMMON);
    . = ALIGN(2);

    PROVIDE(__bss_end = .);
    PROVIDE(__bssend = .);
  } > ram

  .noinit (NOLOAD):
  {
    . = ALIGN(2);
    *(.noinit)
    . = ALIGN(2);
    PROVIDE(__noinit_end = .);
    end = .;
  } > ram

  .MP430.attributes 0 :
  {
    KEEP (*(.MSP430.attributes))
    KEEP (*(.gnu.attributes))
    KEEP (*(__TI_build_attributes))
  }

  /* The rest are all not normally part of the runtime image.  */

  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end ) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /DISCARD/ : { *(.note.GNU-stack) }

PROVIDE(__data_start_rom  = _etext);
PROVIDE(__data_end_rom    = _etext + SIZEOF(.data));
PROVIDE(__romdatastart    = __data_start_rom);
PROVIDE(__romdatacopysize = SIZEOF(.data));
PROVIDE(__bsssize         = SIZEOF(.bss));

}
