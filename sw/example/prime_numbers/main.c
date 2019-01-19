// #################################################################################################
// #  < Computes and prints prime numbers >                                                        #
// # ********************************************************************************************* #
// # Computes all primes numbers between 3 and 2^32-1 using the harderr                            #
// # approximation way ;) The printed numbers are formated using sprintf from                      #
// # the C std library <stdio.h>                                                                   #
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
// # Stephan Nolting, Hannover, Germany                                                17.11.2018 #
// #################################################################################################


// Libraries
#include <stdint.h>
#include <neo430.h>

// Configuration
#define BAUD_RATE 19200

/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  neo430_uart_setup(BAUD_RATE);

  // intro text
  neo430_printf("\n\nGenerating prime numbers between 3 and %n", 0xFFFFFFFF);
  neo430_printf(".\n"
                 "Press any key to start.\n"
                 "You can abort the program by pressing any key again.\n");

  // wait for any key
  while(!neo430_uart_char_received());

  uint32_t n = 0, i = 0;
  uint8_t is_prime = 0;

  // generate candidates
  for (n=3; n!=0xFFFFFFFF; n+=2) {

    // check if prime
    is_prime = 1;
    for (i=2; i<=(n>>1); i++) {
      if (n%i == 0) {
        is_prime = 0;
        break;
      }
    }

    // output prime number in decimal representation
    if (is_prime)
      neo430_printf("%n, ", n);

    // abort?
    if (neo430_uart_char_received()) // any key input?
      neo430_soft_reset();
  }

  return 0;
}
