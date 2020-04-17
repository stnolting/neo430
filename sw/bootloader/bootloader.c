// #################################################################################################
// #  < NEO430 Bootloader >                                                                        #
// # ********************************************************************************************* #
// # Boot from IMEM, UART or SPI Flash at SPI.CS[0]                                                #
// #                                                                                               #
// # UART configuration: 8N1 at 19200 baud                                                         #
// # Boot Flash: 8-bit SPI, 24-bit addresses (like Micron N25Q032A) @ NEO430_SPI.CS[0]             #
// # NEO430_GPIO.out[0] is used as high-active status LED                                          #
// #                                                                                               #
// # Auto boot sequence after timeout:                                                             #
// #  -> Try booting from SPI flash at SPI.CS[0]                                                   #
// #  -> Permanently light up status led and freeze if SPI flash booting attempt fails             #
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

// Libraries
#include <stdint.h>
#include <neo430.h>

// SPI flash: NEO430 boot base address
#define SPI_FLASH_BOOT_ADR  0x00040000L

// Configuration
#define BAUD_RATE        19200 // default UART baud rate
#define AUTOBOOT_TIMEOUT 4     // countdown (seconds) to auto boot
#define STATUS_LED       0     // GPIO.out(0) is status LED

// SPI flash hardware configuration
#define SPI_FLASH_CS     0

// SPI flash commands
#define SPI_FLASH_CMD_READ           0x03
#define SPI_FLASH_CMD_READ_STATUS    0x05
#define SPI_FLASH_CMD_WRITE_ENABLE   0x06
#define SPI_FLASH_CMD_PAGE_PROGRAM   0x02
#define SPI_FLASH_CMD_SECTOR_ERASE   0xD8
#define SPI_FLASH_CMD_READ_ID        0x9E
#define SPI_FLASH_CMD_POWER_DOWN     0xB9
#define SPI_FLASH_CMD_RELEASE        0xAB

// Image sources
#define UART_IMAGE       0x00
#define EEPROM_IMAGE_SPI 0x01

// Error codes
#define ERROR_EEPROM     0x00 // EEPROM access error
#define ERROR_ROMACCESS  0x01 // cannot write to IMEM
#define ERROR_EXECUTABLE 0x02 // invalid executable format
#define ERROR_SIZE       0x04 // executable is too big
#define ERROR_CHECKSUM   0x08 // checksum error

// Scratch registers - abuse unused IRQ vectors for this ;)
#define TIMEOUT_CNT IRQVEC_GPIO

// Macros
#define xstr(a) str(a)
#define str(a) #a
#define SPI_FLASH_SEL {SPI_CT |= 1 << (SPI_FLASH_CS+SPI_CT_CS_SEL0);}

// Function prototypes
void     __attribute__((__interrupt__)) timer_irq_handler(void);
void     __attribute__((__naked__)) start_app(void);
void     print_help(void);
void     store_eeprom(void);
void     eeprom_write_word(uint32_t a, uint16_t d);
void     get_image(uint8_t src);
uint16_t get_image_word(uint32_t a, uint8_t src);
void     __attribute__((__naked__)) system_error(uint8_t err_code);

// Function prototypes - SPI flash
uint8_t spi_flash_read_byte();
void    spi_flash_write_byte(uint32_t adr, uint8_t data);
void    spi_flash_erase_sector(uint32_t base_adr);
uint8_t spi_flash_read_status(void);
void    spi_flash_write_cmd(uint16_t cmd);
uint8_t spi_flash_read_1st_id(void);
void    spi_flash_adr_conv(uint32_t adr, uint16_t *hi, uint16_t *mi, uint16_t *lo);


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

  // disable FREQ_GEN
  FREQ_GEN_CT = 0;

  // init GPIO
  GPIO_IRQMASK = 0; // no pin change interrupt please, thanks
  neo430_gpio_port_set(1<<STATUS_LED); // activate status LED, clear all others

  // init interrupt vectors
  IRQVEC_TIMER  = (uint16_t)(&timer_irq_handler); // timer match
