// #################################################################################################
// #  < Custom Functions Unit Test Program >                                                       #
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

