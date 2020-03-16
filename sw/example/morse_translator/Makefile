#################################################################################################
#  < NEO430 Application Compile Script - Linux / Windows Powershell / Windows Linux Subsystem > #
# ********************************************************************************************* #
# BSD 3-Clause License                                                                          #
#                                                                                               #
# Copyright (c) 2020, Stephan Nolting. All rights reserved.                                     #
#                                                                                               #
# Redistribution and use in source and binary forms, with or without modification, are          #
# permitted provided that the following conditions are met:                                     #
#                                                                                               #
# 1. Redistributions of source code must retain the above copyright notice, this list of        #
#    conditions and the following disclaimer.                                                   #
#                                                                                               #
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of     #
#    conditions and the following disclaimer in the documentation and/or other materials        #
#    provided with the distribution.                                                            #
#                                                                                               #
# 3. Neither the name of the copyright holder nor the names of its contributors may be used to  #
#    endorse or promote products derived from this software without specific prior written      #
#    permission.                                                                                #
#                                                                                               #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   #
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               #
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    #
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     #
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE #
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    #
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     #
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  #
# OF THE POSSIBILITY OF SUCH DAMAGE.                                                            #
# ********************************************************************************************* #
# The NEO430 Processor - https://github.com/stnolting/neo430                                    #
#################################################################################################


#*******************************************************************************
# USER CONFIGURATION
#*******************************************************************************
# Compiler effort (-Os = optimize for size)
EFFORT = -Os

# User's application sources (add additional files here)
APP_SRC = main.c 

# User's application include folders (don't forget the '-I' before each entry)
APP_INC = -I .

# Relative or absolute path to the NEO430 home folder
NEO430_HOME = ../../..
#*******************************************************************************



#-------------------------------------------------------------------------------
# NEO430 framework
#-------------------------------------------------------------------------------
# Path to NEO430 linker script and startup file
NEO430_COM_PATH=$(NEO430_HOME)/sw/common
# Path to main NEO430 library include files
NEO430_INC_PATH=$(NEO430_HOME)/sw/lib/neo430/include
# Path to main NEO430 library source files
NEO430_SRC_PATH=$(NEO430_HOME)/sw/lib/neo430/source
# Path to NEO430 executable generator
NEO430_EXE_PATH=$(NEO430_HOME)/sw/tools/image_gen
# Path to NEO430 core rtl folder
NEO430_RTL_PATH=$(NEO430_HOME)/rtl/core


