// #################################################################################################
// #  < neo430.h - MAIN NEO430 INCLUDE FILE >                                                      #
// # ********************************************************************************************* #
// #  This file is crucial for all NEO430 software projects!                                       #
// #  You only need to include this file into your project code (all sub-libraries are included    #
// #  in this library file).                                                                       #
// # ********************************************************************************************* #
// # This file is part of the NEO430 Processor project: http://opencores.org/project,neo430        #
// # Copyright 2015-2016, Stephan Nolting: stnolting@gmail.com                                     #
// #                                                                                               #
// # This source file may be used and distributed without restriction provided that this copyright #
// # statement is not removed from the file and that any derivative work contains the original     #
// # copyright notice and the associated disclaimer.                                               #
// #                                                                                               #
// # This source file is free software; you can redistribute it and/or modify it under the terms   #
// # of the GNU Lesser General Public License as published by the Free Software Foundation,        #
// # either version 3 of the License, or (at your option) any later version.                       #
// #                                                                                               #
// # This source is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;      #
// # without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     #
// # See the GNU Lesser General Public License for more details.                                   #
// #                                                                                               #
// # You should have received a copy of the GNU Lesser General Public License along with this      #
// # source; if not, download it from http://www.gnu.org/licenses/lgpl-3.0.en.html                 #
// # ********************************************************************************************* #
// #  Stephan Nolting, Hannover, Germany                                               27.12.2016  #
// #################################################################################################

#ifndef neo430_h
#define neo430_h

// ----------------------------------------------------------------------------
// CPU Status Register (r2) Flags
// ----------------------------------------------------------------------------
#define C_FLAG_C 0  // carry
#define Z_FLAG_C 1  // zero
#define N_FLAG_C 2  // negative
#define I_FLAG_C 3  // global interrupt enable
#define S_FLAG_C 4  // sleep
#define V_FLAG_C 8  // overflow
#define R_FLAG_C 15 // allow write-access to IMEM


// ----------------------------------------------------------------------------
// Processor peripheral/IO devices
// Beginning of IO area: 0xFF80, size of IO area: 128 Bytes
// ----------------------------------------------------------------------------
#define REG16 (volatile uint16_t*)

/* --- Multiplier --- */
// OBSOLETE! //

/* --- WB32 --- */
#define WB32_LAR (*(REG16 0xFF90)) // -/w: low address for read transfer (+trigger)
#define WB32_LAW (*(REG16 0xFF92)) // -/w: low address for write transfer (+trigger)
#define WB32_HA  (*(REG16 0xFF94)) // -/w: high address
#define WB32_LDO (*(REG16 0xFF96)) // -/w: low data output
#define WB32_HDO (*(REG16 0xFF98)) // -/w: high data output
#define WB32_LDI (*(REG16 0xFF9A)) // r/-; low data input
#define WB32_HDI (*(REG16 0xFF9C)) // r/-; high data input
#define WB32_CT  (*(REG16 0xFF9E)) // r/w: control register

// WB32 control register
#define WB32_CT_WBSEL0_C   0 // r/w: wishbone data byte enable bit 0
#define WB32_CT_WBSEL1_C   1 // r/w: wishbone data byte enable bit 1
#define WB32_CT_WBSEL2_C   2 // r/w: wishbone data byte enable bit 2
#define WB32_CT_WBSEL3_C   3 // r/w: wishbone data byte enable bit 3
#define WB32_CT_TO_EN_C   11 // r/w: enable timeout auto abort
#define WB32_CT_EN_C      12 // r/w: activate wishbone interface adapter when '1'
#define WB32_CT_PMODE_C   13 // r/w: 0: standard transfers, 1: pipelined transfers
#define WB32_CT_TIMEOUT_C 14 // r/-: timeout during bus access
#define WB32_CT_PENDING_C 15 // r/-: pending transfer


