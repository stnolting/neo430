// #################################################################################################
// #  < NEO430 Bootloader >                                                                        #
// # ********************************************************************************************* #
// # Boot from IMEM, UART or SPI EEPROM at SPI.CS[0]                                               #
// #                                                                                               #
// # UART configuration: 8N1 at 19200 baud                                                         #
// # Boot EEPROM: SPI, 16-bit addresses (e.g., 25LC512) @ SPI.CS[0]                                #
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
// # Stephan Nolting, Hannover, Germany                                                 10.12.2019 #
// #################################################################################################

// Libraries
#include <stdint.h>
#include <neo430.h>

// Macros
#define xstr(a) str(a)
#define str(a) #a

// Configuration
#define BAUD_RATE        19200 // default UART baud rate
#define AUTOBOOT_TIMEOUT 8     // countdown (seconds) to auto boot
#define STATUS_LED       0     // GPIO.out(0) is status LED

// 25LC512 SPI EEPROM
#define BOOT_EEP_CS      0    // boot EEPROM CS (SPI.CS0)
#define EEP_IMAGE_BASE   0x00 // base address of NEO430 boot image
#define EEP_WRITE        0x02 // initialize start of write sequence
#define EEP_READ         0x03 // initialize start of read sequence
#define EEP_RDSR         0x05 // read status register
#define EEP_WREN         0x06 // write enable

// 24LC512 TWI EEPROM
#define TWI_BOOT_EEP_ADDR_READ 0b10100001 // READ address of TWI boot eeprom

// Image sources
#define UART_IMAGE       0x00
#define EEPROM_IMAGE_SPI 0x01
#define EEPROM_IMAGE_TWI 0x02

// Error codes
#define ERROR_EEPROM     0x00 // EEPROM access error
#define ERROR_ROMACCESS  0x01 // cannot write to IMEM
#define ERROR_EXECUTABLE 0x02 // invalid executable format
#define ERROR_SIZE       0x04 // executable is too big
#define ERROR_CHECKSUM   0x08 // checksum error

// Scratch registers - abuse unused IRQ vectors for this ;)
#define TIMEOUT_CNT IRQVEC_GPIO

// Function prototypes
void     __attribute__((__interrupt__)) timer_irq_handler(void);
void     __attribute__((__naked__)) start_app(void);
void     print_help(void);
void     core_dump(void);
void     store_eeprom(void);
void     eeprom_write_word(uint16_t a, uint16_t d);
void     spi_eeprom_write_byte(uint16_t a, uint8_t b);
uint8_t  spi_eeprom_read_byte(uint16_t a);
uint8_t  twi_eeprom_read_byte(uint16_t a);
void     get_image(uint8_t src);
uint16_t get_image_word(uint16_t a, uint8_t src);
void     __attribute__((__naked__)) system_error(uint8_t err_code);


/* ------------------------------------------------------------
 * INFO Bootloader main
 * ------------------------------------------------------------ */
