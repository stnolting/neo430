// #################################################################################################
// #  < UART interrupt example >                                                                   #
// # ********************************************************************************************* #
// # UART RECEIVE is conducted using the UART RX interrupt                                         #
// # UART TRANSMIT is conducted by using the timer interrupt                                       #
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
// #  Stephan Nolting, Hannover, Germany                                               16.12.2017  #
// #################################################################################################


// Libraries
#include <stdint.h>
#include "../../lib/neo430/neo430.h"

// Configuration
#define BAUD_RATE 19200
#define UART_FIFO_SIZE 256 // must be a power of two!
#define UART_FIFO_MASK (UART_FIFO_SIZE-1)

// Types
struct uart_fifo {
  uint8_t data[UART_FIFO_SIZE];
  uint16_t get_pnt;
  uint16_t put_pnt;
};

// Global vars
volatile struct uart_fifo uart_rtx_fifo;

// Function prototypes
void __attribute__((__interrupt__)) uart_irq_handler(void);
void __attribute__((__interrupt__)) timer_irq_handler(void);
uint8_t uart_fifo_put(volatile struct uart_fifo *fifo, uint8_t c);
uint8_t uart_fifo_get(volatile struct uart_fifo *fifo, uint8_t *c);

/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  uart_set_baud(BAUD_RATE);
  USI_CT = (1<<USI_CT_EN);


  // init fifo
  uart_rtx_fifo.get_pnt = 0;
  uart_rtx_fifo.put_pnt = 0;


  // set address of UART and TIMER IRQ handlers
  IRQVEC_USART = (uint16_t)(&uart_irq_handler);
  IRQVEC_TIMER = (uint16_t)(&timer_irq_handler);


  // configure UART interrupt
  USI_CT |= (1<<USI_CT_UARTRXIRQ);


  // configure TIMER period

  // set timer threshold value
  // f_tick = 1000Hz @ PRSC = 4096
  // THRES = f_main / (1000Hz * 4096) = f_main >> 12
  uint32_t f_clock = CLOCKSPEED_32bit;
  TMR_THRES = (uint16_t)(f_clock  / 4096000);

  // clear timer counter
  TMR_CNT = 0;

  // configure timer operation
  TMR_CT = (1<<TMR_CT_EN)   | // enable timer
           (1<<TMR_CT_ARST) | // auto reset on threshold match
           (1<<TMR_CT_IRQ)  | // enable IRQ
           (7<<TMR_CT_PRSC0); // PRSC = 7 -> prescaler =4096


  uart_fifo_put(&uart_rtx_fifo, (uint8_t)'\r');
  uart_fifo_put(&uart_rtx_fifo, (uint8_t)'\n');
  uart_fifo_put(&uart_rtx_fifo, (uint8_t)'U');
  uart_fifo_put(&uart_rtx_fifo, (uint8_t)'A');
  uart_fifo_put(&uart_rtx_fifo, (uint8_t)'R');
  uart_fifo_put(&uart_rtx_fifo, (uint8_t)'T');
  uart_fifo_put(&uart_rtx_fifo, (uint8_t)' ');
  uart_fifo_put(&uart_rtx_fifo, (uint8_t)'I');
  uart_fifo_put(&uart_rtx_fifo, (uint8_t)'R');
  uart_fifo_put(&uart_rtx_fifo, (uint8_t)'Q');
  uart_fifo_put(&uart_rtx_fifo, (uint8_t)' ');
  uart_fifo_put(&uart_rtx_fifo, (uint8_t)'E');
  uart_fifo_put(&uart_rtx_fifo, (uint8_t)'c');
  uart_fifo_put(&uart_rtx_fifo, (uint8_t)'h');
  uart_fifo_put(&uart_rtx_fifo, (uint8_t)'o');
  uart_fifo_put(&uart_rtx_fifo, (uint8_t)'\r');
  uart_fifo_put(&uart_rtx_fifo, (uint8_t)'\n');


  // enable global IRQs
  eint();

  while(1) {
    sleep();
  }

  return 0;
}



/* ------------------------------------------------------------
 * INFO UART interrupt handler
 * INFO Put received char into buffer
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) uart_irq_handler(void) {

  uart_fifo_put(&uart_rtx_fifo, (uint8_t)uart_char_read());
}



/* ------------------------------------------------------------
 * INFO Timer interrupt handler
 * INFO Send char from buffer if available
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) timer_irq_handler(void) {

  uint8_t c;

  // char in buffer available?
  if (uart_fifo_get(&uart_rtx_fifo, &c) == 0) {
    // UART transceiver idle?
    if ((USI_CT & (1<<USI_CT_UARTTXBSY)) == 0) {
      USI_UARTRTX = (uint16_t)c;
    }
  }
}


/* ------------------------------------------------------------
 * INFO Write to RTX buffer, return 0 if success
 * ------------------------------------------------------------ */
uint8_t uart_fifo_put(volatile struct uart_fifo *fifo, uint8_t c) {

  uint16_t next = ((fifo->put_pnt + 1) & UART_FIFO_MASK);

  if (fifo->get_pnt == next)
    return 1; // fifo full

  fifo->data[fifo->put_pnt & UART_FIFO_MASK] = c;
  fifo->put_pnt = next;

  return 0;
}


/* ------------------------------------------------------------
 * INFO Read from UART RTX buffer, returns 0 if success
 * ------------------------------------------------------------ */
uint8_t uart_fifo_get(volatile struct uart_fifo *fifo, uint8_t *c) {

  if (fifo->get_pnt == fifo->put_pnt)
    return 1; // fifo empty

  *c = fifo->data[fifo->get_pnt];

  fifo->get_pnt = (fifo->get_pnt+1) & UART_FIFO_MASK;

  return 0;
}

