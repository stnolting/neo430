// #################################################################################################
// #  < simple timer usage example >                                                               #
// # ********************************************************************************************* #
// # Flashes LED using the timer interrupt.                                                        #
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
// # Stephan Nolting, Hannover, Germany                                                 13.11.2019 #
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
  if (neo430_timer_config_period(BLINK_FREQ))
    neo430_uart_br_print("Invalid TIMER frequency!\n");

  TMR_CT |= (1<<TMR_CT_EN) | (1<<TMR_CT_ARST) | (1<<TMR_CT_IRQ) | (1<<TMR_CT_RUN); // enable timer, auto-reset, irq enabled, timer start

  // enable global IRQs
  neo430_eint();

  // do something else...
  while (1) {
    neo430_sleep(); // go to power down mode
  }

  return 0;
}


/* ------------------------------------------------------------
 * INFO Timer interrupt handler
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) timer_irq_handler(void) {

  neo430_gpio_pin_toggle(BLINK_LED);
}

