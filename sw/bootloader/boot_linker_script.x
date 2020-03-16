/* ################################################################################################# */
/* # < NEO430 MSP430GCC bootloader linker script >                                                 # */
/* # ********************************************************************************************* # */
/* # BSD 3-Clause License                                                                          # */
/* #                                                                                               # */
/* # Copyright (c) 2020, Stephan Nolting. All rights reserved.                                     # */
/* #                                                                                               # */
/* # Redistribution and use in source and binary forms, with or without modification, are          # */
/* # permitted provided that the following conditions are met:                                     # */
/* #                                                                                               # */
/* # 1. Redistributions of source code must retain the above copyright notice, this list of        # */
/* #    conditions and the following disclaimer.                                                   # */
/* #                                                                                               # */
/* # 2. Redistributions in binary form must reproduce the above copyright notice, this list of     # */
/* #    conditions and the following disclaimer in the documentation and/or other materials        # */
/* #    provided with the distribution.                                                            # */
/* #                                                                                               # */
/* # 3. Neither the name of the copyright holder nor the names of its contributors may be used to  # */
/* #    endorse or promote products derived from this software without specific prior written      # */
/* #    permission.                                                                                # */
/* #                                                                                               # */
/* # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   # */
/* # OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               # */
/* # MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    # */
/* # COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     # */
/* # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE # */
/* # GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    # */
/* # AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     # */
/* # NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  # */
/* # OF THE POSSIBILITY OF SUCH DAMAGE.                                                            # */
/* # ********************************************************************************************* # */
/* # The NEO430 Processor - https://github.com/stnolting/neo430                                    # */
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
