// #################################################################################################
// #  < neo430.h - MAIN NEO430 INCLUDE FILE >                                                      #
// # ********************************************************************************************* #
// # BSD 3-Clause License                                                                          #
// #                                                                                               #
// # Copyright (c) 2020, Stephan Nolting. All rights reserved.                                     #
// #                                                                                               #
// # Redistribution and use in source and binary forms, with or without modification, are          #
// # permitted provided that the following conditions are met:                                     #
// #                                                                                               #
// # 1. Redistributions of source code must retain the above copyright notice, this list of        #
// #    conditions and the following disclaimer.                                                   #
// #                                                                                               #
// # 2. Redistributions in binary form must reproduce the above copyright notice, this list of     #
// #    conditions and the following disclaimer in the documentation and/or other materials        #
// #    provided with the distribution.                                                            #
// #                                                                                               #
// # 3. Neither the name of the copyright holder nor the names of its contributors may be used to  #
// #    endorse or promote products derived from this software without specific prior written      #
// #    permission.                                                                                #
// #                                                                                               #
// # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   #
// # OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               #
// # MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    #
// # COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     #
// # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE #
// # GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    #
// # AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     #
// # NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  #
// # OF THE POSSIBILITY OF SUCH DAMAGE.                                                            #
// # ********************************************************************************************* #
// # The NEO430 Processor - https://github.com/stnolting/neo430                                    #
// #################################################################################################

#ifndef neo430_h
#define neo430_h

// Standard libraries
#include <stdint.h>
#include <stdlib.h>

// ----------------------------------------------------------------------------
// Aux data types
// ----------------------------------------------------------------------------
union uint16_u { uint16_t uint16; uint8_t  uint8[ sizeof(uint16_t)/1]; };
union uint32_u { uint32_t uint32; uint16_t uint16[sizeof(uint32_t)/2]; uint8_t  uint8[ sizeof(uint32_t)/1]; };
union uint64_u { uint64_t uint64; uint32_t uint32[sizeof(uint64_t)/4]; uint16_t uint16[sizeof(uint64_t)/2]; uint8_t uint8[sizeof(uint64_t)/1]; };

union  int16_u { int16_t  int16; int8_t   int8[ sizeof(int16_t)/1]; };
union  int32_u { int32_t  int32; int16_t  int16[sizeof(int32_t)/2]; int8_t  int8[ sizeof(int32_t)/1]; };
union  int64_u { int64_t  int64; int32_t  int32[sizeof(int64_t)/4]; int16_t int16[sizeof(int64_t)/2]; int8_t int8[sizeof(int64_t)/1]; };


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
#define MULDIV_OPA_RESX      (*(REG16 0xFF80)) // r/w: operand A (dividend or factor1) / resx: quotient or product low word
#define MULDIV_OPB_UMUL_RESY (*(REG16 0xFF82)) // r/w: operand B (factor2) for unsigned multiplication / resy: remainder or product high word
#define MULDIV_OPB_SMUL      (*(REG16 0xFF84)) // -/w: operand B (factor2) for signed multiplication
#define MULDIV_OPB_UDIV      (*(REG16 0xFF86)) // -/w: operand B (divisor) for unsigned division
#define MULDIV_R32bit        (*(ROM32 (&MULDIV_OPA_RESX))) // r/-: read result as 32-bit data word


// ----------------------------------------------------------------------------
// Frequency Generator (FREQ_GEN)
// ----------------------------------------------------------------------------
#define FREQ_GEN_CT     (*(REG16 0xFF88)) // r/w: control register
#define FREQ_GEN_TW_CH0 (*(REG16 0xFF8A)) // -/w: tuning word channel 0
#define FREQ_GEN_TW_CH1 (*(REG16 0xFF8C)) // -/w: tuning word channel 1
#define FREQ_GEN_TW_CH2 (*(REG16 0xFF8E)) // -/w: tuning word channel 2

// FREQ_GEN control register
#define FREQ_GEN_CT_CH0_EN     0 // r/w: enable NCO channel 0
#define FREQ_GEN_CT_CH1_EN     1 // r/w: enable NCO channel 1
#define FREQ_GEN_CT_CH2_EN     2 // r/w: enable NCO channel 2
#define FREQ_GEN_CT_CH0_PRSC0  3 // r/w: prescaler select bit 0 for channel 0
#define FREQ_GEN_CT_CH0_PRSC1  4 // r/w: prescaler select bit 1 for channel 0
#define FREQ_GEN_CT_CH0_PRSC2  5 // r/w: prescaler select bit 2 for channel 0
#define FREQ_GEN_CT_CH1_PRSC0  6 // r/w: prescaler select bit 0 for channel 1
#define FREQ_GEN_CT_CH1_PRSC1  7 // r/w: prescaler select bit 1 for channel 1
#define FREQ_GEN_CT_CH1_PRSC2  8 // r/w: prescaler select bit 2 for channel 1
#define FREQ_GEN_CT_CH2_PRSC0  9 // r/w: prescaler select bit 0 for channel 2
#define FREQ_GEN_CT_CH2_PRSC1 10 // r/w: prescaler select bit 1 for channel 2
#define FREQ_GEN_CT_CH2_PRSC2 11 // r/w: prescaler select bit 2 for channel 2

