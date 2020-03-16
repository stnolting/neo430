// #################################################################################################
// #  < CRC unit test program >                                                                    #
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

