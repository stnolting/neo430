// #################################################################################################
// #  < TRNG test program >                                                                        #
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
// # Stephan Nolting, Hannover, Germany                                                03.08.2018 #
// #################################################################################################


// Libraries
#include <stdint.h>
#include <string.h>
#include "../../lib/neo430/neo430.h"

// Configuration
#define BAUD_RATE 19200


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  char input_buffer[10];
  uint16_t data;
  uint32_t num, eternal;

  // setup UART
  neo430_uart_set_baud(BAUD_RATE);
  USI_CT = (1<<USI_CT_EN);

  // intro text
  _neo430_printf("\nTrue Random Number Generator (TRNG) test program\n");

  // check if TRNG unit was synthesized, exit if no TRNG controller is available
  if (!(SYS_FEATURES & (1<<SYS_TRNG_EN))) {
    _neo430_printf("Error! No TRNG synthesized!");
    return 1;
  }

  neo430_gpio_port_set(0); // clear GPIO output port

  while(1) {
    _neo430_printf("\nEnter number (8 hex chars) of displayed random numbers (0=infinite): 0x");
    neo430_uart_scan(input_buffer, 8+1, 1);
    num = neo430_hexstr_to_uint(input_buffer, 8);
    if (num == 0)
      eternal = 1;
    else
      eternal = 0;

    // wait for any key
    _neo430_printf("\nPress any key to start/stop.\n\n");
    while(!neo430_uart_char_received());

    // intro header
    _neo430_printf("#==============================================\n");
    _neo430_printf("# generator NEO430TRNG\n");
    _neo430_printf("#==============================================\n");
    _neo430_printf("type: d\n");
    _neo430_printf("count: %l\n", num);
    _neo430_printf("numbit: 8\n");

    neo430_trng_enable();
    while (num-- || eternal) {
      data = (uint16_t)neo430_trng_get_byte();
      neo430_gpio_port_set(data & 1); // output LSB
      _neo430_printf("%u\n", data);

      // stop?
      if (neo430_uart_char_received()) { // any key input?
        neo430_trng_disable();
        break;
      }
    }
  }

  return 0;
}