/* --- USART/USI --- */
#define USI_SPIRTX  (*(REG16 0xFFA0)) // r/w: spi receive/transmit register
#define USI_UARTRTX (*(REG16 0xFFA2)) // r/w: uart receive/transmit register
#define USI_BAUD    (*(REG16 0xFFA4)) // r/w: uart baud rate generator value
#define USI_CT      (*(REG16 0xFFA6)) // r/w: control register

// UART USI_BAUD[7:0]:  baud rate value
// UART USI_BAUD[10:8]: baud prescaler select:
// 0: CLK/2
// 1: CLK/4
// 2: CLK/8
// 3: CLK/64
// 4: CLK/128
// 5: CLK/1024
// 6: CLK/2048
// 7: CLK/4096

// USART UART RTX register
#define USI_UARTRTX_UARTRXAVAIL_C 15 // r/-: uart receiver data available

// USART control register
#define USI_CT_EN_C         0 // r/w: USART enable
#define USI_CT_UARTRXIRQ_C  1 // r/w: uart rx done interrupt enable
#define USI_CT_UARTTXIRQ_C  2 // r/w: uart tx done interrupt enable
#define USI_CT_UARTTXBSY_C  3 // r/-: uart transmitter is busy
#define USI_CT_SPICPHA_C    4 // r/w: spi clock phase (idle polarity = '0')
#define USI_CT_SPIIRQ_C     5 // r/w: spi transmission done interrupt enable
#define USI_CT_SPIBSY_C     6 // r/-: spi transceiver is busy
#define USI_CT_SPIPRSC0_C   7 // r/w: spi prescaler select bit 0
#define USI_CT_SPIPRSC1_C   8 // r/w: spi prescaler select bit 1
#define USI_CT_SPIPRSC2_C   9 // r/w: spi prescaler select bit 2
#define USI_CT_SPICS0_C    10 // r/w: spi direct CS 0
#define USI_CT_SPICS1_C    11 // r/w: spi direct CS 1
#define USI_CT_SPICS2_C    12 // r/w: spi direct CS 2
#define USI_CT_SPICS3_C    13 // r/w: spi direct CS 3
#define USI_CT_SPICS4_C    14 // r/w: spi direct CS 4
#define USI_CT_SPICS5_C    15 // r/w: spi direct CS 5

// SPI clock prescaler select:
// 0: CLK/2
// 1: CLK/4
// 2: CLK/8
// 3: CLK/64
// 4: CLK/128
// 5: CLK/1024
// 6: CLK/2048
// 7: CLK/4096


/* --- Parallel IO --- */
#define PIO_IN    (*(REG16 0xFFB0)) // r/-: parallel input
#define PIO_OUT   (*(REG16 0xFFB2)) // r/w: parallel output
#define PIO_CTRL  (*(REG16 0xFFB4)) // -/w: control register

// bits 1:0 of PIO CTRL reg: Trigger
// 00: low level
// 01: high level
// 10: falling edge
// 11: rising edge
// bit 2: General interrupt enable flag


/* --- High-Precision Timer --- */
#define TMR_CNT   (*(REG16 0xFFC0)) // r/w: counter register
#define TMR_THRES (*(REG16 0xFFC2)) // r/w: threshold register
#define TMR_CT    (*(REG16 0xFFC4)) // r/w: control register

// Timer control register
#define TMR_CT_EN_C    0 // r/w: timer enable
#define TMR_CT_ARST_C  1 // r/w: auto reset on match
#define TMR_CT_IRQ_C   2 // r/w: interrupt enable
#define TMR_CT_PRSC0_C 3 // r/w: prescaler select bit 0
#define TMR_CT_PRSC1_C 4 // r/w: prescaler select bit 1
#define TMR_CT_PRSC2_C 5 // r/w: prescaler select bit 2

// timer clock prescaler select:
// 0: CLK/2
// 1: CLK/4
// 2: CLK/8
// 3: CLK/64
// 4: CLK/128
// 5: CLK/1024
// 6: CLK/2048
// 7: CLK/4096


/* --- Watchdog Timer --- */
#define WDT_CTRL (*(REG16 0xFFD0)) // r/w: Watchdog control register

