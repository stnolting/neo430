// #################################################################################################
// #  < Wishbone bus explorer >                                                                    #
// # ********************************************************************************************* #
// # Manual access to the registers of modules, which are connected to Wishbone bus. This is also  #
// # a neat example to illustrate the construction of a console-like user interface.               #
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
// #  Stephan Nolting, Hannover, Germany                                               06.10.2017  #
// #################################################################################################


// Libraries
#include <stdint.h>
#include <string.h>
#include "../../lib/neo430/neo430.h"

// Global variables
uint8_t wb_config = 0;

// Prototypes
void setup_wb(void);
void read_wb_address(void);
void write_wb_address(void);
void dump_wb(void);
uint32_t hex_str_to_uint32(char *buffer);

// Configuration
#define MAX_CMD_LENGTH 16
#define BAUD_RATE 19200


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  uart_set_baud(BAUD_RATE);
  USI_CT = (1<<USI_CT_EN);

  char buffer[MAX_CMD_LENGTH];
  uint16_t length = 0;
  uint16_t selection = 0;

  uart_br_print("\n--------------------------------------\n"
                  "--- Wishbone Bus Explorer Terminal ---\n"
                  "--------------------------------------\n\n");

  // check if WB unit was synthesized, exit if no WB is available
  if (!(SYS_FEATURES & (1<<SYS_WB32_EN))) {
    uart_br_print("Error! No Wishbone adapter synthesized!");
    return 1;
  }

  // default config
  wb_config = 1;

  uart_br_print("Configure the actual data transfer size (1, 2 or 4 bytes)\n"
                "using 'setup'. Addresses are always 32-bit wide.\n"
                "Type 'help' to see the help menu.\n\n"

                "Note: Do not access unused addreses! This program uses\n"
                "blocking accesses, so performing transactions to unimplemented\n"
                "addresses will permanently stall the program!\n");

  // Main menu
  for (;;) {
    uart_br_print("WB_EXPLORER:> ");
    length = uart_scan(buffer, MAX_CMD_LENGTH);
    uart_br_print("\n");

    if (!length) // nothing to be done
     continue;

    // decode input
    selection = 0;
    if (!strcmp(buffer, "help"))
      selection = 1;
    if (!strcmp(buffer, "setup"))
      selection = 2;
    if (!strcmp(buffer, "read"))
      selection = 3;
    if (!strcmp(buffer, "write"))
      selection = 4;
    if (!strcmp(buffer, "dump"))
      selection = 5;
    if (!strcmp(buffer, "reset"))
      selection = 6;
    if (!strcmp(buffer, "exit"))
      selection = 7;

    // execute command
    switch(selection) {

      case 1: // print help menu
        uart_br_print("Available commands:\n"
                      " help  - show this text\n"
                      " setup - configure WB interface\n"
                      " read  - read from WB address\n"
                      " write - write to WB address\n"
                      " dump  - dump data from WB address\n"
                      " reset - perform soft-reset\n"
                      " exit  - exit program and return to bootloader\n");
        break;

      case 2: // setup Wishbone adapter
        setup_wb();
        break;

      case 3: // read from address
        read_wb_address();
        break;

      case 4: // write to address
        write_wb_address();
        break;

      case 5: // dump data
        dump_wb();
        break;

      case 6: // restart
        while ((USI_CT & (1<<USI_CT_UARTTXBSY)) != 0); // wait for current UART transmission
        soft_reset();
        break;

      case 7: // goto bootloader
        if (!(SYS_FEATURES & (1<<SYS_BTLD_EN)))
          uart_br_print("No bootloader installed!\n");
        else
          asm volatile ("mov #0xF000, r0");
        break;

      default: // invalid command
        uart_br_print("Invalid command. Type 'help' to see all commands.\n");
        break;
    }
  }

  return 0;
}


/* ------------------------------------------------------------
 * INFO Configure Wishbone adapter
 * ------------------------------------------------------------ */
void setup_wb(void) {

  char buffer[2];

  uart_br_print("Select transfer size in bytes (1,2,4): ");
  uart_scan(buffer, 2);

  // process input
  if (!strcmp(buffer, "1"))
    wb_config = 1;
  else if (!strcmp(buffer, "2"))
    wb_config = 2;
  else if (!strcmp(buffer, "4"))
    wb_config = 4;
  else {
    uart_br_print("\nInvalid input. Cancelling setup.\n");
    return;
  }

  uart_br_print("\nSetup done.\n");
}


/* ------------------------------------------------------------
 * INFO Read data from Wishbone address
 * ------------------------------------------------------------ */
