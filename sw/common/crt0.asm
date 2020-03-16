; #################################################################################################
; #  < crt0.asm - general neo430 application start-up code >                                      #
; # ********************************************************************************************* #
; # BSD 3-Clause License                                                                          #
; #                                                                                               #
; # Copyright (c) 2020, Stephan Nolting. All rights reserved.                                     #
; #                                                                                               #
; # Redistribution and use in source and binary forms, with or without modification, are          #
; # permitted provided that the following conditions are met:                                     #
; #                                                                                               #
; # 1. Redistributions of source code must retain the above copyright notice, this list of        #
; #    conditions and the following disclaimer.                                                   #
; #                                                                                               #
; # 2. Redistributions in binary form must reproduce the above copyright notice, this list of     #
; #    conditions and the following disclaimer in the documentation and/or other materials        #
; #    provided with the distribution.                                                            #
; #                                                                                               #
; # 3. Neither the name of the copyright holder nor the names of its contributors may be used to  #
; #    endorse or promote products derived from this software without specific prior written      #
; #    permission.                                                                                #
; #                                                                                               #
; # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   #
; # OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               #
; # MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    #
; # COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     #
; # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE #
; # GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    #
; # AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     #
; # NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  #
; # OF THE POSSIBILITY OF SUCH DAMAGE.                                                            #
; # ********************************************************************************************* #
; # The NEO430 Processor - https://github.com/stnolting/neo430                                    #
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
