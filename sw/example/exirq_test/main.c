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
// # Stephan Nolting, Hannover, Germany                                                 06.11.2019 #
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

void ext_irq_ch7_handler(void);


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

  neo430_uart_br_print("\nTrigger the external interrupt pin (set high) or perform a manual\n"
                       "triggering (sw interrupt) by pressing key 0 to 7.\n");

  // clear output port
  neo430_gpio_port_set(0);


  // use this predefined struct for configuring the EXIRQ controller
  struct neo430_exirq_config_t exirq_config;

  // initialise handler addresses
  exirq_config.address[0] = (uint16_t)(&ext_irq_ch0_handler);
  exirq_config.address[1] = (uint16_t)(&ext_irq_ch1_handler);
  exirq_config.address[2] = (uint16_t)(&ext_irq_ch2_handler);
  exirq_config.address[3] = (uint16_t)(&ext_irq_ch3_handler);
  exirq_config.address[4] = 0; // set unused vectors to zero
  exirq_config.address[5] = 0;
  exirq_config.address[6] = 0;
  exirq_config.address[7] = (uint16_t)(&ext_irq_ch7_handler);

  // only enable the actually used IRQ channels
  exirq_config.enable = 0b10001111;

  // program configuration and activate EXIRQ controller
  neo430_exirq_config(exirq_config);
  neo430_exirq_enable();

  // enable global interrupts
  neo430_eint();

  // trigger EXIRQ channel 0 IRQ by software just for fun
  neo430_exirq_sw_irq(0);

  // wait for key input
  while(1) {
    char c = neo430_uart_getc();
    if ((c >= '0') && (c <= '7')) {
      c = c - '0';
      neo430_exirq_sw_irq((uint8_t)c); // trigger according IRQ by software
    }
  }

  return 0;
}


// handler functions for the external interrupt channels:
// - must not have parameters nor a return value
// - should not use the interrupt attribute, as they can be normal functions called by the actual interrupt handler

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


void ext_irq_ch7_handler(void) {

  neo430_gpio_pin_toggle(7);
}

