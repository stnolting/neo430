// #################################################################################################
// #  < Custom Functions Unit Test Program >                                                       #
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
// # Stephan Nolting, Hannover, Germany                                                 17.11.2018 #
// #################################################################################################


// Libraries
#include <stdint.h>
#include <neo430.h>

// Configuration
#define BAUD_RATE 19200

// Prototypes
void verify16(char *s, uint16_t a, uint16_t b);


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  neo430_uart_setup(BAUD_RATE);

  // intro text
  neo430_uart_br_print("\nCustom Functions Unit (CFU) test program\n");

  // check if CFU present
  if (!(SYS_FEATURES & (1<<SYS_CFU_EN))) {
    neo430_uart_br_print("Error! No CFU synthesized!");
    return 1;
  }

  // wait for user to start
  neo430_uart_br_print("Press any key to start.\n\n");
  while (neo430_uart_char_received() == 0);

  // initialize test vectors
  uint8_t i;
  uint16_t test_data16[16];
  for (i=0; i<8; i++)
    test_data16[i] = i*1728 + 913;


  neo430_uart_br_print("Testing read/write accesses...\n");

  CFU_REG0 = test_data16[0];
  verify16("CFU_REG0", CFU_REG0, test_data16[0]);

  CFU_REG1 = test_data16[1];
  verify16("CFU_REG1", CFU_REG1, test_data16[1]);

  CFU_REG2 = test_data16[2];
  verify16("CFU_REG2", CFU_REG2, test_data16[2]);

  CFU_REG3 = test_data16[3];
  verify16("CFU_REG3", CFU_REG3, test_data16[3]);

  CFU_REG4 = test_data16[4];
  verify16("CFU_REG4", CFU_REG4, test_data16[4]);

  CFU_REG5 = test_data16[5];
  verify16("CFU_REG5", CFU_REG5, test_data16[5]);

  CFU_REG6 = test_data16[6];
  verify16("CFU_REG6", CFU_REG6, test_data16[6]);

  CFU_REG7 = test_data16[7];
  verify16("CFU_REG7", CFU_REG7, test_data16[7]);

  return 0;
}


/* ------------------------------------------------------------
 * INFO Verify 16-bit transfer
 * ------------------------------------------------------------ */
void verify16(char *s, uint16_t a, uint16_t b) {

  neo430_uart_br_print("Checking ");
  neo430_uart_br_print(s);
  neo430_uart_br_print(" - expected: 0x");
  neo430_uart_print_hex_word(b);
  neo430_uart_br_print(", received: 0x");
  neo430_uart_print_hex_word(a);

  if (a == b)
    neo430_uart_br_print(" - OK\n");
  else
    neo430_uart_br_print(" - FAILED\n");
}