int main(void) {

  // ****************************************************************
  // Processor hardware initialization
  // ****************************************************************

  // stack setup
  // -> done in boot_crt0

  // disable watchdog timer
  neo430_wdt_disable();

  // clear status register and disable interrupts, clear interrupt buffer, enable write access to IMEM
  asm volatile ("mov %0, r2" : : "i" ((1<<R_FLAG) | (1<<Q_FLAG)));

  // disable TRNG
  TRNG_CT = 0;

  // disable Wishbone interface
  WB32_CT = 0;

  // disable PWM
  PWM_CT = 0;

  // disable TWI
  TWI_CT = 0;

  // disable EXIRQ
  EXIRQ_CT = 0;

  // init timer interrupt vector
  IRQVEC_TIMER = (uint16_t)(&timer_irq_handler); // timer match

  // init GPIO
  GPIO_IRQMASK = 0; // no pin change interrupt please, thanks
  neo430_gpio_port_set(1<<STATUS_LED); // activate status LED, clear all others

  // set Baud rate & init UART control register:
  // enable UART, no IRQs
  neo430_uart_setup(BAUD_RATE);
  neo430_uart_char_read(); // clear UART RX buffer

  // set SPI config:
  // enable SPI, no IRQs, MSB first, SPI clock mode 0, 1/128 SPI speed, disable all 6 SPI CS lines (set high)
  neo430_spi_enable(SPI_PRSC_64); // this also resets the SPI module
  neo430_spi_trans(0); // clear SPI RTX buffer

  // Timeout counter: init timer, irq tick @ ~1Hz (prescaler = 4096)
  // THR = f_main / (1Hz + 4096) -1
  TMR_CT = 0; // reset timer
  //uint32_t clock = CLOCKSPEED_32bit >> 14; // divide by 4096
  TMR_THRES = (CLOCKSPEED_HI << 2) -1; // "fake" ;D
  // enable timer, auto reset, enable IRQ, prsc = 1:2^16, start timer
  TMR_CT = (1<<TMR_CT_EN) | (1<<TMR_CT_ARST) | (1<<TMR_CT_IRQ) | ((16-1)<<TMR_CT_PRSC0) | (1<<TMR_CT_RUN);
  TIMEOUT_CNT = 0; // console timeout ticker

  neo430_clear_irq_buffer(); // clear all pending interrupts
  neo430_eint(); // enable global interrupts


  // ****************************************************************
  // Show bootloader intro and system information
  // ****************************************************************
  neo430_uart_br_print("\n\n<< NEO430 Bootloader >>\n"
                       "\n"
                       "BLV: "__DATE__"\n"
                       "HWV: 0x");
  neo430_uart_print_hex_word(HW_VERSION);
  neo430_uart_br_print("\nUSR: 0x");
  neo430_uart_print_hex_word(USER_CODE);
  neo430_uart_br_print("\nCLK: 0x");
  neo430_uart_print_hex_word(CLOCKSPEED_HI);
  neo430_uart_print_hex_word(CLOCKSPEED_LO);
  neo430_uart_br_print("\nROM: 0x");
  neo430_uart_print_hex_word(IMEM_SIZE);
  neo430_uart_br_print("\nRAM: 0x");
  neo430_uart_print_hex_word(DMEM_SIZE);
  neo430_uart_br_print("\nSYS: 0x");
  neo430_uart_print_hex_word(SYS_FEATURES);


  // ****************************************************************
  // Auto boot sequence
  // ****************************************************************
  neo430_uart_br_print("\n\nAutoboot in "xstr(AUTOBOOT_TIMEOUT)"s. Press key to abort.\n");
  while (1) { // wait for any key to be pressed or timeout

    // timeout? start auto boot sequence
    if (TIMEOUT_CNT == 4*AUTOBOOT_TIMEOUT) { // in 0.25 seconds
      get_image(EEPROM_IMAGE_SPI); // try loading from EEPROM
      neo430_uart_br_print("\n");
      start_app(); // start app if loading was successful
    }

    // key pressed? -> enter user console
    if ((UART_RTX & (1<<UART_RTX_AVAIL)) != 0)
      break;
  }
  print_help();


  // ****************************************************************
  // Bootloader console
  // ****************************************************************
  while (1) {

    neo430_uart_br_print("\nCMD:> ");
    char c = neo430_uart_getc();
    neo430_uart_putc(c); // echo
    neo430_uart_br_print("\n");

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
      get_image(EEPROM_IMAGE_SPI);
    else if (c == 's') // start program in RAM
      start_app();
    else if (c == 'c')
      neo430_uart_br_print("By Stephan Nolting");
    else // unknown command
      neo430_uart_br_print("Bad CMD!");
  }
}


/* ------------------------------------------------------------
 * INFO Timer IRQ handler
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) timer_irq_handler(void) {

  TIMEOUT_CNT++; // increment system ticker
  neo430_gpio_port_toggle(1<<STATUS_LED); // toggle status LED
}


/* ------------------------------------------------------------
 * INFO Start application in IMEM
 * INFO "naked" since this is final...
 * ------------------------------------------------------------ */
void __attribute__((__naked__)) start_app(void) {

  neo430_uart_br_print("Booting...\n\n");

  // wait for UART to finish transmitting
  while ((UART_CT & (1<<UART_CT_TX_BUSY)) != 0);

  // deactivate IRQs, no more write access to IMEM, clear all pending IRQs
  asm volatile ("mov %0, r2" : : "i" (1<<Q_FLAG));

  // start app in IMEM at address 0x0000
  while (1) {
    asm volatile ("mov #0x0000, r0");
  }
}


/* ------------------------------------------------------------
 * INFO Print help text
 * ------------------------------------------------------------ */