// clock prescalers 
#define FREQ_GEN_PRSC_2    0 // CLK/2
#define FREQ_GEN_PRSC_4    1 // CLK/4
#define FREQ_GEN_PRSC_8    2 // CLK/8
#define FREQ_GEN_PRSC_64   3 // CLK/64
#define FREQ_GEN_PRSC_128  4 // CLK/128
#define FREQ_GEN_PRSC_1024 5 // CLK/1024
#define FREQ_GEN_PRSC_2048 6 // CLK/2048
#define FREQ_GEN_PRSC_4096 7 // CLK/4096


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
#define UART_CT_RXOR     11 // r/-: RX data overrun
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
#define SPI_CT_CS_SEL0  0 // r/w: spi CS 0
#define SPI_CT_CS_SEL1  1 // r/w: spi CS 1
#define SPI_CT_CS_SEL2  2 // r/w: spi CS 2
#define SPI_CT_CS_SEL3  3 // r/w: spi CS 3
#define SPI_CT_CS_SEL4  4 // r/w: spi CS 4
#define SPI_CT_CS_SEL5  5 // r/w: spi CS 5
#define SPI_CT_EN       6 // r/w: spi enable
#define SPI_CT_CPHA     7 // r/w: spi clock phase (idle polarity = '0')
#define SPI_CT_IRQ      8 // r/w: spi transmission done interrupt enable
#define SPI_CT_PRSC0    9 // r/w: spi clock prescaler select bit 0
#define SPI_CT_PRSC1   10 // r/w: spi clock prescaler select bit 1
#define SPI_CT_PRSC2   11 // r/w: spi clock prescaler select bit 2
#define SPI_CT_DIR     12 // r/w: shift direction (0: MSB first, 1: LSB first)
#define SPI_CT_SIZE    13 // r/w: 0 = 8-bit, 1 = 16-bit
// ...
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
#define GPIO_IRQMASK (*(REG16 0xFFA8)) // -/w: irq mask register
#define GPIO_INPUT   (*(ROM16 0xFFAA)) // r/-: parallel input
#define GPIO_OUTPUT  (*(REG16 0xFFAC)) // r/w: parallel output
//#define reserved   (*(REG16 0xFFAE)) // reserved


// ----------------------------------------------------------------------------
// High-Precision Timer (TIMER)
// ----------------------------------------------------------------------------
#define TMR_CT     (*(REG16 0xFFB0)) // r/w: control register
#define TMR_CNT    (*(ROM16 0xFFB2)) // r/-: counter register
#define TMR_THRES  (*(REG16 0xFFB4)) // -/w: threshold register
//#define reserved (*(REG16 0xFFB6)) // reserved

// Timer control register
#define TMR_CT_EN     0 // r/w: timer unit global enable
#define TMR_CT_ARST   1 // r/w: auto reset on match
#define TMR_CT_IRQ    2 // r/w: interrupt enable
#define TMR_CT_RUN    3 // r/w: start/stop timer
#define TMR_CT_PRSC0  4 // r/w: clock prescaler select bit 0
#define TMR_CT_PRSC1  5 // r/w: clock prescaler select bit 1
#define TMR_CT_PRSC2  6 // r/w: clock prescaler select bit 2

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
#define TWI_CT_MACK     8 // r/w: send ack by master after transmission

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
#define TRNG_CT_DATA0     0 // r/-: TRNG random data byte bit 0
#define TRNG_CT_DATA1     1 // r/-: TRNG random data byte bit 1
#define TRNG_CT_DATA2     2 // r/-: TRNG random data byte bit 2
#define TRNG_CT_DATA3     3 // r/-: TRNG random data byte bit 3
#define TRNG_CT_DATA4     4 // r/-: TRNG random data byte bit 4
#define TRNG_CT_DATA5     5 // r/-: TRNG random data byte bit 5
#define TRNG_CT_DATA6     6 // r/-: TRNG random data byte bit 6
#define TRNG_CT_DATA7     7 // r/-: TRNG random data byte bit 7
#define TRNG_CT_DATA8     8 // r/-: TRNG random data byte bit 8
#define TRNG_CT_DATA9     9 // r/-: TRNG random data byte bit 9
#define TRNG_CT_DATA10   10 // r/-: TRNG random data byte bit 10
#define TRNG_CT_DATA11   11 // r/-: TRNG random data byte bit 11
// --
#define TRNG_CT_TAP00_EN  0 // -/w: Activate tap 0 switch
#define TRNG_CT_TAP01_EN  1 // -/w: Activate tap 1 switch
#define TRNG_CT_TAP02_EN  2 // -/w: Activate tap 2 switch
#define TRNG_CT_TAP03_EN  3 // -/w: Activate tap 3 switch
#define TRNG_CT_TAP04_EN  4 // -/w: Activate tap 4 switch
#define TRNG_CT_TAP05_EN  5 // -/w: Activate tap 5 switch
#define TRNG_CT_TAP06_EN  6 // -/w: Activate tap 6 switch
#define TRNG_CT_TAP07_EN  7 // -/w: Activate tap 7 switch
#define TRNG_CT_TAP08_EN  8 // -/w: Activate tap 8 switch
#define TRNG_CT_TAP09_EN  9 // -/w: Activate tap 9 switch
#define TRNG_CT_TAP10_EN 10 // -/w: Activate tap 10 switch
#define TRNG_CT_TAP11_EN 11 // -/w: Activate tap 11 switch
#define TRNG_CT_TAP12_EN 12 // -/w: Activate tap 12 switch
#define TRNG_CT_TAP13_EN 13 // -/w: Activate tap 13 switch
// --
#define TRNG_CT_EN       14 // r/w: TRNG enable
#define TRNG_CT_VALID    15 // r/-: TRNG output byte is valid



