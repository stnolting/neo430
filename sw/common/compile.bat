@REM #################################################################################################
@REM #  < NEO430 application compilation script - Windows version >                                  #
@REM # ********************************************************************************************* #
@REM # Edit the BIN_PATH variable to point to your compiler's binary folder!                         #
@REM # ********************************************************************************************* #
@REM # This file is part of the NEO430 Processor project: https://github.com/stnolting/neo430        #
@REM # Copyright by Stephan Nolting: stnolting@gmail.com                                             #
@REM #                                                                                               #
@REM # This source file may be used and distributed without restriction provided that this copyright #
@REM # statement is not removed from the file and that any derivative work contains the original     #
@REM # copyright notice and the associated disclaimer.                                               #
@REM #                                                                                               #
@REM # This source file is free software; you can redistribute it and/or modify it under the terms   #
@REM # of the GNU Lesser General Public License as published by the Free Software Foundation,        #
@REM # either version 3 of the License, or (at your option) any later version.                       #
@REM #                                                                                               #
@REM # This source is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;      #
@REM # without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     #
@REM # See the GNU Lesser General Public License for more details.                                   #
@REM #                                                                                               #
@REM # You should have received a copy of the GNU Lesser General Public License along with this      #
@REM # source; if not, download it from https://www.gnu.org/licenses/lgpl-3.0.en.html                #
@REM # ********************************************************************************************* #
@REM #  Stephan Nolting, Hannover, Germany                                               20.05.2017  #
@REM #################################################################################################


@REM ----------------------USER CONFIGURATION----------------------
@REM Path of compiler binaries:
@set BIN_PATH=C:\msp430-gcc-6.2.1.16_win32\bin

@REM Compiler effort (-Os = optimize for size)
@set EFFORT=-Os
@REM --------------------------------------------------------------



@REM This script is still compatible with the old MSPGCC.
@REM Use MSPGCC only if you know what you are doing!
@REM MSPGCC is obsolete!
@set USE_TIMSP430_GCC=true
@set MULTIPLIER=false

@REM MAC/MUL option (OBSOLETE!!!)
@if %MULTIPLIER%==true (
  @if %USE_TIMSP430_GCC%==true (
    @echo WARNING! NEO430 hardware multiplier not supported by msp430-elf-gcc!
    @set MCU=msp430f133
  ) else (
    @set MCU=msp430f233
  )
) else (
  @set MCU=msp430f133
)

@REM Compiler tools (obsolete!!!)
@if %USE_TIMSP430_GCC%==false (
  @set AS=%BIN_PATH%\msp430-as
  @set CC=%BIN_PATH%\msp430-gcc
  @set LD=%BIN_PATH%\msp430-ld
  @set OBJDUMP=%BIN_PATH%\msp430-objdump
  @set OBJCOPY=%BIN_PATH%\msp430-objcopy
  @set SIZE=%BIN_PATH%\msp430-size
) else (
  @set AS=%BIN_PATH%\msp430-elf-as
  @set CC=%BIN_PATH%\msp430-elf-gcc
  @set LD=%BIN_PATH%\msp430-elf-ld
  @set OBJDUMP=%BIN_PATH%\msp430-elf-objdump
  @set OBJCOPY=%BIN_PATH%\msp430-elf-objcopy
  @set SIZE=%BIN_PATH%\msp430-elf-size
)
@set IMAGE_GEN=..\tools\image_gen\image_gen

@REM Compiler flags
@set CC_OPTS= -nostartfiles -pipe -fwhole-program -fdata-sections -ffunction-sections -Xlinker --gc-sections -Wl,-static -Wall
@if %USE_TIMSP430_GCC%==true (
  @set CC_OPTS=%CC_OPTS% -minrt -Xassembler --mY -mhwmult=none
)

@REM Assemble start-up code
@%AS% -mmcu=msp430f133 crt0.asm -mY -o crt0.elf

@REM Compile
@%CC% %CC_OPTS% %EFFORT% -mmcu=%MCU% %1 -T neo430_linker_script.x -o main.elf -lm

@REM Print memory utilization
@echo Memory utilization:
@%SIZE% main.elf

@REM Extract final image section
@%OBJCOPY% -I elf32-little main.elf -j .text   -O binary text.bin
@%OBJCOPY% -I elf32-little main.elf -j .rodata -O binary rodata.bin
@%OBJCOPY% -I elf32-little main.elf -j .data   -O binary data.bin
@copy /B /Y text.bin+rodata.bin+data.bin image.bin > NUL

@REM Generate ASM listing file
@%OBJDUMP% -D -S -z main.elf > main.s

@REM Generate NEO430 executable image file for bootloader upload
@%IMAGE_GEN% -app_bin image.bin main.bin

@REM Generate NEO430 VHDL memory initialization image file
@%IMAGE_GEN% -app_img image.bin neo430_application_image.vhd

@REM Install it to the rtl memory init image folder
@echo Installing application image to rtl\core\neo430_application_image.vhd
@copy neo430_application_image.vhd ..\..\rtl\core\. > nul

@REM Check if "DADD" is used
@echo off
@find /I /C "dadd " main.s > NUL
@echo on
@if %errorlevel% equ 0 echo NEO430: WARNING! 'DADD' instruction might be used! Make sure it is synthesized!

@REM Clean up
@del crt0.elf text.bin rodata.bin data.bin image.bin neo430_application_image.vhd
