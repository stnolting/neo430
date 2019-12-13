// #################################################################################################
// #  < Nested IRQs example >                                                                      #
// # ********************************************************************************************* #
// # Generates a run time clock using the timer IRQ.                                               #
// # Whenever a char via the UART is received, the according ISR show the current time.            #
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
#define BAUD_RATE  19200

// Function prototypes
void __attribute__((__interrupt__)) timer_irq_handler(void);
void __attribute__((__interrupt__)) uart_irq_handler(void);

// Variable
volatile uint64_t time_ms;


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


  // reset time
  time_ms = 0;


  // intro text
//neo430_uart_br_print("\nClock example. Press any key to show the current time.\n");


  // init TIMER IRQ
  // ------------------------------------------------------

  // set address of timer IRQ handler
  IRQVEC_TIMER = (uint16_t)(&timer_irq_handler);

  // configure timer frequency
  neo430_timer_disable();
  if (neo430_timer_config_period(1000)) // 1kHz to increment every 1ms
    neo430_uart_br_print("Invalid TIMER frequency!\n");

  neo430_printf("THR: %x, CTR: %x\n", TMR_THRES, TMR_CT);
  TMR_CT |= (1<<TMR_CT_EN) | (1<<TMR_CT_ARST) | (1<<TMR_CT_IRQ) | (1<<TMR_CT_RUN); // enable timer, auto-reset, irq enabled, start timer
  neo430_printf("THR: %x, CTR: %x\n", TMR_THRES, TMR_CT);


  // init UART RX IRQ
  // ------------------------------------------------------

  // set address of UART IRQ handler
  IRQVEC_SERIAL = (uint16_t)(&uart_irq_handler);

  // activate UART RX interrupt
  UART_CT |= (1<<UART_CT_RX_IRQ);


  // enable global IRQs
  neo430_eint();


  // do nothing
  while (1) {
    neo430_sleep();
  }

  return 0;
}


/* ------------------------------------------------------------
 * INFO Timer interrupt handler
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) timer_irq_handler(void) {

  time_ms++;
}


/* ------------------------------------------------------------
 * INFO UART interrupt handler
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) uart_irq_handler(void) {

  // reactive IRQs to allow nested interrupts
  neo430_eint();

  // show time
  uint32_t current_time = time_ms; // in seconds
  uint16_t hour     = (uint16_t)( (((current_time/10000)/60)/3600)%24 );
  uint16_t minute   = (uint16_t)( ((current_time/1000)/60)%60 );
  uint16_t second   = (uint16_t)( (current_time/1000)%60 );
  uint16_t m_second = (uint16_t)( (current_time%1000)%1000 );
  neo430_printf("Current runtime: %u:%u:%u:%u\n", hour, minute, second, m_second);
}