//IRQVEC_EXT    = 0; // unused
//IRQVEC_SERIAL = 0; // unused

  // set Baud rate & init UART control register:
  // enable UART, no IRQs
  neo430_uart_setup(BAUD_RATE);
  neo430_uart_char_read(); // clear UART RX buffer

  // set SPI config:
  // enable SPI, no IRQ, MSB first, 8-bit mode, SPI clock mode 0, set SPI speed, disable all SPI CS lines (set high)
  neo430_spi_enable(SPI_PRSC_8); // this also resets the SPI module

  // Timeout counter: init timer, irq tick @ ~1Hz (prescaler = 4096)
  // THR = f_main / (1Hz + 4096) -1
  TMR_CT = 0; // reset timer
  //uint32_t clock = CLOCKSPEED_32bit >> 14; // divide by 4096
  TMR_THRES = (CLOCKSPEED_HI << 2) -1; // "fake" ;D
  // enable timer, auto reset, enable IRQ, prsc = 1:2^16, start timer
  TMR_CT = (1<<TMR_CT_EN) | (1<<TMR_CT_ARST) | (1<<TMR_CT_IRQ) | ((16-1)<<TMR_CT_PRSC0) | (1<<TMR_CT_RUN);
  TIMEOUT_CNT = 0; // console timeout ticker

  neo430_eint(); // enable global interrupts


  // ****************************************************************
  // Show bootloader intro and system information
  // ****************************************************************
  neo430_uart_br_print("\n\nNEO430 Bootloader\n"
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


  // get SPI flash out of power down mode
  spi_flash_write_cmd((uint16_t)SPI_FLASH_CMD_RELEASE);


  // ****************************************************************
  // Auto boot sequence
  // ****************************************************************
  neo430_uart_br_print("\n\nAutoboot in "xstr(AUTOBOOT_TIMEOUT)"s. Press key to abort.\n\n");
  while (1) { // wait for any key to be pressed or timeout

    // timeout? start auto boot sequence
    if (TIMEOUT_CNT == 4*AUTOBOOT_TIMEOUT) { // in 0.25 seconds
      get_image(EEPROM_IMAGE_SPI); // try loading from EEPROM
      neo430_uart_br_print("\n");
      start_app(); // start app
    }

    // key pressed? -> enter user console
    if ((UART_RTX & (1<<UART_RTX_AVAIL)) != 0) {
      break;
    }
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

    if (c == 'r') { // restart bootloader
      asm volatile ("mov #0xF000, r0"); // jump to beginning of bootloader ROM
    }
    else if (c == 'h') { // help menu
      print_help();
    }
    else if (c == 'u') { // upload program to RAM via UART
      get_image(UART_IMAGE);
    }
    else if (c == 'p') { // program EEPROM from RAM
      store_eeprom();
    }
    else if (c == 'e') { // start program in RAM
      start_app();
    }
    else { // unknown command
      neo430_uart_br_print("Bad CMD");
    }
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

  // put SPI flah into power-down mode
  spi_flash_write_cmd((uint16_t)SPI_FLASH_CMD_POWER_DOWN);

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
                       "h: Help\n"
                       "r: Restart\n"
                       "u: Upload\n"
                       "p: Prog\n"
                       "e: Execute");
}


/* ------------------------------------------------------------
 * INFO Store full IMEM content to SPI EEPROM at SPI.CS0
 * ------------------------------------------------------------ */
