// #################################################################################################
// #  < NEO430 Bootloader >                                                                        #
// # ********************************************************************************************* #
// # Boot from IMEM, UART or SPI EEPROM at SPI.CS[0]                                               #
// #                                                                                               #
// # UART configuration: 8N1 at 19200 baud                                                         #
// # Boot EEPROM: SPI, 16-bit addresses, SPI.CS[0], e.g. 25LC512                                   #
// # GPIO.out[0] is used as high-active status LED                                                 #
// #                                                                                               #
// # Auto boot sequence after timeout:                                                             #
// #  -> Try booting from SPI EEPROM at SPI.CS[0]                                                  #
// #  -> permanently light up status led and freeze if SPI EEPROM booting attempt fails            #
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
// #  Stephan Nolting, Hannover, Germany                                               30.05.2018  #
// #################################################################################################

// Libraries
#include <stdint.h>
#include "../lib/neo430/neo430.h"

// Macros
#define xstr(a) str(a)
#define str(a) #a

// Configuration
#define BAUD_RATE        19200 // default UART baud rate
#define AUTOBOOT_TIMEOUT 8     // countdown (seconds) to auto boot
#define STATUS_LED       0     // GPIO.out(0) is status LED

// 25LC512 EEPROM
#define BOOT_EEP_CS      0    // boot EEPROM CS (SPI.CS0)
#define EEP_IMAGE_BASE   0x00 // base address of NEO430 boot image
#define EEP_WRITE        0x02 // initialize start of write sequence
#define EEP_READ         0x03 // initialize start of read sequence
#define EEP_RDSR         0x05 // read status register
#define EEP_WREN         0x06 // write enable

// Image sources
#define UART_IMAGE       0x00
#define EEPROM_IMAGE     0x01

// Error codes
#define ERROR_EEPROM     0x00 // EEPROM access error
#define ERROR_ROMACCESS  0x01 // cannot write to IMEM
#define ERROR_EXECUTABLE 0x02 // invalid executable format
#define ERROR_SIZE       0x04 // executable is too big
#define ERROR_CHECKSUM   0x08 // checksum error
#define ERROR_UNKNOWN    0xFF // unknown error

// Scratch registers - abuse unused IRQ vectors for this ;)
#define TIMEOUT_CNT IRQVEC_GPIO
#define VALID_IMAGE IRQVEC_USART

// Function prototypes
void __attribute__((__interrupt__)) timer_irq_handler(void);
void start_app(void);
void print_help(void);
void core_dump(void);
void store_eeprom(void);
void eeprom_write(uint16_t a, uint16_t d);
uint16_t eeprom_read(uint16_t a);
void get_image(uint8_t src);
uint16_t get_image_word(uint16_t a, uint8_t src);
void system_error(uint8_t err_code);


/* ------------------------------------------------------------
 * INFO Bootloader main
 * ------------------------------------------------------------ */
