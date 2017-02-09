// #################################################################################################
// #  < NEO430 Bootloader >                                                                        #
// # ********************************************************************************************* #
// # Boot from IMEM, UART or SPI EEPROM at CS0                                                     #
// #                                                                                               #
// # UART configuration: 8N1 at 19200 baud                                                         #
// # Boot EEPROM: SPI, 16-bit addresses, SPI.CS0, e.g. 25LC512                                     #
// # PIO.out[0] is used as high-active status LED                                                  #
// #                                                                                               #
// # Auto boot sequence after timeout:                                                             #
// #  -> Try booting from SPI EEPROM                                                               #
// #  -> permanently light up status led and freeze if SPI EEPOMR booting attempt fails            #
// # ********************************************************************************************* #
// # This file is part of the NEO430 Processor project: http://opencores.org/project,neo430        #
// # Copyright 2015-2017, Stephan Nolting: stnolting@gmail.com                                     #
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
// #  Stephan Nolting, Hannover, Germany                                               09.02.2017  #
// #################################################################################################

// Libraries
#include <stdint.h>
#include "../lib/neo430/neo430.h"

// Macros
#define xstr(a) str(a)
#define str(a) #a

// Configuration
#define BAUD_RATE_C        19200 // default UART baud rate
#define AUTOBOOT_TIMEOUT_C     8 // countdown (seconds) to auto boot
#define STATUS_LED_C           0 // PIO.0 is status LED

// 25LC512 EEPROM
#define BOOT_EEP_CS_C USI_CT_SPICS0_C // boot EEPROM CS (SPI.CS0)
#define EEP_WRITE_C   0x02 // initialize start of write sequence
#define EEP_READ_C    0x03 // initialize start of read sequence
#define EEP_RDSR_C    0x05 // read status register
#define EEP_WREN_C    0x06 // write enable

// Image sources
#define UART_IMAGE_C   0x00
#define EEPROM_IMAGE_C 0x01

// Function prototypes
void __attribute__((__interrupt__)) timer_irq_handler(void);
void start_app(void);
void print_help(void);
void core_dump(void);
void store_eeprom(void);
void eeprom_write(uint16_t a, uint16_t d);
uint16_t eeprom_read(uint16_t a);
uint16_t get_image(uint8_t src);
uint16_t get_image_word(uint16_t a, uint8_t src);


/* ------------------------------------------------------------
 * INFO Bootloader main
 * ------------------------------------------------------------ */
int main(void) {

  // ****************************************************************
  // Processor hardware initialization
  // ****************************************************************

  // stack setup
  // -> done in boot_crt0.asm

  // disable watchdog timer
  // -> done in boot_crt0.asm

  // clear status register and disable interrupts, just enable write access to IMEM
  asm volatile ("mov %0, r2" : : "i" (1<<R_FLAG_C));

  // disable Wishbone interface
  WB32_CT = 0;

  // init timer interrupt vector
  IRQVEC_TIMER = (uint16_t)(&timer_irq_handler); // timer match

  // init parallel IO
  pio_port_set(1<<STATUS_LED_C); // activate status LED
  PIO_CTRL = 0; // disable all pin change IRQs

  // set Baud rate & init USART control register:
  // enable USART, no IRQs, SPI clock mode 0, 1/1024 SPI speed, disable all 6 SPI CS lines (set high)
  USI_CT = 0; // reset USART
  uart_set_baud(BAUD_RATE_C);
  USI_CT = (1<<USI_CT_EN_C)  | (5<<USI_CT_SPIPRSC0_C) | (63<<USI_CT_SPICS0_C);
  spi_trans(0); // clear SPI RTX buffer
  uart_char_read(); // clear UART RX buffer

  // Timeout counter: init timer, irq tick @ ~1Hz (prescaler = 4096)
  // THR = f_main / (1Hz + 4096) -1
  TMR_CT = 0; // reset timer
  //uint32_t clock = (((uint32_t)CLOCKSPEED_HI<<16) | (uint32_t)CLOCKSPEED_LO) >> 14; // divide by 4096
  TMR_THRES = (CLOCKSPEED_HI << 2) -1; // "fake" ;D
  GPREG0 = 0; // timeout ticker
  // enable timer, auto reset, enable IRQ, prsc = 1:2^16
  TMR_CT = (1<<TMR_CT_EN_C) | (1<<TMR_CT_ARST_C) | (1<<TMR_CT_IRQ_C) | ((16-1)<<TMR_CT_PRSC0_C);
  TMR_CNT = 0;

  eint(); // enable global interrupts


  // ****************************************************************
  // Show bootloader intro and system information
  // ****************************************************************
  uart_br_print("\n\nNEO430 Bootloader V20170209 by Stephan Nolting\n"
                "HV=$");
  uart_print_hex_word(HW_VERSION);
  uart_br_print(" IM=$");
  uart_print_hex_word(IMEM_SIZE);
  uart_br_print(" DM=$");
  uart_print_hex_word(DMEM_SIZE);
  uart_br_print("\n\nAutoboot in "xstr(AUTOBOOT_TIMEOUT_C)"s. Press key to abort.\n");


  // ****************************************************************
  // Auto boot sequence
  // ****************************************************************
  // wait for any key to be pressed or timeout
  while (1) {
    // timeout? start auto boot sequence
    if (GPREG0 == 4*AUTOBOOT_TIMEOUT_C) { // in 0.25 seconds
      if (get_image(EEPROM_IMAGE_C) != 0) { // try booting from EEPROM
        asm volatile ("mov #0, r2"); // deactivate IRQs, no more write access to IMEM
        pio_port_set(1<<STATUS_LED_C); // permanently light up status LED
        while(1); // freeze
      }
      else {
        uart_br_print("\n");
        start_app();
      }
    }
    // key pressed?
    if ((USI_UARTRTX & (1<<USI_UARTRTX_UARTRXAVAIL_C)) != 0)
      break;
  }
  uart_br_print("Aborted.\n\n");
  print_help();


  // ****************************************************************
  // Bootloader console
  // ****************************************************************
  while (1) {
    uart_br_print("\n>> ");
    char c = uart_getc();
    uart_putc(c); // echo
    uart_br_print("\n");

    if (c == 'r') // restart bootloader
      asm volatile ("mov #0xF000, r0"); // jump to beginning of bootloader ROM
    else if (c == 'h') // help menu
      print_help();
    else if (c == 'd') // core dump
      core_dump();
    else if (c == 'u') // upload program to RAM via UART
      get_image(UART_IMAGE_C);
    else if (c == 'p') // program EEPROM from RAM
      store_eeprom();
    else if (c == 'e') // copy program from EEPROM to RAM
      get_image(EEPROM_IMAGE_C);
    else if (c == 's') // start program in RAM
      start_app();
    else // unknown command
      uart_br_print("Bad CMD!");
  }
}



