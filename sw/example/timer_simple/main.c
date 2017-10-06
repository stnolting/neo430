// #################################################################################################
// #  < simple timer usage example >                                                               #
// # ********************************************************************************************* #
// # Prints "TICK" every second (1Hz) using the timer module.                                      #
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
// #  Stephan Nolting, Hannover, Germany                                               06.10.2017  #
// #################################################################################################


// Libraries
#include <stdint.h>
#include "../../lib/neo430/neo430.h"

// Configuration
#define BAUD_RATE 19200

// Function prototypes
void __attribute__((__interrupt__)) timer_irq_handler(void);


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  uart_set_baud(BAUD_RATE);
  USI_CT = (1<<USI_CT_EN);

  // check if TIMER unit was synthesized, exit if no TIMER is available
  if (!(SYS_FEATURES & (1<<SYS_TIMER_EN))) {
    uart_br_print("Error! No TIMER unit synthesized!");
    return 1;
  }

  // intro text
  uart_br_print("\nSimple timer example.\n"
                "Press any key to start.\n\n");

  // wait for key input
  while(!uart_char_received());

  // set address of timer IRQ handler
  IRQVEC_TIMER = (uint16_t)(&timer_irq_handler);

  // set timer threshold value
  // f_tick = 1Hz @ PRSC = 4096
  // THRES = f_main / (1Hz * 4096) = f_main >> 12
  uint32_t f_clock = ((uint32_t)CLOCKSPEED_HI<<16) | (uint32_t)CLOCKSPEED_LO;
  TMR_THRES = (uint16_t)(f_clock >> 12);

  // clear timer counter
  TMR_CNT = 0;

  // configure timer operation
  TMR_CT = (1<<TMR_CT_EN) |   // enable timer
           (1<<TMR_CT_ARST) | // auto reset on threshold match
           (1<<TMR_CT_IRQ) |  // enable IRQ
           (7<<TMR_CT_PRSC0); // PRSC = 7 -> prescaler =4096

  // enable global IRQs
  eint();

  // do something else...
  while (1) {
    sleep(); // go to power down mode
  }

  return 0;
}


/* ------------------------------------------------------------
 * INFO Timer interrupt handler
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) timer_irq_handler(void) {

  uart_br_print("TICK\n");
}
