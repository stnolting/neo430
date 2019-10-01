puts {NEO430 Modelsim Simulation Script}

-- set up sources --
set sim_src ".."
set rtl_src "../../rtl/core"

quit -sim

-- compile sources --
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_package.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_addr_gen.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_alu.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_application_image.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_bootloader_image.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_boot_rom.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_cfu.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_control.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_cpu.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_crc.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_dmem.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_gpio.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_imem.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_muldiv.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_pwm.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_reg_file.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_sysconfig.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_timer.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_uart.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_spi.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_wb_interface.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_wdt.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_twi.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${rtl_src}/neo430_top.vhd
vcom -work neo430 -2002 -explicit -nocasestaticerror ${sim_src}/neo430_tb.vhd

-- start simulation --
vsim -t 1ps neo430.neo430_tb

-- waveform configuration --
do ${sim_src}/modelsim/wave.do

-- run --
run 200 us

