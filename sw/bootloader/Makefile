#################################################################################################
#  < NEO430 bootloader (!) compilation script - Linux version >                                 #
# ********************************************************************************************* #
# Edit the BIN_PATH variable to point to your MSPGCC\bin folder.                                #
# ********************************************************************************************* #
# This file is part of the NEO430 Processor project: http://opencores.org/project,neo430        #
# Copyright 2015-2016, Stephan Nolting: stnolting@gmail.com                                     #
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
# source; if not, download it from http://www.gnu.org/licenses/lgpl-3.0.en.html                 #
# ********************************************************************************************* #
#  Stephan Nolting, Hannover, Germany                                               13.01.2017  #
#################################################################################################


#-------------------------------------------------------------------------------
# USER CONFIGURATION
#-------------------------------------------------------------------------------
# Path to MSPGCC binaries:
BIN_PATH = C:/msp430-gcc/bin

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
AS        = $(BIN_PATH)/msp430-elf-as
CC        = $(BIN_PATH)/msp430-elf-gcc
LD        = $(BIN_PATH)/msp430-elf-ld
OBJDUMP   = $(BIN_PATH)/msp430-elf-objdump
OBJCOPY   = $(BIN_PATH)/msp430-elf-objcopy
SIZE      = $(BIN_PATH)/msp430-elf-size
IMAGE_GEN = $(TOOL_PATH)/image_gen/image_gen

# Remove -g to exclude debugging information
CC_OPTS  = -nostartfiles -pipe -fwhole-program -fdata-sections -ffunction-sections -Xlinker --gc-sections -Wl,-static -s -Wall
CC_OPTS += -lm -Os -mmcu=msp430f133 -T boot_linker_script.x -minrt -Xassembler --mY -mhwmult=none

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
	@$(AS) -g -mY -mmcu=msp430 $< -o $@

# Compile app sources
main.elf: bootloader.c crt0.elf
	@echo NEO430 BOOTLOADER compilation script
	@$(CC) $(CC_OPTS) $< -o $@
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
	@echo Image size \(bytes\):
	@stat -c "%s" bootloader.bin
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
