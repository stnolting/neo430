#################################################################################################
#  < NEO430 make file - Linux version >                                                         #
# ********************************************************************************************* #
# Application's relative path from the common folder and application's main file are passed as  #
# arguments. Use "make MAIN=<file>" to define other main file. "make" or "make all" will build  #
# the default main "make clean" to clean up generated files.                                    #
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
#  Stephan Nolting, Hannover, Germany                                               23.09.2016  #
#################################################################################################


#-------------------------------------------------------------------------------
# Make defaults
#-------------------------------------------------------------------------------
.SUFFIXES:
.PHONY: all
.DEFAULT_GOAL = all


#-------------------------------------------------------------------------------
# Definitions
#-------------------------------------------------------------------------------
# default main file
ifeq ($(MAIN),)
  MAIN = main.c
endif

# relative path TO common foler
COMMON_PATH	= ../../common

# relative path from common folder to current folder
mkfile_path = $(abspath $(lastword $(MAKEFILE_LIST)))
curr_folder = $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))
APP_PATH = ../example/$(curr_folder)


#-------------------------------------------------------------------------------
# Main target
#-------------------------------------------------------------------------------
all:
	@echo $(WARNING)
	@make -r -s -C $(COMMON_PATH) APP_DIR=$(APP_PATH) MAIN=$(MAIN)
	@echo Final executable size '(bytes)':
	@stat -c "%s" main.bin


#-------------------------------------------------------------------------------
# Clean up
#-------------------------------------------------------------------------------
clean:
	@rm -f main.bin main.s
	@make -r -s -C $(COMMON_PATH) clean


#-------------------------------------------------------------------------------
# eof