/* ------------------------------------------------------------
 * INFO Timer IRQ handler
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) timer_irq_handler(void) {

  GPREG0++; // increment system ticker
  pio_port_toggle(1<<STATUS_LED_C); // toggle status LED
}


/* ------------------------------------------------------------
 * INFO Start application
 * ------------------------------------------------------------ */
void start_app(void) {

  // deactivate IRQs, no more write access to IMEM
  asm volatile ("mov #0, r2");

  uart_br_print("Booting...\n");

  // wait for UART to finish transmitting
  while(USI_CT & (1<<USI_CT_UARTTXBSY_C));

  // start app in IMEM at address 0x0000
  while (1) {
    asm volatile ("mov #0x0000, r0");
  }
}


/* ------------------------------------------------------------
 * INFO Print help text
 * ------------------------------------------------------------ */
void print_help(void) {

  uart_br_print("d: Dump\n"
                "e: Load EEPROM\n"
                "h: Help\n"
                "p: Store EEPROM\n"
                "r: Restart\n"
                "s: Start app\n"
                "u: Upload");
}


/* ------------------------------------------------------------
 * INFO Print whole address space content
 * ------------------------------------------------------------ */
void core_dump(void) {

  uint8_t *pnt = 0;
  uint8_t i = 0;
  char c = 0;

  while (1) {
    uart_br_print("\n");
    uart_print_hex_word((uint16_t)pnt); // print address
    uart_br_print(":  ");

    // print hexadecimal data
    for (i=0; i<16; i++) {
      uart_print_hex_byte(*(pnt+i));
      uart_putc(' ');
    }

    // print ascii data
    uart_br_print(" |");

    // print single ascii char
    for (i=0; i<16; i++) {
      c = (char)*(pnt+i);
      if ((c >= ' ') && (c <= '~'))
        uart_putc(c);
      else
        uart_putc('.');
    }

    uart_putc('|');

    // user abort or all done?
    if ((uart_char_received() != 0) || ((uint16_t)pnt == 0xFFF0))
      return;

    pnt += 16;
  }
}


/* ------------------------------------------------------------
 * INFO Store full IMEM content to SPI EEPROM at SPI.CS0
 * ------------------------------------------------------------ */
void store_eeprom(void) {

  uart_br_print("Proceed y/n?");
  if (uart_getc() != 'y')
    return;

  uart_br_print("\nWriting... ");

  spi_cs_en(BOOT_EEP_CS_C);
  spi_trans(EEP_WREN_C); // write enable
  spi_cs_dis(BOOT_EEP_CS_C);

  // check if eeprom ready (or available at all)
  spi_cs_en(BOOT_EEP_CS_C);
  spi_trans(EEP_RDSR_C); // read status register CMD
  uint8_t b = spi_trans(0x00); // read status register data
  spi_cs_dis(BOOT_EEP_CS_C);

  if ((b & 0x8F) != 0x02) {
    uart_br_print("ErrE");
    return;
  }

  // write signature
  eeprom_write(0x0000, 0xCAFE);

  // write size
  uint16_t end = IMEM_SIZE;
  eeprom_write(0x0002, end);

  // store data from IMEM and update checksum
  uint16_t *mem_pnt = 0;
  uint16_t eep_index = 0x0006;
  uint16_t checksum = 0;
  uint16_t d = 0;
  while ((uint16_t)mem_pnt < end) {
    d = *mem_pnt;
    eeprom_write(eep_index, d);
    checksum ^= d;
    mem_pnt++;
    eep_index += 2;
  }

  // write checksum
  eeprom_write(0x0004, checksum);

  uart_br_print("OK");
}


