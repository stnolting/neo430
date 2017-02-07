; #################################################################################################
; #  < crt0.asm - general neo430 application start-up code >                                      #
; # ********************************************************************************************* #
; # This file is part of the NEO430 Processor project: http://opencores.org/project,neo430        #
; # Copyright 2015-2016, Stephan Nolting: stnolting@gmail.com                                     #
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
; # source; if not, download it from http://www.gnu.org/licenses/lgpl-3.0.en.html                 #
; # ********************************************************************************************* #
; #  Stephan Nolting, Hannover, Germany                                               27.11.2016  #
; #################################################################################################

  .file	"crt0.asm"
  .section .text
  .p2align 1,0

__crt0_begin:
    nop

; -----------------------------------------------------------
; Get required system info
; -----------------------------------------------------------
    mov  &0xFFE8, r8 ; = DMEM (RAM) base address
    mov  &0xFFEA, r1 ; = DMEM (RAM) size in byte


; -----------------------------------------------------------
; Minimal required hardware setup
; -----------------------------------------------------------
    clr  r2               ; clear status register & disable interrupts
    add  r8, r1           ; r1 = stack pointer = end of RAM
    mov  #0x4700, &0xFFD0 ; deactivate watchdog


; -----------------------------------------------------------
; Additional hardware setup
; -----------------------------------------------------------
    mov  #0, &0xFF9E ; clear WB control register
    mov  #0, &0xFFA6 ; clear USI control register
    mov  #0, &0xFFB4 ; clear PIO control register
    mov  #0, &0xFFB2 ; clear PIO.OUT register
    mov  #0, &0xFFC4 ; clear TIMER control register


; -----------------------------------------------------------
; Set all interrupt vectors to 0x0000 (reset)
; -----------------------------------------------------------
    mov  #0, &0xFFF8 ; set timer match IRQ
    mov  #0, &0xFFFA ; set uart rx avail / uart tx done / spi rtx done IRQ
    mov  #0, &0xFFFC ; set pio pin change IRQ
    mov  #0, &0xFFFE ; set external IRQ


; -----------------------------------------------------------
; Clear complete DMEM (including .bss section)
; -----------------------------------------------------------
__crt0_clr_dmem:
      cmp  r8, r1    ; base address equal to end of RAM?
      jeq  __crt0_clr_dmem_end
      mov  #0, 0(r8) ; clear entry
      incd r8
      jmp  __crt0_clr_dmem
__crt0_clr_dmem_end:


; -----------------------------------------------------------
; Copy .data section from ROM to RAM
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
; Clear all CPU data registers
; -----------------------------------------------------------
    clr  r4
    clr  r5
    clr  r6
    clr  r7
    clr  r8
    clr  r9
    clr  r10
    clr  r11
    clr  r12 ; set argc = 0
    clr  r13
    clr  r14
    clr  r15


; -----------------------------------------------------------
; This is where the actual application is started
; -----------------------------------------------------------
    call  #main


; -----------------------------------------------------------
; Go to endless sleep mode if main returns
; -----------------------------------------------------------
__crt0_this_is_the_end:
    mov  #0x4700, &0xFFD0 ; deactivate watchdog
    mov  #0, r2 ; deativate IRQ
    mov  #16, r2 ; set CPU to sleep mode
    nop

.Lfe0:
    .size	__crt0_begin,.Lfe0-__crt0_begin
