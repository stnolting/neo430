// #################################################################################################
// #  < TRNG test program >                                                                        #
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
// # Stephan Nolting, Hannover, Germany                                                 27.11.2019 #
// #################################################################################################


// Libraries
#include <stdint.h>
#include <neo430.h>

// Configuration
#define BAUD_RATE 19200
#define NUM_SAMPLES 2000000000

// Global variables
uint32_t rnd_hist[256];


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  neo430_uart_setup(BAUD_RATE);

  // intro text
  neo430_printf("\n<<< TRNG Test >>>\n");

  // check if TRNG was synthesized, exit if not available
  if (!(SYS_FEATURES & (1<<SYS_TRNG_EN))) {
    neo430_printf("Error! No TRNG synthesized!");
    return 1;
  }

  // start TRNG
  neo430_trng_enable();

  while(1) {

    // main menu
    neo430_printf("\nCommands:\n"
                  " a: Print random numbers (abort by pressing any key)\n"
                  " b: Generate random data histogram (%n samples)\n"
                  " x: Return to bootloader\n", (uint32_t)NUM_SAMPLES);

    neo430_printf("CMD:> ");
    char cmd = neo430_uart_getc();
    neo430_uart_putc(cmd); // echo
    neo430_printf("\n");

    // output RND data
    if (cmd == 'a') {
      uint32_t num_samples = 0;
      while(1) {
        neo430_printf("%u ", (uint16_t)neo430_trng_get());
        num_samples++;
        if (neo430_uart_char_received()) { // abort when key pressed
          neo430_printf("\nNumber of samples: %n\n", num_samples);
          break;
        }
      }
    }

    // compute histogram
    else if (cmd == 'b') {
      // clear histogram memory
      uint16_t i;
      for (i=0; i<256; i++) {
        rnd_hist[i] = 0;
      }

      // generate histogram
      neo430_printf("Sampling data (%n samples). This may take some time...\n", (uint32_t)NUM_SAMPLES);
      uint32_t j;
      for (j=0; j<NUM_SAMPLES; j++) {
        uint8_t rnd_data = neo430_trng_get();
        rnd_hist[rnd_data]++;
      }

      // output results
      neo430_printf("rnd_hist:\n");
      for(i=0; i<256; i++) {
        neo430_printf("%u =%n\n", i, rnd_hist[i]);
      }
    }

    // exit
    else if (cmd == 'x') {
      if (!(SYS_FEATURES & (1<<SYS_BTLD_EN)))
        neo430_printf("No bootloader installed!\n");
      else
        asm volatile ("mov #0xF000, r0");
    }

    else {
      neo430_printf("Invalid option.\n");
    }
  }

  return 0;
}

