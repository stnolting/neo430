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
// #  Stephan Nolting, Hannover, Germany                                               25.04.2018  #
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

  // setup UART
  uart_set_baud(BAUD_RATE);
  USI_CT = (1<<USI_CT_EN);

  // intro text
  _printf("\nTrue Random Number Generator (TRNG) test program\n\n");

  // check if TRNG unit was synthesized, exit if no TRNG controller is available
  if (!(SYS_FEATURES & (1<<SYS_TRNG_EN))) {
    _printf("Error! No TRNG synthesized!");
    return 1;
  }

  while(1) {
    // wait for any key
    _printf("\n\nPress any key to start/stop\n\n");
    while(!uart_char_received());

    trng_enable();
    while(1) {
      _printf("%u, ", (uint16_t)trng_get_byte());

      // stop?
      if (uart_char_received()) { // any key input?
        trng_disable();
        break;
      }
    }
  }

  return 0;
}