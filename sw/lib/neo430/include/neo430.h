// #################################################################################################
// #  < neo430.h - MAIN NEO430 INCLUDE FILE >                                                      #
// # ********************************************************************************************* #
// # This file is crucial for all NEO430 software projects!                                        #
// # You only need to include THIS file into your project code (all sub-libraries are included     #
// # within this library file).                                                                    #
// # ********************************************************************************************* #
// # This file is part of the NEO430 Processor project: https://github.com/stnolting/neo430        #
// # Copyright by Stephan Nolting: stnolting@gmail.com                                             #
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
// # source; if not, download it from https://www.gnu.org/licenses/lgpl-3.0.en.html                #
// # ********************************************************************************************* #
// # Stephan Nolting, Hannover, Germany                                                 10.12.2019 #
// #################################################################################################

#ifndef neo430_h
#define neo430_h

// Standard libraries
#include <stdint.h>
#include <stdlib.h>

// ----------------------------------------------------------------------------
// CPU Status Register (r2) Flags
// ----------------------------------------------------------------------------
#define C_FLAG 0  // r/w: carry
#define Z_FLAG 1  // r/w: zero
#define N_FLAG 2  // r/w: negative
#define I_FLAG 3  // r/w: global interrupt enable
#define S_FLAG 4  // r/w: sleep
#define P_FLAG 5  // r/w: parity (if enabled for synthesis)
#define V_FLAG 8  // r/w: overflow
#define Q_FLAG 14 // -/w: clear pending IRQ buffer when set
#define R_FLAG 15 // r/w: allow write-access to IMEM


// ----------------------------------------------------------------------------
// Processor peripheral/IO devices
// Beginning of IO area: 0xFF80
// Size of IO area: 128 bytes
// ----------------------------------------------------------------------------
#define REG8  (volatile uint8_t*)        // memory-mapped register
#define REG16 (volatile uint16_t*)       // memory-mapped register
#define REG32 (volatile uint32_t*)       // memory-mapped register
#define ROM8  (const volatile uint8_t*)  // memory-mapped read-only constant
#define ROM16 (const volatile uint16_t*) // memory-mapped read-only constant
#define ROM32 (const volatile uint32_t*) // memory-mapped read-only constant


// ----------------------------------------------------------------------------
// Start of memory sections
// ----------------------------------------------------------------------------
#define IMEM_ADDR_BASE 0x0000 // r/(w)/x: instruction memory
#define DMEM_ADDR_BASE 0xC000 // r/w/x:   data memory
#define BTLD_ADDR_BASE 0xF000 // r/-/x:   bootloader memory


// ----------------------------------------------------------------------------
// Interrupt vectors, located at the beginning of DMEM
// ----------------------------------------------------------------------------
#define IRQVEC_TIMER  (*(REG16 (DMEM_ADDR_BASE + 0))) // r/w: timer match
#define IRQVEC_SERIAL (*(REG16 (DMEM_ADDR_BASE + 2))) // r/w: uart/spi/twi irqs
#define IRQVEC_GPIO   (*(REG16 (DMEM_ADDR_BASE + 4))) // r/w: gpio pin change
#define IRQVEC_EXT    (*(REG16 (DMEM_ADDR_BASE + 6))) // r/w: external IRQ


// ----------------------------------------------------------------------------
// Unsigned Multiplier/Divider Unit (MULDIV)
// ----------------------------------------------------------------------------
#define MULDIV_OPA     (*(REG16 0xFF80)) // -/w: operand A (dividend or factor1)
#define MULDIV_OPB_DIV (*(REG16 0xFF82)) // -/w: operand B (divisor) for division
#define MULDIV_OPB_MUL (*(REG16 0xFF84)) // -/w: operand B (factor2) for multiplication
//#define reserved     (*(REG16 0xFF86))
//#define reserved     (*(REG16 0xFF88))
//#define reserved     (*(REG16 0xFF8A))
#define MULDIV_RESX    (*(ROM16 0xFF8C)) // r/-: quotient or product low word
#define MULDIV_RESY    (*(ROM16 0xFF8E)) // r/-: remainder or product high word
#define MULDIV_R32bit  (*(ROM32 (&MULDIV_RESX))) // r/-: read result as 32-bit data word


