#!/bin/bash

# Abort if any command returns != 0
set -e

# Project home foler
homedir=/mnt/data

# The directories of the hw source files
srcdir_core=$homedir/rtl/core
srcdir_top_templates=$homedir/rtl/top_templates
srcdir_sim=$homedir/sim

# Show GHDL version
ghdl -v

# List files
ls -al $srcdir_core
ls -al $srcdir_top_templates
ls -al $srcdir_sim

# Analyse sources
ghdl -a --work=neo430 $srcdir_core/neo430_package.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_application_image.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_bootloader_image.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_addr_gen.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_alu.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_boot_rom.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_cfu.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_control.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_cpu.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_crc.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_dmem.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_exirq.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_freq_gen.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_gpio.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_imem.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_muldiv.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_pwm.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_reg_file.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_spi.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_sysconfig.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_timer.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_top.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_trng.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_twi.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_uart.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_wb_interface.vhd
ghdl -a --work=neo430 $srcdir_core/neo430_wdt.vhd

ghdl -a --work=neo430 $srcdir_top_templates/*.vhd $srcdir_sim/*.vhd

# Elaborate top entity
ghdl -e --work=neo430 neo430_top

# Prepare UART tx output log file and run simulation
touch neo430.uart_tx.txt
chmod 777 neo430.uart_tx.txt
ghdl -e --work=neo430 neo430_tb
ghdl -r --work=neo430 neo430_tb --stop-time=20ms --ieee-asserts=disable-at-0 --assert-level=error

# Check output
uart_res_reference="Blinking LED demo program"
echo "Checking UART output. Should be:" $uart_res_reference
echo "UART output is:"
cat neo430.uart_tx.txt
grep -q "$uart_res_reference" neo430.uart_tx.txt