#-------------------------------------------------------------------------------
# Add NEO430 sources to input SRCs
#-------------------------------------------------------------------------------
APP_SRC += $(wildcard $(NEO430_SRC_PATH)/*.c)


#-------------------------------------------------------------------------------
# Make defaults
#-------------------------------------------------------------------------------
.SUFFIXES:
.PHONY: all
.DEFAULT_GOAL := help


#-------------------------------------------------------------------------------
# Application output definitions
#-------------------------------------------------------------------------------
APP_BIN = main.bin
APP_ASM = main.s

compile: $(APP_ASM) $(APP_BIN)
install: $(APP_ASM) neo430_application_image.vhd
all:     $(APP_ASM) $(APP_BIN) neo430_application_image.vhd

# define all object files
OBJ = $(APP_SRC:.c=.o)


#-------------------------------------------------------------------------------
# Tools
#-------------------------------------------------------------------------------
#C ompiler tools
AS        = msp430-elf-as
CC        = msp430-elf-gcc
LD        = msp430-elf-ld
STRIP     = msp430-elf-strip
OBJDUMP   = msp430-elf-objdump
OBJCOPY   = msp430-elf-objcopy
SIZE      = msp430-elf-size
IMAGE_GEN = $(NEO430_EXE_PATH)/image_gen

# Compiler flags
CC_OPTS = -mcpu=msp430 -pipe -Wall -Xassembler --mY -mhwmult=none -fno-delete-null-pointer-checks
CC_OPTS += -Wl,-static -mrelax -minrt -nostartfiles -fdata-sections -ffunction-sections -Xlinker --gc-sections
# Add user flags if available
CC_OPTS += ${CC_USER_FLAGS}

# Linker flags
LD_OPTS = -mcpu=msp430 -Wl,--gc-sections -mrelax -minrt -nostartfiles


#-------------------------------------------------------------------------------
# PC Host Compiler
#-------------------------------------------------------------------------------
CC_X86 = g++ -Wall -O -g


#-------------------------------------------------------------------------------
# Tool Targets
#-------------------------------------------------------------------------------
# install/compile tools
$(IMAGE_GEN): $(NEO430_EXE_PATH)/main.cpp
	@echo Compiling $(IMAGE_GEN)
	@$(CC_X86) $< -o $(IMAGE_GEN)

  
#-------------------------------------------------------------------------------
# Application Targets
#-------------------------------------------------------------------------------
# Assemble startup code
crt0.elf: $(NEO430_COM_PATH)/crt0.asm
	@$(AS) -mY -mcpu=msp430 $< -o $@

# Compile app sources
$(OBJ): %.o : %.c crt0.elf
	@$(CC) -c $(CC_OPTS) $(EFFORT) -I $(NEO430_INC_PATH) $(APP_INC) $< -o $@

# Link object files
main.elf: $(OBJ)
	@$(CC) $(LD_OPTS) $(EFFORT) -I $(NEO430_INC_PATH) $(APP_INC) -T $(NEO430_COM_PATH)/neo430_linker_script.x $(OBJ) -o $@ -lm
	@echo Memory utilization:
	@$(SIZE) main.elf

# Generate final executable (from .image section only)
image.dat: main.elf
	@$(OBJCOPY) -I elf32-little $< -j .text   -O binary text.dat
	@$(OBJCOPY) -I elf32-little $< -j .rodata -O binary rodata.dat
	@$(OBJCOPY) -I elf32-little $< -j .data   -O binary data.dat
	@cat text.dat rodata.dat data.dat > $@
	@rm -f text.dat rodata.dat data.dat

# Assembly listing file (for debugging)
$(APP_ASM): main.elf
	@$(OBJDUMP) -D -S -z  $< > $@
	@if grep -qR "dadd" $@; then echo "NEO430: WARNING! 'DADD' instruction might be used!"; fi

# Generate NEO430 executable image for bootloader update
$(APP_BIN): image.dat $(IMAGE_GEN)
	@set -e
	@$(IMAGE_GEN) -app_bin $< $@

# Generate NEO430 executable VHDL boot image
neo430_application_image.vhd: image.dat $(IMAGE_GEN)
	@$(IMAGE_GEN) -app_img $< $@
	@echo Installing application image to $(NEO430_RTL_PATH)/neo430_application_image.vhd
	@cp neo430_application_image.vhd $(NEO430_RTL_PATH)/.
	@rm -f neo430_application_image.vhd


#-------------------------------------------------------------------------------
# Help
#-------------------------------------------------------------------------------
help:
	@echo "NEO430 Application Compilation Script"
	@echo "Make sure to add the msp430-gcc bin folder to your system's PATH variable."
	@echo "Targets:"
	@echo " help      - show this text"
	@echo " compile   - compile and generate *.bin executable for upload via bootloader"
	@echo " install   - compile, generate and install VHDL boot image"
	@echo " all       - compile and generate *.bin executable for upload via bootloader and generate and install VHDL boot image"
	@echo " clean     - clean up project"
	@echo " clean_all - clean up project, core libraries and helper tools"


#-------------------------------------------------------------------------------
# Clean up
#-------------------------------------------------------------------------------
clean:
	@rm -f *.elf *.o *.dat *.vhd *.s *.bin

clean_all:
	@rm -f $(OBJ) *.elf *.dat *.bin *.vhd *.s $(IMAGE_GEN)

  
#-------------------------------------------------------------------------------
# eof
