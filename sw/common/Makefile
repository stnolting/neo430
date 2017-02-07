#################################################################################################
#  < NEO430 Application Compile Script - Linux version >                                        #
# ********************************************************************************************* #
# Edit the BIN_PATH variable to point to your compiler's binary folder!                         #
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
# Path to compiler binaries:
BIN_PATH = C:/msp430-gcc/bin

# Compiler effort (-Os = optimize for size)
EFFORT = -Os


#-------------------------------------------------------------------------------
# Make defaults
#-------------------------------------------------------------------------------
.SUFFIXES:
.PHONY: all
.DEFAULT_GOAL := all

#-------------------------------------------------------------------------------
# Application definitions
#-------------------------------------------------------------------------------
APP_SRC = $(APP_DIR)/$(MAIN)
APP_BIN = $(APP_DIR)/main.bin
APP_ASM = $(APP_DIR)/main.s

all: $(APP_ASM) $(APP_BIN) neo430_application_image.vhd clean

#-------------------------------------------------------------------------------
# Definitions
#-------------------------------------------------------------------------------
#This script is still compatible with the old MSPGCC.
#Use MSPGCC only if you know what you are doing!
#MSPGCC is obsolete!
USE_TIMSP430_GCC = true
MULTIPLIER = false

# MAC/MUL option (OBSOLETE!!!)
ifeq ($(MULTIPLIER),true)
ifeq ($(USE_TIMSP430_GCC),true)
	@echo WARNING! NEO430 hardware multiplier not supported by msp430-elf-gcc!
  MCU = msp430f133
else
  MCU = msp430f233
endif
else
  MCU = msp430f133
endif

#-------------------------------------------------------------------------------
# Tools
#-------------------------------------------------------------------------------
#C ompiler tools (obsolete!!!)
ifeq ($(USE_TIMSP430_GCC),false)
  AS        = $(BIN_PATH)/msp430-as
  CC        = $(BIN_PATH)/msp430-gcc
  LD        = $(BIN_PATH)/msp430-ld
  OBJDUMP   = $(BIN_PATH)/msp430-objdump
  OBJCOPY   = $(BIN_PATH)/msp430-objcopy
  SIZE      = $(BIN_PATH)/msp430-size
else
  AS        = $(BIN_PATH)/msp430-elf-as
  CC        = $(BIN_PATH)/msp430-elf-gcc
  LD        = $(BIN_PATH)/msp430-elf-ld
  OBJDUMP   = $(BIN_PATH)/msp430-elf-objdump
  OBJCOPY   = $(BIN_PATH)/msp430-elf-objcopy
  SIZE      = $(BIN_PATH)/msp430-elf-size
endif
IMAGE_GEN = $(TOOL_PATH)/image_gen/image_gen

# Compiler flags
CC_OPTS = -pipe -nostartfiles -fwhole-program -fdata-sections -ffunction-sections -Xlinker --gc-sections -Wl,-static -s -Wall
ifeq ($(USE_TIMSP430_GCC),true)
  CC_OPTS += -minrt -Xassembler --mY -mhwmult=none
endif

#-------------------------------------------------------------------------------
# Paths
#-------------------------------------------------------------------------------
# Path to NEO430 image generator:
TOOL_PATH   = ../tools
# PAth to NEO430 common SW folder:
COMMON_PATH = ../common

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
	@echo Compiling $@
	@$(CC_X86) $< -o $@

#-------------------------------------------------------------------------------
# Application Targets
#-------------------------------------------------------------------------------
# Assemble startup code
crt0.elf: $(COMMON_PATH)/crt0.asm
	@$(AS) -g -mY -mmcu=$(MCU) $< -o $@

# Compile app sources
main.elf: $(APP_SRC) crt0.elf
	@$(CC) $(CC_OPTS) $(EFFORT) -mmcu=$(MCU) $< -T neo430_linker_script.x -o $@ -lm
	@echo Memory utilization:
	@$(SIZE) main.elf

# Generate final executable (from .image section only)
image.bin: main.elf
	@$(OBJCOPY) -I elf32-little $< -j .text   -O binary text.bin
	@$(OBJCOPY) -I elf32-little $< -j .rodata -O binary rodata.bin
	@$(OBJCOPY) -I elf32-little $< -j .data   -O binary data.bin
	@cat text.bin rodata.bin data.bin > $@

# Assembly listing file
$(APP_ASM): main.elf
	@$(OBJDUMP) -D -S -z  $< > $@

# Generate NEO430 executable image for bootloader update
$(APP_BIN): image.bin $(IMAGE_GEN)
	@$(IMAGE_GEN) -app_bin $< $@

# Generate NEO430 executable VHDL boot image
neo430_application_image.vhd: image.bin $(IMAGE_GEN)
	@$(IMAGE_GEN) -app_img $< $@
	@echo Installing application image to rtl/core/neo430_application_image.vhd
	@cp neo430_application_image.vhd ../../rtl/core/.

#-------------------------------------------------------------------------------
# Clean up
#-------------------------------------------------------------------------------
.PHONY: clean
clean:
	@rm -f crt0.elf *.bin *.vhd

#-------------------------------------------------------------------------------
# eof
