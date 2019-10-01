onerror {resume}
quietly WaveActivateNextPane {} 0

add wave -noupdate -divider Global
add wave -noupdate /neo430_tb/neo430_top_inst/clk_i
add wave -noupdate /neo430_tb/neo430_top_inst/clk_gen
add wave -noupdate /neo430_tb/neo430_top_inst/rst_i
add wave -noupdate /neo430_tb/neo430_top_inst/rst_gen
add wave -noupdate /neo430_tb/neo430_top_inst/rst_gen_sync
add wave -noupdate /neo430_tb/neo430_top_inst/ext_rst
add wave -noupdate /neo430_tb/neo430_top_inst/sys_rst

add wave -noupdate -divider CPUControl
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_control_inst/ir
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_control_inst/irq_i
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_control_inst/irq_buf
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_control_inst/irq_ack
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_control_inst/irq_start
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_control_inst/irq_run
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_control_inst/irq_ret
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_control_inst/am
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_control_inst/state
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_control_inst/ctrl_o

add wave -noupdate -divider CPU_Regfile
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_reg_file_inst/sreg
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_reg_file_inst/reg_file

add wave -noupdate -divider CPU_ALU
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_alu_inst/op_a_ff
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_alu_inst/op_b_ff
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_alu_inst/alu_res
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_addr_gen_inst/mem_addr_reg
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_addr_gen_inst/mem_addr_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_addr_gen_inst/irq_sel_i
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/neo430_addr_gen_inst/imm_i

add wave -noupdate -divider Bus_Interface
add wave -noupdate /neo430_tb/neo430_top_inst/io_acc
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/mem_rd_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/mem_wr_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/mem_addr_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/mem_data_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_cpu_inst/mem_data_i

add wave -noupdate -divider Main_Memory
add wave -noupdate /neo430_tb/neo430_top_inst/imem_up_en
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_imem_inst/imem_init_file
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_imem_inst/imem_file_h
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_imem_inst/imem_file_l
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_dmem_inst/dmem_file

add wave -noupdate -divider Bootloader_ROM
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_boot_rom_inst_true/neo430_boot_rom_inst/boot_img

add wave -noupdate -divider IO_UART
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_usart_inst_true/neo430_uart_inst/baud
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_usart_inst_true/neo430_uart_inst/ctrl
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_usart_inst_true/neo430_uart_inst/uart_tx_sreg
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_usart_inst_true/neo430_uart_inst/uart_tx_busy
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_usart_inst_true/neo430_uart_inst/uart_txd_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_usart_inst_true/neo430_uart_inst/uart_rx_busy
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_usart_inst_true/neo430_uart_inst/uart_rx_reg
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_usart_inst_true/neo430_uart_inst/uart_rx_avail
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_usart_inst_true/neo430_uart_inst/uart_rxd_i
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_usart_inst_true/neo430_uart_inst/usart_irq_o

add wave -noupdate -divider IO_SPI
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_usart_inst_true/neo430_spi_inst/spi_busy
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_usart_inst_true/neo430_spi_inst/spi_sclk_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_usart_inst_true/neo430_spi_inst/spi_mosi_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_usart_inst_true/neo430_spi_inst/spi_miso_i
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_usart_inst_true/neo430_spi_inst/spi_cs_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_usart_inst_true/neo430_spi_inst/spi_rtx_sreg

add wave -noupdate -divider IO_Timer
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_timer_inst_true/neo430_timer_inst/tctrl
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_timer_inst_true/neo430_timer_inst/thres
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_timer_inst_true/neo430_timer_inst/tcnt
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_timer_inst_true/neo430_timer_inst/irq_o

add wave -noupdate -divider IO_GPIO
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_gpio_inst_true/neo430_gpio_inst/trigger
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_gpio_inst_true/neo430_gpio_inst/gpio_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_gpio_inst_true/neo430_gpio_inst/gpio_i
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_gpio_inst_true/neo430_gpio_inst/irq_o

add wave -noupdate IO_Watchdog
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wdt_inst_true/neo430_wdt_inst/wren
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wdt_inst_true/neo430_wdt_inst/enable
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wdt_inst_true/neo430_wdt_inst/clk_sel
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wdt_inst_true/neo430_wdt_inst/cnt
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wdt_inst_true/neo430_wdt_inst/rst_gen
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wdt_inst_true/neo430_wdt_inst/rst_sync
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wdt_inst_true/neo430_wdt_inst/rst_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wdt_inst_true/neo430_wdt_inst/source

add wave -noupdate -divider IO_Wishbone
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wb32_if_inst_true/neo430_wb32_inst/enable
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wb32_if_inst_true/neo430_wb32_inst/pending
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wb32_if_inst_true/neo430_wb32_inst/pipelined
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wb32_if_inst_true/neo430_wb32_inst/terr
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wb32_if_inst_true/neo430_wb32_inst/wb_adr_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wb32_if_inst_true/neo430_wb32_inst/wb_dat_i
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wb32_if_inst_true/neo430_wb32_inst/wb_dat_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wb32_if_inst_true/neo430_wb32_inst/wb_we_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wb32_if_inst_true/neo430_wb32_inst/wb_sel_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wb32_if_inst_true/neo430_wb32_inst/wb_stb_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wb32_if_inst_true/neo430_wb32_inst/wb_cyc_o
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_wb32_if_inst_true/neo430_wb32_inst/wb_ack_i

add wave -noupdate -divider IO_Sysconfig
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_sysconfig_inst/sysinfo_mem
add wave -noupdate /neo430_tb/neo430_top_inst/neo430_sysconfig_inst/sysconfig_mem


TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {92 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 150
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 1
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ps
update
WaveRestoreZoom {0 ps} {1 ns}