void print_help(void) {

  neo430_uart_br_print("CMDs:\n"
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

  uint16_t *pnt = (uint16_t*)0x0000;
  uint16_t i = 0, j = 0;
  
  while (1) {
    neo430_uart_br_print("\n");
    neo430_uart_print_hex_word((uint16_t)pnt); // print address
    neo430_uart_br_print(":  ");
  
    // print hexadecimal data
    for (i=0; i<16; i++) {
      neo430_uart_print_hex_word(*pnt++);
      neo430_uart_putc(' ');
    }
  
    // user abort or all done?
    if ((neo430_uart_char_received() != 0) || (j == 0xFFE0))
      return;
  
    j += 32;
  }
}


/* ------------------------------------------------------------
 * INFO Store full IMEM content to SPI EEPROM at SPI.CS0
 * ------------------------------------------------------------ */
void store_eeprom(void) {

  neo430_uart_br_print("Proceed (y/n)?");
  if (neo430_uart_getc() != 'y')
    return;

  neo430_uart_br_print("\nWriting... ");

  neo430_spi_cs_en(BOOT_EEP_CS);
  neo430_spi_trans(EEP_WREN); // write enable
  neo430_spi_cs_dis();

  // check if eeprom ready (or available at all)
  neo430_spi_cs_en(BOOT_EEP_CS);
  neo430_spi_trans(EEP_RDSR); // read status register CMD
  uint8_t b = neo430_spi_trans(0x00); // read status register data
  neo430_spi_cs_dis();

  if ((b & 0x8F) != 0x02)
    system_error(ERROR_EEPROM);

  // write EXE signature
  eeprom_write_word(EEP_IMAGE_BASE + 0, 0xCAFE);

  // write size
  uint16_t end = IMEM_SIZE;
  eeprom_write_word(EEP_IMAGE_BASE + 2, end);

  // store data from IMEM and update checksum
  uint16_t checksum = 0;
  uint16_t i = 0;
  uint16_t *pnt = (uint16_t*)0x0000;;

  while (i < end) {
    uint16_t d = (uint16_t)*pnt++;
    checksum ^= d;
    eeprom_write_word(EEP_IMAGE_BASE + 6 + i, d);
    i+= 2;
  }

  // write checksum
  eeprom_write_word(EEP_IMAGE_BASE + 4, checksum);

  neo430_uart_br_print("OK");
}


/* ------------------------------------------------------------
 * INFO EEPROM write data word
 * PARAM a destination address (16 bit)
 * PARAM d word to be written
 * ------------------------------------------------------------ */
void eeprom_write_word(uint16_t a, uint16_t d) {

  uint8_t lo = (uint8_t)(d);
  uint8_t hi = (uint8_t)(d >> 8);
  
  spi_eeprom_write_byte(a+0, hi);
  spi_eeprom_write_byte(a+1, lo);
}


/* ------------------------------------------------------------
 * INFO SPI EEPROM write single byte
 * PARAM a destination address (16 bit)
 * PARAM b byte to be written
 * ------------------------------------------------------------ */
void spi_eeprom_write_byte(uint16_t a, uint8_t b) {

  neo430_spi_cs_en(BOOT_EEP_CS);
  neo430_spi_trans(EEP_WREN); // write enable
  neo430_spi_cs_dis();

  neo430_spi_cs_en(BOOT_EEP_CS);
  neo430_spi_trans(EEP_WRITE); // byte write instruction
  neo430_spi_trans((uint8_t)(a >> 8));
  neo430_spi_trans((uint8_t)(a >> 0));
  neo430_spi_trans(b);
  neo430_spi_cs_dis();

  // wait for write to finish
  while(1) {
    neo430_spi_cs_en(BOOT_EEP_CS);
    neo430_spi_trans(EEP_RDSR); // read status register CMD
    uint8_t s = neo430_spi_trans(0x00);
    neo430_spi_cs_dis();

    if ((s & 0x01) == 0) { // check WIP flag
      break; // done!
    }
  }
}


/* ------------------------------------------------------------
 * INFO SPI EEPROM read data
 * PARAM a destination address (16 bit)
 * RETURN byte read data
 * ------------------------------------------------------------ */
uint8_t spi_eeprom_read_byte(uint16_t a) {

  neo430_spi_cs_en(BOOT_EEP_CS);
  neo430_spi_trans(EEP_READ); // byte read instruction
  neo430_spi_trans((uint8_t)(a >> 8));
  neo430_spi_trans((uint8_t)(a >> 0));
  uint8_t d = neo430_spi_trans(0);
  neo430_spi_cs_dis();

  return d;
}


/* ------------------------------------------------------------
 * INFO TWI EEPROM read data
 * PARAM a destination address (16 bit)
 * RETURN byte read data
 * ------------------------------------------------------------ */
uint8_t twi_eeprom_read_byte(uint16_t a) {

  uint8_t twi_err = neo430_twi_start_trans(TWI_BOOT_EEP_ADDR_READ);
  twi_err |= neo430_twi_trans((uint8_t)(a >> 8));
  twi_err |= neo430_twi_trans((uint8_t)(a >> 0));
  twi_err |= !neo430_twi_trans(0xFF); // read data
  uint8_t d = neo430_twi_get_data();
  neo430_twi_generate_stop();

  //if (twi_err) {
  //  
  //}

  return d;
}


/* ------------------------------------------------------------
 * INFO Get IMEM image from SPI EEPROM at SPI.CS0 or from UART
 * PARAM src Image source 0: UART, 1: SPI_EEPROM
 * RETURN error code (0 if successful)
 * ------------------------------------------------------------ */
void get_image(uint8_t src) {

  // abort if IMEM was implemented as true ROM
  if (SYS_FEATURES & (1<<SYS_IROM_EN)) {
    system_error(ERROR_ROMACCESS);
  }

  // print intro
  if (src == UART_IMAGE) // boot via UART
    neo430_uart_br_print("Awaiting BINEXE... ");
  else //if (src == EEPROM_IMAGE_SPI)// boot from EEPROM
    neo430_uart_br_print("Loading... ");

  // check if valid image
  if (get_image_word(0x0000, src) != 0xCAFE) {
    system_error(ERROR_EXECUTABLE);
  }

  // image size & check
  uint16_t size  = get_image_word(0x0002, src);
  uint16_t check = get_image_word(0x0004, src);
  uint16_t end = IMEM_SIZE;
  if (size > end)
    system_error(ERROR_SIZE);

  // transfer program data
  uint16_t *pnt = (uint16_t*)0x0000;
  uint16_t addr = 0x0006;
  uint16_t checksum = 0;
  uint16_t d = 0, i = 0;
  while (i < size/2) { // in words
    d = get_image_word(addr, src);
    checksum ^= d;
    *pnt++ = d;
    i++;
    addr += 2;
  }

  // clear rest of IMEM
  while(i < end/2) // in words
    pnt[i++] = 0x0000;

  // error during transfer?
  if (checksum == check) {
    neo430_uart_br_print("OK");
  }
  else
    system_error(ERROR_CHECKSUM);
}


/* ------------------------------------------------------------
 * INFO Get image word from SPI_EEPROM or UART
 * PARAM a source address (16 bit)
 * PARAM src: 0: UART, 1: SPI_EEPROM
 * RETURN accessed data word
 * ------------------------------------------------------------ */
uint16_t get_image_word(uint16_t a, uint8_t src) {

  uint8_t c0 = 0, c1 = 0;

  // reads have to be consecutive when reading from the UART
  if (src == UART_IMAGE) { // get image data via UART
    c0 = (uint8_t)neo430_uart_getc();
    c1 = (uint8_t)neo430_uart_getc();
  }
  else {// if (src == EEPROM_IMAGE_SPI) { // get image data from SPI EEPROM
    c0 = spi_eeprom_read_byte(a+0);
    c1 = spi_eeprom_read_byte(a+1);
  }
//else { // if (src == EEPROM_IMAGE_TWI) // get image data from TWI EEPROM
//  //c0 = twi_eeprom_read_byte(a+0);
//  //c1 = twi_eeprom_read_byte(a+1);
//}

  //uint16_t r = (((uint16_t)c0) << 8) | (((uint16_t)c1) << 0);
  uint16_t r = neo430_combine_bytes(c0, c1);

  return r;
}


/* ------------------------------------------------------------
 * INFO Print error message, light up status LED and freeze system
 * INFO "naked" since this is final
 * PARAM error code
 * ------------------------------------------------------------ */
void __attribute__((__naked__)) system_error(uint8_t err_code){

  neo430_uart_br_print("\a\nERR_"); // output error code with annoying bell sound
  neo430_uart_print_hex_byte(err_code);

  asm volatile ("mov #0, r2"); // deactivate IRQs, no more write access to IMEM
  neo430_gpio_port_set(1<<STATUS_LED); // permanently light up status LED

  while(1); // freeze
}
