// #################################################################################################
// #  < GPIO interrupt example program >                                                           #
// # ********************************************************************************************* #
// # Prints a message whenever a GPIO input pin goes HIGH. Uses the PIO pin-change interrupt.      #
// # Also outputs a counter on GPIO.OUT[7:0], driven by the timer interrupt.                       #
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
void __attribute__((__interrupt__)) gpio_irq_handler(void);
void __attribute__((__interrupt__)) timer_irq_handler(void);
void delay(uint16_t t);


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  uart_set_baud(BAUD_RATE);
  USI_CT = (1<<USI_CT_EN);

  // intro text
  uart_br_print("\nGPIO interrupt (rising edge) demo program\n\n");

  // check if GPIO & TIMER units present
  if (!(SYS_FEATURES & (1<<SYS_GPIO_EN))) {
    uart_br_print("Error! No GPIO unit synthesized!");
    return 1;
  }
  if (!(SYS_FEATURES & (1<<SYS_TIMER_EN))) {
    uart_br_print("Error! No TIMER unit synthesized!");
    return 1;
  }

  // deactivate all LEDs
  GPIO_OUT = 0;

  // set address of IRQ handler
  IRQVEC_GPIO  = (uint16_t)(&gpio_irq_handler);
  IRQVEC_TIMER = (uint16_t)(&timer_irq_handler);

  // configure GPIO module
  GPIO_CTRL    = (1<<2) | (3<<0); // enable IRQs and trigger on rising edge
  GPIO_IRQMASK = 0xFFFF; // use all input pins as trigger

  // set timer period:
  // LED cnt update frequency = 5Hz
  // f_tick := 5Hz @ PRSC := 4096
  // f_tick = 5Hz = f_clock / (PRSC * (TMR_THRES + 1))
  // TMR_THRES = f_clock / (f_tick * PRSC) - 1
  //           = f_clock / (5 * 4096) - 1
  uint32_t f_clock = ((uint32_t)CLOCKSPEED_HI<<16) | (uint32_t)CLOCKSPEED_LO;
  TMR_THRES = (uint16_t)((f_clock / (uint32_t)(20480)) - 1);

  // configure timer operation
  TMR_CT = (1<<TMR_CT_EN) |   // enable timer
           (1<<TMR_CT_ARST) | // auto reset on threshold match
           (1<<TMR_CT_IRQ) |  // enable IRQ
           (7<<TMR_CT_PRSC0); // 7 -> PRSC = 4096

  // enable global IRQs
  eint();

  // do nothing
  while (1) {
    sleep();
  }

  return 0;
}


/* ------------------------------------------------------------
 * INFO GPIO pin-change interrupt handler
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) gpio_irq_handler(void) {

  // we cannot ensure to actually get the specific state of
  // the input register, which caused the IRQ

  uart_br_print("GPIO pin.change interrupt! Current input state: 0x");
  uart_print_hex_word(GPIO_IN);
  uart_br_print("\n");
}


/* ------------------------------------------------------------
 * INFO Timer interrupt handler
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) timer_irq_handler(void) {

  // increment LED counter
  GPIO_OUT = (GPIO_OUT + 1) & 0x00FF;
}

