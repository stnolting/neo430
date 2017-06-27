@REM #################################################################################################
@REM #  < NEO430 make file - Windows version >                                                       #
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
@REM #  Stephan Nolting, Hannover, Germany                                               23.09.2016  #
@REM #################################################################################################


@REM Path to common files
@set COMMON_PATH=..\..\common

@REM Path of this project
@set PROJECT_PATH=%~dp0

@REM Delete old project files
@del main.bin main.s main.vhd

@REM Check if MAIN source file is specified
@REM set "main.c" if not defined
@if [%1]==[] (
  @set MAIN=main.c
) else (
  @set MAIN=%1
)

@REM Compile project's main.c and get main.bin (executable),
@REM main.s (listing file) and VHDL memory initialzation file
@cd %COMMON_PATH%
@call compile.bat %PROJECT_PATH%\%MAIN%
@move main.bin %PROJECT_PATH%\. > nul
@move main.s %PROJECT_PATH%\. > nul
@cd %PROJECT_PATH%


@REM Display file size
@for %%A in (main.bin) do @set fileSize=%%~zA
@echo Final executable size (bytes):
@echo %fileSize%