// ----------------------------------------------------------------------------
// Wishbone Bus Adapter (WB32)
// ----------------------------------------------------------------------------
#define WB32_CT  (*(REG16 0xFF90)) // r/w: control register
#define WB32_LRA (*(REG16 0xFF92)) // -/w: low address for read transfer
#define WB32_HRA (*(REG16 0xFF94)) // -/w: high address for read transfer (+trigger)
#define WB32_LWA (*(REG16 0xFF96)) // -/w: low address for write transfer
#define WB32_HWA (*(REG16 0xFF98)) // -/w: high address for write transfer (+trigger)
#define WB32_LD  (*(REG16 0xFF9A)) // r/w: low data
#define WB32_HD  (*(REG16 0xFF9C)) // r/w: high data
//#define reserved (*(REG16 0xFF9E)) // -/-: reserved

// WB32 - 32-bit register access
#define WB32_RA_32bit (*(REG32 (&WB32_LRA))) // -/w: address for read transfer (+trigger)
#define WB32_WA_32bit (*(REG32 (&WB32_LWA))) // -/w: address for write transfer (+trigger)
#define WB32_D_32bit  (*(REG32 (&WB32_LD)))  // r/w: read/write data (for 32-bit access)

// WB32 control register
#define WB32_CT_WBSEL0   0 // -/w: wishbone data byte enable bit 0
#define WB32_CT_WBSEL1   1 // -/w: wishbone data byte enable bit 1
#define WB32_CT_WBSEL2   2 // -/w: wishbone data byte enable bit 2
#define WB32_CT_WBSEL3   3 // -/w: wishbone data byte enable bit 3
#define WB32_CT_PENDING 15 // r/-: pending transfer


// ----------------------------------------------------------------------------
// Universal Asynchronous Receiver and Transmitter (UART)
// ----------------------------------------------------------------------------
#define UART_CT  (*(REG16 0xFFA0)) // r/w: control register
#define UART_RTX (*(REG16 0xFFA2)) // r/w: receive/transmit register

// UART control register
#define UART_CT_BAUD0     0 // r/w: baud config bit 0
#define UART_CT_BAUD1     1 // r/w: baud config bit 1
#define UART_CT_BAUD2     2 // r/w: baud config bit 2
#define UART_CT_BAUD3     3 // r/w: baud config bit 3
#define UART_CT_BAUD4     4 // r/w: baud config bit 4
#define UART_CT_BAUD5     5 // r/w: baud config bit 5
#define UART_CT_BAUD6     6 // r/w: baud config bit 6
#define UART_CT_BAUD7     7 // r/w: baud config bit 7
#define UART_CT_PRSC0     8 // r/w: baud presclaer bit 0
#define UART_CT_PRSC1     9 // r/w: baud presclaer bit 1
#define UART_CT_PRSC2    10 // r/w: baud presclaer bit 2

#define UART_CT_EN       12 // r/w: UART enable
#define UART_CT_RX_IRQ   13 // r/w: Rx done interrupt enable
#define UART_CT_TX_IRQ   14 // r/w: Tx done interrupt enable
#define UART_CT_TX_BUSY  15 // r/-: transmitter busy

// UART RTX register flags
#define UART_RTX_AVAIL 15 // r/-: uart receiver data available

// clock prescalers 
#define UART_PRSC_2    0 // CLK/2
#define UART_PRSC_4    1 // CLK/4
#define UART_PRSC_8    2 // CLK/8
#define UART_PRSC_64   3 // CLK/64
#define UART_PRSC_128  4 // CLK/128
#define UART_PRSC_1024 5 // CLK/1024
#define UART_PRSC_2048 6 // CLK/2048
#define UART_PRSC_4096 7 // CLK/4096


// ----------------------------------------------------------------------------
// Serial Peripheral Interface (SPI)
// ----------------------------------------------------------------------------
#define SPI_CT  (*(REG16 0xFFA4)) // r/w: control register
#define SPI_RTX (*(REG16 0xFFA6)) // r/w: receive/transmit register

