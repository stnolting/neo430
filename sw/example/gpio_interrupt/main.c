// #################################################################################################
// #  < GPIO interrupt example program >                                                           #
// # ********************************************************************************************* #
// # Prints a message whenever a GPIO input pin goes HIGH. Uses the PIO pin-change interrupt.      #
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

// Function prototypes
void __attribute__((__interrupt__)) gpio_irq_handler(void);


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  neo430_uart_setup(BAUD_RATE);

  // intro text
  neo430_uart_br_print("\nGPIO pin change interrupt demo program\n\n");

  // check if GPIO present
  if (!(SYS_FEATURES & (1<<SYS_GPIO_EN))) {
    neo430_uart_br_print("Error! No GPIO unit synthesized!");
    return 1;
  }

  // deactivate all LEDs
  GPIO_OUTPUT = 0;

  // set address of IRQ handler
  IRQVEC_GPIO  = (uint16_t)(&gpio_irq_handler);

  // configure GPIO pin-change interrupt
  GPIO_IRQMASK = 0xFFFF; // use all input pins as trigger

  // enable global IRQs
  neo430_eint();

  // go to sleep mode, increment counter whenever the CPU wakes up
  while (1) {
    neo430_sleep();
    GPIO_OUTPUT = (GPIO_OUTPUT + 1) & 0x00FF; // increment LED counter
  }

  return 0;
}


/* ------------------------------------------------------------
 * INFO GPIO pin-change interrupt handler
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) gpio_irq_handler(void) {

  // we cannot 100% ensure to actually get the specific state of
  // the input, which caused the IRQ

  neo430_uart_br_print("GPIO pin change interrupt! Current input state: 0x");
  neo430_uart_print_hex_word(GPIO_INPUT);
  neo430_uart_br_print("\n");
}

