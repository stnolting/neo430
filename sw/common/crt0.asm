; #################################################################################################
; #  < crt0.asm - general neo430 application start-up code >                                      #
; # ********************************************************************************************* #
; # This file is part of the NEO430 Processor project: https://github.com/stnolting/neo430        #
; # Copyright by Stephan Nolting: stnolting@gmail.com                                             #
; #                                                                                               #
; # This source file may be used and distributed without restriction provided that this copyright #
; # statement is not removed from the file and that any derivative work contains the original     #
; # copyright notice and the associated disclaimer.                                               #
; #                                                                                               #
; # This source file is free software; you can redistribute it and/or modify it under the terms   #
; # of the GNU Lesser General Public License as published by the Free Software Foundation,        #
; # either version 3 of the License, or (at your option) any later version.                       #
; #                                                                                               #
; # This source is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;      #
; # without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     #
; # See the GNU Lesser General Public License for more details.                                   #
; #                                                                                               #
; # You should have received a copy of the GNU Lesser General Public License along with this      #
; # source; if not, download it from https://www.gnu.org/licenses/lgpl-3.0.en.html                #
; # ********************************************************************************************* #
; # Stephan Nolting, Hannover, Germany                                                 27.11.2019 #
; #################################################################################################

  .file	"crt0.asm"
  .section .text
  .p2align 1,0


__crt0_begin:
; -----------------------------------------------------------
; Get required system info
; -----------------------------------------------------------
    mov  #0xC000, r8 ; = DMEM (RAM) base address
    mov  &0xFFFA, r1 ; = DMEM (RAM) size in byte


; -----------------------------------------------------------
; Minimal required hardware setup
; -----------------------------------------------------------
    mov  #0, r2           ; clear status register & disable interrupts
    add  r8, r1           ; r1 = stack pointer = end of RAM
    sub  #2, r1           ; address of last entry of stack
    mov  #0x4700, &0xFFB8 ; deactivate watchdog


; -----------------------------------------------------------
; Initialize all IO device registers (set to zero)
; -----------------------------------------------------------
; This loop does not trigger any operations as the CTRL registers, which are located
; at offset 0 of the according device, are set to zero resulting in disabling the
; specific device.
    mov  #0xFF80, r9 ; beginning of IO section
__crt0_clr_io:
      tst  r9 ; until the end -> wrap-around to 0
      jeq  __crt0_clr_io_end
      mov  #0, 0(r9) ; clear entry
      incd r9
      jmp  __crt0_clr_io
__crt0_clr_io_end:


; -----------------------------------------------------------
; Clear complete DMEM (including .bss section)
; -----------------------------------------------------------
__crt0_clr_dmem:
      cmp  r8, r1 ; base address equal to end of RAM?
      jeq  __crt0_clr_dmem_end
      mov  #0, 0(r8) ; clear entry
      incd r8
      jmp  __crt0_clr_dmem
__crt0_clr_dmem_end:


; -----------------------------------------------------------
; Copy initialized .data section from ROM to RAM
; -----------------------------------------------------------
    mov  #__data_start_rom, r5
    mov  #__data_end_rom, r6
    mov  #__data_start, r7
__crt0_cpy_data:
      cmp  r5, r6
      jeq  __crt0_cpy_data_end
      mov  @r5+, 0(r7)
      incd r7
      jmp  __crt0_cpy_data
__crt0_cpy_data_end:


; -----------------------------------------------------------
; Re-init SR and clear all pending IRQs from buffer
; -----------------------------------------------------------
    mov  #(1<<14), r2 ; this flag auto clears


; -----------------------------------------------------------
; Initialize all remaining registers
; -----------------------------------------------------------
    mov  #0, r4
;   mov  #0, r5 ; -- is already initialized
;   mov  #0, r6 ; -- is already initialized
;   mov  #0, r7 ; -- is already initialized
;   mov  #0, r8 ; -- is already initialized
;   mov  #0, r9 ; -- is already initialized
    mov  #0, r10
    mov  #0, r11
    mov  #0, r12 ; set argc = 0
    mov  #0, r13
    mov  #0, r14
    mov  #0, r15


; -----------------------------------------------------------
; This is where the actual application is started
; -----------------------------------------------------------
__crt0_start_main:
    call  #main


; -----------------------------------------------------------
; Go to endless sleep mode if main returns
; -----------------------------------------------------------
__crt0_this_is_the_end:
    mov  #0, r2 ; deactivate IRQs
    mov  #0x4700, &0xFFB8 ; deactivate watchdog
    mov  #(1<<4), r2 ; set CPU to sleep mode
    nop

.Lfe0:
    .size	__crt0_begin, .Lfe0-__crt0_begin
