// #################################################################################################
// #  < Wishbone bus explorer >                                                                    #
// # ********************************************************************************************* #
// # Manual access to the registers of modules, which are connected to the Wishbone bus. This tool #
// # uses NONBLOCKING Wishbone transactions.                                                       #
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
// # Stephan Nolting, Hannover, Germany                                                 10.10.2019 #
// #################################################################################################


// Libraries
#include <stdint.h>
#include <string.h>
#include <neo430.h>

// Global variables
uint8_t wb_config = 0;

// Prototypes
void setup_wb(void);
void read_wb_address(void);
void write_wb_address(void);
void dump_wb(void);

// Configuration
#define MAX_CMD_LENGTH 16
#define BAUD_RATE 19200


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  neo430_uart_setup(BAUD_RATE);

  char buffer[MAX_CMD_LENGTH];
  uint16_t length = 0;
  uint16_t selection = 0;

  neo430_uart_br_print("\n--------------------------------------\n"
                  "--- Wishbone Bus Explorer Terminal ---\n"
                  "--------------------------------------\n\n");

  // check if WB unit was synthesized, exit if no WB is available
  if (!(SYS_FEATURES & (1<<SYS_WB32_EN))) {
    neo430_uart_br_print("Error! No Wishbone adapter synthesized!");
    return 1;
  }

  // default config
  wb_config = 4;
  neo430_wishbone_terminate(); // terminate current transfer

  neo430_uart_br_print("Configure the actual data transfer size (1, 2 or 4 bytes)\n"
                       "using 'setup'. Addresses are always 32-bit wide.\n"
                       "This tool uses non-blocking Wishbone transactions.\n\n"
                       "Type 'help' to see the help menu.\n\n");

  // Main menu
  for (;;) {
    neo430_uart_br_print("WB_EXPLORER:> ");
    length = neo430_uart_scan(buffer, MAX_CMD_LENGTH, 1);
    neo430_uart_br_print("\n");

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
        neo430_uart_br_print("Available commands:\n"
                             " help  - show this text\n"
                             " setup - configure WB interface\n"
                             " read  - read from WB address\n"
                             " write - write to WB address\n"
                             " dump  - dump data from WB addresses\n"
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
        while ((UART_CT & (1<<UART_CT_TX_BUSY)) != 0); // wait for current UART transmission
        neo430_soft_reset();
        break;

      case 7: // goto bootloader
        if (!(SYS_FEATURES & (1<<SYS_BTLD_EN)))
          neo430_uart_br_print("No bootloader installed!\n");
        else
          asm volatile ("mov #0xF000, r0");
        break;

      default: // invalid command
        neo430_uart_br_print("Invalid command. Type 'help' to see all commands.\n");
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

  neo430_uart_br_print("Select transfer size in bytes (1,2,4): ");
  neo430_uart_scan(buffer, 2, 1);

  // process input
  if (!strcmp(buffer, "1"))
    wb_config = 1;
  else if (!strcmp(buffer, "2"))
    wb_config = 2;
  else if (!strcmp(buffer, "4"))
    wb_config = 4;
  else {
    neo430_uart_br_print("\nInvalid input. Cancelling setup.\n");
    return;
  }

  neo430_uart_br_print("\nSetup done.\n");
}


/* ------------------------------------------------------------
 * INFO Read data from Wishbone address
 * ------------------------------------------------------------ */
void read_wb_address(void) {

  char buffer[9];

  neo430_uart_br_print("Enter hexadecimal target address: 0x");
  neo430_uart_scan(buffer, 9, 1); // 8 hex chars for address plus '\0'
  uint32_t address = neo430_hexstr_to_uint(buffer, strlen(buffer));

  neo430_uart_br_print("\nReading from [0x");
  neo430_uart_print_hex_dword(address);
  neo430_uart_br_print("]... ");

  // print result
  neo430_uart_br_print("Read data: 0x");

  if (wb_config == 1)
    neo430_wishbone32_read8_start(address);
  else if (wb_config == 2)
    neo430_wishbone32_read16_start(address);
  else if (wb_config == 4)
    neo430_wishbone32_read32_start(address);

  // wait for transfer to finish
  uint16_t timeout = 0;
  while(1){
    if (!neo430_wishbone_busy())
      break;
    if (timeout++ == 100) {
      neo430_uart_br_print("\nError! Device not responding! Press key to proceed.\n");
      neo430_wishbone_terminate(); // terminate current transfer
      while(!neo430_uart_char_received());
      return;
    }
  }

  // read data
  if (wb_config == 1)
    neo430_uart_print_hex_byte(neo430_wishbone32_get_data8(address));
  else if (wb_config == 2)
    neo430_uart_print_hex_word(neo430_wishbone32_get_data16(address));
  else if (wb_config == 4)
    neo430_uart_print_hex_dword(neo430_wishbone32_get_data32());

  neo430_uart_br_print("\n");
}


/* ------------------------------------------------------------
 * INFO Write data to Wishbone address
 * ------------------------------------------------------------ */
void write_wb_address(void) {

  char buffer[9];

  neo430_uart_br_print("Enter hexadecimal target address: 0x");
  neo430_uart_scan(buffer, 9, 1); // 8 hex chars for address plus '\0'
  uint32_t address = neo430_hexstr_to_uint(buffer, strlen(buffer));

  neo430_uart_br_print("\nEnter hexadecimal write data: 0x");
  neo430_uart_scan(buffer, wb_config*2+1, 1); // get right number of hex chars for data plus '\0'
  uint32_t data = neo430_hexstr_to_uint(buffer, strlen(buffer));

  neo430_uart_br_print("\nWriting '0x");
  neo430_uart_print_hex_dword(data);
  neo430_uart_br_print("' to [0x");
  neo430_uart_print_hex_dword(address);
  neo430_uart_br_print("]... ");

  // perform access
  if (wb_config == 1)
    neo430_wishbone32_write8_start(address, (uint8_t)data);
  else if (wb_config == 2)
    neo430_wishbone32_write16_start(address, (uint16_t)data);
  else if (wb_config == 4)
    neo430_wishbone32_write32_start(address, data);

  // wait for transfer to finish
  uint16_t timeout = 0;
  while(1){
    if (!neo430_wishbone_busy())
      break;
    if (timeout++ == 100) {
      neo430_uart_br_print("\nError! Device not responding! Press key to proceed.\n");
      neo430_wishbone_terminate(); // terminate current transfer
      while(!neo430_uart_char_received());
      return;
    }
  }

  neo430_uart_br_print("Done.\n"); 
}


/* ------------------------------------------------------------
 * INFO Dump data from Wishbone address
 * ------------------------------------------------------------ */
void dump_wb(void) {

  char buffer[9];
  uint16_t i = 0;

  neo430_uart_br_print("Enter hexadecimal start address: 0x");
  neo430_uart_scan(buffer, 9, 1); // 8 hex chars for address plus '\0'
  uint32_t address = neo430_hexstr_to_uint(buffer, strlen(buffer));

  neo430_uart_br_print("\nPress any key to start.\n"
                       "You can abort dumping by pressing any key.\n");
  while(!neo430_uart_char_received());

  while(1) {
    neo430_uart_br_print("0x");
    neo430_uart_print_hex_dword(address);
    neo430_uart_br_print(":  ");

    uint16_t border = 16 / wb_config;
    for (i=0; i<border; i++) {

      // trigger access
      if (wb_config == 1)
        neo430_wishbone32_read8_start(address);
      else if (wb_config == 2)
        neo430_wishbone32_read16_start(address);
      else if (wb_config == 4)
        neo430_wishbone32_read32_start(address);

      // wait for transfer to finish
      uint16_t timeout = 0;
      while(1){
        if (!neo430_wishbone_busy())
          break;
        if (timeout++ == 100) {
          neo430_uart_br_print("\nError! Device not responding! Press key to proceed.\n");
          neo430_wishbone_terminate(); // terminate current transfer
          while(!neo430_uart_char_received());
          return;
        }
      }

      // read data
      if (wb_config == 1) {
        neo430_uart_print_hex_byte(neo430_wishbone32_get_data8(address));
        address += 1;
      }
      else if (wb_config == 2) {
        neo430_uart_print_hex_word(neo430_wishbone32_get_data16(address));
        address += 2;
      }
      else if (wb_config == 4) {
        neo430_uart_print_hex_dword(neo430_wishbone32_get_data32());
        address += 4;
      }

      neo430_uart_putc(' ');
    }

    neo430_uart_br_print("\n");
    if (neo430_uart_char_received()) // abort
      return;
  }
}