int  main(void) {

  // ****************************************************************
  // Processor hardware initialization
  // ****************************************************************

  // stack setup
  // -> done in boot_crt0

  // disable watchdog timer
  wdt_disable();

  // clear status register and disable interrupts, clear interrupt buffer, enable write access to IMEM
  asm volatile ("mov %0, r2" : : "i" ((1<<R_FLAG) | (1<<Q_FLAG)));

  // disable Wishbone interface
  WB32_CT = 0;

  // disable PWM
  PWM_CT = 0;

  // disable TRNG
  TRNG_CT = 0;

  // no valid boot image in IMEM yet
  VALID_IMAGE = 0;

  // init timer interrupt vector
  IRQVEC_TIMER = (uint16_t)(&timer_irq_handler); // timer match

  // init GPIO
  GPIO_IRQMASK = 0; // no pin change interrupt please, thanks
  gpio_port_set(1<<STATUS_LED); // activate status LED, clear all others

  // set Baud rate & init USART control register:
  // enable USART, no IRQs, SPI clock mode 0, 1/1024 SPI speed, disable all 6 SPI CS lines (set high)
  USI_CT = 0; // reset USART
  uart_set_baud(BAUD_RATE);
  USI_CT = (1<<USI_CT_EN)  | (5<<USI_CT_SPIPRSC0);
  spi_trans(0); // clear SPI RTX buffer
  uart_char_read(); // clear UART RX buffer

  // Timeout counter: init timer, irq tick @ ~1Hz (prescaler = 4096)
  // THR = f_main / (1Hz + 4096) -1
  TMR_CT = 0; // reset timer
  //uint32_t clock = CLOCKSPEED_32bit >> 14; // divide by 4096
  TMR_THRES = (CLOCKSPEED_HI << 2) -1; // "fake" ;D
  // enable timer, auto reset, enable IRQ, prsc = 1:2^16
  TMR_CT = (1<<TMR_CT_EN) | (1<<TMR_CT_ARST) | (1<<TMR_CT_IRQ) | ((16-1)<<TMR_CT_PRSC0);
  TMR_CNT = 0;
  TIMEOUT_CNT = 0; // timeout ticker

  clear_irq_buffer(); // clear all pending interrupts
  eint(); // enable global interrupts


  // ****************************************************************
  // Show bootloader intro and system information
  // ****************************************************************
  uart_br_print("\n\nNEO430 Bootloader V20180530 by Stephan Nolting\n\n"
                "HWV: 0x");
  uart_print_hex_word(HW_VERSION);
  uart_br_print("\nCLK: 0x");
  uart_print_hex_word(CLOCKSPEED_HI);
  uart_print_hex_word(CLOCKSPEED_LO);
  uart_br_print("\nROM: 0x");
  uart_print_hex_word(IMEM_SIZE);
  uart_br_print("\nRAM: 0x");
  uart_print_hex_word(DMEM_SIZE);
  uart_br_print("\nSYS: 0x");
  uart_print_hex_word(SYS_FEATURES);


  // ****************************************************************
  // Auto boot sequence
  // ****************************************************************
  uart_br_print("\n\nAutoboot in "xstr(AUTOBOOT_TIMEOUT)"s. Press key to abort.\n");
  while (1) { // wait for any key to be pressed or timeout

    // timeout? start auto boot sequence
    if (TIMEOUT_CNT == 4*AUTOBOOT_TIMEOUT) { // in 0.25 seconds
      get_image(EEPROM_IMAGE); // try loading from EEPROM
      uart_br_print("\n");
      start_app(); // start app if loading was successful
    }

    // key pressed? -> enter user console
    if ((USI_UARTRTX & (1<<USI_UARTRTX_UARTRXAVAIL)) != 0)
      break;
  }
  uart_br_print("Aborted.\n\n");
  print_help();


  // ****************************************************************
  // Bootloader console
  // ****************************************************************
  while (1) {

    uart_br_print("\nCMD:> ");
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
      get_image(UART_IMAGE);
    else if (c == 'p') // program EEPROM from RAM
      store_eeprom();
    else if (c == 'e') // copy program from EEPROM to RAM
      get_image(EEPROM_IMAGE);
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

  TIMEOUT_CNT++; // increment system ticker
  gpio_port_toggle(1<<STATUS_LED); // toggle status LED
}


/* ------------------------------------------------------------
 * INFO Start application in IMEM
 * ------------------------------------------------------------ */
void start_app(void) {

  // valid image in IMEM?
  if (VALID_IMAGE == 0) {
    uart_br_print("Image still valid? Boot anyway (y/n)? ");
    if (uart_getc() != 'y')
      return;
  }

  // deactivate IRQs, no more write access to IMEM, clear all pending IRQs
  asm volatile ("mov %0, r2" : : "i" (1<<Q_FLAG));

  uart_br_print("Booting...\n\n");

  // wait for UART to finish transmitting
  while ((USI_CT & (1<<USI_CT_UARTTXBSY)) != 0);

  // start app in IMEM at address 0x0000
  while (1) {
    asm volatile ("mov #0x0000, r0");
  }
}


/* ------------------------------------------------------------
 * INFO Print help text
 * ------------------------------------------------------------ */
void print_help(void) {

  uart_br_print("Commands:\n"
                " d: Dump MEM\n"
                " e: Load EEPROM\n"
                " h: Help\n"
                " p: Store EEPROM\n"
                " r: Restart\n"
                " s: Start app\n"
                " u: Upload");
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

  uart_br_print("Proceed (y/n)?");
  if (uart_getc() != 'y')
    return;

  uart_br_print("\nWriting... ");

  spi_cs_en(BOOT_EEP_CS);
  spi_trans(EEP_WREN); // write enable
  spi_cs_dis(BOOT_EEP_CS);

  // check if eeprom ready (or available at all)
  spi_cs_en(BOOT_EEP_CS);
  spi_trans(EEP_RDSR); // read status register CMD
  uint8_t b = spi_trans(0x00); // read status register data
  spi_cs_dis(BOOT_EEP_CS);

  if ((b & 0x8F) != 0x02)
    system_error(ERROR_EEPROM);

  // write signature
  eeprom_write(EEP_IMAGE_BASE + 0, 0xCAFE);

  // write size
  uint16_t end = IMEM_SIZE;
  eeprom_write(EEP_IMAGE_BASE + 2, end);

  // store data from IMEM and update checksum
  uint16_t *mem_pnt = 0;
  uint16_t eep_index = EEP_IMAGE_BASE + 6;
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
  eeprom_write(EEP_IMAGE_BASE + 4, checksum);

  uart_br_print("OK");
}


/* ------------------------------------------------------------
 * INFO EEPROM write data
 * PARAM a destination address (16 bit)
 * PARAM d word to be written
 * ------------------------------------------------------------ */
void eeprom_write(uint16_t a, uint16_t d) {

  spi_cs_en(BOOT_EEP_CS);
  spi_trans(EEP_WREN); // write enable
  spi_cs_dis(BOOT_EEP_CS);

  spi_cs_en(BOOT_EEP_CS);
  spi_trans(EEP_WRITE); // byte write instruction
  spi_trans((uint8_t)(a >> 8));
  spi_trans((uint8_t)(a >> 0));
  spi_trans((uint8_t)(d >> 8));
  spi_trans((uint8_t)(d >> 0));
  spi_cs_dis(BOOT_EEP_CS);

  // wait for write to finish
  uint8_t s = 0;
  do {
    spi_cs_en(BOOT_EEP_CS);
    spi_trans(EEP_RDSR); // read status register CMD
    s = spi_trans(0x00);
    spi_cs_dis(BOOT_EEP_CS);
  } while (s & 0x01); // check WIP flag
}


/* ------------------------------------------------------------
 * INFO EEPROM read data
 * PARAM a destination address (16 bit)
 * RETURN word read data
 * ------------------------------------------------------------ */
uint16_t eeprom_read(uint16_t a) {

  spi_cs_en(BOOT_EEP_CS);
  spi_trans(EEP_READ); // byte read instruction
  spi_trans((uint8_t)(a >> 8));
  spi_trans((uint8_t)(a >> 0));
  uint8_t b0 = spi_trans(0x00);
  uint8_t b1 = spi_trans(0x00);
  spi_cs_dis(BOOT_EEP_CS);

  return __combine_bytes(b0, b1);
}


/* ------------------------------------------------------------
 * INFO Get IMEM image from SPI EEPROM at SPI.CS0 or from UART
 * PARAM src Image source 0: UART, 1: EEPROM
 * RETURN error code (0 if successful)
 * ------------------------------------------------------------ */
void get_image(uint8_t src) {

  // abort if IMEM was implemented as true ROM
  if (SYS_FEATURES & (1<<SYS_IROM_EN)) {
    system_error(ERROR_ROMACCESS);
  }

  // print intro
  if (src == UART_IMAGE) // boot via UART
    uart_br_print("Awaiting BINEXE... ");
  else //if (src == EEPROM_IMAGE)// boot from EEPROM
    uart_br_print("Loading... ");

  // check if valid image
  if (get_image_word(0x0000, src) != 0xCAFE)
    system_error(ERROR_EXECUTABLE);

  // image size & check
  uint16_t size  = get_image_word(0x0002, src);
  uint16_t check = get_image_word(0x0004, src);
  uint16_t end = IMEM_SIZE;
  if (size > end)
    system_error(ERROR_SIZE);

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
    VALID_IMAGE = 1;
  }
  else
    system_error(ERROR_CHECKSUM);
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
  if (src == UART_IMAGE) { // get image data via UART
    uint8_t c0 = (uint8_t)uart_getc();
    uint8_t c1 = (uint8_t)uart_getc();
    d = __combine_bytes(c0, c1);
  }
  else //if (src == EEPROM_IMAGE) // get image data from EEPROM
    d = eeprom_read(a);

  return d;
}


/* ------------------------------------------------------------
 * INFO Print error message, light up status LED and freeze system
 * PARAM error code
 * ------------------------------------------------------------ */
void system_error(uint8_t err_code){

  uart_br_print("\a\nERR_");
  uart_print_hex_byte(err_code);

  asm volatile ("mov #0, r2"); // deactivate IRQs, no more write access to IMEM
  gpio_port_set(1<<STATUS_LED); // permanently light up status LED

  while(1); // freeze
}