void store_eeprom(void) {

  neo430_uart_br_print("...");

  // clear memory before writing
  spi_flash_erase_sector(SPI_FLASH_BOOT_ADR);

  // check if eeprom ready (or available at all)
  if (spi_flash_read_1st_id() == 0x00) { // manufacturer ID
    system_error(ERROR_EEPROM);
  }

  // write EXE signature
  eeprom_write_word(SPI_FLASH_BOOT_ADR + 0, 0xCAFE);

  // write size
  uint16_t end = IMEM_SIZE;
  eeprom_write_word(SPI_FLASH_BOOT_ADR + 2, end);

  // store data from IMEM and update checksum
  uint16_t checksum = 0;
  uint16_t i = 0;
  uint16_t *pnt = (uint16_t*)0x0000;

  while (i < end) {
    uint16_t d = (uint16_t)*pnt++;
    checksum ^= d;
    eeprom_write_word(SPI_FLASH_BOOT_ADR + 6 + i, d);
    i+= 2;
  }

  // write checksum
  eeprom_write_word(SPI_FLASH_BOOT_ADR + 4, checksum);

  neo430_uart_br_print("OK");
}


/* ------------------------------------------------------------
 * INFO EEPROM write data word
 * PARAM a destination address (24 bit effective)
 * PARAM d word to be written
 * ------------------------------------------------------------ */
void eeprom_write_word(uint32_t a, uint16_t d) {

  uint8_t lo = (uint8_t)(d);
  uint8_t hi = (uint8_t)neo430_bswap(d);
  
  spi_flash_write_byte(a+0, hi);
  spi_flash_write_byte(a+1, lo);
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
  if (src == UART_IMAGE) { // boot via UART
    neo430_uart_br_print("Awaiting BINEXE...");
  }
  else { //if (src == EEPROM_IMAGE_SPI)// boot from EEPROM
    neo430_uart_br_print("Loading...");
  }

  // check if valid image
  if (get_image_word(SPI_FLASH_BOOT_ADR + 0, src) != 0xCAFE) { // signature
    system_error(ERROR_EXECUTABLE);
  }

  // image size and checksum
  uint16_t size = get_image_word(SPI_FLASH_BOOT_ADR + 2, src); // size in bytes
  uint16_t check = get_image_word(SPI_FLASH_BOOT_ADR + 4, src); // XOR checksum
  uint16_t end = IMEM_SIZE;
  if (size > end) {
    system_error(ERROR_SIZE);
  }

  // transfer program data
  uint16_t *pnt = (uint16_t*)0x0000;
  uint16_t checksum = 0x0000;
  uint16_t d = 0, i = 0;
  while (i < size/2) { // in words
    d = get_image_word(SPI_FLASH_BOOT_ADR + 2*i + 6, src);
    checksum ^= d;
    pnt[i++] = d;
  }

  // clear rest of IMEM
  while(i < end/2) { // in words
    pnt[i++] = 0x0000;
  }

  // error during transfer?
  if (checksum == check) {
    neo430_uart_br_print("OK");
  }
  else {
    system_error(ERROR_CHECKSUM);
  }
}


/* ------------------------------------------------------------
 * INFO Get image word from SPI_EEPROM or UART
 * PARAM a source address (24 bit effective)
 * PARAM src: 0: UART, 1: SPI_EEPROM
 * RETURN accessed data word
 * ------------------------------------------------------------ */
