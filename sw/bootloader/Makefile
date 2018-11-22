#################################################################################################
#  < NEO430 bootloader (!) compilation script - Linux version >                                 #
# ********************************************************************************************* #
# Edit the MSP430GCC_BIN_PATH variable to point to your compiler's binary folder.               #
# Alternatively, you can set it in your environment variables.                                  #
# ********************************************************************************************* #
# This file is part of the NEO430 Processor project: https://github.com/stnolting/neo430        #
# Copyright by Stephan Nolting: stnolting@gmail.com                                             #
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
# source; if not, download it from https://www.gnu.org/licenses/lgpl-3.0.en.html                #
# ********************************************************************************************* #
# Stephan Nolting, Hannover, Germany                                                 29.09.2018 #
#################################################################################################


#-------------------------------------------------------------------------------
# USER CONFIGURATION
#-------------------------------------------------------------------------------
# Path of compiler binaries:
MSP430GCC_BIN_PATH ?= /mnt/c/msp430-gcc-7.3.2.154_linux64/bin
# Path to main include path
NEO430_LIB_PATH=../lib/neo430

#-------------------------------------------------------------------------------
# Make defaults
#-------------------------------------------------------------------------------
.SUFFIXES:
.PHONY: all
.DEFAULT_GOAL := all

#-------------------------------------------------------------------------------
# Application definitions
#-------------------------------------------------------------------------------
all: bootloader.s neo430_bootloader_image.vhd
$(warning Making bootloader.c)

#-------------------------------------------------------------------------------
# Tools
#-------------------------------------------------------------------------------
AS        = $(MSP430GCC_BIN_PATH)/msp430-elf-as
CC        = $(MSP430GCC_BIN_PATH)/msp430-elf-gcc
LD        = $(MSP430GCC_BIN_PATH)/msp430-elf-ld
OBJDUMP   = $(MSP430GCC_BIN_PATH)/msp430-elf-objdump
OBJCOPY   = $(MSP430GCC_BIN_PATH)/msp430-elf-objcopy
SIZE      = $(MSP430GCC_BIN_PATH)/msp430-elf-size
IMAGE_GEN = $(TOOL_PATH)/image_gen/image_gen

# Remove -g to exclude debugging information
CC_OPTS  = -nostartfiles -pipe -fwhole-program -fdata-sections -ffunction-sections -Xlinker --gc-sections -Wl,-static -Wall
CC_OPTS += -lm -Os -mcpu=msp430 -T boot_linker_script.x -minrt -Xassembler --mY -mhwmult=none

#-------------------------------------------------------------------------------
# Paths
#-------------------------------------------------------------------------------
# Path to NEO430 image generator:
TOOL_PATH   = ../tools

#-------------------------------------------------------------------------------
# PC Host Compiler
#-------------------------------------------------------------------------------
CC_X86 = g++ -Wall -O -g
DWIN32 = -DWIN32

#-------------------------------------------------------------------------------
# Tool Targets
#-------------------------------------------------------------------------------
# Tools
.PHONY: tools
tools: $(IMAGE_GEN)

$(IMAGE_GEN): $(TOOL_PATH)/image_gen/main.cpp
	@echo Creating $@
	@$(CC_X86) $< -o $@

#-------------------------------------------------------------------------------
# Application Targets
#-------------------------------------------------------------------------------
# Assemble startup code
crt0.elf: boot_crt0.asm
	@$(AS) -g -mY -mcpu=msp430 -I $(NEO430_LIB_PATH) $< -o $@

# Compile app sources
main.elf: bootloader.c crt0.elf
	@echo NEO430 BOOTLOADER compilation script
	@$(CC) $(CC_OPTS) -I $(NEO430_LIB_PATH) $< -o $@
	@echo Memory utilization:
	@$(SIZE) main.elf

# Generate final executable (from .image section only)
bootloader.bin: main.elf
	@$(OBJCOPY) -I elf32-little $< -j .image -O binary $@

# Assembly listing file
bootloader.s: main.elf
	@$(OBJDUMP) -S --disassemble $< > $@

# Converter for VHLD memory init image
neo430_bootloader_image.vhd: bootloader.bin $(IMAGE_GEN)
	@$(IMAGE_GEN) -bld_img $< $@
	@echo Installing bootloader image to rtl/core/neo430_bootloader_image.vhd
	@cp neo430_bootloader_image.vhd ../../rtl/core/.
	@rm -f *.elf *.bin main.s neo430_bootloader_image.vhd


#-------------------------------------------------------------------------------
# Clean up
#-------------------------------------------------------------------------------
.PHONY: clean
clean:
	@rm -f *.elf *.bin *.s neo430_bootloader_image.vhd

#-------------------------------------------------------------------------------
# eof
