// #################################################################################################
// #  < GPIO interrupt example program >                                                           #
// # ********************************************************************************************* #
// # Prints a message whenever a GPIO input pin goes HIGH. Uses the PIO pin-change interrupt.      #
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
// # Stephan Nolting, Hannover, Germany                                                 10.12.2019 #
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

