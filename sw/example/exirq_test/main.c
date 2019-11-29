// #################################################################################################
// #  < External Interrupts Controller test program >                                              #
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
// # Stephan Nolting, Hannover, Germany                                                 29.11.2019 #
// #################################################################################################


// Libraries
#include <neo430.h>

// Configuration
#define BAUD_RATE 19200

// Prototypes
void ext_irq_ch0_handler(void);
void ext_irq_ch1_handler(void);
void ext_irq_ch2_handler(void);
void ext_irq_ch3_handler(void);


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  neo430_uart_setup(BAUD_RATE);

  // intro text
  neo430_uart_br_print("\n<<< EXIRQ Test >>>\n");

  // check if EXIRQ was synthesized, exit if not available
  if (!(SYS_FEATURES & (1<<SYS_EXIRQ_EN))) {
    neo430_uart_br_print("Error! No EXIRQ synthesized!");
    return 1;
  }

  // clear output port
  neo430_gpio_port_set(0);


  // use this struct for configuring the EXIRQ controller
  struct neo430_exirq_config_t exirq_config;

  // initialise handler addresses
  exirq_config.address[0] = (uint16_t)(&ext_irq_ch0_handler);
  exirq_config.address[1] = (uint16_t)(&ext_irq_ch1_handler);
  exirq_config.address[2] = (uint16_t)(&ext_irq_ch2_handler);
  exirq_config.address[3] = (uint16_t)(&ext_irq_ch3_handler);
  exirq_config.address[4] = 0; // set unused vectors to zero
  exirq_config.address[5] = 0;
  exirq_config.address[6] = 0;
  exirq_config.address[7] = 0;

  // enable used IRQ channels
  exirq_config.enable = 0b00001111;

  // use rising edge as trigger for all channels
  exirq_config.trigger = 1;

  // send configuration and activate EXIRQ controller
  neo430_exirq_config(exirq_config);
  neo430_exirq_enable();

  // enable global interrupts and go to sleep
  neo430_eint();
  while(1) {
    neo430_sleep();
  }

  return 0;
}


// handler functions for the external interrupt channels:
// - must not have parameters nor a return value
// - must not use the interrupt attribute, as they are normal functions, called by an actual interrupt handler

void ext_irq_ch0_handler(void) {

  neo430_gpio_pin_toggle(0);
}


void ext_irq_ch1_handler(void) {

  neo430_gpio_pin_toggle(1);
}


void ext_irq_ch2_handler(void) {

  neo430_gpio_pin_toggle(2);
}


void ext_irq_ch3_handler(void) {

  neo430_gpio_pin_toggle(3);
}

