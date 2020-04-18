// #################################################################################################
// #  < simple timer usage example >                                                               #
// # ********************************************************************************************* #
// # Flashes LED using the timer interrupt.                                                        #
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
#define BAUD_RATE  19200
#define BLINK_LED  0 // pin 0 on GPIO.out
#define BLINK_FREQ 4 // in Hz

// Function prototypes
void __attribute__((__interrupt__)) timer_irq_handler(void);


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  neo430_uart_setup(BAUD_RATE);

  // check if TIMER unit was synthesized, exit if no TIMER is available
  if (!(SYS_FEATURES & (1<<SYS_TIMER_EN))) {
    neo430_uart_br_print("Error! No TIMER unit synthesized!");
    return 1;
  }

  neo430_gpio_pin_clr(BLINK_LED); // clear LED

  // intro text
  neo430_uart_br_print("\nTimer blinking status LED at "xstr(BLINK_FREQ)" Hz.\n");

  // set address of timer IRQ handler
  IRQVEC_TIMER = (uint16_t)(&timer_irq_handler);

  // configure timer frequency
  neo430_timer_disable();
  uint16_t timer_thres;
  if (neo430_timer_config_freq(BLINK_FREQ, &timer_thres)) {
    neo430_uart_br_print("Invalid TIMER frequency!\n");
  }

  TMR_CT |= (1<<TMR_CT_EN) | (1<<TMR_CT_ARST) | (1<<TMR_CT_IRQ) | (1<<TMR_CT_RUN); // enable timer, auto-reset, irq enabled, timer start

  // enable global IRQs
  neo430_eint();

  while(1) {
    neo430_sleep();
  }

  return 0;
}


/* ------------------------------------------------------------
 * INFO Timer interrupt handler
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) timer_irq_handler(void) {

  neo430_gpio_pin_toggle(BLINK_LED);
}