uint16_t get_image_word(uint32_t a, uint8_t src) {

  uint8_t c0 = 0, c1 = 0;

  // reads have to be consecutive when reading from the UART
  if (src == UART_IMAGE) { // get image data via UART
    c0 = (uint8_t)neo430_uart_getc();
    c1 = (uint8_t)neo430_uart_getc();
  }
  else {// if (src == EEPROM_IMAGE_SPI) { // get image data from SPI EEPROM
    c0 = spi_flash_read_byte(a+0);
    c1 = spi_flash_read_byte(a+1);
  }

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



// *************************************************************************************
// SPI flash functions
// *************************************************************************************

/* ------------------------------------------------------------
 * Read single byte from flash (24-bit adress)
 * ------------------------------------------------------------ */
uint8_t spi_flash_read_byte(uint32_t adr) {

  uint16_t adr_lo;
  uint16_t adr_mi;
  uint16_t adr_hi;
  spi_flash_adr_conv(adr, &adr_hi, &adr_mi, &adr_lo);

  SPI_FLASH_SEL;

  neo430_spi_trans((uint16_t)SPI_FLASH_CMD_READ);
  // no masking required, SPI unit in 8 bit mode ignores upper 8 bits
  neo430_spi_trans(adr_hi);
  neo430_spi_trans(adr_mi);
  neo430_spi_trans(adr_lo);
  uint16_t data = neo430_spi_trans(0);

  neo430_spi_cs_dis();

  return (uint8_t)data;
}


/* ------------------------------------------------------------
 * Write single data byte to flash at base adress
 * ------------------------------------------------------------ */
void spi_flash_write_byte(uint32_t adr, uint8_t data) {

  uint16_t adr_lo;
  uint16_t adr_mi;
  uint16_t adr_hi;
  spi_flash_adr_conv(adr, &adr_hi, &adr_mi, &adr_lo);

  spi_flash_write_cmd((uint16_t)SPI_FLASH_CMD_WRITE_ENABLE); // allow write-access

  SPI_FLASH_SEL;

  neo430_spi_trans((uint16_t)SPI_FLASH_CMD_PAGE_PROGRAM);
  // no masking required, SPI unit in 8 bit mode ignores upper 8 bits
  neo430_spi_trans(adr_hi);
  neo430_spi_trans(adr_mi);
  neo430_spi_trans(adr_lo);
  neo430_spi_trans((uint16_t)data);

  neo430_spi_cs_dis();

  while(spi_flash_read_status());
}


/* ------------------------------------------------------------
 * Erase sector (64kB) at base adress
 * ------------------------------------------------------------ */
void spi_flash_erase_sector(uint32_t base_adr) {

  uint16_t adr_lo;
  uint16_t adr_mi;
  uint16_t adr_hi;
  spi_flash_adr_conv(base_adr, &adr_hi, &adr_mi, &adr_lo);

  spi_flash_write_cmd((uint16_t)SPI_FLASH_CMD_WRITE_ENABLE); // allow write-access

  SPI_FLASH_SEL;

  neo430_spi_trans((uint16_t)SPI_FLASH_CMD_SECTOR_ERASE);
  // no masking required, SPI unit in 8 bit mode ignores upper 8 bits
  neo430_spi_trans(adr_hi);
  neo430_spi_trans(adr_mi);
  neo430_spi_trans(adr_lo);

  neo430_spi_cs_dis();

  while(spi_flash_read_status());
}


/* ------------------------------------------------------------
 * Read status register
 * ------------------------------------------------------------ */
uint8_t spi_flash_read_status(void) {

  SPI_FLASH_SEL;

  neo430_spi_trans((uint16_t)SPI_FLASH_CMD_READ_STATUS);
  uint16_t status = neo430_spi_trans(0);

  neo430_spi_cs_dis();

  return (uint8_t)status;
}


/* ------------------------------------------------------------
 * Read first byte of ID (manufacturer ID), should be != 0x00
 * ------------------------------------------------------------ */
uint8_t spi_flash_read_1st_id(void) {

  SPI_FLASH_SEL;

  neo430_spi_trans((uint16_t)SPI_FLASH_CMD_READ_ID);
  uint16_t id = neo430_spi_trans(0);

  neo430_spi_cs_dis();

  return (uint8_t)id;
}


/* ------------------------------------------------------------
 * Write command to flash
 * ------------------------------------------------------------ */
void spi_flash_write_cmd(uint16_t cmd) {

  SPI_FLASH_SEL;

  neo430_spi_trans(cmd);

  neo430_spi_cs_dis();
}


/* ------------------------------------------------------------
 * Adress conversion helper
 * ------------------------------------------------------------ */
void spi_flash_adr_conv(uint32_t adr, uint16_t *hi, uint16_t *mi, uint16_t *lo) {

  uint16_t adr_hi16 = (uint16_t)(adr >> 16);
  uint16_t adr_lo16 = (uint16_t)(adr >>  0);

  *lo = adr_lo16;
  *mi = neo430_bswap(adr_lo16);
  *hi = adr_hi16;
}

