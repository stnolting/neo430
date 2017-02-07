About: The NEO430 example programs
by Stephan Nolting, 25th of November, 2016

=========================================================================
Adding new projects:
=========================================================================
Copy an existing example project (e.g. "blink_led") into this folder and start from there. ;)
This will ensure, that all path depencies will be correct.


=========================================================================
Compiling an application:
=========================================================================
All example projects feature batch make scripts (make.bat) for using Windows and Linux-style
makefiles (Makefile) for using Cygwin/Linux.

For Windows:
Execute "make.bat" inside an example program folder to generate the target files.
The compile script uses "main.c" as default main source file.
Execute "make.bat <your_main_file>" to set your specific main source file.

For Linux/Cygwin:
Execute "make" inside an example program folder to generate the target files.
The compile script uses "main.c" as default main source file.
Execute "make MAIN=<your_main_file>" to set your specific main source file.


=========================================================================
Generated target files:
=========================================================================
 main.s   : Assembler listing file (for debugging).
 main.bin : Binary executable (including EXE header), meant for uploading via the bootloader (serial console).

 Additionally, a main.elf object file is created in the sw\common\ folder for further objecft file analysis.

Also, the IMEM initialization image "application_image.txt" in the rtl\init_images folder is generated.
This image is automatically loaded into the processor's IMEM during VHDL synthesis. This allows to start
the non-volatile application directly from the internal ROM (if you are not using the bootloader to install the image).