// SPI control register
#define SPI_CT_EN       0 // r/w: spi enable
#define SPI_CT_CPHA     1 // r/w: spi clock phase (idle polarity = '0')
#define SPI_CT_IRQ      2 // r/w: spi transmission done interrupt enable
#define SPI_CT_PRSC0    3 // r/w: spi clock prescaler select bit 0
#define SPI_CT_PRSC1    4 // r/w: spi clock prescaler select bit 1
#define SPI_CT_PRSC2    5 // r/w: spi clock prescaler select bit 2
#define SPI_CT_CS_SEL0  6 // r/w: spi CS select 0
#define SPI_CT_CS_SEL1  7 // r/w: spi CS select 1
#define SPI_CT_CS_SEL2  8 // r/w: spi CS select 2
#define SPI_CT_CS_SET   9 // r/w: selected CS becomes active ('0') when set
#define SPI_CT_DIR     10 // r/w: shift direction (0: MSB first, 1: LSB first)

#define SPI_CT_BUSY    15 // r/-: spi transceiver is busy

// clock prescalers 
#define SPI_PRSC_2    0 // CLK/2
#define SPI_PRSC_4    1 // CLK/4
#define SPI_PRSC_8    2 // CLK/8
#define SPI_PRSC_64   3 // CLK/64
#define SPI_PRSC_128  4 // CLK/128
#define SPI_PRSC_1024 5 // CLK/1024
#define SPI_PRSC_2048 6 // CLK/2048
#define SPI_PRSC_4096 7 // CLK/4096


// ----------------------------------------------------------------------------
// General Purpose Inputs/Outputs (GPIO)
// ----------------------------------------------------------------------------
//#define reserved   (*(REG16 0xFFA8)) // reserved
#define GPIO_IRQMASK (*(REG16 0xFFAA)) // -/w: irq mask register
#define GPIO_INPUT   (*(ROM16 0xFFAC)) // r/-: parallel input
#define GPIO_OUTPUT  (*(REG16 0xFFAE)) // r/w: parallel output


// ----------------------------------------------------------------------------
// High-Precision Timer (TIMER)
// ----------------------------------------------------------------------------
#define TMR_CT    (*(REG16 0xFFB0)) // r/w: control register
#define TMR_CNT   (*(ROM16 0xFFB2)) // r/-: counter register
#define TMR_THRES (*(REG16 0xFFB4)) // r/w: threshold register
//#define reserved     (*(REG16 0xFFB6))

// Timer control register
#define TMR_CT_EN    0 // r/w: timer enable
#define TMR_CT_ARST  1 // r/w: auto reset on match
#define TMR_CT_IRQ   2 // r/w: interrupt enable
#define TMR_CT_RUN   3 // r/w: start/stop timer
#define TMR_CT_PRSC0 4 // r/w: clock prescaler select bit 0
#define TMR_CT_PRSC1 5 // r/w: clock prescaler select bit 1
#define TMR_CT_PRSC2 6 // r/w: clock prescaler select bit 2

// Timer clock prescaler select:
#define TMR_PRSC_2    0 // CLK/2
#define TMR_PRSC_4    1 // CLK/4
#define TMR_PRSC_8    2 // CLK/8
#define TMR_PRSC_64   3 // CLK/64
#define TMR_PRSC_128  4 // CLK/128
#define TMR_PRSC_1024 5 // CLK/1024
#define TMR_PRSC_2048 6 // CLK/2048
#define TMR_PRSC_4096 7 // CLK/4096


// ----------------------------------------------------------------------------
// Watchdog Timer (WTD)
// ----------------------------------------------------------------------------
#define WDT_CT (*(REG16 0xFFB8)) // r/w: Watchdog control register

