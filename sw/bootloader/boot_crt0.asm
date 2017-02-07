; #################################################################################################
; #  < boot_crt0.asm - neo430 bootloader start-up code >                                          #
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
; #  Stephan Nolting, Hannover, Germany                                               26.09.2016  #
; #################################################################################################

    .file	"boot_crt0.asm"
    .section .text
    .p2align 1,0

__boot_crt0:
    nop ; for internal init of CPU

; -----------------------------------------------------------
; Minimal required hardware setup
; -----------------------------------------------------------
    clr  r2 ; clear status register & disable interrupts
    
    mov  &0xFFE8, r1 ; DMEM (RAM) base address
    mov  &0xFFEA, r5 ; DMEM (RAM) size in byte
    add  r5, r1

    mov  #0x4700, &0xFFD0 ; deactivate watchdog


; -----------------------------------------------------------
; This is where the actual application is started
; -----------------------------------------------------------
    jmp  main

.Lfe0:
    .size	__boot_crt0,.Lfe0-__boot_crt0
