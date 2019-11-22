// #################################################################################################
// #  < CRC unit test program >                                                                    #
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
// # Stephan Nolting, Hannover, Germany                                                 17.11.2018 #
// #################################################################################################


// Libraries
#include <stdint.h>
#include <string.h>
#include <neo430.h>

// Configuration
#define BAUD_RATE 19200


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  char buffer[256];

  // CRC results were validated using http://crccalc.com/

  // setup UART
  neo430_uart_setup(BAUD_RATE);

  neo430_uart_br_print("\n<<< CRC16/CRC32 module test >>>\n");

  // check if CRC unit was synthesized, exit if not available
  if (!(SYS_FEATURES & (1<<SYS_CRC_EN))) {
    neo430_uart_br_print("Error! No CRC unit synthesized!");
    return 1;
  }

  // get start values
  neo430_uart_br_print("\nEnter start value for CRC16 shift register (4hex): 0x");
  neo430_uart_scan(buffer, 4+1, 1);
  uint16_t crc16_start = (uint16_t)neo430_hexstr_to_uint(buffer, 4);

  neo430_uart_br_print("\nEnter start value for CRC32 shift register (8hex): 0x");
  neo430_uart_scan(buffer, 8+1, 1);
  uint32_t crc32_start = neo430_hexstr_to_uint(buffer, 8);

  // get polynomial masks
  neo430_uart_br_print("\nEnter polynomial mask for CRC16 (4hex): 0x");
  neo430_uart_scan(buffer, 4+1, 1);
  uint16_t crc16_poly = (uint16_t)neo430_hexstr_to_uint(buffer, 4);

  neo430_uart_br_print("\nEnter polynomial mask for CRC32 (8hex): 0x");
  neo430_uart_scan(buffer, 8+1, 1);
  uint32_t crc32_poly = neo430_hexstr_to_uint(buffer, 8);

  while(1) {
    // get actual data
    neo430_uart_br_print("\nEnter text for CRC16/32 computation: ");
    neo430_uart_scan(buffer, 255, 1);
    uint16_t crc16_res = neo430_crc16(crc16_start, crc16_poly, (uint8_t*)buffer, strlen(buffer));
    uint32_t crc32_res = neo430_crc32(crc32_start, crc32_poly, (uint8_t*)buffer, strlen(buffer));

    // show results
    neo430_uart_br_print("\nCRC16 = 0x");
    neo430_uart_print_hex_word(crc16_res);
    neo430_uart_br_print("\nCRC32 = 0x");
    neo430_uart_print_hex_word((uint16_t)(crc32_res >> 16));
    neo430_uart_print_hex_word((uint16_t)crc32_res);

    neo430_uart_br_print("\n");
  }

  return 0;
}