void read_wb_address(void) {

  char buffer[9];

  uart_br_print("Enter hexadecimal target address: 0x");
  uart_scan(buffer, 9); // 8 hex chars for address plus '\0'
  uint32_t address = hex_str_to_uint32(buffer);

  uart_br_print("\nReading from [0x");
  uart_print_hex_dword(address);
  uart_br_print("]... ");

  uint8_t r_data8 = 0;
  uint16_t r_data16 = 0;
  uint32_t r_data32 = 0;

  // perform access
  if (wb_config == 1)
    r_data8 = wishbone_read8(address);
  if (wb_config == 2)
    r_data16 = wishbone_read16(address);
  if (wb_config == 4)
    r_data32 = wishbone_read32(address);

  // print result
  uart_br_print("Read data: 0x");
  if (wb_config == 1)
    uart_print_hex_byte(r_data8);
  if (wb_config == 2)
    uart_print_hex_word(r_data16);
  if (wb_config == 4)
    uart_print_hex_dword(r_data32);
  uart_br_print("\n");
}


/* ------------------------------------------------------------
 * INFO Write data to Wishbone address
 * ------------------------------------------------------------ */
void write_wb_address(void) {

  char buffer[9];

  uart_br_print("Enter hexadecimal target address: 0x");
  uart_scan(buffer, 9); // 8 hex chars for address plus '\0'
  uint32_t address = hex_str_to_uint32(buffer);

  uart_br_print("\nEnter hexadecimal write data: 0x");
  uart_scan(buffer, wb_config*2+1); // get right number of hex chars for data plus '\0'
  uint32_t data = hex_str_to_uint32(buffer);

  uart_br_print("\nWriting '0x");
  uart_print_hex_dword(data);
  uart_br_print("' to [0x");
  uart_print_hex_dword(address);
  uart_br_print("]... ");

  uint8_t w_data8 = (uint8_t)data;
  uint16_t w_data16 = (uint16_t)data;
  uint32_t w_data32 = data;

  // perform access
  if (wb_config == 1)
    wishbone_write8(address, w_data8);
  if (wb_config == 2)
    wishbone_write16(address, w_data16);
  if (wb_config == 4)
    wishbone_write32(address, w_data32);

  uart_br_print("Done.\n"); 
}


/* ------------------------------------------------------------
 * INFO Dump data from Wishbone address
 * ------------------------------------------------------------ */
void dump_wb(void) {

  char buffer[9];
  uint16_t i = 0;

  uart_br_print("Enter hexadecimal start address: 0x");
  uart_scan(buffer, 9); // 8 hex chars for address plus '\0'
  uint32_t address = hex_str_to_uint32(buffer);

  uart_br_print("\nPress any key to start.\n"
                "You can abort dumping by pressing any key.\n");
  while(!uart_char_received());

  uint8_t r_data8 = 0;
  uint16_t r_data16 = 0;
  uint32_t r_data32 = 0;

  while(1) {
    uart_br_print("0x");
    uart_print_hex_dword(address);
    uart_br_print(":  ");

    uint16_t border = 16 / wb_config;
    for (i=0; i<border; i++) {

      // perform access
      if (wb_config == 1)
        r_data8 = wishbone_read8(address);
      if (wb_config == 2)
        r_data16 = wishbone_read16(address);
      if (wb_config == 4)
        r_data32 = wishbone_read32(address);

      if (wb_config == 1) {
        uart_print_hex_byte(r_data8);
        address += 1;
      }
      else if (wb_config == 2) {
        uart_print_hex_word(r_data16);
        address += 2;
      }
      else {
        uart_print_hex_dword(r_data32);
        address += 4;
      }
      uart_putc(' ');

    }
    uart_br_print("\n");
    if (uart_char_received()) // abort
      return;
  }
}


/* ------------------------------------------------------------
 * INFO Hex-char-string conversion function
 * PARAM String with hex-chars (zero-terminated)
 * not case-sensitive, non-hex chars are treated as '0'
 * RETURN Conversion result (32-bit)
 * ------------------------------------------------------------ */
uint32_t hex_str_to_uint32(char *buffer) {

  uint16_t length = strlen(buffer);
  uint32_t res = 0, d = 0;
  char c = 0;

  while (length--) {
    c = *buffer++;

    if ((c >= '0') && (c <= '9'))
      d = (uint32_t)(c - '0');
    else if ((c >= 'a') && (c <= 'f'))
      d = (uint32_t)((c - 'a') + 10);
    else if ((c >= 'A') && (c <= 'F'))
      d = (uint32_t)((c - 'A') + 10);
    else
      d = 0;

    res = res + (d << (length*4));
  }

  return res;
}
