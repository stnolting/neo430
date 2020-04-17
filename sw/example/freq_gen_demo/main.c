// #################################################################################################
// #  < Frequency generator example program >                                                      #
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

// Macros
#define xstr(a) str(a)
#define str(a) #a

// Configuration
#define BAUD_RATE        19200
#define FREQ_GEN_CHANNEL 0


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  neo430_uart_setup(BAUD_RATE);

  // check if FREQ_GEN unit was synthesized, exit if not
  if (!(SYS_FEATURES & (1<<SYS_FREQ_GEN_EN))) {
    neo430_uart_br_print("Error! No frequency generator unit synthesized!");
    return 1;
  }

  // intro text
  neo430_uart_br_print("\nFrequency generator test program\n"
                       "Using FREQ_GEN channel "xstr(FREQ_GEN_CHANNEL)"\n\n");

  neo430_freq_gen_reset(); // reset frequency generator
  neo430_freq_gen_enable_ch(FREQ_GEN_CHANNEL); // enable channel

  // just sweep through all possible frequency configurations
  uint32_t target_frequency = 0;
  for (target_frequency=0; target_frequency<0xffffffffL; target_frequency++) {
    
    neo430_uart_br_print("Target frequency: 0x");
    neo430_uart_print_hex_dword(target_frequency);
    neo430_uart_br_print(" Hz, real frequency: 0x");
    neo430_uart_print_hex_dword(neo430_freq_gen_set_freq(FREQ_GEN_CHANNEL, target_frequency)); // try to set target frequency, returns actual freq.
    neo430_uart_br_print(" Hz\n");

    neo430_cpu_delay_ms(500); // wait 0.5 seconds
  }

  return 0;
}