// Watchdog control register
#define WDT_PASSWORD_C 0x47
#define WDT_CLKSEL0_C  0 // r/w: prescaler select bit 0
#define WDT_CLKSEL1_C  1 // r/w: prescaler select bit 1
#define WDT_CLKSEL2_C  2 // r/w: prescaler select bit 2
#define WDT_ENABLE_C   3 // r/w: WDT enable
#define WDT_RCAUSE_C   4 // r/-: reset cause (0: external, 1: watchdog timeout)
#define WDT_SYSRST_C   5 // -/w: set one to generate system hardware reset

// watchdog clock prescaler select:
// 0: CLK/2
// 1: CLK/4
// 2: CLK/8
// 3: CLK/64
// 4: CLK/128
// 5: CLK/1024
// 6: CLK/2048
// 7: CLK/4096


/* --- SysConfig --- */
#define CPUID0 (*(REG16 0xFFE0)) // r/-: HW version
#define CPUID1 (*(REG16 0xFFE2)) // r/-: system configuration
#define CPUID2 (*(REG16 0xFFE4)) // r/-: CPU identifier
#define CPUID3 (*(REG16 0xFFE6)) // r/-: IMEM/ROM size in bytes
#define CPUID4 (*(REG16 0xFFE8)) // r/-: DMEM/RAM base address
#define CPUID5 (*(REG16 0xFFEA)) // r/-: DMEM/RAM size in bytes
#define CPUID6 (*(REG16 0xFFEC)) // r/-: clock speed lo
#define CPUID7 (*(REG16 0xFFEE)) // r/-: clock speed hi

// General purpose registers & Interrupt vectors
#define GPREG0       (*(REG16 0xFFF0)) // r/w: gp register 0
#define GPREG1       (*(REG16 0xFFF2)) // r/w: gp register 1
#define GPREG2       (*(REG16 0xFFF4)) // r/w: gp register 2
#define GPREG3       (*(REG16 0xFFF6)) // r/w: gp register 3
#define IRQVEC_TIMER (*(REG16 0xFFF8)) // r/w: timer match
#define IRQVEC_USART (*(REG16 0xFFFA)) // r/w: uart rx avail / uart tx done / spi rtx done
#define IRQVEC_PIO   (*(REG16 0xFFFC)) // r/w: pio pin change
#define IRQVEC_EXT   (*(REG16 0xFFFE)) // r/w: external IRQ

// aliases
#define HW_VERSION    CPUID0 // HW verison number
#define SYS_FEATURES  CPUID1 // synthesized system features
#define CPU_ORIGIN    CPUID2 // CPU identifier
#define IMEM_SIZE     CPUID3 // IMEM/ROM size in bytes
#define DMEM_BASE     CPUID4 // DMEM/RAM base address
#define DMEM_SIZE     CPUID5 // DMEM/RAM size in bytes
#define CLOCKSPEED_LO CPUID6 // clock speed (in Hz) low part
#define CLOCKSPEED_HI CPUID7 // clock speed (in Hz) high part

// SYS features
//#define SYS_MAC16_EN_C  0 // MAC16 - OBSOLETE
#define SYS_WB32_EN_C   1 // WB32 synthesized
#define SYS_WDT_EN_C    2 // WDT synthesized
#define SYS_PIO_EN_C    3 // PIO synthesized
#define SYS_TIMER_EN_C  4 // timer synthesized
#define SYS_USART_EN_C  5 // USART synthesized
#define SYS_DADD_EN_C   6 // DADD instruction synthesized
#define SYS_BTLD_EN_C   7 // Bootloader installed?
#define SYS_IROM_EN_C   8 // Implement IMEM as true ROM?


// ----------------------------------------------------------------------------
// Include IO libraries
// ----------------------------------------------------------------------------
#include "neo430_cpu.h"
#include "neo430_pio.h"
#include "neo430_usart.h"
#include "neo430_wishbone.h"


#endif // neo430_h
