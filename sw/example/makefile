#################################################################################################
# < NEO430 Application Compile Script - Linux/Cygwin/WinBash version >                          #
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
# Stephan Nolting, Hannover, Germany                                                 13.11.2019 #
#################################################################################################


#-------------------------------------------------------------------------------
# Make defaults and targets
#-------------------------------------------------------------------------------
.SUFFIXES:
.DEFAULT_GOAL := help

TOPTARGETS := compile clean_all

SUBDIRS := $(wildcard */.)
# ignore dummy folders (starting with '~')
DUMMYDIRS := $(wildcard ~*/.)
SUBDIRS := $(filter-out $(DUMMYDIRS), $(SUBDIRS))

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	@$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)


#-------------------------------------------------------------------------------
# Help
#-------------------------------------------------------------------------------
help:
	@echo "Compile / clean up ALL project folders in this directory"
	@echo "Targets:"
	@echo " help      - show this text"
	@echo " compile   - compile all projects"
	@echo " clean_all - clean up everything"

  
#-------------------------------------------------------------------------------
# eof