/* ------------------------------------------------------------
 * INFO EEPROM write data
 * PARAM a destination address (16 bit)
 * PARAM d word to be written
 * ------------------------------------------------------------ */
void eeprom_write(uint16_t a, uint16_t d) {

  spi_cs_en(BOOT_EEP_CS_C);
  spi_trans(EEP_WREN_C); // write enable
  spi_cs_dis(BOOT_EEP_CS_C);

  spi_cs_en(BOOT_EEP_CS_C);
  spi_trans(EEP_WRITE_C); // byte write instruction
  spi_trans((uint8_t)(a >> 8));
  spi_trans((uint8_t)(a >> 0));
  spi_trans((uint8_t)(d >> 8));
  spi_trans((uint8_t)(d >> 0));
  spi_cs_dis(BOOT_EEP_CS_C);

  // wait for write to finish
  uint8_t s = 0;
  do {
    spi_cs_en(BOOT_EEP_CS_C);
    spi_trans(EEP_RDSR_C); // read status register CMD
    s = spi_trans(0x00);
    spi_cs_dis(BOOT_EEP_CS_C);
  } while (s & 0x01); // check WIP flag
}


/* ------------------------------------------------------------
 * INFO EEPROM read data
 * PARAM a destination address (16 bit)
 * RETURN word read data
 * ------------------------------------------------------------ */
uint16_t eeprom_read(uint16_t a) {

  spi_cs_en(BOOT_EEP_CS_C);
  spi_trans(EEP_READ_C); // byte read instruction
  spi_trans((uint8_t)(a >> 8));
  spi_trans((uint8_t)(a >> 0));
  uint8_t b0 = spi_trans(0x00);
  uint8_t b1 = spi_trans(0x00);
  spi_cs_dis(BOOT_EEP_CS_C);

  return ((uint16_t)b0 << 8) | (uint16_t)b1;
}


/* ------------------------------------------------------------
 * INFO Get IMEM image from SPI EEPROM at SPI.CS0 or from UART
 * PARAM src Image source 0: UART, 1: EEPROM
 * RETURN error code (0 if successfull)
 * ------------------------------------------------------------ */
uint16_t get_image(uint8_t src) {

  // abort if IMEM was implemented as true ROM
  if (SYS_FEATURES & (1<<SYS_IROM_EN_C)) {
    uart_br_print("ErrM");
    return 1;
  }

  // print intro
  if (src == UART_IMAGE_C) // boot via UART
    uart_br_print("Awaiting BINEXE... ");
  else //if (src == EEPROM_IMAGE_C)// boot from EEPROM
    uart_br_print("Loading... ");

  // check if valid image
  if (get_image_word(0x0000, src) != 0xCAFE) {
    uart_br_print("ErrX");
    return 1;
  }

  // image size & check
  uint16_t size  = get_image_word(0x0002, src);
  uint16_t check = get_image_word(0x0004, src);
  uint16_t end = IMEM_SIZE;
  if (size > end) {
    uart_br_print("ErrS");
    return 1;
  }

  // transfer program data
  uint16_t *pnt = 0x0000;
  uint16_t addr = 0x0006;
  uint16_t checksum = 0;
  uint16_t d = 0;
  while ((uint16_t)pnt < size) {
    d = get_image_word(addr, src);
    checksum ^= d;
    *pnt++ = d;
    addr += 2;
  }

  // clear rest of IMEM
  while((uint16_t)pnt < end)
    *pnt++ = 0x0000;

  // error during transfer?
  if (checksum == check) {
    uart_br_print("OK");
    return 0;
  }
  else {
    uart_br_print("ErrC");
    return 1;
  }
}


/* ------------------------------------------------------------
 * INFO Get image word from EEPROM or UART
 * PARAM a source address (16 bit)
 * PARAM src: 0: UART, 1: EEPROM
 * RETURN accessed data word
 * ------------------------------------------------------------ */
uint16_t get_image_word(uint16_t a, uint8_t src) {

  uint16_t d;

  // reads have to be consecutive when reading from the UART ;)
  if (src == UART_IMAGE_C) { // get image data via UART
    uint8_t c0 = (uint8_t)uart_getc();
    uint8_t c1 = (uint8_t)uart_getc();
    d = ((uint16_t)c0 << 8) | (uint16_t)c1;
  }
  else //if (src == EEPROM_IMAGE_C) // get image data from EEPROM
    d = eeprom_read(a);

  return d;
}