// Watchdog control register
#define WDT_CT_PASSWORD 0x47 // must be set in the upper 8 bits of the WDT CTRL register
#define WDT_CT_PRSC0    0 // r/w: clock prescaler select bit 0
#define WDT_CT_PRSC1    1 // r/w: clock prescaler select bit 1
#define WDT_CT_PRSC2    2 // r/w: clock prescaler select bit 2
#define WDT_CT_EN       3 // r/w: WDT enable
#define WDT_CT_RCAUSE   4 // r/-: reset cause (0: external, 1: watchdog timeout)
#define WDT_CT_RPWFAIL  5 // r/-: watchdog resed caused by wrong WDT access password

// Watchdog clock prescaler select:
#define WDT_PRSC_2    0 // CLK/2
#define WDT_PRSC_4    1 // CLK/4
#define WDT_PRSC_8    2 // CLK/8
#define WDT_PRSC_64   3 // CLK/64
#define WDT_PRSC_128  4 // CLK/128
#define WDT_PRSC_1024 5 // CLK/1024
#define WDT_PRSC_2048 6 // CLK/2048
#define WDT_PRSC_4096 7 // CLK/4096


// ----------------------------------------------------------------------------
// Cyclic Redundancy Check (CRC16/32)
// ----------------------------------------------------------------------------
#define CRC_POLY_LO (*(REG16 0xFFC0)) // -/w: low part of polynomial
#define CRC_POLY_HI (*(REG16 0xFFC2)) // -/w: high part of polynomial
#define CRC_CRC16IN (*(REG16 0xFFC4)) // -/w: input for CRC16
#define CRC_CRC32IN (*(REG16 0xFFC6)) // -/w: input for CRC32
//#define ???       (*(REG16 0xFFC8)) // -/-: reserved
//#define ???       (*(REG16 0xFFCA)) // -/-: reserved
#define CRC_RESX    (*(REG16 0xFFCC)) // r/w: crc shift register low
#define CRC_RESY    (*(REG16 0xFFCE)) // r/w: crc shift register high

#define CRC_POLY32bit (*(REG32 (&CRC_POLY_LO))) // -/w: write polynomial as 32-bit data word
#define CRC_R32bit    (*(REG32 (&CRC_RESX)))    // r/w: crc shift register as 32-bit data word


// ----------------------------------------------------------------------------
// Custom Functions Unit (CFU)
// ----------------------------------------------------------------------------
#define CFU_REG0 (*(REG16 0xFFD0)) // r/w: user defined...
#define CFU_REG1 (*(REG16 0xFFD2)) // r/w: user defined...
#define CFU_REG2 (*(REG16 0xFFD4)) // r/w: user defined...
#define CFU_REG3 (*(REG16 0xFFD6)) // r/w: user defined...
#define CFU_REG4 (*(REG16 0xFFD8)) // r/w: user defined...
#define CFU_REG5 (*(REG16 0xFFDA)) // r/w: user defined...
#define CFU_REG6 (*(REG16 0xFFDC)) // r/w: user defined...
#define CFU_REG7 (*(REG16 0xFFDE)) // r/w: user defined...


// ----------------------------------------------------------------------------
// Pulse Width Modulation Controller (PWM)
// ----------------------------------------------------------------------------
#define PWM_CT   (*(REG16 0xFFE0)) // r/w: control register
#define PWM_CH10 (*(REG16 0xFFE2)) // r/w: duty cycle channel 1 and 0
#define PWM_CH32 (*(REG16 0xFFE4)) // -/w: duty cycle channel 3 and 2

// PWM controller control register
#define PWM_CT_EN       0 // -/w: PWM enable
#define PWM_CT_PRSC0    1 // -/w: clock prescaler select bit 0
#define PWM_CT_PRSC1    2 // -/w: clock prescaler select bit 1
#define PWM_CT_PRSC2    3 // -/w: clock prescaler select bit 2
#define PWM_CT_GPIO_PWM 4 // -/w: use channel 3 for PWM modulation of GPIO unit's output port
#define PWM_CT_SIZE_SEL 5 // -/w: cnt size select (0 = 4-bit, 1 = 8-bit)