// ----------------------------------------------------------------------------
// External Interrupts Controller (EXIRQ)
// ----------------------------------------------------------------------------
#define EXIRQ_CT (*(REG16 0xFFEE)) // r/w: control register

// EXIRQ control register
#define EXIRQ_CT_SEL0         0 // r/w: IRQ source bit 0 / SW_IRQ select
#define EXIRQ_CT_SEL1         1 // r/w: IRQ source bit 1 / SW_IRQ select
#define EXIRQ_CT_SEL2         2 // r/w: IRQ source bit 2 / SW_IRQ select
#define EXIRQ_CT_EN           3 // r/w: unit enable
#define EXIRQ_CT_SW_IRQ       4 // -/w: use irq_sel as SW IRQ trigger, auto-clears
#define EXIRQ_CT_ACK_IRQ      5 // -/w: use irq_sel as ACK select, auto-clears
// ...
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
#define CPUID4 (*(ROM16 0xFFF8)) // r/-: advanced/experimental hardware configuration features
#define CPUID5 (*(ROM16 0xFFFA)) // r/-: DMEM/RAM size in bytes
#define CPUID6 (*(ROM16 0xFFFC)) // r/-: clock speed (in Hz) low part
#define CPUID7 (*(ROM16 0xFFFE)) // r/-: clock speed (in Hz) high part

// Aliases
#define HW_VERSION    CPUID0 // r/-: HW version number
#define SYS_FEATURES  CPUID1 // r/-: synthesized system features
#define USER_CODE     CPUID2 // r/-: custom user code
#define IMEM_SIZE     CPUID3 // r/-: IMEM/ROM size in bytes
#define NX_FEATURES   CPUID4 // r/-: advanced/experimental hardware configuration features
#define DMEM_SIZE     CPUID5 // r/-: DMEM/RAM size in bytes
#define CLOCKSPEED_LO CPUID6 // r/-: clock speed (in Hz) low part
#define CLOCKSPEED_HI CPUID7 // r/-: clock speed (in Hz) high part

// SysConfig - 32-bit register access
#define CLOCKSPEED_32bit (*(ROM32 (&CLOCKSPEED_LO))) // r/-: clock speed (in Hz)

// SYS features
#define SYS_MULDIV_EN    0 // r/-: MULDIV synthesized
#define SYS_WB32_EN      1 // r/-: WB32 synthesized
#define SYS_WDT_EN       2 // r/-: WDT synthesized
#define SYS_GPIO_EN      3 // r/-: GPIO synthesized
#define SYS_TIMER_EN     4 // r/-: TIMER synthesized
#define SYS_UART_EN      5 // r/-: UART synthesized
#define SYS_FREQ_GEN_EN  6 // r/-: FREQ_GEN synthesized
#define SYS_BTLD_EN      7 // r/-: Bootloader installed and enabled
#define SYS_IROM_EN      8 // r/-: Implement IMEM as true ROM
#define SYS_CRC_EN       9 // r/-: CRC synthesized
#define SYS_CFU_EN      10 // r/-: CFU synthesized
#define SYS_PWM_EN      11 // r/-: PWM controller synthesized
#define SYS_TWI_EN      12 // r/-: TWI synthesized
#define SYS_SPI_EN      13 // r/-: SPI synthesized
#define SYS_TRNG_EN     14 // r/-: TRNG synthesized
#define SYS_EXIRQ_EN    15 // r/-: EXIRQ synthesized

// NX features (advanced/experimental features)
#define NX_DSP_MUL_EN   0 // r/-: using DSP-blocks for MULDIV.multiplier
#define NX_XALU_EN      1 // r/-: implement eXtended ALU functions
#define NX_LOWPOWER_EN  2 // r/-: use low-power implementation (experimental!)


// ----------------------------------------------------------------------------
// ~~ EXPERIMENTAL ~~
// ----------------------------------------------------------------------------
#define NEO430_DEVNULL (*(REG16 0xFF00)) // r/w: read: 0, write: no effect


// ----------------------------------------------------------------------------
// Include all IO library headers
// ----------------------------------------------------------------------------
#include "neo430_cpu.h"
#include "neo430_crc.h"
#include "neo430_exirq.h"
#include "neo430_freq_gen.h"
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
