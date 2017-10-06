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
// #  Stephan Nolting, Hannover, Germany                                               06.10.2017  #
// #################################################################################################


// Libraries
#include <stdint.h>
#include "../../lib/neo430/neo430.h"

// Configuration
#define BAUD_RATE 19200

// Prototypes
void verify16(char *s, uint16_t a, uint16_t b);
void verify8(char *s, uint8_t a, uint8_t b);


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  uart_set_baud(BAUD_RATE);
  USI_CT = (1<<USI_CT_EN);

  // intro text
  uart_br_print("\nCustom Functions Unit (CFU) test program\n");

  // check if CFU present
  if (!(SYS_FEATURES & (1<<SYS_CFU_EN))) {
    uart_br_print("Error! No CFU synthesized!");
    return 1;
  }

  // wait for user to start
  uart_br_print("Press any key to start.\n\n");
  while (uart_char_received() == 0);

  // initialize test vectors
  uint8_t  test_data8[16];
  uint16_t test_data16[8];
  uint8_t i;
  for (i=0; i<8; i++)
    test_data16[i] = i*1728 + 913;
  for (i=0; i<16; i++)
    test_data8[i] = i*10 + 5;


  // 16-bit access tests
  // ---------------------------------------
  uart_br_print("Testing 16-bit access...\n");

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


  // 8-bit access tests
  // ---------------------------------------
  uart_br_print("\nTesting 8-bit access...\n");

  CFU_REG0_LO = test_data8[0];
  verify8("CFU_REG0_LO", CFU_REG0_LO, test_data8[0]);
  CFU_REG0_HI = test_data8[1];
  verify8("CFU_REG0_HI", CFU_REG0_HI, test_data8[1]);

  CFU_REG1_LO = test_data8[2];
  verify8("CFU_REG1_LO", CFU_REG1_LO, test_data8[2]);
  CFU_REG1_HI = test_data8[3];
  verify8("CFU_REG1_HI", CFU_REG1_HI, test_data8[3]);

  CFU_REG2_LO = test_data8[4];
  verify8("CFU_REG2_LO", CFU_REG2_LO, test_data8[4]);
  CFU_REG2_HI = test_data8[5];
  verify8("CFU_REG2_HI", CFU_REG2_HI, test_data8[5]);

  CFU_REG3_LO = test_data8[6];
  verify8("CFU_REG3_LO", CFU_REG3_LO, test_data8[6]);
  CFU_REG3_HI = test_data8[7];
  verify8("CFU_REG3_HI", CFU_REG3_HI, test_data8[7]);

  CFU_REG4_LO = test_data8[8];
  verify8("CFU_REG4_LO", CFU_REG4_LO, test_data8[8]);
  CFU_REG4_HI = test_data8[9];
  verify8("CFU_REG4_HI", CFU_REG4_HI, test_data8[9]);

  CFU_REG5_LO = test_data8[10];
  verify8("CFU_REG5_LO", CFU_REG5_LO, test_data8[10]);
  CFU_REG5_HI = test_data8[11];
  verify8("CFU_REG5_HI", CFU_REG5_HI, test_data8[11]);

  CFU_REG6_LO = test_data8[12];
  verify8("CFU_REG6_LO", CFU_REG6_LO, test_data8[12]);
  CFU_REG6_HI = test_data8[13];
  verify8("CFU_REG6_HI", CFU_REG6_HI, test_data8[13]);

  CFU_REG7_LO = test_data8[14];
  verify8("CFU_REG7_LO", CFU_REG7_LO, test_data8[14]);
  CFU_REG7_HI = test_data8[15];
  verify8("CFU_REG7_HI", CFU_REG7_HI, test_data8[15]);

  return 0;
}


/* ------------------------------------------------------------
 * INFO Verify 16-bit transfer
 * ------------------------------------------------------------ */
void verify16(char *s, uint16_t a, uint16_t b) {

  uart_br_print("Checking ");
  uart_br_print(s);
  uart_br_print(" - expected: 0x");
  uart_print_hex_word(b);
  uart_br_print(", received: 0x");
  uart_print_hex_word(a);

  if (a == b)
    uart_br_print(" - OK\n");
  else
    uart_br_print(" - FAILED\n");
}


/* ------------------------------------------------------------
 * INFO Verify 8-bit transfer
 * ------------------------------------------------------------ */
void verify8(char *s, uint8_t a, uint8_t b) {

  uart_br_print("Checking ");
  uart_br_print(s);
  uart_br_print(" - expected: 0x");
  uart_print_hex_byte(b);
  uart_br_print(", received: 0x");
  uart_print_hex_byte(a);

  if (a == b)
    uart_br_print(" - OK\n");
  else
    uart_br_print(" - FAILED\n");
}