// PWM clock prescaler select:
#define PWM_PRSC_2    0 // CLK/2
#define PWM_PRSC_4    1 // CLK/4
#define PWM_PRSC_8    2 // CLK/8
#define PWM_PRSC_64   3 // CLK/64
#define PWM_PRSC_128  4 // CLK/128
#define PWM_PRSC_1024 5 // CLK/1024
#define PWM_PRSC_2048 6 // CLK/2048
#define PWM_PRSC_4096 7 // CLK/4096


// ----------------------------------------------------------------------------
// Two Wire Serial Interface (TWI)
// ----------------------------------------------------------------------------
#define TWI_CT   (*(REG16 0xFFE8)) // r/w: control register
#define TWI_DATA (*(REG16 0xFFEA)) // r/w: RX (r) / TX (w) data

// TWI control register
#define TWI_CT_EN       0 // r/w: TWI enable
#define TWI_CT_START    1 // -/w: generate START condition
#define TWI_CT_STOP     2 // -/w: generate STOP condition
#define TWI_CT_BUSY     3 // r/-: TWI busy
#define TWI_CT_PRSC0    4 // r/w: clock prescaler select bit 0
#define TWI_CT_PRSC1    5 // r/w: clock prescaler select bit 1
#define TWI_CT_PRSC2    6 // r/w: clock prescaler select bit 2
#define TWI_CT_IRQ_EN   7 // r/w: transmission done interrupt enable

// TWI clock prescaler select:
#define TWI_PRSC_2    0 // CLK/2
#define TWI_PRSC_4    1 // CLK/4
#define TWI_PRSC_8    2 // CLK/8
#define TWI_PRSC_64   3 // CLK/64
#define TWI_PRSC_128  4 // CLK/128
#define TWI_PRSC_1024 5 // CLK/1024
#define TWI_PRSC_2048 6 // CLK/2048
#define TWI_PRSC_4096 7 // CLK/4096

// TWI data register flags
#define TWI_DT_ACK    15 // r/-: ACK received


// ----------------------------------------------------------------------------
// True Random Number Generator (TRNG)
// ----------------------------------------------------------------------------
#define TRNG_CT (*(REG16 0xFFEC)) // r/w: control register

// TRNG control register
#define TRNG_CT_RND0  0 // r/-: TRNG random data byte bit 0
#define TRNG_CT_RND1  1 // r/-: TRNG random data byte bit 1
#define TRNG_CT_RND2  2 // r/-: TRNG random data byte bit 2
#define TRNG_CT_RND3  3 // r/-: TRNG random data byte bit 3
#define TRNG_CT_RND4  4 // r/-: TRNG random data byte bit 4
#define TRNG_CT_RND5  5 // r/-: TRNG random data byte bit 5
#define TRNG_CT_RND6  6 // r/-: TRNG random data byte bit 6
#define TRNG_CT_RND7  7 // r/-: TRNG random data byte bit 7
#define TRNG_CT_EN   15 // r/w: TRNG enable


// ----------------------------------------------------------------------------
// External Interrupts Controller (EXIRQ)
// ----------------------------------------------------------------------------
#define EXIRQ_CT (*(REG16 0xFFEE)) // r/w: control register

// EXIRQ control register
#define EXIRQ_CT_SRC0         0 // r/-: IRQ source bit 0
#define EXIRQ_CT_SRC1         1 // r/-: IRQ source bit 1
#define EXIRQ_CT_SRC2         2 // r/-: IRQ source bit 2
#define EXIRQ_CT_EN           3 // r/w: unit enable
#define EXIRQ_CT_SW_IRQ       4 // -/w: enable SW IRQ trigger, auto-clears
#define EXIRQ_CT_SW_IRQ_SEL0  5 // -/w: SW IRQ select bit 0, read as zero
#define EXIRQ_CT_SW_IRQ_SEL1  6 // -/w: SW IRQ select bit 1, read as zero
#define EXIRQ_CT_SW_IRQ_SEL2  7 // -/w: SW IRQ select bit 2, read as zero
#define EXIRQ_CT_IRQ0_EN      8 // r/w: Enable IRQ channel 0
#define EXIRQ_CT_IRQ1_EN      9 // r/w: Enable IRQ channel 1
#define EXIRQ_CT_IRQ2_EN     10 // r/w: Enable IRQ channel 2
#define EXIRQ_CT_IRQ3_EN     11 // r/w: Enable IRQ channel 3
#define EXIRQ_CT_IRQ4_EN     12 // r/w: Enable IRQ channel 4
#define EXIRQ_CT_IRQ5_EN     13 // r/w: Enable IRQ channel 5
#define EXIRQ_CT_IRQ6_EN     14 // r/w: Enable IRQ channel 6
#define EXIRQ_CT_IRQ7_EN     15 // r/w: Enable IRQ channel 7


