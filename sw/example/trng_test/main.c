// #################################################################################################
// #  < TRNG test program >                                                                        #
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
#define BAUD_RATE     19200
#define NUM_SAMPLES   2000000000
#define TRNG_TAP_MASK 0b01010001000000 // highly experimental!

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

  // reset & start TRNG
  uint8_t rnd_data = 0;
  uint16_t rnd_status = 0;
  neo430_trng_enable(TRNG_TAP_MASK);

  // make sure TRNG is running
  int k;
  for(k=0; k<1024; k++){
    rnd_status = neo430_trng_get(&rnd_data);
    if (rnd_status) {
      neo430_trng_disable();
      neo430_cpu_delay(100);
      neo430_trng_enable(TRNG_TAP_MASK); // reset TRNG
    }
    else {
      break;
    }
    if (k == 1000) {
      neo430_printf("\nTRNG calibration error!\n");
      return 0;
    }
  }

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
        rnd_status = neo430_trng_get(&rnd_data);
        if (rnd_status) {
          neo430_printf("\nTRNG error!\n");
          break;
        }
        neo430_printf("%u ", (uint16_t)rnd_data);
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
        rnd_status = neo430_trng_get(&rnd_data);
        if (rnd_status
        ) {
          neo430_printf("\nTRNG error!\n");
          break;
        }
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