// ----------------------------------------------------------------------------
// System Configuration (SYSCONFIG)
// ----------------------------------------------------------------------------
#define CPUID0 (*(ROM16 0xFFF0)) // r/-: HW version number
#define CPUID1 (*(ROM16 0xFFF2)) // r/-: synthesized system features
#define CPUID2 (*(ROM16 0xFFF4)) // r/-: custom user code
#define CPUID3 (*(ROM16 0xFFF6)) // r/-: IMEM/ROM size in bytes
#define CPUID4 (*(ROM16 0xFFF8)) // r/-: reserved
#define CPUID5 (*(ROM16 0xFFFA)) // r/-: DMEM/RAM size in bytes
#define CPUID6 (*(ROM16 0xFFFC)) // r/-: clock speed (in Hz) low part
#define CPUID7 (*(ROM16 0xFFFE)) // r/-: clock speed (in Hz) high part

// Aliases
#define HW_VERSION    CPUID0 // r/-: HW version number
#define SYS_FEATURES  CPUID1 // r/-: synthesized system features
#define USER_CODE     CPUID2 // r/-: custom user code
#define IMEM_SIZE     CPUID3 // r/-: IMEM/ROM size in bytes
//#define             CPUID4 // r/-: reserved
#define DMEM_SIZE     CPUID5 // r/-: DMEM/RAM size in bytes
#define CLOCKSPEED_LO CPUID6 // r/-: clock speed (in Hz) low part
#define CLOCKSPEED_HI CPUID7 // r/-: clock speed (in Hz) high part

// SysConfig - 32-bit register access
#define CLOCKSPEED_32bit (*(ROM32 (&CLOCKSPEED_LO))) // r/-: clock speed (in Hz)

// SYS features
#define SYS_MULDIV_EN 0 // r/-: MULDIV synthesized
#define SYS_WB32_EN   1 // r/-: WB32 synthesized
#define SYS_WDT_EN    2 // r/-: WDT synthesized
#define SYS_GPIO_EN   3 // r/-: GPIO synthesized
#define SYS_TIMER_EN  4 // r/-: TIMER synthesized
#define SYS_UART_EN   5 // r/-: UART synthesized
#define SYS_DADD_EN   6 // r/-: DADD instruction synthesized
#define SYS_BTLD_EN   7 // r/-: Bootloader installed and enabled
#define SYS_IROM_EN   8 // r/-: Implement IMEM as true ROM
#define SYS_CRC_EN    9 // r/-: CRC synthesized
#define SYS_CFU_EN   10 // r/-: CFU synthesized
#define SYS_PWM_EN   11 // r/-: PWM controller synthesized
#define SYS_TWI_EN   12 // r/-: TWI synthesized
#define SYS_SPI_EN   13 // r/-: SPI synthesized
#define SYS_TRNG_EN  14 // r/-: TRNG synthesized
#define SYS_EXIRQ_EN 15 // r/-: EXIRQ synthesized


// ----------------------------------------------------------------------------
// Include all IO library headers
// ----------------------------------------------------------------------------
#include "neo430_cpu.h"
#include "neo430_crc.h"
#include "neo430_exirq.h"
#include "neo430_gpio.h"
#include "neo430_muldiv.h"
#include "neo430_pwm.h"
#include "neo430_spi.h"
#include "neo430_timer.h"
#include "neo430_trng.h"
#include "neo430_twi.h"
#include "neo430_uart.h"
#include "neo430_wdt.h"
#include "neo430_wishbone.h"


#endif // neo430_h
